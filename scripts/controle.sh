
#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Uso: $0 [a|b|x]"
    exit 1
fi

case "$1" in
    a|b|x)

        echo "Vou mandar LT + $1"

        # Faz 3 vezes
        ros2 topic pub --times 3 \
             /remote_controller_state \
             booster_interface/msg/RemoteControllerState \
             "{lt: true, $1: true}"

        # Fica mandando indefinidamente
        #ros2 topic pub -r 10 \
        #    /remote_controller_state \
        #    booster_interface/msg/RemoteControllerState \
        #    "{lt: true, $1: true}"
        ;;
    *)
        echo "Botão inválido: $1"
        echo "Opções disponíveis: a, b ou x"
        exit 1
        ;;
esac
