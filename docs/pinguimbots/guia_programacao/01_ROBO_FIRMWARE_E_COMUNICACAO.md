# 1. O robô por dentro: placas, firmware e comunicação

## 1.1 Dois computadores dentro do robô

| | Placa de **movimento** | Placa de **percepção** |
|---|---|---|
| Endereço | `192.168.10.101` (usuário `master`) | `192.168.10.102` (usuário `booster`) |
| Hardware | Intel Core i7, 8 GB | NVIDIA Jetson AGX Orin, 32 GB, JetPack 6.2, TensorRT 10.3 |
| Função | Motores, equilíbrio, marcha, IMU, odometria, controle remoto | Câmera ZED, visão, áudio, agentes, **código de futebol** |
| Sistema | Ubuntu 22.04 + ROS 2 Humble | Ubuntu 22.04 + ROS 2 Humble |

As duas ficam ligadas por uma rede interna (`192.168.10.x`). O PC do time entra nessa rede pelo cabo.

**Regra prática:** o **código do time roda na `.102`**. A `.101` é do firmware; só lemos logs dela.

## 1.2 O firmware: serviços em "contêineres"

O firmware da Booster é um conjunto de programas supervisionados pelo serviço `booster-daemon`, que os inicia
como "contêineres" (`/opt/booster/bin/container/...`). Os principais, conferidos no robô:

**Na `.101` (movimento):**

| Programa | Para que serve |
|---|---|
| `motion` | O coração do robô: modos, marcha, equilíbrio, chutes, levantar. Recebe os pedidos de RPC |
| `robot_core`, `device_gateway`, `rs_message` | Conversa com os motores e sensores (barramento CAN), bateria, emergência |
| `remote_controller` | Lê o controle (joystick) |
| `server_motion` | Comunicação com o app (Wi-Fi, Bluetooth) |
| `rerun_logger` | Grava registros de depuração |
| `rpc_service_node` | Ponte entre o RPC da Booster e o ROS 2 |

**Na `.102` (percepção):**

| Programa | Para que serve |
|---|---|
| `booster_camera` | Liga o driver da câmera (ZED 2i, conforme `/opt/booster/perception_info.yaml`) |
| `audio`, `rtc_client`, `booster-video-stream` | Som, vídeo para o app |
| `battery_daemon`, `lui` | Avisos de bateria, interface de luz/voz |
| `booster_agent_manager_node` + `pyagent_aarch64` | Gerenciador de agentes e o agente ativo (hoje o *default*) |
| `booster_daemon_input` | Lê os botões **F/P** das costas |

**Onde ficam os registros:** `/var/log/booster/<data_hora_do_boot>/<serviço>/<serviço>.log`.
Exemplo útil: `/var/log/booster/<boot>/motion/motion.log` na `.101` mostra cada mudança de modo e cada pedido recebido.

> **Atenção:** o relógio do robô está ~11 h adiantado em relação ao PC. Leve isso em conta ao procurar horários nos logs.

## 1.3 Modos do robô

O firmware sempre está em um **modo** (`RobotMode` no SDK):

| Valor | Modo | O que acontece | Como entrar |
|---|---|---|---|
| 0 | **DAMP** | Motores "moles"; o robô desaba se não estiver apoiado | `LT + Back` |
| 1 | **PREP** | Fica em pé, parado | `LT + Start` ou botão P |
| 2 | **WALK** | Anda, gira, mexe a cabeça, faz gestos | `RT + A` (a partir do PREP) |
| 3 | **Custom** | O **seu programa** controla cada motor diretamente (perigoso) | Só por código |
| 4 | **Soccer** | Marcha e chutes **de futebol** | Pelo cérebro da demo (`LT + A`) ou pelo agente de futebol |

Dentro de cada modo, o firmware usa um **controlador de corpo** (`BodyControl`): `kHumanlikeGait` (andar normal),
`kSoccerGait` (andar de futebol), `kGetUp` (levantar), `kShoot`, `kGoalie`, `kCustom` etc.

**Sequência segura sempre:** DAMP → PREP → WALK. Voltar para DAMP só com o robô apoiado.

## 1.4 Como os programas conversam

