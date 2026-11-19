# 4. Booster Studio: o que dá para fazer com o T1

O **Booster Studio** é a IDE da Booster, baseada no **VS Code** (mesma aparência: arquivos à esquerda, editor
no centro, terminal embaixo). Além de editar código, ele **conecta no robô** e oferece painéis para **ver**,
**comandar**, **programar**, **simular** e **diagnosticar** o T1.

| | No nosso PC (19/11/2026) |
|---|---|
| Versão | **1.14.5** (pacote `booster-studio`, em `/usr/share/booster-studio/`) |
| Docker (necessário para simulador e agentes) | ✅ funcionando |
| Seus dados | `~/.config/Booster Studio/`, projetos em `~/BoosterStudioProjects/` |

---

## 4.1 Mapa: o que fazer × onde fazer

| Quero… | Use | Precisa de |
|---|---|---|
| Ver o estado do robô (modo, bateria, temperaturas) | Painéis **Booster Robot Status**, **Battery History**, **Joints Temperature History** | Robô conectado |
| Ver o robô em 3D com as juntas se mexendo | **Booster Robot Body Display** | Robô conectado |
| Ver a câmera | **Booster Vision** ou **Image** | Ponte da câmera (Booster Vision) |
| Mexer a cabeça | **Booster Head Control** | Robô em **WALK** |
| Fazer o robô andar pelo PC | **Booster Motion Control** | Robô em **WALK**, área livre |
| Ver/plotar qualquer dado ROS 2 | **Plot**, **Raw Messages**, **Table**, **Topic Graph**… | Robô conectado |
| Escrever e rodar Python no robô | **Notebook** (Jupyter no robô) ou **Terminal** | Robô conectado |
| Editar o código C++ da demo | **Remote - SSH** | Robô conectado |
| Criar um programa que vira botões no app | **Agentes** | Docker |
| Testar sem o robô real | **Robô virtual** (simulador) | Docker |
| Gravar dados e analisar depois | Gravação (rosbag) + painéis | Robô conectado |

---

## 4.2 Conectar ao T1

1. Robô ligado há 1 minuto, cabo do PC conectado (PC em `192.168.10.224`).
2. No painel de robôs: **Connect to Booster Robot** (o Studio procura robôs na rede) ou **Connect to Custom Robot**
   com IP `192.168.10.102` e usuário `booster`.
3. Rode **Run Booster diagnostics** para conferir.

**O que o Studio liga no robô ao conectar** (conferido no robô):

| Serviço no robô | Porta | Para quê |
|---|---|---|
| Ponte ROS (dados dos tópicos para os painéis) | 8765 | Painéis de dados |
| Gerenciador de agentes | 6868 | Agentes |
| Agentes (websocket) | 16888 | Agentes |
| **Jupyter** (Python em `~/.booster_venv`, pasta inicial `/home/booster`) | 8888 | Painel Notebook |

---

## 4.3 Painéis (o coração do Studio)

O Studio monta telas com **painéis** (como o Foxglove/Lichtblick, usado por muitos robôs ROS). Você escolhe os
painéis, arruma lado a lado e salva o **layout**.

### Painéis da Booster (específicos do robô)
| Painel | O que faz no T1 | Observações |
|---|---|---|
| **Booster Robot Status** | Modo, estado geral | Primeiro painel a abrir |
| **Booster Robot Body Display** | Modelo 3D do T1 seguindo as juntas reais | Ótimo para entender as 23 juntas (guia 1) |
| **Booster Battery History** | Histórico da bateria | Útil em treinos longos |
| **Booster Joints Temperature History** | Temperatura de cada motor ao longo do tempo | **Acompanhe nos testes de campo**: motor quente = parar |
| **Booster Head Control** | Mexe a cabeça (API 2004) | **Só em WALK**. Fora dele aparece *"please check the motion status"* (testado: em WALK funcionou) |
| **Booster Motion Control** | Faz o robô **andar** (API 2001, velocidade) | **Só em WALK**. ⚠️ o robô anda de verdade: área livre e alguém junto |
| **Booster Vision** | Imagem da câmera | No nosso robô **só com a ponte da câmera** ligada (inicia no boot; ver [PONTE_CAMERA_ZED.md](../PONTE_CAMERA_ZED.md)) |

