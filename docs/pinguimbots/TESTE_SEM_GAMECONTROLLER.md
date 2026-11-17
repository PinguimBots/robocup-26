# Teste sem GameController: firmware 1.8 + `robocup_public`

**Objetivo:** confirmar que o código que já jogou funciona com o firmware novo, **sem chutar**.
**Código:** `~/robocup_public` (não alterado; já compilado).

## Nunca usar neste teste
`RB` (chuta) · `LB` (persegue sem parar) · `LT + Y` (chute do firmware) · `A/B/X/Y` sozinhos (gestos)

**Emergência:** `LT + Back` (DAMP), **segurando o robô**.

## Etapas

**Perto do PC, com o cabo**

1. Robô apoiado → **PREP**: `LT + Start` (ou botão **P**).
2. No chão, firme e segurado → **WALK**: `RT + A`.
3. No terminal do robô:
   ```bash
   ssh booster@192.168.10.102
   ~/Demo_PinguimBots/scripts/stop.sh; cd ~/robocup_public && ./scripts/start.sh
   ```
   O programa inicia em automático, mas fica **parado** (sem GameController, não há estado de jogo).
4. Espere ~20 s e **desconecte o cabo**.

**No campo, só com o controle**

| Etapa | Botões | Quem controla |
|---|---|---|
| 5. Andar até o campo | Analógicos | **Você** |
| 6. Localizar | `LT + A` | Só a **cabeça** (varre o campo e segue a bola) |
| 7. Mover a bola devagar com o pé | — | A cabeça deve acompanhar |
| 8. Cancelar | `LT + X` | Robô para |

**Ao terminar**

9. Ande até perto do PC → **PREP** (`LT + Start`) → **DAMP** (`LT + Back`), apoiado.
10. Cabo conectado:
    ```bash
    cd ~/robocup_public && ./scripts/stop.sh
    grep -iE "error|fail" brain.log | tail
    ```

## O que anotar
- O programa iniciou sem erro?
- A cabeça achou e seguiu a bola?
- O robô andou bem com os analógicos com o programa rodando?
