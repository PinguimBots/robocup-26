#!/bin/bash
# PinguimBots: inicia visao + cerebro + GameController (T1 com ZED 2i).
# Nao mexe em servicos do robo. Parar: ./scripts/stop.sh
cd `dirname $0`
cd ..
WS=$(pwd)
VCFG="$WS/src/vision/config"

echo "[STOP EXISTING NODES (IF ANY), TO AVOID CONFLICT]"
./scripts/stop.sh

source ./install/setup.bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile_udp_only.xml

echo "[START VISION]"
nohup ros2 launch vision launch.py vision_config_path:="$VCFG" save_data:=false save_depth:=false > vision.log 2>&1 &
echo "[START BRAIN]"
nohup ros2 launch brain launch.py vision_config_path:="$VCFG" "$@" > brain.log 2>&1 &
echo "[START GAME_CONTROLLER]"
nohup ros2 launch game_controller launch.py > game_controller.log 2>&1 &
echo "[DONE]"
