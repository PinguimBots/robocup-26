# 2. Programando com o SDK (Python e C++)

O **SDK** é a biblioteca que o seu programa usa para pedir coisas ao firmware. Existem três "portas de entrada":

| Porta | Linguagem | Nível | Quando usar |
|---|---|---|---|
| **`boosteros`** (`BoosterRobot`) | Python | Alto, mais fácil | Aprender, testar ideias, agentes |
| `booster_robotics_sdk_python` | Python | Médio (espelho do C++) | Quando precisar de uma API que o `boosteros` não expõe |
| **C++ SDK** (`B1LocoClient`) | C++ | Médio/baixo, mais rápido | Código de futebol (a demo usa C++/ROS) |

Os três já vêm instalados com o firmware 1.8 na placa de percepção (`.102`), em `/opt/booster/python3/site-packages/`.

## 2.1 Preparando o terminal (sempre)

Na `.102` (`ssh booster@192.168.10.102`):
```bash
source /opt/booster/python3/env.sh      # Python do SDK + ROS 2
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile.xml
```
Sem a segunda linha, os pedidos falham com código 100 (guia 1, seção 1.4).
Depois de ligar o robô, espere 30–60 s antes de usar o SDK (o manual cita o erro `LocoClientInitError`).

## 2.2 Python com `boosteros` (recomendado para começar)

Arquivo principal: `/opt/booster/python3/site-packages/boosteros/robots/booster/booster_robot.py`.
Crie **um único** `BoosterRobot()` por programa.

### Exemplo 1: só ler (100% seguro)
```python
from boosteros.robots.booster import BoosterRobot

robot = BoosterRobot()
print("modo:", robot.get_mode())          # "damping", "prepare", "walk", ...
print("bateria:", robot.get_battery())
print("imu:", robot.get_imu())
print("odometria:", robot.get_odom())
print("ações disponíveis:", [a for a in robot.list_actions()][:5])
```

### Exemplo 2: mexer a cabeça (robô em WALK, segurado)
Já testado no nosso T1 (foi assim que inclinamos a cabeça no teste de distância da bola):
```python
robot.set_head_angle(pitch=0.45, yaw=0.0)   # radianos: pitch > 0 olha para baixo
```

### Exemplo 3: andar um pouco (robô em WALK, área livre, alguém junto)
```python
import time
robot.set_velocity(0.2, 0.0, 0.0)   # vx (m/s, frente), vy (m/s, lado), vyaw (rad/s, giro)
time.sleep(2.0)
robot.set_velocity(0.0, 0.0, 0.0)   # SEMPRE mandar parar
```

### Exemplo 4: tarefas que demoram (gesto) e podem ser canceladas
```python
tarefa = robot.do_action("hand_wave")   # devolve um "TaskHandle"
# ... depois, se precisar: tarefa.cancel()
```

### Principais métodos de `BoosterRobot` (conferidos no robô)

| Grupo | Métodos |
|---|---|
| Informação | `robot_info`, `get_mode`, `get_gait`, `list_gaits`, `list_joints`, `list_actions`, `list_frames`, `list_sensors` |
| Leitura instantânea | `get_imu`, `get_odom`, `get_joint_states`, `get_battery`, `get_fall_down_state`, `get_transform`, `get_image`, `get_camera_info` |
| Leitura contínua (callback) | `subscribe_imu`, `subscribe_odom`, `subscribe_battery`, `subscribe_fall_down_state`, `subscribe_image` |
| Movimento | `set_mode`, `set_gait` (ex.: futebol), `set_velocity`, `set_head_angle`, `upper_body_control`, `set_joints`, `reset_odom` |
| Tarefas (`TaskHandle`) | `do_action`, `get_up`, `execute_trajectory`, `play_sound`, `get_active_tasks` |
| Gerenciadores | `audio_manager`, `hand_guiding_manager`, `LightManager`, `SoccerKickManager` (⚠️ **chuta**) |

> **Limitação no nosso robô:** `get_image()` lê a câmera `head` (`/boostercamera/head/rgb`), que **fica vazia com a ZED**.
> Para imagem, assine direto o tópico `/zed/zed_node/left/image_rect_color` (ROS 2), como a demo faz.

## 2.3 Python "espelho do C++" (`booster_robotics_sdk_python`)

