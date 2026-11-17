# Controle (joystick) no firmware 1.8

Fonte: tabela oficial da Booster para o firmware 1.8 ("Basic Capabilities" / "Agent Capabilities").
Comandos antigos como **`RT + Y` = PREP não valem mais** (agora `RT + Y` é a saudação proativa).

## Comandos básicos (firmware)

| Ação | Botões | Pré-requisito |
|---|---|---|
| Entrar no agente Default | `LT + RT + A` | Modo PREP |
| Entrar no agente de futebol (Soccer) | `LT + RT + B` | Modo PREP |
| Entrar no agente Hi Chat | `LT + RT + X` | Modo PREP (precisa de internet) |
| Entrar no agente Dance | `LT + RT + Y` | Modo PREP |
| **Modo DAMP** | **`LT + Back`** | — |
| **Modo PREP** | **`LT + Start`** | — |
| **Modo WALK** | **`RT + A`** | Modo PREP |
| Andar | Analógico esquerdo | Modo WALK |
| Girar | Analógico direito | Modo WALK |
| Mover a cabeça | Direcionais | Modo WALK |
| Iniciar/parar conversa por voz | `RT + X` | — |
| Iniciar/parar saudação proativa (vai até a pessoa detectada e cumprimenta) | `RT + Y` | Conversa por voz ativa |

## Ações do agente Default (firmware)

| Ação | Botões | Pré-requisito |
|---|---|---|
| Aperto de mão | `A` | Modo WALK |
| Acenar | `B` | Modo WALK |
| Ultraman | `X` | Modo WALK |
| Reverência | `Y` | Modo WALK |
| Gato da sorte | `RB + A` | Modo WALK |
| Dabbing | `RB + B` | Modo WALK |
| Comemorar | `RB + X` | Modo WALK |
| Carregar | `RB + Y` | Modo WALK |
| Dança 1 / 2 / 3 | `LB + ↑` / `LB + ←` / `LB + →` | Modo WALK |
| Levantar | `LT + ↑` | Modo DAMP ou PREP |
| Chutar (Shoot) | `LT + Y` | Modo WALK |

## Botões do cérebro do robocup (só com o código de futebol rodando)

Não vêm do firmware: são lidos pelo `brain` do `robocup_public` e da demo 1.7+ (`src/brain/src/brain.cpp`).

| Botões | Ação |
|---|---|
| Botões | `robocup_public` | Demo 1.7+ |
|---|---|---|
| `LT + A` | Relocalizar **só com a cabeça** (varre o campo e segue a bola); **as pernas não se mexem e o modo do firmware não muda** | Relocalizar **e** trocar para o modo futebol do firmware (`kSoccer`) |
| `LT + B` | Estratégia automática (segue o GameController) | Idem |
| `LT + X` | Cancelar o automático: para e centraliza a cabeça | Idem |
| `LT + Y` | **Desativado** (comentado no código) | Trocar o papel (atacante ↔ goleiro) |
| `LB` (segurando) | ⚠️ Persegue a bola **sem parar antes dela** (`stop_dist = 0`) | Idem |
| `RB` (segurando) | ⚠️ **Chuta** (assistência de chute) | Idem |
| Analógicos | Assumem o controle manual | Idem |

> O `robocup_public` **joga no modo WALK normal**: as funções que ativam o modo futebol do firmware
> (`walkMode()` / `robocupWalk()`, API `kEnableRobocupWalkMode`) existem em `robot_client.cpp`, mas **nunca são
> chamadas**. Os chutes usam diretamente as APIs `kRLKickBall` / `kRLFancyKickBall`.

> Na demo 1.7+, o `game.xml` já começa no automático (equivale a `LT + B`), sem esperar o botão.

## ⚠️ Conflitos (cérebro rodando + robô em WALK)

Os dois conjuntos valem ao mesmo tempo:
- **`LT + Y`**: no firmware é **Chutar**. No `robocup_public` o cérebro não usa esse botão, então **o único efeito é o
  chute do firmware**; na demo 1.7+ ele também troca o papel. Evitar sempre.
- **`RB`** e **`LB`** (cérebro) podem chutar ou empurrar a bola: não usar quando não se quer chute.
- **`A`, `B`, `X`, `Y` sozinhos** fazem gestos no firmware. Nos comandos `LT + A/B/X`, segurar o `LT` **antes** e soltar **depois** da letra.
- **`LB` + direcionais**: danças no firmware; no cérebro, `LB` é perseguir a bola e os direcionais tocam sons.

**A testar:** se o agente de futebol (`LT + RT + B`) desliga os gestos e danças do agente Default.

## Botões F e P nas costas

O botão **P** coloca em **PREP**. O **F** (modo futebol) ainda não foi testado após a correção.
Ver [FIRMWARE_1.8_CORRECOES.md](FIRMWARE_1.8_CORRECOES.md), item 6.
