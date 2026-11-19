# Ponte da câmera ZED para o app e o Booster Studio

## O problema
As ferramentas da Booster (câmera do app, Booster Vision do Studio, `get_image()` do SDK, exemplos do Studio)
leem a câmera sempre em `/boostercamera/head/rgb`. O firmware só publica ali quando a câmera é a **RealSense**.
O nosso T1 usa a **ZED 2i**, que publica em `/zed/zed_node/...`. Resultado: robô conectado, tudo "ok", mas
**nenhuma imagem** no app ou no Studio (`/boostercamera/head/rgb` com 0 publicadores).

## A solução
Um programa pequeno (`ponte_camera_zed.py`, Python/ROS 2) que copia a ZED para os tópicos da Booster:

| Origem (ZED) | Destino (Booster) | Conversão |
|---|---|---|
| `/zed/zed_node/left/image_rect_color` | `/boostercamera/head/rgb` | `bgra8` → `bgr8` |
| `/zed/zed_node/left/camera_info` | `/boostercamera/head/rgb/camera_info` | cópia |
| `/zed/zed_node/depth/depth_registered` | `/boostercamera/head/depth` | `32FC1` (m) → `16UC1` (mm), como a RealSense |
| `/zed/zed_node/depth/camera_info` | `/boostercamera/head/depth/camera_info` | cópia |

Medido em 19/11/2026: imagem a ~15 Hz, pouca CPU. O serviço de vídeo da Booster passou a gerar 10 JPEG/s para o app.

> Dica de desempenho: no Python do ROS 2, atribuir `bytes` a `msg.data` é lento (valida byte a byte).
> Usar `array.array('B', ...)` levou a ponte de 6 Hz/100% de CPU para 15 Hz com CPU baixa.

**Não afeta a demo de futebol:** a demo lê a ZED direto (redirecionamento próprio e `config_local.yaml`).

## Arquivos ([ferramentas/ponte_camera/](ferramentas/ponte_camera/))
| Arquivo | Onde fica no robô (`.102`) |
|---|---|
| `ponte_camera_zed.py` | `~/PinguimBots_ponte_camera/` |
| `ponte.sh` | `~/PinguimBots_ponte_camera/` |
| `pinguimbots-ponte-camera.service` | `~/.config/systemd/user/` (serviço do usuário `booster`, sem sudo) |

## Instalação (na `.102`)
```bash
mkdir -p ~/PinguimBots_ponte_camera ~/.config/systemd/user
# copiar ponte_camera_zed.py e ponte.sh para ~/PinguimBots_ponte_camera/ e o .service para ~/.config/systemd/user/
chmod +x ~/PinguimBots_ponte_camera/ponte.sh
systemctl --user daemon-reload && systemctl --user enable --now pinguimbots-ponte-camera
```
O robô faz login automático do usuário `booster`, então o serviço inicia sozinho ~30 s após ligar.

## Uso
| Comando | O que faz |
|---|---|
| `~/PinguimBots_ponte_camera/ponte.sh estado` | Mostra se está rodando e se inicia no boot |
| `~/PinguimBots_ponte_camera/ponte.sh parar` / `iniciar` | Para / inicia agora |
| `~/PinguimBots_ponte_camera/ponte.sh sem-boot` / `com-boot` | Desliga / religa o início automático |

## Desfazer
```bash
~/PinguimBots_ponte_camera/ponte.sh sem-boot
rm -r ~/PinguimBots_ponte_camera ~/.config/systemd/user/pinguimbots-ponte-camera.service
systemctl --user daemon-reload
```

## Observação: vídeo lento no app
Dentro do robô o vídeo sai normal (10 JPEG/s, ~1,4 Mbit/s). A lentidão observada no app está no envio ao
celular: o Wi-Fi do robô estava fraco (−64 dBm, envio a 26 Mbit/s). Testar com o celular na mesma rede e perto
do roteador; para ver a câmera com fluidez, usar o PC pelo cabo (Lichtblick do Booster Studio).
