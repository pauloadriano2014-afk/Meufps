# Como montar uma missão no editor

O código das missões está em `Source/MeuFPS/Variant_Shooter/Mission/`. No editor, uma missão é montada
arrastando atores para a fase. Não é preciso criar Blueprints.

## Peças

Todas aparecem no painel **Colocar Atores** (digite o nome na busca) ou em **Classes C++ → MeuFPS → Variant_Shooter → Mission**.

| Ator | Para que serve |
|---|---|
| **Mission Manager** | Coordena a missão. **Um por fase.** Tem o título, o briefing e o texto final |
| **Reach Location Objective** | Objetivo "chegar a um lugar". A caixa de gatilho é o destino |
| **Eliminate Objective** | Objetivo "eliminar N inimigos" |
| **Survive Objective** | Objetivo "sobreviver por X segundos" |
| **Checkpoint** | Salva o progresso. O jogador reaparece nele ao morrer |

## Receita mínima

1. Abra a fase (por exemplo `Lvl_Shooter`).
2. Arraste um **Mission Manager** para a fase. No painel Detalhes, preencha **Mission Title**, **Briefing** e **Completion Text**.
3. Arraste os objetivos que quiser. Em cada um, no painel Detalhes:
   - **Description**: o texto que aparece na tela.
   - **Order**: a ordem na missão. O menor número vem primeiro. Objetivos com o mesmo número ficam ativos ao mesmo tempo.
4. **Reach Location Objective**: mova e escale a caixa até o destino.
5. **Eliminate Objective**: defina **Kills Required**. Conta mortes de inimigos (time 1), colocados à mão ou vindos de um spawner.
6. **Checkpoint**: mova a caixa para o ponto e gire o ator para a seta apontar para onde o jogador deve olhar ao reaparecer.
7. Dê **Play**. O briefing aparece no centro da tela e os objetivos ativos ficam à esquerda, abaixo da barra de vida.

## Exemplo de mini-missão

| Ator | Order | Configuração |
|---|---|---|
| Eliminate Objective | 0 | Kills Required = 3, Description = "Elimine os saqueadores" |
| Checkpoint | , | Perto da área de combate |
| Reach Location Objective | 1 | Description = "Chegue à saída" |

Resultado: elimina 3 inimigos, depois o objetivo de chegar à saída é liberado. Ao terminar aparece "MISSÃO CUMPRIDA".

## Observações

- Quando a missão acaba, o jogo continua rodando. Carregar a próxima fase ou voltar ao menu fica para uma etapa futura.
- A morte do jogador não reinicia a missão: o progresso é mantido e o jogador volta ao último checkpoint (ou ao Player Start, se não passou por nenhum).
- Para criaturas e inimigos novos, a morte precisa chamar `IncrementTeamScore` no game mode para contar no objetivo de eliminar.
