# Programando o Booster T1: comece aqui

Guia para quem quer **programar** o robô (escrever código novo), e não apenas ajustar parâmetros.
Escrito para um programador iniciante, a partir do código real do nosso T1 (firmware 1.8.1.2, 18/11/2026).

## Os guias

| # | Arquivo | O que você aprende |
|---|---|---|
| 0 | este | O mapa geral, o vocabulário e a ordem de estudo |
| 1 | [01_ROBO_FIRMWARE_E_COMUNICACAO.md](01_ROBO_FIRMWARE_E_COMUNICACAO.md) | O que roda dentro do robô e como os programas conversam |
| 2 | [02_SDK_PYTHON_E_CPP.md](02_SDK_PYTHON_E_CPP.md) | Como **o seu programa** manda o robô fazer coisas: onde criar os arquivos, como rodar e 5 exercícios |
| 3 | [03_DEMO_ROBOCUP_POR_DENTRO.md](03_DEMO_ROBOCUP_POR_DENTRO.md) | Como o código de futebol funciona: visão, cérebro, localização |
| 4 | [04_BOOSTER_STUDIO_E_AGENTES.md](04_BOOSTER_STUDIO_E_AGENTES.md) | O que dá para fazer com o Booster Studio no T1: painéis, Notebook, Remote-SSH, agentes, simulador |
| 5 | [05_COMO_MELHORAR_O_CODIGO.md](05_COMO_MELHORAR_O_CODIGO.md) | Passo a passo para escrever código novo e ideias de melhoria |

## O robô em camadas

Programar um robô é escolher **em qual camada** você vai trabalhar. Quanto mais embaixo, mais poder e mais risco.

```
 ┌───────────────────────────────────────────────────────────────┐
 │ 5. Aplicações  │ demo de futebol (C++)  │ agentes (Python)   │  ← onde o time programa
 ├───────────────────────────────────────────────────────────────┤
 │ 4. ROS 2        tópicos, nós, mensagens (/odom, /low_state…)  │
 ├───────────────────────────────────────────────────────────────┤
 │ 3. SDK          boosteros (Python) · B1LocoClient (C++)        │  ← "biblioteca" para falar com o firmware
 ├───────────────────────────────────────────────────────────────┤
 │ 2. Firmware     modos, marcha (andar), equilíbrio, chutes      │  ← da Booster, não editamos
 ├───────────────────────────────────────────────────────────────┤
 │ 1. Hardware     23 motores, IMU, câmera ZED, 2 computadores    │
 └───────────────────────────────────────────────────────────────┘
```

- **Você não programa como o robô anda.** A marcha (o equilíbrio de cada passo) é um controlador do firmware.
  Você diz "ande a 0,3 m/s para frente" e o firmware cuida das pernas.
- **Você programa o que o robô decide fazer:** para onde andar, para onde olhar, quando chutar, como achar a bola,
  como saber onde está no campo, como cooperar com os colegas.
- Existe uma camada mais baixa (**controle direto dos motores**, modo *Custom*), mas ela é perigosa e só faz
  sentido para pesquisa avançada (ver guia 2).

## Vocabulário mínimo

| Termo | Explicação curta |
|---|---|
| **ROS 2** | "Sistema nervoso" do robô: vários programas (**nós**) trocam **mensagens** por **tópicos**. Usamos o ROS 2 *Humble* |
| **Nó** | Um programa ROS (ex.: `vision_node`, `brain_node`) |
| **Tópico** | Um "canal" com nome (ex.: `/odom`). Quem publica escreve, quem assina lê |
| **DDS** | A rede por baixo do ROS 2. No robô é o *Fast DDS*, com um **perfil** de configuração obrigatório |
| **RPC** | "Pedir para o firmware fazer algo e receber resposta" (ex.: mudar de modo). No T1 é feito por tópicos `LocoApiTopicReq/Resp` |
| **SDK** | Biblioteca pronta que esconde o RPC: você chama `robot.set_velocity(...)` em vez de montar mensagens |
| **Modo** | Estado do firmware: DAMP, PREP, WALK, Custom, Soccer (guia 1) |
| **Árvore de comportamento** | Jeito de organizar decisões em XML (BehaviorTree.CPP). É o "roteiro" do jogador (guia 3) |
| **Blackboard** | "Quadro de avisos" compartilhado pela árvore (ex.: `ball_location_known = true`) |
| **TensorRT / YOLO** | Rede neural que acha bola, gol e pessoas na imagem, otimizada para a placa NVIDIA |
| **Odometria** | Estimativa de quanto o robô andou, calculada pelo firmware |
| **Localização** | Descobrir onde o robô está no campo comparando o que vê com o mapa |
| **Agente** | Programa Python empacotado no formato da Booster, que aparece como botões no app (guia 4) |
| **colcon** | Ferramenta que compila os pacotes ROS (`./scripts/build.sh` usa ela) |

## Ordem de estudo sugerida

1. **Ler os guias 1 e 2** e fazer os exercícios 1 a 3 do guia 2 (pasta `~/PinguimBots_exercicios` no robô).
   Em paralelo, abrir no Booster Studio os painéis de estado e câmera (guia 4, seção 4.3).
2. **Ler o guia 3** com o código aberto ao lado (`~/Demo_PinguimBots/src/brain`).
3. **Fazer o exercício do guia 5**: criar um nó novo na árvore que só **registra** informações (não move o robô).
4. Escolher **uma** melhoria do guia 5, fazer numa pasta/ramo separado, testar parado, depois no campo.
5. Em paralelo, se quiser simular sem o robô: liberar o Docker e usar o simulador do Booster Studio (guia 4).

## Regras de segurança para programar

1. **Nunca edite o `~/robocup_public`.** Trabalhe na `~/Demo_PinguimBots` ou em cópias, sempre com Git (ramo novo).
2. **Robô segurado** em todo teste novo. Emergência: `LT + Back` (DAMP), segurando o robô.
3. **Velocidades pequenas** primeiro (≤ 0,3 m/s). O código da demo força velocidade mínima de 0,3 (guia 3).
4. **Sem chutes** até decidirem como testá-los. Na demo, o chute é feito pela árvore `Kick`/`RLVisionKick`.
5. **Nunca** `apt upgrade` no robô, nunca controle direto de motores sem supervisão.
6. **Um programa de futebol por vez** (`./scripts/stop.sh` antes de trocar).
