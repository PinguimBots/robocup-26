# Firmware 1.8.1.2 — problemas encontrados e correções

Atualização feita em 07/11/2026 nas duas placas. Todas as correções abaixo foram feitas
**criando arquivos novos ou alterando uma única configuração**, sempre com forma de desfazer.

---

## 1. Câmera ZED não publicava imagem

**Sintoma:** nenhum tópico de imagem; o log da câmera dizia `camera=RealsenseD455`.

**Causa:** o firmware escolhe o driver da câmera lendo a linha `Camera:` de
`/opt/booster/perception_info.yaml`. Esse arquivo não existe após a atualização, então o padrão
(RealSense) é usado — e não há RealSense no nosso robô.

**Correção (placa de percepção):**
```bash
printf '# ARQUIVO NOVO criado pela PinguimBots\nCamera: Zed2i\n' | sudo tee /opt/booster/perception_info_NOVO_PinguimBots.yaml
sudo ln -s /opt/booster/perception_info_NOVO_PinguimBots.yaml /opt/booster/perception_info.yaml
sudo reboot
```
**Verificação:** o log `ros.camera` mostra `camera=Zed2i` e `ros2 topic hz /zed/zed_node/left/image_rect_color` ≈ 15 Hz.

**Desfazer:** `sudo rm /opt/booster/perception_info.yaml /opt/booster/perception_info_NOVO_PinguimBots.yaml && sudo reboot`

> Observação: o mesmo arquivo pode ter um campo `Audio:`; o único valor existente (`BoosterAEC`) exige a
> placa de microfones NationalChip — **não usar** neste robô (o serviço de áudio se recusa a iniciar).

---

## 2. Resolução da ZED errada (336×188 em vez de 640×360)

**Sintoma:** imagem pequena; a calibração da visão (feita em 640×360) deixa de valer.

**Causa:** a atualização reinstalou o driver da ZED (`~/ThirdParty/zed-ros`) e apagou o ajuste do time
(`grab_resolution`). O padrão é `VGA ÷ 2` = 336×188; o time usava `HD720 ÷ 2` = 640×360.

**Correção (placa de percepção, sem sudo):**
```bash
F=~/ThirdParty/zed-ros/install/zed_wrapper/share/zed_wrapper/config/zed2i.yaml
cp -p "$F" "$F.ORIGINAL_antes_PinguimBots"
sed -i "s/grab_resolution: 'VGA'/grab_resolution: 'HD720'/" "$F"
sudo reboot
```
**Verificação:** `ros2 topic echo --once --field width /zed/zed_node/left/camera_info` → `640`.

**Desfazer:** `cp -p "$F.ORIGINAL_antes_PinguimBots" "$F" && sudo reboot`

---

## 3. Serviço `ros.booster.vision_detection` com erro no app

**Sintoma:** o app acusa erro; log com `ImportError: libcudnn.so.9`.

**Causa:** um `apt upgrade` instalou e depois removeu à força o pacote errado `nvidia-cudnn` (8.2, do Ubuntu).
A remoção apagou os arquivos do cuDNN 9 correto do JetPack, que continuava registrado como instalado.

**Correção (placa de percepção):** reinstalar a **mesma versão** do repositório oficial da NVIDIA para Jetson:
```bash
wget https://repo.download.nvidia.com/jetson/common/pool/main/c/cudnn/libcudnn9-cuda-12_9.3.0.75-1_arm64.deb
wget https://repo.download.nvidia.com/jetson/common/pool/main/c/cudnn/libcudnn9-dev-cuda-12_9.3.0.75-1_arm64.deb
sudo dpkg -i libcudnn9-cuda-12_9.3.0.75-1_arm64.deb libcudnn9-dev-cuda-12_9.3.0.75-1_arm64.deb && sudo ldconfig
```
Depois, **reiniciar o robô inteiro** (a placa de movimento desiste do serviço após 3 falhas e só tenta de novo no boot).

**Verificação:** `dpkg -V libcudnn9-cuda-12` não lista nada; log do serviço mostra `Vision Server Ready.`

⚠️ **Nunca instalar o pacote `nvidia-cudnn`** neste robô.

---

## 4. Agentes do app sem botões ("não fazem nada")

**Sintoma:** os agentes abrem mas o painel fica vazio; log com
`Failed to get robot info, fallback to unknown` e `Built components for robot_model=unknown ... walk=0`.

**Causa:** o instalador do firmware criou `/opt/booster/tmp` com permissão `700` (só root). Os agentes rodam
como usuário `booster` e não conseguem ler `/opt/booster/tmp/robot_info_tmp.txt`; a alternativa (perguntar à
placa de movimento) é tentada uma vez só no boot, antes de ela estar pronta. Sem saber o modelo, nenhum botão é criado.

**Correção (placa de percepção):**
```bash
sudo chmod 755 /opt/booster/tmp
```
Reiniciar o robô. **Verificação:** log do agente `Robot model from env: Booster T1`.

**Desfazer:** `sudo chmod 700 /opt/booster/tmp`

> No firmware 1.8 os movimentos de acenar, aperto de mão, danças e gestos **continuam no firmware**, mas não
> há botões para eles nos agentes do T1. Podem ser chamados pelo SDK Python (`robot.do_action("hand_wave")` etc.).

---

## 5. Voz do agente não saía no alto-falante

**Causa:** a saída padrão do áudio é a da Jetson (sem alto-falante); o alto-falante está no adaptador USB.

**Correção (placa de percepção, sem sudo):**
```bash
pactl set-default-sink alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo
```
**Desfazer:** `pactl set-default-sink alsa_output.platform-sound.analog-stereo`

> Sem microfone, o agente de conversa não consegue ouvir (ver [ROBO_T1_ZED.md](ROBO_T1_ZED.md)).
