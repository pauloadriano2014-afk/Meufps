# Missão 1 — plano de montagem no editor (blockout)

"Blockout" é montar a fase só com formas simples (cubos e rampas) para testar o ritmo do jogo antes de gastar tempo com arte.
O projeto já traz os blocos em `Content/LevelPrototyping/Meshes` (SM_Cube, SM_Ramp, SM_Cylinder, SM_Plane, SM_ChamferCube).

## Como criar a fase

1. No editor: **Arquivo → Novo Nível**, escolha **Básico**. Salve como `Content/Maps/Lvl_Mission01` (crie a pasta `Maps`).
2. Abra `Lvl_Shooter`, selecione tudo o que quiser reaproveitar (Player Start, luz, spawner), copie (**Ctrl+C**) e cole na fase nova (**Ctrl+V**).
3. Em **Configurações do Mundo** da fase nova, em **Sobrescrever GameMode**, escolha o mesmo GameMode que `Lvl_Shooter` usa. Para saber qual é, abra `Lvl_Shooter` e olhe as Configurações do Mundo dela (provavelmente `BP_ShooterGameMode`, em `Variant_Shooter/Blueprints`). Sem isso você não tem jogador nem armas.
4. Para o jogador ter armas, copie também os pickups de arma de `Lvl_Shooter`.

## Layout sugerido (aproximado)

Tamanho total: cerca de **250 m de comprimento por 80 m de largura** (25000 × 8000 unidades do Unreal). Sentido: da esquerda para a direita.

| Zona | Distância do início | O que montar | Objetivo da missão |
|---|---|---|---|
| **A. Saída do Abrigo** | 0 a 20 m | Área aberta, Player Start, uma arma no chão | Ensina andar |
| **B. Estrada** | 20 a 100 m | Faixa larga com 6 a 8 cubos grandes como carros destruídos (cobertura) | Objetivo 0: Reach no fim da estrada |
| **C. Infectados** | 100 a 150 m | Dois grupos de 2 infectados entre os carros. Checkpoint no fim | Objetivo 1: Eliminar 4 |
| **D. Cerca do posto** | 150 a 180 m | Muro baixo com duas entradas, um atirador atrás do muro | Objetivo 2: Eliminar 4 saqueadores |
| **E. Posto** | 180 a 250 m | Construção com cubos, bombas (cilindros), cobertura em volta. Checkpoint na entrada | Objetivos 3 a 5 |

## Dicas de ritmo

- **Munição escassa:** coloque pouca munição no chão. O jogador deve precisar poupar balas e usar a pistola na zona C.
- **Cobertura a cada 10 a 15 m:** FPS de tiro instantâneo fica muito difícil em área aberta sem onde se esconder.
- **Dois caminhos** na zona D (esquerda e direita) para o jogador escolher como atacar.
- **Teste cada zona sozinha** antes de conectar tudo. Dê Play a partir de um Player Start dentro da zona.
- **Checkpoints:** um depois da zona C e um na entrada do posto, para a morte não custar 10 minutos.

## Mission Manager e objetivos desta fase

Veja `docs/MISSION_SETUP.md`. Para esta fase:

| Ator | Order | Configuração |
|---|---|---|
| Mission Manager | , | Title = "Estrada" e o briefing de `docs/MISSIONS.md` |
| Reach Location Objective | 0 | Description = "Siga a estrada até o posto" |
| Eliminate Objective | 1 | Kills Required = 4, Description = "Abra caminho" |
| Eliminate Objective | 2 | Kills Required = 4, Description = "Limpe o posto" |
| Reach Location Objective | 3 | Description = "Encha o galão" (provisório até existir o objetivo de interagir) |
| Survive Objective | 4 | Duration = 40, Description = "Aguente enquanto o tanque enche" |
| Reach Location Objective | 5 | Description = "Volte para a estrada" |
| Checkpoint | , | Um no fim da zona C e outro na entrada do posto |

Nota: hoje os infectados ainda não existem como tipo próprio. Para testar a fase, use os NPCs do template no lugar deles (os dois grupos contam
como inimigos do time 1). Quando as criaturas forem criadas, é só trocar.

Também nota: o objetivo "Eliminar" conta qualquer inimigo do time 1. Os objetivos 1 e 2 contam as mortes desde o momento em que ficam ativos,
então as mortes da zona C não adiantam as da zona D.
