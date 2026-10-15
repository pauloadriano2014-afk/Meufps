# Construir a Missão 1 automaticamente

O ator **Mission 01 Builder** monta a Missão 1 ("Estrada") inteira na fase onde ele for colocado.
Ele cria a estrada, os carros, a casa em ruínas, o posto, as armas, os inimigos, os checkpoints e todos os objetivos.
O que ele cria são atores normais: depois dá para mover ou apagar qualquer coisa à mão.

## Passo a passo

1. Compile o projeto e abra o editor.
2. Abra `Lvl_Shooter` e vá em **Arquivo → Salvar Nível Atual Como**. Salve uma cópia com o nome **`Lvl_Mission01`** (o nome precisa conter `Mission01`).
   Isso é obrigatório: o construtor **apaga** os blocos, inimigos e armas que já estão na fase e só roda em um nível com esse nome, para não destruir `Lvl_Shooter`.
3. Na fase `Lvl_Mission01`, coloque um ator **Mission 01 Builder** (menu inserir, busque `Mission 01 Builder`).
4. Com ele selecionado, no painel **Detalhes**, procure a categoria **Mission 01** e clique em **Build Mission**.
5. Espere alguns segundos. A fase aparece montada. Se algo der errado, abra a **Log de Saída** e filtre por `LogMeuFPS`.
6. Em **Compilar → Construir Caminhos** (menu superior), para os inimigos poderem andar pela fase. Se não houver um `NavMeshBoundsVolume` na fase, adicione um (Colocar Atores) e estique sobre toda a estrada.
7. Salve a fase (**Ctrl+S**) e aperte **Play**.

Para refazer do zero depois de mudar algo no código, clique em **Build Mission** de novo. **Clear Mission** apaga só o que o construtor criou.

## O que a fase tem

Do início (X = 0) ao fim (X = 26000), em 260 metros:

| Zona | O que tem |
|---|---|
| Abrigo | O portão e a pistola inicial. O objetivo "Saia do Abrigo" completa ao passar pelo portão |
| Estrada | 14 carros e 8 barreiras para cobertura, 4 andarilhos, a espingarda (rifle) no chão, munição de pistola |
| Emboscada | Um ônibus e um caminhão atravessados, com 2 corredores escondidos. Checkpoint depois |
| Casa em ruínas | Munição de rifle dentro, 2 andarilhos por perto |
| Posto de gasolina | 5 saqueadores atrás dos muros baixos e dentro da loja. Checkpoint na entrada |
| Bomba | Você segura **E** por 12 segundos para encher o galão. O barulho acorda a horda |
| Horda | 8 andarilhos, 3 corredores e 1 brutamontes ao redor do posto |
| Saída | A estrada para leste |

## Objetivos, na ordem

1. Saia do Abrigo
2. Abra caminho pela estrada (4 infectados)
3. Chegue ao posto de gasolina
4. Limpe o posto (4 inimigos)
5. Encha o galão (segurar E, 12 s, faz barulho)
6. Resista à horda (45 s)
7. Saia pela estrada a leste

No final aparece "MISSÃO CUMPRIDA" com o texto de encerramento.

## Teclas

| Tecla | Ação |
|---|---|
| WASD e mouse | Mover e olhar |
| Botão esquerdo | Atirar |
| Botão direito (segurar) | Mirar |
| R | Recarregar |
| Shift (segurar) | Correr |
| Ctrl (segurar) | Agachar |
| E (segurar) | Interagir |

## Infectados

| Tipo | Vida | Velocidade | Dano | Comportamento |
|---|---|---|---|---|
| Andarilho | 80 | lento | 15 | Vagueia perto de onde nasceu, até ouvir ou ver você |
| Corredor | 40 | muito rápido | 12 | Fica parado até notar você, e então corre direto |
| Brutamontes | 450 | médio | 45 | Grande e muito resistente. Cada golpe dói |

- **Eles ouvem os tiros** (até 35 m) e o barulho da bomba (até 45 m). Atirar atrai a horda.
- **Enxergam** a até 18 m, num cone na frente deles.
- Se você sair de vista por 6 segundos, eles vão ao último lugar onde viram você e depois desistem.
- O corpo é o boneco do template (Quinn). Quando você escolher um personagem infectado na Fab, é só trocar a malha e a animação.

## Ajustes sem mexer em código

Selecione qualquer ator da fase e use o painel **Detalhes**:

| Para ajustar | Onde |
|---|---|
| Vida, velocidade e dano de um infectado | Infectado → categoria **Infected → Stats** |
| Tempo de segurar E, alcance do barulho | Objetivo "Encha o galão" → categoria **Interact** |
| Duração da resistência | Objetivo "Resista à horda" → **Duration** |
| Número de mortes necessárias | Os objetivos de eliminar → **Kills Required** |
| Tamanho do destino | Os objetivos de chegar → a caixa de gatilho (escale o ator) |
| Alcance de quem pode interagir | O objetivo de interagir → o componente **Range** (raio da esfera) |
| Textos da missão | O **MissionManager** → categoria **Mission** |

## Limitações desta versão

- Os visuais são blocos cinzas e bonecos do template. Cenário, personagens e armas definitivos vêm de assets (veja `docs/ASSETS.md`).
- Os infectados não têm animação de ataque nem sons. O código já avisa (eventos **On Attack** e **On Death**) para você ligar sons e animações depois, no Blueprint.
- Não há som de tiro, de recarga nem trilha.
- Os saqueadores são os NPCs do template e usam o comportamento deles.
