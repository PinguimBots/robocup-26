# PinguimBots — Booster T1 (variante com ZED 2i)

Documentação do time **PinguimBots** sobre o nosso Booster T1, mantida neste fork da
[demo RoboCup oficial da Booster](https://github.com/BoosterRobotics/robocup_demo).

> Os ramos `main` e `sandbox/*` deste fork vieram da Booster. Esta pasta (`docs/pinguimbots/`)
> é o único conteúdo do time; o código da Booster não foi alterado.

## Índice

| Documento | Conteúdo |
|---|---|
| [ROBO_T1_ZED.md](ROBO_T1_ZED.md) | Como é o nosso robô (hardware, placas, câmera) e o que difere do T1 padrão |
| [FIRMWARE_1.8_CORRECOES.md](FIRMWARE_1.8_CORRECOES.md) | Problemas encontrados após atualizar para o firmware 1.8.1.2 e como foram corrigidos (com reversão) |
| [VERSOES_DO_CODIGO.md](VERSOES_DO_CODIGO.md) | Mapa das versões de código de futebol (qual funciona, qual está em teste) |
| [OPERACAO.md](OPERACAO.md) | Como iniciar/parar cada versão sem conflito, sequência no campo, segurança |

## Situação atual (17/11/2026)

| Item | Estado |
|---|---|
| Firmware | 1.8.1.2 (última versão que a Booster lançará para o T1) |
| Câmera ZED 2i | ✅ funcionando (640×360, ~15 Hz) |
| Visão do `robocup_public` (versão que já jogou) | ✅ detecta bola/pessoas com o firmware 1.8 |
| Distância da bola (calibração) | ⏳ falta medir com a bola no chão |
| Movimento/chutes com firmware 1.8 | ⏳ falta testar |
| Demo oficial `sandbox/support_firmware_1.7+` | ⏳ planejada para teste em pasta separada |
