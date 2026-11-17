# Operação — rodar sem conflito e com segurança

## Regra principal: uma versão de cada vez

As versões usam os mesmos nomes de programa (`vision_node`, `brain_node`, `game_controller`) e os mesmos
tópicos. **Antes de iniciar uma, pare a outra:**

```bash
~/robocup_public/scripts/stop.sh
~/robocup-26/scripts/stop.sh   # se existir/compilada
```

## Iniciar o `robocup_public` (a que funciona)

```bash
cd ~/robocup_public && ./scripts/start.sh
tail -f brain.log vision.log
./scripts/stop.sh
```
> O `start.sh` liga `save_data:=true` (grava imagens em `~/Workspace/vision_log`). Para testes curtos de visão,
> preferir rodar só a visão (abaixo).

## Testar só a visão (robô parado)

```bash
ssh -X booster@192.168.10.102          # -X: a janela de detecção abre no PC
cd ~/robocup_public && source install/setup.bash && export FASTRTPS_DEFAULT_PROFILES_FILE=./configs/fastdds.xml
ros2 launch vision launch.py
# em outro terminal: ros2 topic echo --once /booster_vision/ball
```
Na janela de detecção as cores aparecem trocadas (azul ↔ vermelho): é só a exibição; o modelo recebe as cores certas.

## Sequência no campo (`robocup_public`)

Mapa completo do controle no firmware 1.8: [CONTROLE_FIRMWARE_1.8.md](CONTROLE_FIRMWARE_1.8.md).

1. Robô apoiado → **PREP** (`LT + Start` ou botão **P**) → no chão, firme → **WALK** (`RT + A`).
2. Iniciar o programa.
3. **LT + A** (cérebro): relocaliza **só com a cabeça** (as pernas não se mexem; o modo do firmware continua WALK). Nunca pular.
4. Esperar o robô fixar o olhar na bola (se a cabeça ficar girando → problema de visão/calibração).
5. **LT + B** (cérebro): estratégia automática (segue o GameController).
6. **LT + X** (cérebro): devolve o controle manual (mexer o joystick para cancelar o último comando).

> Na demo 1.7+ o cérebro **já começa no automático**, sem esperar o `LT + B`, e tenta levantar sozinho se
> achar que o robô caiu em DAMP: nunca iniciá-lo com o robô sentado em DAMP.

## Segurança

- Sequência de modos: **DAMP → PREP → WALK**. Voltar a DAMP (`LT + Back`) só com o robô apoiado.
- Com o cérebro rodando, evitar `LT + Y` (no firmware é **Chutar**) e `A/B/X/Y` sozinhos (gestos).
- Para testar **sem chute**: não usar `RB` (chuta), `LB` (persegue sem parar antes da bola) nem `LT + Y`, e não usar
  GameController (sem o estado PLAY, o automático não decide chutar).
- Reiniciar a placa de movimento desliga os motores: robô sentado, deitado ou no suporte.
- Depois de mudar qualquer `config.yaml`, recompilar (`./scripts/build.sh`).
- GameController: `team_id` igual nos dois lados, `player_id` único, IP do GameController correto.
