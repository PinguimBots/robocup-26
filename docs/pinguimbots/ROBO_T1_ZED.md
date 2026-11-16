# O nosso T1 — o que é diferente do T1 padrão

## Duas placas de computador

| Placa | IP (rede interna) | Usuário | Hardware | Função |
|---|---|---|---|---|
| Movimento | `192.168.10.101` | `master` | Intel Core i7-1370PE, 8 GB RAM | Motores, marcha, IMU, odometria. **É a "mestre"**: controla os serviços das duas placas |
| Percepção | `192.168.10.102` | `booster` | NVIDIA Jetson AGX Orin, 32 GB (memória unificada CPU/GPU), JetPack 6.2, TensorRT 10.3 | Câmera, visão, voz, agentes do app |

Ambas: Ubuntu 22.04 + ROS 2 Humble. Comunicação entre elas por DDS (FastDDS).

## Diferenças em relação ao T1 padrão

| Item | T1 padrão (o que o firmware e a demo esperam) | O nosso T1 |
|---|---|---|
| Cabeça | Padrão | Diferente (mais estreita na altura, mais larga nos lados) |
| Câmera | Intel RealSense D455 | **STEREOLABS ZED 2i** (USB) |
| Tópicos da câmera | `/boostercamera/head/*` | `/zed/zed_node/*` |
| Microfone | Placa de microfones "NationalChip" (modo `BoosterAEC`) | **Nenhum** (confirmado: entrada da Jetson devolve zeros; entradas da placa de movimento vazias) |
| Alto-falante | — | Adaptador USB C-Media (somente saída) na placa de percepção |

## Consequências práticas

- A demo oficial da Booster é escrita para **K1 + RealSense**. Para o T1 com ZED é preciso ajustar
  tópicos, calibração e parâmetros do T1 (`robot_height 1.12`, `odom_factor 1.2`, chute visual automático desligado).
- O que o app chama de "câmera" e de detecção de pessoas usa `/boostercamera/head/rgb`, que **não recebe imagem
  com a ZED** (o firmware só remapeia os tópicos da RealSense). Não afeta a visão de futebol, que lê `/zed/...`.
- Sem microfone, o agente de conversa ("Hi Chat") fala mas não ouve. Para ouvir, é preciso um microfone USB
  na placa de percepção.

## Joint layout (23 DoF)

```
0 HeadYaw  1 HeadPitch
2-5  braço esquerdo (ShoulderPitch, ShoulderRoll, ElbowPitch, ElbowYaw)
6-9  braço direito
10   Waist
11-16 perna esquerda (HipPitch, HipRoll, HipYaw, KneePitch, AnkleUp, AnkleDown)
17-22 perna direita
```
