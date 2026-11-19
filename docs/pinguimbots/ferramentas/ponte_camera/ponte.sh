#!/bin/bash
# ARQUIVO NOVO - PinguimBots: controla a ponte da camera ZED -> /boostercamera/head
# A ponte inicia sozinha no boot (servico de usuario pinguimbots-ponte-camera, ~30 s apos o login).
# Uso: ~/PinguimBots_ponte_camera/ponte.sh iniciar | parar | estado | sem-boot | com-boot
S=pinguimbots-ponte-camera
case "$1" in
  iniciar)  systemctl --user start "$S" && echo "Ponte iniciando (imagens em ~30 s).";;
  parar)    systemctl --user stop "$S" && echo "Ponte parada (volta no proximo boot).";;
  estado)   echo "ativa: $(systemctl --user is-active $S) | inicia no boot: $(systemctl --user is-enabled $S)"
            journalctl --user -u "$S" -n 2 --no-pager -o cat 2>/dev/null;;
  sem-boot) systemctl --user disable --now "$S" && echo "Ponte parada e NAO inicia mais no boot.";;
  com-boot) systemctl --user enable --now "$S" && echo "Ponte volta a iniciar no boot.";;
  *) echo "Uso: $0 iniciar | parar | estado | sem-boot | com-boot";;
esac