### Painéis ROS 2 (genéricos, servem para qualquer tópico)
| Painel | Exemplo útil no T1 |
|---|---|
| **Image** | `/zed/zed_node/left/image_rect_color` (imagem original da ZED, funciona mesmo sem a ponte) |
| **Plot** | Gráfico no tempo: inclinação da IMU (`/imu/data`), velocidade da odometria (`/odom`) |
| **Raw Messages** | Ver uma mensagem inteira: `/booster_vision/detection` (o que a visão da demo achou), `/robocup/game_controller` |
| **Table** | Mensagens em tabela: `/low_state` (posição e temperatura de cada motor) |
| **Topic Graph** | Desenho de quem publica e quem assina cada tópico (ótimo para entender o guia 3) |
| **Log (RosOut)** | Mensagens de log dos nós (ex.: `brain_node`, `vision_node`) filtradas por gravidade |
| **Parameters** | Ler (e mudar) parâmetros ROS de um nó em execução |
| **Publish** | ⚠️ Publicar uma mensagem num tópico (cuidado: pode comandar o robô) |
| **Call Service** | Chamar um serviço ROS e ver a resposta |
| **State Transitions** | Ver quando um valor muda (ex.: o estado do jogo) |
| **Gauge / Indicator / Pie Chart** | Mostradores (ex.: bateria) |
| **Diagnostics (Summary/Detail)** | Mensagens de diagnóstico ROS |
| **2D / Map** | Desenhos 2D (pontos, posições) |
| **Node Playground** | Pequenas transformações de dados escritas em TypeScript |
| **Teleop** | Controle por setas publicando num tópico. O T1 é comandado por RPC, então prefira o Motion Control |
| **Tab** | Agrupar painéis em abas |

### Layouts sugeridos
- **"Saúde do robô"**: Robot Status + Battery History + Joints Temperature History + Body Display.
- **"Visão"**: Image (`/zed/.../image_rect_color`) + Raw Messages (`/booster_vision/detection`) + Plot (`/booster_vision/ball` → `x`, `y`).
- **"Depuração da demo"**: Topic Graph + Log + Raw Messages (`/robocup/game_controller`) + Booster Vision.

---

## 4.4 Programar no robô pelo Studio

### Notebook (Jupyter no robô)
- Painel **Notebook**: abre cadernos que rodam **dentro do robô**, na pasta `/home/booster`.
- Crie seus cadernos em `/home/booster/PinguimBots_exercicios/` (a mesma pasta dos exercícios do guia 2).
- Exemplos prontos da Booster para o T1 (dentro do Studio): `imu_example.py` (lê a IMU), `walk_example.py`
  (⚠️ anda 0,3 m/s em várias direções; exige WALK), `camera_image_viewer.py` (precisa da ponte da câmera).
- Atalhos: **Shift+Enter** roda a célula; **■** interrompe.

### Terminal
- Painel **Terminal** ou terminal integrado (`Ctrl+'`): um terminal **no robô**, sem abrir outro programa.

### Remote - SSH (editar a demo de futebol)
1. **Remote - SSH → Connect to Host** → `booster@192.168.10.102`.
2. **Open Folder** → `/home/booster/Demo_PinguimBots`.
3. Edite `src/brain/...` com o editor completo; compile e rode pelo terminal integrado (guia 5).

---

## 4.5 Agentes: programas que viram botões no app

Um **agente** é um programa **Python** empacotado no formato da Booster, que roda **no robô** e aparece como
**botões no app** (e atalhos no controle). O agente *default* da Booster está rodando no nosso robô agora.

**O que um agente pode fazer no T1:** botões que disparam ações (gestos, falar, andar até algo, mexer a cabeça),
telas que mudam com o modo do robô, parâmetros ajustáveis pelo app, atalhos no controle, ler sensores e câmera.

### Criar e rodar (passo a passo)
1. **Create Agent** → escolha o modelo de exemplo → nome do projeto (fica em `~/BoosterStudioProjects/<nome>/` no PC).
2. Arquivos do projeto:
   ```
   <nome>/
   ├── agent.toml     ← identidade: id, versão, classe de entrada, modelos (T1), api level mínimo
   ├── build.toml     ← dependências pip e plataformas (simulador, Jetson real…)
   ├── src/main.py    ← o código (classe que herda de AgentBase)
   └── res/           ← ícones dos botões
   ```
