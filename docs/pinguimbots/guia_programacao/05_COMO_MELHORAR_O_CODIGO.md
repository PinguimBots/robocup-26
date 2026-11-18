# 5. Como escrever código novo e melhorar a demo

## 5.1 O ciclo de trabalho

```
editar (ramo Git próprio) → compilar só o pacote mudado → testar parado → testar no campo → commit
```

1. **Trabalhe na `~/Demo_PinguimBots`**, nunca no `~/robocup_public`. Ela já está no ramo local `pinguimbots/fw17-t1`.
   Para cada ideia, crie um ramo:
   ```bash
   cd ~/Demo_PinguimBots && git checkout -b pinguimbots/minha-ideia
   ```
2. **Editar:** pelo terminal (`nano`, `vim`) ou, mais confortável, pelo **Booster Studio/VS Code com "Remote - SSH"**
   (já vem no Studio), abrindo a pasta `/home/booster/Demo_PinguimBots` direto no robô.
3. **Compilar só o que mudou** (minutos em vez de 15–20):
   ```bash
   ./scripts/build.sh --packages-select brain     # ou vision
   ```
4. **Testar:** `./PinguimBots/start_PinguimBots.sh tudo` com o robô **segurado**; ler `brain.log`.
5. **Voltar atrás** se der errado: `git checkout -- <arquivo>` ou `git checkout pinguimbots/fw17-t1`, e recompilar.

## 5.2 Exercício: seu primeiro nó (só registra, não move o robô)

Objetivo: entender o caminho **XML → C++ → registro → compilação** com zero risco.

**1. Declarar** em `src/brain/include/brain_tree.h` (perto de `SetVelocity`):
```cpp
// PinguimBots: registra no log a posição da bola, no máximo 1 vez por segundo
class LogBola : public SyncActionNode
{
public:
    LogBola(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}
    NodeStatus tick() override;
    static PortsList providedPorts() { return {}; }
private:
    Brain *brain;
    rclcpp::Time _ultimo;   // mesmo padrão dos outros nós (ex.: _timeLastCmd)
};
```

**2. Implementar** em `src/brain/src/brain_tree.cpp`:
```cpp
NodeStatus LogBola::tick()
{
    if (brain->msecsSince(_ultimo) < 1000) return NodeStatus::SUCCESS;   // no máximo 1x por segundo
    _ultimo = brain->get_clock()->now();
    auto &b = brain->data->ball;
    std::cout << "[LogBola] vista=" << brain->data->ballDetected
              << " dist=" << b.range << " m  angulo=" << b.yawToRobot << " rad"
              << "  campo=(" << b.posToField.x << ", " << b.posToField.y << ")" << std::endl;
    return NodeStatus::SUCCESS;
}
```

**3. Registrar** em `BrainTree::init` (mesmo arquivo, junto dos outros `REGISTER_BUILDER`):
```cpp
REGISTER_BUILDER(LogBola)
```

**4. Usar** no XML: em `behavior_trees/game.xml`, como **primeiro filho** do bloco do estado SET:
```xml
<ReactiveSequence _while="gc_game_state=='SET'" name="...">
    <LogBola />
    ...
```

**5. Compilar e testar:** `./scripts/build.sh --packages-select brain`, iniciar, colocar o GameController em SET,
e ver no `brain.log` as linhas `[LogBola]`. Mova a bola e confira se a distância muda coerentemente.

> Se a compilação reclamar de algum nome (`ballDetected`, `msecsSince`…), procure como outros nós usam a mesma coisa
> com `grep -n "msecsSince" src/brain_tree.cpp` e copie o padrão.

## 5.3 Ideias de melhoria (onde mexer)

Todas conferidas no código do robô. As de chute ficam para quando decidirem testar chutes.

