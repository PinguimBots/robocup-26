# Teste com o GameController novo: firmware 1.8 + `Demo_PinguimBots`

**Objetivo:** o robô seguir os estados do jogo e andar sozinho **no modo futebol**, **sem chegar a chutar**.
**Código:** `~/Demo_PinguimBots` (demo 1.7+ da Booster, compilada, com a configuração do T1/ZED).
O `robocup_public` **não funciona** com este GameController (protocolo 12 × 20).

## Atenção
- O cérebro **começa no automático** e **tenta levantar sozinho** se achar que caiu: nunca iniciar com o robô sentado em DAMP.
- **Não avançar para PLAYING**: nesse estado o atacante **chuta**.
- **Emergência:** `LT + Back` (DAMP), **segurando o robô**. `LT + X` ou analógicos tiram do automático.

## Etapas

**1. PC: abrir o GameController** (cabo conectado, IP `192.168.10.224`; ver a interface com `ip -4 -br a`)
```bash
cd <pasta do GameController>          # compilado com: cargo build -r
./target/release/game_controller_app -c middle_foundation --home-team 42 -i <interface do cabo>
```
Clicar em **Start** e **deixar em INITIAL**.

**2. Robô: modos**
- Apoiado → **PREP** (`LT + Start` ou botão **P**) → no chão, firme e segurado → **WALK** (`RT + A`).

**3. Robô: iniciar a demo**
```bash
ssh booster@192.168.10.102
cd ~/Demo_PinguimBots && ./PinguimBots/start_PinguimBots.sh tudo
```
Conferir no GameController: **jogador 3 do time 42 online**.

**4. Controle e GameController**

| Etapa | Ação | Quem controla |
|---|---|---|
| 4.1 Modo futebol | `LT + A` | Muda para o **modo futebol** do firmware; só a **cabeça** se mexe |
| 4.2 Automático | `LT + B` | **Robô** (obedece ao GameController) |
| 4.3 INITIAL | — | Parado; cabeça segue a bola |
| 4.4 **READY** | Botão no GameController | ⚠️ **Robô anda sozinho** até a posição de saída |
| 4.5 **SET** | Botão no GameController | Para na posição |
| ❌ PLAYING | **Não usar** | Chutaria |

**5. Parar**
1. `LT + X` (cancela o automático).
2. No terminal do robô:
   ```bash
   ./scripts/stop.sh
   ```
3. Segurar o robô → **PREP** (`LT + Start`) → **DAMP** (`LT + Back`).

## Se algo falhar
| Sintoma | Verificar |
|---|---|
| Robô não recebe o GameController | `grep handled ~/Demo_PinguimBots/game_controller.log` (IP precisa ser `192.168.10.224`) |
| Robô não aparece online | `tail -n 30 ~/Demo_PinguimBots/brain.log` |
| PC mudou de IP | `./PinguimBots/start_PinguimBots.sh tudo gc_ip:=<IP>` e ajustar `game_control_ip` no `config_local.yaml` (recompilar) |
