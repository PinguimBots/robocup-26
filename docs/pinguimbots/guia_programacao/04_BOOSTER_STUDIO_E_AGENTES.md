# 4. Booster Studio, simulador e agentes

## 4.1 O que é o Booster Studio

Uma **IDE** (editor de código) da Booster baseada no **VS Code**, instalada neste PC:
- Pacote `booster-studio` em `/usr/share/booster-studio/`. Versão do Studio: **1.12.5** (o "1.110.1" do pacote é a base VS Code).
- Seus dados: `~/.config/Booster Studio/` e `~/.booster-studio/`. Projetos: `~/BoosterStudioProjects/`.

O que ele oferece além de um editor:

| Recurso | Para que serve |
|---|---|
| **Robôs** | Conectar ao T1 real (acha na rede) ou criar um **robô virtual** |
| **Simulador** | MuJoCo 3.10 rodando em Docker, com cenários de campo de futebol para o T1 (`football_pitch_T1`) e GameController embutido |
| **Agentes** | Criar a partir de modelo, compilar, enviar ao robô, rodar e depurar (Python, depurador na porta 5678) |
| **Jupyter no robô** | Cadernos com exemplos prontos para o T1: `walk_example.py`, `imu_example.py`, `camera_image_viewer.py` |
| **Visualizadores** | Lichtblick (tipo Foxglove, para tópicos ROS e gravações) e Rerun (o mesmo dos logs da demo) |
| **Outros** | Terminal do robô, gravação de *rosbag*, criação de movimentos, diagnóstico, exportar logs, assistente de IA (pede login) |

## 4.2 ⚠️ Hoje o simulador e a compilação de agentes não funcionam neste PC

Ambos dependem do **Docker**. O Docker está instalado (snap) e rodando, mas o seu usuário **não tem acesso**
(`/var/run/docker.sock` só para root, e não existe o grupo `docker`). Os logs do Studio já registraram
"cannot connect to Docker, so virtual robot features are unavailable".

A solução comum para o Docker em snap (**não testada aqui**; você decide e roda):
```bash
sudo addgroup --system docker
sudo adduser $USER docker
sudo snap disable docker && sudo snap enable docker
```
Depois, sair e entrar de novo na sessão. **Atenção:** quem está no grupo `docker` tem, na prática, poder de administrador no PC.

## 4.3 O que é um "agente"

Um **agente** é um programa **Python** empacotado no formato da Booster, que roda **no robô** e aparece como
**botões no app** (o app desenha os botões; o toque volta para o agente). O agente *default* e o de futebol
são agentes da Booster. No nosso robô, o *default* está rodando agora (`pyagent_aarch64`).

### O projeto de exemplo (`~/BoosterStudioProjects/MyAgent`)
```
MyAgent/
├── agent.toml      ← "carteira de identidade": id, versão, classe de entrada, requisitos
├── build.toml      ← dependências pip, plataformas (simulador, Jetson…), assinatura
├── src/main.py     ← o código do agente
└── res/            ← ícones dos botões
```
`agent.toml`:
```toml
id = "com.example.myagent"
version = "1.0.0"
entry = "src/main.py:ExampleAgent"
debuggable = true
[requirements]
min_api_level = 10700
models = [ "Booster T1", "Booster K1" ]
```
`src/main.py` (resumo): uma classe que herda de `AgentBase`, cria um `BoosterRobot()` (o mesmo do guia 2) e
registra um botão "Wave" que só aparece em WALK:
```python
class ExampleAgent(AgentBase):
    def __init__(self):
        super().__init__(AgentFeatures(enable_auto_getup=True))
        self.robot = BoosterRobot()
        self.setup_components()          # cria os botões
    def on_agent_activated(self): self.logger.info("ExampleAgent is activated")
    # ao tocar no botão: self.robot.do_action("hand_wave")
```

### Fluxo de trabalho no Studio
1. Conectar ao robô (ou criar o robô virtual, com Docker liberado).
2. **Create Agent** a partir de um modelo.
3. Editar `src/main.py`, `agent.toml`, `build.toml`.
4. **Ativar, compilar, enviar e rodar** (compila num contêiner Docker e envia ao robô por SSH/rsync).
5. Acompanhar os logs; depurar com pontos de parada (`debuggable = true`).
6. Gerar o pacote `.agent` final (assinado) para instalar pelo app.

### Versões ("API level")
- `API level = maior × 10000 + menor × 100 + revisão`. Firmware 1.8.1 → **10801**.
- O Studio **< 1.13** gera agentes para 10700; o manual atual já descreve 10800 (Studio ≥ 1.13). Vale atualizar o Studio.
- O agente de futebol 1.0.3 que veio no robô pede `min_api_level 10800` e JetPack 6.2, que o nosso robô atende.
  A recusa do app ("agent version outdated") não aparece em nenhum log do robô nem no manual: é decidida pelo
  app/servidor da Booster. Caso para o suporte da Booster.

### Agente × código de futebol do time
| | Agente (Python, Studio) | Demo de futebol (C++, ROS) |
|---|---|---|
| Linguagem | Python | C++ |
| Interface | Botões no app | Controle + GameController |
| Desempenho | Bom para comportamentos e testes | Feito para tempo real (visão + decisão a 100 Hz) |
| Quando usar | Prototipar ideias, demonstrações, ferramentas | O jogador de futebol em si |

O próprio Studio traz um modelo de agente de futebol simplificado (`soccer_sim`, partida 3×3 com `py_trees`
e `SoccerKickManager`). Bom para **estudar** e **simular**, mas o simulador de 3×3 só aceita o K1.

## 4.4 O manual de desenvolvimento

Cópia local do time: `Manual_Booster_Developer_Guide/md/`, no PC (é a mesma versão do site, `release-0.0.42`, conferida em 17/11/2026).

| Seção | Conteúdo |
|---|---|
| `booster-os-python-sdk/` | O `boosteros` (guia 2): instalação, `BoosterRobot`, sensores, movimento, tarefas, áudio, visão e fala, FAQ |
| `agent-development/` | Agentes: conceitos, API, botões/páginas, parâmetros (`schema.json`), configuração, versões e publicação |
| `cpp/` | SDK C++: instalação, canais DDS, `B1LocoClient`, tabela de APIs com versão mínima de firmware, códigos de erro |
| `open-source/` | Projetos abertos: `robocup_demo` (a demo; instruções para **K1**), `booster_train` (treinar com RL no Isaac Lab), `booster_deploy` (rodar políticas no robô ou no MuJoCo/Webots), `booster_assets` (modelos URDF/MJCF do T1) |