### Para começar (pequenas, efeito claro)
| Ideia | Onde | Como está hoje |
|---|---|---|
| **Velocidade mínima configurável** | `src/brain/src/robot_client.cpp`, linha ~243 (`minx = 0.3, miny = 0.3, mintheta = 0.3`) | Fixa em 0,3: o robô nunca anda mais devagar que isso. Transformar em parâmetro do `config_local.yaml` (declarar em `Brain::Brain()`) |
| **Goleiro no campo pequeno** | `brain.cpp:927` e `brain_tree.cpp:1885` (`-circleRadius - 2.0`) | Limite pensado para campo grande: no campo da UFPel (meio comprimento 2,19 m) dá −2,375 m, **atrás da linha do gol**, e o goleiro nunca sai para a bola. Usar uma medida relativa (ex.: `-length/2 + penaltyAreaLength`) |
| **Decisão `safe_shoot` sem nó** | `StrikerDecide::tick` (`brain_tree.cpp`) e `subtrees/subtree_striker_play.xml` | A decisão existe no C++, mas nenhum nó do XML a executa. Hoje nunca acontece; se um parâmetro mudar, o atacante fica parado. Tratar no XML ou trocar por `kick` |
| **Mais logs úteis** | qualquer nó; `brain->log->log("caminho", rerun::TextLog(msg))` | Ver decisões e posições no Rerun |

### Intermediárias
| Ideia | Onde | Explicação |
|---|---|---|
| **Posição da bola com a pose da hora da foto** | `getGameObjects` (`brain.cpp:2238`) | Hoje usa a pose **atual** do robô para converter detecções; com o robô girando, a bola "escorrega". Já existe `lookupRobotPoseToOdomAt` (`brain.cpp:1649`) para buscar a pose do instante da imagem |
| **Estimador da bola mais realista** | `src/ball_state_estimator.cpp`, alimentado em `updateBallPrediction` (`brain.cpp:1159`) | O ruído da medida é fixo (0,05 m); na prática, cresce com a distância. Isso faz a bola "parecer rolar" |
| **Gols no mapa da localização** | `calcFieldMarkers` (`locator.cpp:8`) | Só usa cruzamentos e marca do pênalti; gols ajudariam, principalmente com a câmera estreita e o campo pequeno |
| **Entrar no modo futebol sozinho** | `game.xml:14` (`<RunOnce><RobocupWalk /></RunOnce>` comentado) | Hoje só com `LT + A`. Descomentar ativa ao iniciar (testar com o robô segurado) |
| **Quem vai na bola** | `updateCostToKick` (`brain.cpp:744`), `handleCooperation` (`brain.cpp:463`) | Custo com pesos "na mão" (distância, ângulo, idade da bola, queda). Dá para pesar melhor, por exemplo considerando o tempo até a bola |

### Avançadas
| Ideia | Onde | Explicação |
|---|---|---|
| **Avaliar chutes e ameaças** | `Brain::kickValue` (`brain.cpp:4443`), `Brain::threatLevel` (`brain.cpp:4459`) | O próprio código diz "implementação básica": a ameaça só conta adversários a 1 m ou 2,5 m |
| **Localização contínua** | `Locator::locateRobot` (`locator.cpp`) | Hoje é uma busca pontual; um filtro de partículas ou EKF rodando o tempo todo com a odometria seria mais estável |
| **Chute de verdade com segurança** (quando decidirem) | nó `Kick` (`brain_tree.cpp`) | Hoje anda contra a bola; o chute do firmware (`kickBall`) existe e não é usado |
| **Testes automáticos** | `src/brain/test/test_kick_geometry.cpp` | Já há um teste de exemplo; dá para testar funções de decisão sem o robô |

## 5.4 Dicas para não se perder

- **Comece lendo um nó pequeno** (`SetVelocity`, `CamTrackBall`) e siga as chamadas com `grep -n`.
- **Uma mudança por vez**, um ramo por ideia, e anote no commit o que testou.
- **Meça antes e depois**: use os logs (`[LogBola]`, Rerun) para mostrar ao seu orientador que melhorou.
- **Documente** no `docs/pinguimbots/` do GitHub o que funcionou.
