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
| [CONTROLE_FIRMWARE_1.8.md](CONTROLE_FIRMWARE_1.8.md) | Mapa do controle no firmware 1.8 (oficial) + botões do cérebro do robocup e conflitos |
| [TESTE_SEM_GAMECONTROLLER.md](TESTE_SEM_GAMECONTROLLER.md) | Roteiro: firmware 1.8 + `robocup_public`, só pelo controle, sem chute |
| [TESTE_COM_GAMECONTROLLER.md](TESTE_COM_GAMECONTROLLER.md) | Roteiro: firmware 1.8 + demo 1.7+ com o GameController novo (até SET) |
| [guia_programacao/](guia_programacao/00_COMECE_AQUI.md) | **Guia de programação do T1** (6 partes): firmware, SDK, código de futebol por dentro, Booster Studio/agentes e como melhorar o código |
| [OPERACAO.md](OPERACAO.md) | Como iniciar/parar cada versão sem conflito, sequência no campo, segurança |

## Situação atual (17/11/2026)

| Item | Estado |
|---|---|
| Firmware | 1.8.1.2 (última versão que a Booster lançará para o T1) |
| Câmera ZED 2i | ✅ funcionando (640×360, ~15 Hz) |
| Visão do `robocup_public` (versão que já jogou) | ✅ detecta bola/pessoas com o firmware 1.8 |
| Distância da bola (calibração) | ✅ erro de ~10 cm (17/11/2026, robô em pé, cabeça inclinada como no jogo) |
| Movimento/chutes com firmware 1.8 | ⏳ falta testar |
| Botões F/P das costas | ✅ corrigido em 17/11/2026 (ver FIRMWARE_1.8_CORRECOES.md, item 6) |
| PREP pelo controle | ✅ `LT + START` (o `RT + Y` não existe mais no firmware 1.8) |
| Demo oficial `sandbox/support_firmware_1.7+` | ⏳ planejada para teste em pasta separada |
