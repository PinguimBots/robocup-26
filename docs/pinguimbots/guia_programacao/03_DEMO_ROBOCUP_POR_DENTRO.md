# 3. O código de futebol por dentro (`~/Demo_PinguimBots`)

Base: demo oficial da Booster (ramo `sandbox/support_firmware_1.7+`), na placa `.102`, com os ajustes do time
(pasta `PinguimBots/`, `*_local.yaml`, 2 linhas de código). São ~11 mil linhas de C++ só no cérebro.

## 3.1 Os pacotes

| Pacote (`src/`) | Programa | Função |
|---|---|---|
| `vision` | `vision_node` | Rede neural (TensorRT/YOLOv8) acha objetos e calcula onde estão no chão |
| `brain` | `brain_node` | Memória, localização, decisões (árvore de comportamento), comandos ao robô, comunicação com o time |
| `game_controller` | `game_controller_node` | Recebe o GameController (UDP 3838, protocolos 12 e 20) |
| `booster_ros2_interface`, `booster_msgs` | — | Formatos das mensagens do firmware |
| `robocup_ros2_interface` | — | Formatos das mensagens de visão e do GameController |

## 3.2 O caminho dos dados

```
Câmera ZED ──/zed/.../image_rect_color──► vision_node ──/booster_vision/detection──► brain_node
(redirecionada pelo nosso launch)          ▲                /booster_vision/line_segments   │
firmware ──/head_pose─────────────────────┘                                                 │
firmware ──/odometer_state, /low_state, /fall_down…──────────────────────────────────────► │
controle ──/remote_controller_state──────────────────────────────────────────────────────► │
game_controller_node ──/robocup/game_controller──────────────────────────────────────────► │
                                                                                            ▼
              firmware ◄── LocoApiTopicReq (andar, cabeça, modo, chutes) ◄── robot_client
              colegas  ◄── UDP broadcast porta 10000 + time, a cada 100 ms
              GameController ◄── UDP porta 3939 ("estou vivo"), a cada 1 s
```

## 3.3 Visão (`src/vision`)

**1. Achar objetos.** A cada imagem, `VisionNode::ColorCallback` (`src/vision_node.cpp`) roda o detector
`YoloV8Detector` (TensorRT, `src/model/trt/`). Classes: `Ball, Goalpost, Person, LCross, TCross, XCross,
PenaltyPoint, Opponent, BRMarker`. Uma segunda rede (segmentação) acha as **linhas do campo**.

**2. Do pixel ao chão.** A ideia é geometria pura: o pixel define um **raio** saindo da câmera; onde o raio
toca o chão é a posição do objeto.
```cpp
Pose p_eye2base = p_head2base * p_headprime2head_ * p_eye2head_;  // pose da cabeça × ajuste × calibração
const cv::Point3f ray_eye = intr.BackProject(target_uv);           // pixel → raio (intrínsecos)
const float scale = (plane_z - camera_z) / ray_z;                  // onde o raio cruza o plano
```
- Para a **bola**, o plano é a altura do centro da bola (raio); para gols, pessoas e cruzamentos, o chão.
- É por isso que a **calibração** (intrínsecos + `extrin`) e a **pose da cabeça** precisam estar certas:
  um erro de 1° no ângulo vira dezenas de centímetros a 3 m.

**3. Publicar.** Resultado em `/booster_vision/detection` (confiança ×100) e `/booster_vision/line_segments`.

## 3.4 O cérebro (`src/brain`)

### O laço principal
`main.cpp` cria três threads; a principal chama `Brain::tick()` (`src/brain.cpp`) ~100 vezes por segundo:
```
tick:  registrar logs → updateMemory() → handleSpecialStates() → handleCooperation()
       → tree->tick()  (árvore de comportamento)  → pubKickMsg()
```
- `updateMemory`: transforma detecções em posições no campo, atualiza a memória da bola
  (some após 3 s sem ver) e o **estimador da bola** (filtro de Kalman com hipóteses "parada" e "rolando").
- `handleCooperation`: decide quem é o "líder" (quem vai na bola) com um custo calculado por robô.

### A árvore de comportamento (BehaviorTree.CPP v4)
As decisões ficam em XML (`behavior_trees/game.xml` e `subtrees/`). Cada tag é um **nó** em C++.

**Exemplo real, do XML ao C++:**
```xml
<SetVelocity x="0" y="0" theta="0"/>
```
```cpp
// include/brain_tree.h
class SetVelocity : public SyncActionNode {
public:
  SetVelocity(const string &name, const NodeConfig &config, Brain *_brain)
      : SyncActionNode(name, config), brain(_brain) {}
  NodeStatus tick() override;
  static PortsList providedPorts() {
    return { InputPort<double>("x", 0, ""), InputPort<double>("y", 0, ""), InputPort<double>("theta", 0, "") };
  }
private:
  Brain *brain;
};
// src/brain_tree.cpp
NodeStatus SetVelocity::tick() {
  double x, y, theta;
  getInput("x", x); getInput("y", y); getInput("theta", theta);
  brain->client->setVelocity(x, y, theta);
  return NodeStatus::SUCCESS;
}
// e o registro, em BrainTree::init:
REGISTER_BUILDER(SetVelocity)
```
Há **52 nós registrados**. Atributos especiais no XML:
- `_while="condição"`: o nó só roda enquanto a condição for verdadeira.
- `_autoremap="true"`: a subárvore usa o mesmo blackboard.

### O blackboard (quadro de avisos)
Variáveis compartilhadas que o C++ escreve e o XML lê. As principais:

| Entrada | Quem escreve | Significado |
|---|---|---|
| `control_state` | controle (`LT+X`=1, `LT+A`=2, `LT+B`=3) | Manual / relocalizar / automático. Começa em 3 |
| `gc_game_state` | `Brain::gameControlCallback` | INITIAL, READY, SET, PLAY, END |
| `gc_play_stopped`, `gc_is_under_penalty` | idem | Jogo parado, robô penalizado |
| `ball_location_known` | `detectProcessBalls` / `updateBallMemory` | Sabe onde a bola está? |
| `player_role`, `is_lead` | `handleCooperation` | Atacante/goleiro, quem vai na bola |
| `decision` | nó `StrikerDecide` | find / chase / adjust / kick / assist… |

### O roteiro do jogo (`game.xml`)
| Estado | Subárvore / nós |
|---|---|
| Jogo parado | `SetVelocity` (fica parado) |
| INITIAL | `CamScanField` + `SelfLocateEnterField` |
| READY | `GoToReadyPosition` + `Locate` (anda até a posição de saída) |
| SET | segue a bola com a cabeça, parado |
| PLAY | `StrikerPlay` (atacante) ou `GoalKeeperPlay` (goleiro) |
| Pênalti / tiro livre | subárvores próprias |

### Como o atacante decide (`StrikerDecide::tick`)
```
não sabe onde está a bola         → "find"   (procura girando)
não é o líder                     → "assist" (fica de apoio)
bola a mais de ~1 m               → "chase"  (vai até um ponto atrás da bola)
alinhado e perto (< 1 m, < 0,2 rad) → "kick" / "power_kick" / "cross"
senão                             → "adjust" (contorna a bola até mirar o gol)
```

### ⚠️ Como o "chute" funciona de verdade
| Código | Chute |
|---|---|
| **Demo 1.7+** | O nó `Kick` **anda contra a bola** (`crabWalk`, com rampa de velocidade). O chute do firmware (`kickBall`) existe mas não é chamado. O chute visual `RLVisionKick` só é usado se habilitado (desligado no T1) e pelo goleiro |
| **`robocup_public`** | O nó `Kick` anda até a bola e **depois dispara o chute do firmware** (`highKick` = `kRLKickBall`, ou `fancyKick` = `kRLFancyKickBall`) |

## 3.5 Localização (`src/locator.cpp`)

**Odometria + correção.** O firmware informa quanto o robô andou (`/odometer_state`, multiplicado por
`odom_factor`). Com o tempo, isso acumula erro. Para corrigir, o robô se **localiza** olhando marcas do campo.

**O algoritmo (`Locator::locateRobot`):** uma busca com **200 partículas** (palpites de posição):
1. espalha palpites dentro de uma região provável;
2. para cada palpite, mede o quanto as marcas vistas (cruzamentos L/T/X, marca do pênalti) **batem com o mapa**;
3. fica com os melhores, aperta a região e repete (até 20 rodadas);
4. aceita se convergiu e se o erro médio ≤ `locator.max_residual` (0,4) com ≥ `locator.min_marker_count` (4) marcas.

O **mapa** sai de `FieldDimensions` (`include/types.h`); o time colocou as medidas do campo da UFPel (4,38 × 3,79 m).
Gols ainda não entram no mapa.

## 3.6 Como os comandos chegam ao robô (`src/robot_client.cpp`)

- Cada comando vira uma mensagem `RpcReqMsg` publicada em `LocoApiTopicReq` **sem esperar resposta**.
- `setVelocity` **força velocidade mínima de 0,3** em cada eixo quando o comando não é zero, e depois limita por
  `robot.vx_limit/vy_limit/vtheta_limit`.
- `moveToPoseOnField2/3`: longe, gira e anda; perto, controle proporcional; opcionalmente desvia de obstáculos
  (pela profundidade da câmera).

## 3.7 Configuração

| Arquivo | Como funciona | Precisa recompilar? |
|---|---|---|
| `src/brain/config/config.yaml` + `config_local.yaml` | Parâmetros ROS do cérebro (o `_local` sobrepõe) | **Sim**: `./scripts/build.sh --packages-select brain` |
| `src/vision/config/vision.yaml` + `vision_local.yaml` | YAML mesclado pela visão | Não (lido direto da pasta `src`) |
| `behavior_trees/*.xml` | Árvores | **Sim** (são copiadas na compilação) |

Todo parâmetro novo do cérebro precisa ser **declarado** em `Brain::Brain()` (`brain.cpp`), senão não é lido.

## 3.8 Depuração

- **Logs de texto:** `brain.log`, `vision.log`, `game_controller.log` na raiz da demo. O cérebro imprime de tempos
  em tempos a pose, a bola, o estado do jogo e um **mapa ASCII do campo** (`debug.low_frequency_status`).
- **Rerun** (visualizador gráfico): o cérebro grava arquivos `.rrd` em `~/Workspace/rrlog/` (abrir com `rerun arquivo.rrd`)
  ou envia ao vivo para um PC (`rerunLog.enable_tcp: true` + `server_ip`). Mostra o campo, a bola, as decisões.
- **Visão:** `show_det:=true` abre a janela de detecção (com `ssh -X`).

## 3.9 Diferenças para o `robocup_public`

| | `robocup_public` | Demo 1.7+ |
|---|---|---|
| GameController | Protocolo 12 | 12 e 20 |
| Modo futebol do firmware | Nunca usa (joga em WALK) | `LT + A` ativa (`kSoccer`) |
| Chute | Chute do firmware (`kRLKickBall`) | Andar contra a bola |
| Estimador de bola (Kalman) | Não tem | Tem |
| Goleiro | `GoalieDecide` (sai e chuta) | `Analyze2`/`Intercept2` (intercepta) |
| Configuração | Edita `config.yaml` direto | `*_local.yaml` por cima |