Útil para APIs que o `boosteros` não expõe. Exemplo de leitura (usado no diagnóstico dos botões F/P):
```python
import time
from booster_robotics_sdk_python import ChannelFactory, B1LocoClient
ChannelFactory.Instance().Init(0)
client = B1LocoClient(); client.Init(); time.sleep(1.5)
print(client.GetMode().mode)      # ex.: RobotMode.kDamping
```

## 2.4 C++ (o que a demo de futebol usa)

```cpp
#include <booster/robot/b1/b1_loco_client.hpp>
booster::robot::ChannelFactory::Instance()->Init(0);       // domínio DDS 0
booster::robot::b1::B1LocoClient client;
client.Init();
client.ChangeMode(booster::robot::RobotMode::kWalking);     // devolve um código (0 = ok)
client.Move(0.2F, 0.0F, 0.0F);                              // vx, vy, vyaw
client.RotateHead(0.3F, 0.0F);                              // pitch, yaw
```

### ⚠️ Existem DUAS cópias do SDK C++ no robô
| Onde | Versão | Observação |
|---|---|---|
| `/usr/local/include/booster` + `/usr/local/lib` | **Antiga** | Usada por padrão na compilação. **Não conhece** `RobotMode::kSoccer` |
| `/opt/booster/python3/site-packages/include/booster` + `.../lib` | **Nova**, a do firmware 1.8 (SDK 1.6.3) | Completa |

Foi por isso que a demo deu `'kSoccer' is not a member of RobotMode` na compilação: trocamos o nome pelo número `4`.
Ao usar APIs novas em C++, consulte o cabeçalho **novo** para saber o número.

### APIs de alto nível (`LocoApiId`, cabeçalho novo `b1/b1_loco_api.hpp`)

| Grupo | APIs (número) |
|---|---|
| Modo e estado | `kChangeMode` 2000, `kGetMode` 2017, `kGetStatus` 2018, `kGetRobotInfo` 2022, `kGetSensors` 2044, `kSwitchGait` 2042 |
| Andar e odometria | `kMove` 2001, `kResetOdometry` 2031, `kEnterWBCGait` 2035, `kExitWBCGait` 2036 |
| Cabeça | `kRotateHead` 2004, `kRotateHeadWithDirection` 2006, `kRotateHeadWithTime` 2043 |
| Braços e mãos | `kMoveHandEndEffector` 2009, `kControlGripper` 2010, `kControlDexterousHand` 2013, `kUpperBodyCustomControl` 2030 |
| Gestos | `kWaveHand` 2005, `kHandshake` 2015, `kDance` 2016, `kWholeBodyDance` 2029, `kHandOnChestGreeting` 2050 |
| Corpo | `kLieDown` 2007, `kGetUp` 2008, `kGetUpWithMode` 2025, `kPushUp` 2019 |
| Futebol | `kShoot` 2024, `kVisualKick` 2038 (⚠️ chutes) |
| Som | `kPlaySound` 2020, `kStopSound` 2021 |
| Trajetórias | `kRecordTrajectory` 2027, `kReplayTrajectory` 2028, `kLoadCustomTrainedTraj` 2032 … |

Há também APIs **internas** (não documentadas, `booster_internal/.../b1_loco_internal_api.hpp`), usadas pelos
códigos de futebol: `kEnableRobocupWalkMode`, `kRLKickBall` (100011), `kRLFancyKickBall` (100012) etc.

## 2.5 Controle direto dos motores (modo Custom): só para depois

Mensagem `booster_interface/msg/LowCmd`: uma lista de `MotorCmd` (um por junta, guia 1, seção 1.5):
```
MotorCmd: mode, q (posição), dq (velocidade), tau (torque), kp (rigidez), kd (amortecimento), weight
```
Publicada em `/joint_ctrl`, **só vale em modo Custom**, e **o firmware deixa de equilibrar o robô**.
Usado em pesquisa (ex.: políticas treinadas por aprendizado por reforço, projeto `booster_deploy` da Booster,
que no T1 roda na placa de movimento). **Não é o caminho para melhorar o futebol agora.**

## 2.6 Exercícios sugeridos (em ordem)

1. Rodar o **Exemplo 1** e anotar o que cada leitura devolve.
2. Escrever um programa que imprime a **bateria a cada 5 s** (`subscribe_battery` ou um laço com `get_battery`).
3. Com o robô em WALK e segurado: fazer a cabeça **varrer** de −0,8 a +0,8 rad em yaw, devagar.
4. Assinar `/booster_vision/ball` (com a visão da demo ligada) e **mover a cabeça para seguir a bola**.
   É uma versão simples do que o nó `CamTrackBall` da demo faz.
