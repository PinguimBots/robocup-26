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

## Sequência no campo (vale para todas as versões)

1. Robô apoiado → **PREP** → no chão, firme → **WALK**.
2. Iniciar o programa.
3. **LT + A**: relocaliza **e** entra no modo `kSoccer`. Nunca pular.
4. Esperar o robô fixar o olhar na bola (se a cabeça ficar girando → problema de visão/calibração).
5. **LT + B**: estratégia automática (segue o GameController).
6. **LT + X**: devolve o controle manual (mexer o joystick para cancelar o último comando).

## Segurança

- Sequência de modos: **DAMP → PREP → WALK**. Voltar a DAMP só com o robô apoiado.
- Reiniciar a placa de movimento desliga os motores: robô sentado, deitado ou no suporte.
- Depois de mudar qualquer `config.yaml`, recompilar (`./scripts/build.sh`).
- GameController: `team_id` igual nos dois lados, `player_id` único, IP do GameController correto.
