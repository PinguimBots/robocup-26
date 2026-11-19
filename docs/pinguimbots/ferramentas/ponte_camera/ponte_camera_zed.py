#!/usr/bin/env python3
# ARQUIVO NOVO - PinguimBots
# Ponte da camera: copia a imagem da ZED 2i para os topicos que as ferramentas da Booster
# esperam (camera "head" da RealSense). Nao altera firmware nem outros programas.
#   /zed/zed_node/left/image_rect_color (bgra8)  -> /boostercamera/head/rgb (bgr8)
#   /zed/zed_node/left/camera_info               -> /boostercamera/head/rgb/camera_info
#   /zed/zed_node/depth/depth_registered (32FC1 m) -> /boostercamera/head/depth (16UC1 mm)
#   /zed/zed_node/depth/camera_info              -> /boostercamera/head/depth/camera_info
import array
import numpy as np
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
from sensor_msgs.msg import Image, CameraInfo

QOS = QoSProfile(reliability=ReliabilityPolicy.RELIABLE, history=HistoryPolicy.KEEP_LAST, depth=2)


class PonteCameraZed(Node):
    def __init__(self):
        super().__init__('pinguimbots_ponte_camera_zed')
        self.pub_rgb = self.create_publisher(Image, '/boostercamera/head/rgb', QOS)
        self.pub_rgb_info = self.create_publisher(CameraInfo, '/boostercamera/head/rgb/camera_info', QOS)
        self.pub_depth = self.create_publisher(Image, '/boostercamera/head/depth', QOS)
        self.pub_depth_info = self.create_publisher(CameraInfo, '/boostercamera/head/depth/camera_info', QOS)
        self.create_subscription(Image, '/zed/zed_node/left/image_rect_color', self.on_rgb, QOS)
        self.create_subscription(CameraInfo, '/zed/zed_node/left/camera_info', self.pub_rgb_info.publish, QOS)
        self.create_subscription(Image, '/zed/zed_node/depth/depth_registered', self.on_depth, QOS)
        self.create_subscription(CameraInfo, '/zed/zed_node/depth/camera_info', self.pub_depth_info.publish, QOS)
        self.n = 0
        self.create_timer(10.0, self.report)
        self.get_logger().info('Ponte ZED -> /boostercamera/head ativa')

    def on_rgb(self, msg):
        if msg.encoding.lower() in ('bgra8', 'rgba8'):
            img = np.frombuffer(msg.data, dtype=np.uint8).reshape(msg.height, msg.step)[:, :msg.width * 4]
            img = img.reshape(msg.height, msg.width, 4)[:, :, :3]
            out = Image(header=msg.header, height=msg.height, width=msg.width,
                        encoding='bgr8' if msg.encoding.lower() == 'bgra8' else 'rgb8',
                        is_bigendian=msg.is_bigendian, step=msg.width * 3)
            out.data = array.array('B', np.ascontiguousarray(img).tobytes())  # rapido: evita validar byte a byte
            self.pub_rgb.publish(out)
        else:
            self.pub_rgb.publish(msg)
        self.n += 1

    def on_depth(self, msg):
        if msg.encoding.upper() == '32FC1':
            d = np.frombuffer(msg.data, dtype=np.float32).reshape(msg.height, msg.step // 4)[:, :msg.width]
            mm = np.nan_to_num(d * 1000.0, nan=0.0, posinf=0.0, neginf=0.0)
            mm = np.clip(mm, 0, 65535).astype(np.uint16)
            out = Image(header=msg.header, height=msg.height, width=msg.width, encoding='16UC1',
                        is_bigendian=0, step=msg.width * 2)
            out.data = array.array('B', mm.tobytes())
            self.pub_depth.publish(out)
        else:
            self.pub_depth.publish(msg)

    def report(self):
        self.get_logger().info(f'imagens repassadas nos ultimos 10 s: {self.n}')
        self.n = 0


def main():
    rclpy.init()
    rclpy.spin(PonteCameraZed())


if __name__ == '__main__':
    main()
