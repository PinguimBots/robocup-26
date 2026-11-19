# 2. Programando com o SDK (Python e C++)

O **SDK** é a biblioteca que o **seu programa** usa para pedir coisas ao robô ("qual o seu modo?", "olhe para a
esquerda", "ande 0,2 m/s"). Este guia mostra **onde criar os arquivos, como rodar e o que escrever**, com
exercícios do mais seguro ao mais arriscado.

---

## 2.1 As três "portas" do SDK

| Porta | Linguagem | Nível | Onde está no robô (`.102`) | Quando usar |
|---|---|---|---|---|
| **`boosteros`** (classe `BoosterRobot`) | Python | Alto, mais fácil | `/opt/booster/python3/site-packages/boosteros/` (versão 1.2.0) | **Comece por aqui.** Testes, ferramentas, agentes |
| `booster_robotics_sdk_python` | Python | Médio (espelho do C++) | `/opt/booster/python3/site-packages/` (1.6.3) | APIs que o `boosteros` não expõe |
| **SDK C++** (`B1LocoClient`) | C++ | Médio, mais rápido | cabeçalhos novos em `/opt/booster/python3/site-packages/include/booster/` | Código de futebol (a demo é C++/ROS 2) |

Tudo já vem instalado com o firmware 1.8. **Não precisa instalar nada** para os exercícios.

> O seu programa roda **dentro do robô** (na placa de percepção `.102`), não no PC. O PC só serve para
> editar e mandar rodar (por SSH, Jupyter ou Booster Studio).

---

## 2.2 Onde criar os arquivos e como rodar

### A pasta dos exercícios
Use **uma pasta só sua no robô**, separada de tudo:
```
/home/booster/PinguimBots_exercicios/      ← no robô (.102)
├── ex01_ler_estado.py
├── ex02_imu_continuo.py
├── ex03_cabeca_varredura.py
├── ex04_andar_quadrado.py
└── ex05_cabeca_segue_bola.py
```
Por que no robô e nessa pasta:
- o SDK só funciona **lá dentro** (ele fala com o firmware pela rede interna);
- `/home/booster` é a pasta que o **Jupyter do Booster Studio** mostra, então os arquivos aparecem também no Studio;
- fica longe do `robocup_public` e da demo, sem risco de estragar o código de futebol.

Criar a pasta (uma vez):
```bash
ssh booster@192.168.10.102
mkdir -p ~/PinguimBots_exercicios
```

### Três jeitos de escrever e rodar (escolha um)

**Jeito A: terminal (o mais simples)**
```bash
ssh booster@192.168.10.102
cd ~/PinguimBots_exercicios
nano ex01_ler_estado.py            # cole o código, Ctrl+O salva, Ctrl+X sai
source /opt/booster/python3/env.sh
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile.xml
python3 ex01_ler_estado.py
```

**Jeito B: Booster Studio editando direto no robô** (mais confortável)
1. No Studio: **Remote - SSH → Connect to Host** → `booster@192.168.10.102`.
2. **File → Open Folder** → `/home/booster/PinguimBots_exercicios`.
3. Crie e edite os arquivos como num editor comum (com cores e autocompletar do Python).
4. Rode pelo **terminal integrado** do Studio (`Ctrl+'`), com os mesmos comandos do jeito A.

**Jeito C: Jupyter (caderno) pelo Booster Studio**
- O Studio liga no robô um servidor Jupyter (porta 8888) com um Python próprio (`~/.booster_venv`, que já
  enxerga o `boosteros`). A pasta inicial dele é `/home/booster`.
- Abra o painel **Notebook** no Studio, entre em `PinguimBots_exercicios`, crie um caderno e cole o código numa
  célula. **Shift+Enter** roda; o botão **■ (interrupt)** para.
- Vantagem: a conexão com o robô fica viva entre células (bom para experimentar aos poucos).

### As duas linhas obrigatórias (jeitos A e B)
```bash
source /opt/booster/python3/env.sh      # aponta para o Python e o ROS 2 do SDK
export FASTRTPS_DEFAULT_PROFILES_FILE=/opt/booster/BoosterRos2/fastdds_profile.xml   # rede interna do robô
```
Sem a segunda, todo pedido falha com **código 100** (sem resposta). Para não digitar sempre, coloque as duas no
final do `~/.bashrc` do robô.

### Antes de rodar qualquer coisa
- Robô ligado há **mais de 1 minuto** (antes disso aparece `LocoClientInitError`).
- **Um programa com `BoosterRobot()` por vez.** Dois ao mesmo tempo disputam o robô.
- Se a demo de futebol estiver rodando, **pare** (`~/Demo_PinguimBots/scripts/stop.sh`): ela também manda comandos.

---

## 2.3 Exercícios (em ordem de risco)

| # | Arquivo | Mexe o robô? | Modo exigido | Segurança |
|---|---|---|---|---|
| 1 | `ex01_ler_estado.py` | Não | Qualquer | Nenhuma preocupação |
| 2 | `ex02_imu_continuo.py` | Não | Qualquer | Nenhuma preocupação |
| 3 | `ex03_cabeca_varredura.py` | Só a cabeça | **WALK** | Robô em pé e segurado |
| 4 | `ex04_andar_quadrado.py` | **Anda** | **WALK** | Área livre de 2 m, alguém junto |
| 5 | `ex05_cabeca_segue_bola.py` | Só a cabeça | **WALK** + visão da demo | Robô segurado |

### Exercício 1: ler o estado (seguro)
```python
"""ex01 - so LE o estado do robo (seguro em qualquer modo)."""
from boosteros.robots.booster import BoosterRobot

robot = BoosterRobot()                         # conecta ao robo (1 por programa)
print("modo   :", robot.get_mode())            # "damping", "prepare", "walk", ...
print("bateria:", robot.get_battery())
print("IMU    :", robot.get_imu().rpy)         # rolagem, arfagem, guinada (rad)
print("odom   :", robot.get_odom())            # quanto andou
print("marchas:", robot.list_gaits())          # ex.: padrao e futebol
print("acoes  :", robot.list_actions()[:5])    # gestos disponiveis
```
**O que observar:** o `BoosterRobot()` imprime várias linhas `[Booster OS] ...` ao iniciar (normal). Mude o modo
pelo controle (PREP/WALK) e rode de novo: o `get_mode()` acompanha.

### Exercício 2: ler a IMU sem parar (callback)
```python
"""ex02 - imprime a inclinacao do corpo continuamente (Ctrl+C para sair)."""
import math, time
from boosteros.robots.booster import BoosterRobot

robot = BoosterRobot()

def ao_chegar_imu(imu):                        # chamada a cada leitura nova
    r, p, y = (math.degrees(v) for v in imu.rpy)
    print(f"\rrolagem {r:+6.1f}°  arfagem {p:+6.1f}°  guinada {y:+6.1f}°", end="", flush=True)

assinatura = robot.subscribe_imu(ao_chegar_imu)  # passa a receber os dados
try:
    while True:
        time.sleep(0.5)                        # o programa so precisa ficar vivo
except KeyboardInterrupt:
    print("\nfim")
```
**Conceito novo:** *callback* (o SDK chama a sua função quando chega dado novo), igual aos tópicos ROS 2.
Incline o robô (segurado, em PREP) e veja os ângulos mudarem.

### Exercício 3: varrer com a cabeça (robô em WALK, segurado)
```python
"""ex03 - a cabeca olha da esquerda para a direita e volta ao centro."""
import time
from boosteros.robots.booster import BoosterRobot

robot = BoosterRobot()
if robot.get_mode() != "walk":
    raise SystemExit("Coloque o robo em WALK (RT + A) antes de rodar.")

try:
    for yaw in [0.8, 0.4, 0.0, -0.4, -0.8]:   # radianos: + esquerda, - direita
        robot.set_head_angle(pitch=0.3, yaw=yaw)  # pitch + olha para baixo
        time.sleep(1.0)
finally:
    robot.set_head_angle(pitch=0.0, yaw=0.0)    # SEMPRE volta ao centro
```
**Conceitos novos:** checar o modo antes de agir; `try/finally` garante o "voltar ao centro" mesmo com erro ou
`Ctrl+C`. É exatamente o que o painel **Head Control** do Studio faz (API 2004).

### Exercício 4: andar um pequeno quadrado (robô em WALK, área livre)
```python
"""ex04 - anda 4 lados curtos. Area livre de 2 m e alguem junto do robo."""
import time
from boosteros.robots.booster import BoosterRobot

robot = BoosterRobot()
if robot.get_mode() != "walk":
    raise SystemExit("Coloque o robo em WALK (RT + A) antes de rodar.")

def mover(vx, vy, vyaw, segundos):
    robot.set_velocity(vx, vy, vyaw)   # m/s frente, m/s lado (+esq), rad/s giro (+anti-horario)
    time.sleep(segundos)
    robot.set_velocity(0.0, 0.0, 0.0)
    time.sleep(1.0)

try:
    for _ in range(4):
        mover(0.2, 0.0, 0.0, 2.0)      # ~0,4 m para frente
        mover(0.0, 0.0, 0.5, 3.1)      # ~90 graus para a esquerda
finally:
    robot.set_velocity(0.0, 0.0, 0.0)  # SEMPRE mandar parar
```
**O que observar:** o quadrado não fecha perfeito. A diferença é o **erro de odometria**, o mesmo que a demo
corrige com a localização (guia 3). Compare com `robot.get_odom()` antes e depois.

### Exercício 5: a cabeça segue a bola (avançado)
Junta **ROS 2** (ler a bola que a visão da demo publica) com o **SDK** (mexer a cabeça).
Pré-requisitos: visão da demo ligada (`cd ~/Demo_PinguimBots && ./PinguimBots/start_PinguimBots.sh visao`) e, no
terminal, também `source ~/Demo_PinguimBots/install/setup.bash` (para o Python conhecer a mensagem `Ball`).
```python
"""ex05 - segue a bola com a cabeca (WALK, robo segurado). Ctrl+C para sair."""
import math, rclpy
from rclpy.node import Node
from vision_interface.msg import Ball            # mensagem publicada pela visao da demo
from boosteros.robots.booster import BoosterRobot

ALTURA_CAMERA = 1.1                               # metros (robo em pe)
robot = BoosterRobot()
if not rclpy.ok():
    rclpy.init()

class SegueBola(Node):
    def __init__(self):
        super().__init__("pinguimbots_segue_bola")
        self.create_subscription(Ball, "/booster_vision/ball", self.ao_ver_bola, 10)

    def ao_ver_bola(self, bola):
        if bola.confidence < 0.4:                 # neste topico a confianca vai de 0 a 1
            return                                # (x, y) = bola em metros, relativa ao robo
        dist = math.hypot(bola.x, bola.y)
        yaw = max(-1.0, min(1.0, math.atan2(bola.y, bola.x)))
        pitch = max(0.0, min(0.8, math.atan2(ALTURA_CAMERA, max(dist, 0.3))))
        robot.set_head_angle(pitch=pitch, yaw=yaw)

try:
    rclpy.spin(SegueBola())
except KeyboardInterrupt:
    pass
finally:
    robot.set_head_angle(pitch=0.0, yaw=0.0)
```
> Este exercício **ainda não foi testado no robô**: a junção do `rclpy` com o `BoosterRobot` (que também usa
> ROS 2 por dentro) pode precisar de ajuste. Se der erro, é um bom problema para investigar com calma.

---

## 2.4 O que o `BoosterRobot` oferece (conferido no robô)

| Grupo | Métodos | Observação |
|---|---|---|
| Informação | `robot_info`, `get_mode`, `get_gait`, `list_gaits`, `list_joints`, `list_actions`, `list_frames`, `list_sensors` | Só leitura |
| Leitura instantânea | `get_imu`, `get_odom`, `get_joint_states`, `get_battery`, `get_fall_down_state`, `get_transform`, `get_image`, `get_camera_info` | `get_image` precisa da **ponte da câmera** ligada |
| Leitura contínua | `subscribe_imu`, `subscribe_odom`, `subscribe_battery`, `subscribe_fall_down_state`, `subscribe_image` | Recebem uma função (*callback*) |
| Modo | `set_mode("prepare" / "walk" / ...)`, `set_gait(...)` | ⚠️ muda o comportamento do robô |
| Cabeça | `set_head_angle(pitch, yaw)` | WALK |
| Andar | `set_velocity(vx, vy, vyaw)`, `reset_odom()` | WALK; comece com ≤ 0,2 m/s |
| Braços/juntas | `upper_body_control`, `set_joints`, `execute_trajectory` | ⚠️ avançado |
| Tarefas (devolvem `TaskHandle`, com `.cancel()`) | `do_action("hand_wave")`, `get_up()`, `play_sound(...)`, `get_active_tasks()` | Gestos em WALK |
| Gerenciadores | `audio_manager`, `hand_guiding_manager`, `LightManager`, `SoccerKickManager` | ⚠️ `SoccerKickManager` **chuta** |

Para ver todos os detalhes de um método: no robô, `python3 -c "from boosteros.robots.booster import BoosterRobot; help(BoosterRobot.set_velocity)"`.

---

## 2.5 Python "espelho do C++" (`booster_robotics_sdk_python`)

Mesmos nomes do C++ (útil quando o `boosteros` não tem a função). Exemplo de leitura:
```python
import time
from booster_robotics_sdk_python import ChannelFactory, B1LocoClient
ChannelFactory.Instance().Init(0)          # domínio DDS 0
client = B1LocoClient(); client.Init(); time.sleep(1.5)
print(client.GetMode().mode)                # ex.: RobotMode.kWalking
```

## 2.6 C++ (o que a demo usa)

```cpp
#include <booster/robot/b1/b1_loco_client.hpp>
booster::robot::ChannelFactory::Instance()->Init(0);
booster::robot::b1::B1LocoClient client;
client.Init();
client.ChangeMode(booster::robot::RobotMode::kWalking);   // devolve int32 (0 = ok)
client.Move(0.2F, 0.0F, 0.0F);                            // vx, vy, vyaw
client.RotateHead(0.3F, 0.0F);                            // pitch, yaw
```
Para praticar C++ no próprio robô, o caminho mais curto é **dentro da demo** (criar um nó novo, guia 5),
porque o projeto, a compilação (`./scripts/build.sh`) e as bibliotecas já estão prontos.

### ⚠️ Existem DUAS cópias do SDK C++ no robô
| Onde | Versão | Observação |
|---|---|---|
| `/usr/local/include/booster` + `/usr/local/lib` | **Antiga** | Usada por padrão na compilação. **Não conhece** `RobotMode::kSoccer` |
| `/opt/booster/python3/site-packages/include/booster` + `.../lib` | **Nova** (SDK 1.6.3, firmware 1.8) | Completa |

Foi por isso que a demo deu `'kSoccer' is not a member of RobotMode`: trocamos o nome pelo número `4`.
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

Dica: esses números aparecem no log da placa de movimento (`/var/log/booster/<boot>/motion/motion.log` na `.101`)
como `Received api request with id: ...`. É o melhor jeito de confirmar que o seu comando chegou.
Códigos de resposta: `0` ok · `100` sem resposta · `400` inválido · `409` conflito · `429` frequente demais ·
`500/501/502` erro / recusado / mudança de estado falhou.

## 2.7 Controle direto dos motores (modo Custom): só para depois

Mensagem `booster_interface/msg/LowCmd` = lista de `MotorCmd` (um por junta; guia 1, seção 1.5):
`mode, q (posição), dq (velocidade), tau (torque), kp (rigidez), kd (amortecimento), weight`.
Publicada em `/joint_ctrl`, **só vale no modo Custom**, e **o firmware deixa de equilibrar o robô**: só com o robô
pendurado no suporte e supervisão. É o caminho de pesquisa com aprendizado por reforço (projetos `booster_train`
e `booster_deploy`, guia 4), não o de melhorar o futebol agora.
