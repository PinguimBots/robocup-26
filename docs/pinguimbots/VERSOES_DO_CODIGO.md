# Mapa das versões de código de futebol

Ficam **em pastas separadas** na placa de percepção (`/home/booster`). Nenhuma depende da outra.

| Pasta | Origem | Situação |
|---|---|---|
| `~/robocup_public` | Fork da RoboFEI da demo da Booster, adaptado pelo time à ZED 2i e ao campo da UFPel | ✅ **A única que já jogou neste robô.** Não alterar |
| `~/robocup-26` | Este fork (`PinguimBots/robocup-26`), ramo `sandbox/support_2026_game_controller` | Demo oficial, sem alterações, ainda não compilada |
| `~/NOVO_PinguimBots_robocup26_fw17` *(planejada)* | Este fork, ramo `sandbox/support_firmware_1.7+` | Versão oficial mais nova (28/10/2026), feita para firmware ≥ 1.7. Será testada separadamente |

## Por que só o `robocup_public` funcionou

A demo oficial supõe **K1 + RealSense**. O `robocup_public` do robô tem adaptações locais:
- câmera `type: zed` e tópicos `/zed/zed_node/...`;
- calibração da ZED (intrínsecos 640×360 e hand-eye feito pelo time);
- dimensões do campo da UFPel (4,38 × 3,79 m) em `FD_KIDSIZE`;
- IPs da rede do laboratório.

Ele usa **APIs internas** da Booster para modo de jogo e chutes (`kEnableRobocupWalkMode`, `kRLKickBall`,
`kRLFancyKickBall`…). O firmware 1.8 contém os modelos de chute do T1 (`T1_vmp_fancy_kick`, `forward_kick`,
`side_kick`) e o modo `RobocupLocomotion`, mas a aceitação dessas APIs **ainda precisa ser testada**.

## Ramos deste fork

| Ramo | Data | Observação |
|---|---|---|
| `main` | 26/06/2026 | Ramo principal da Booster + esta documentação do time |
| `sandbox/support_2026_game_controller` | 09/09/2026 | GameController 2026 (HSL v7.0), detector de apito |
| `sandbox/support_firmware_1.7+` | 28/10/2026 | Reescrita para firmware ≥ 1.7: novo estimador de bola, localização, protocolo v20 do GameController; remove apito e simulação |

## Ajustes indicados pelo README da Booster para o T1

- `robot.robot_height: 1.12`
- `robot.odom_factor: 1.2`
- `RLVisionKick.enableAutoVisualKick: false`
- tópicos de imagem/profundidade/camera_info da câmera usada