3. Edite o `src/main.py` (exemplo do modelo: um botão "Wave" que chama `self.robot.do_action("hand_wave")`).
4. Escolha o robô (real ou virtual) e use **Activate, build, deploy and run**: o Studio compila num contêiner
   Docker, envia ao robô (`/tmp/booster_agent_deploy/`) e inicia.
5. Os botões aparecem no **app**. Com `debuggable = true` no `agent.toml`, dá para pôr pontos de parada (depurador na porta 5678).
6. Para distribuir: gerar o pacote `.agent` (assinado) e instalar pelo app.

### Versões ("API level")
- `API level = maior × 10000 + menor × 100 + revisão` → firmware 1.8.1 = **10801**.
- Studio < 1.13 gerava agentes para 10700; com a **1.14.5** o projeto novo deve sair para 10800 (confira o
  `min_api_level` no `agent.toml` gerado).
- O agente de futebol oficial 1.0.3 foi recusado pelo app ("agent version outdated"): decisão do app/servidor da
  Booster, não do robô (caso para o suporte).

### Agente × código de futebol do time
| | Agente (Python, Studio) | Demo de futebol (C++, ROS 2) |
|---|---|---|
| Interface | Botões no app | Controle + GameController |
| Desempenho | Bom para comportamentos e ferramentas | Tempo real (visão + decisão ~100 Hz) |
| Use para | Protótipos, demonstrações, utilitários (ex.: "calibrar", "checar robô") | O jogador de futebol |

---

## 4.6 Robô virtual (simulador)

- **Create a Virtual Robot** → escolha **T1** → cenário: `default_pitch_T1`, `football_pitch_T1` (campo de futebol)
  ou `kitchen_T1`.
- Roda o simulador **MuJoCo** num contêiner Docker (pelos scripts do Studio, com o software de locomoção da
  Booster dentro), e o painel **Virtual Bot Simulation** mostra a cena.
- Os mesmos códigos Python (exercícios do guia 2, agentes) rodam no robô virtual: exemplos próprios em
  `virtual_t1` (`walk_example.py`, `imu_example.py`, `camera_image_viewer.py`).
- **Limites:** o modelo de partida 3×3 (`soccer_sim`) só aceita o K1; a câmera simulada é a padrão (não a ZED);
  a demo C++ de futebol não foi testada no simulador.
- Use para: testar lógica de andar/cabeça sem risco, aprender o SDK sem gastar bateria, ensaiar agentes.

---

## 4.7 Diagnóstico, logs e gravação

| Recurso | Para quê |
|---|---|
| **Run Booster diagnostics** | Checar conexão e serviços do robô |
| **Export Booster Logs** | Juntar os logs do robô para análise ou suporte da Booster |
| **Gravação de rosbag** | Gravar tópicos (ex.: câmera + detecções durante um teste) e reproduzir depois nos painéis |
| **Rerun** | Abrir os logs `.rrd` do cérebro da demo (campo, bola, decisões) |
| **Motion Creator** | Criar movimentos/trajetórias de braços (avançado) |
| Assistente de IA | Ajuda dentro do Studio (pede conta Booster) |

---

## 4.8 O manual de desenvolvimento

Cópia local do time: `Manual_Booster_Developer_Guide/md/`, no PC (mesma versão do site, `release-0.0.42`).

| Seção | Conteúdo |
|---|---|
| `booster-os-python-sdk/` | O `boosteros` (guia 2): `BoosterRobot`, sensores, movimento, tarefas, áudio, visão e fala, FAQ |
| `agent-development/` | Agentes: conceitos, API, botões/páginas, parâmetros (`schema.json`), versões e publicação |
| `cpp/` | SDK C++: canais DDS, `B1LocoClient`, tabela de APIs com versão mínima de firmware, códigos de erro |
| `open-source/` | `robocup_demo` (instruções para **K1**), `booster_train` (treinar com RL no Isaac Lab), `booster_deploy` (rodar políticas no robô/MuJoCo/Webots), `booster_assets` (modelos URDF/MJCF do T1) |