### Tópicos ROS 2 (publicar/assinar)
Os dados contínuos andam por tópicos. Os mais úteis para programar (`ros2 topic list` na `.102` mostra 255):

| Tópico | Conteúdo |
|---|---|
| `/low_state` | Estado de cada motor (posição `q`, velocidade `dq`, torque estimado, temperatura) + IMU |
| `/joint_states`, `/imu/data` | Juntas e IMU no formato padrão do ROS |
| `/odometer_state`, `/odom` | Odometria (quanto o robô andou) |
| `/head_pose` | Posição da cabeça (usada pela visão para saber para onde a câmera aponta) |
| `/fall_down`, `/fall_down_recovery_state` | Detecção de queda |
| `/remote_controller_state` | Botões e analógicos do controle |
| `/button_event` | Eventos de botões |
| `/battery_state` | Bateria |
| `/zed/zed_node/left/image_rect_color` | Imagem da câmera (640×360, ~15 Hz) |
| `/zed/zed_node/depth/depth_registered` | Profundidade |
| `/joint_ctrl` | Comando direto de motores (**só funciona em modo Custom**) |
| `/kick_ball`, `/robocup_behavior_status` | Usados pelo chute visual da Booster |
| `/LocoApiTopicReq`, `/LocoApiTopicResp` | Pedidos e respostas de RPC (próxima seção) |

Comandos para explorar (na `.102`):
```bash
source /opt/ros/humble/setup.bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile.xml
ros2 topic list
ros2 topic hz /odometer_state
ros2 topic echo --once /battery_state
ros2 interface show booster_interface/msg/LowState   # (com o workspace da demo carregado)
```

### RPC (pedir e receber resposta)
Ações ("mude de modo", "ande", "acene") são **pedidos**. O SDK publica uma mensagem em `LocoApiTopicReq` com
um número de API (`api_id`) e um corpo em JSON; o `motion` executa e responde em `LocoApiTopicResp`.

```
seu programa ──(api_id=2000, {"mode":2})──► LocoApiTopicReq ──► motion (.101)
seu programa ◄──────(código 0 = ok)──────── LocoApiTopicResp ◄──┘
```

Códigos de resposta (manual da Booster): `0` sucesso, `100` sem resposta, `400` pedido inválido,
`409` conflito, `429` frequente demais, `500/501/502` erro / recusado / mudança de estado falhou.

### O perfil de rede (DDS) é obrigatório
Todo programa precisa da variável:
```bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile.xml
```
Ela restringe a comunicação à rede interna e à memória compartilhada. **Sem ela, os pedidos falham com código 100.**
Foi exatamente o defeito dos botões F/P (ver [FIRMWARE_1.8_CORRECOES.md](../FIRMWARE_1.8_CORRECOES.md), item 6).

## 1.5 As 23 juntas do T1

```
 0 HeadYaw (girar cabeça)        1 HeadPitch (inclinar cabeça)
 2–5   braço esquerdo:  ShoulderPitch, ShoulderRoll, ElbowPitch, ElbowYaw
 6–9   braço direito:   idem
 10    Waist (cintura)
 11–16 perna esquerda:  HipPitch, HipRoll, HipYaw, KneePitch, AnkleUp, AnkleDown
 17–22 perna direita:   idem
```
No modo Custom, cada motor recebe: posição alvo `q`, velocidade `dq`, torque `tau`, rigidez `kp`, amortecimento `kd`
e `weight` (mensagem `MotorCmd`). Sem equilíbrio do firmware: um erro derruba o robô.

## 1.6 O que é seguro para um iniciante

| Seguro (robô segurado, em WALK) | Cuidado | Não fazer |
|---|---|---|
| Ler qualquer tópico ou log | Andar por código (comece com 0,2 m/s) | Modo Custom / `/joint_ctrl` sem supervisão |
| Consultar modo, bateria, IMU | Mudar de modo por código | `apt upgrade` no robô |
| Mexer a cabeça | Levantar (`GetUp`) | Mexer em `/opt/booster` |
| Tocar som, gestos (`do_action`) | Modo Soccer | Chutes (`Shoot`, `VisualKick`, `SoccerKickManager`) sem decidir antes |
