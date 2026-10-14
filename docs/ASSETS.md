# Lista de compras de assets (Fab)

O Claude não cria modelos 3D, animações nem sons. Tudo o que está aqui vem da **Fab** (dentro do Epic Games Launcher, aba Fab)
ou de outras fontes. Use os termos de busca como ponto de partida e **confira cada pacote antes de baixar**.

## Como escolher no Fab

- Filtre por **Unreal Engine** e confira que o pacote diz suportar a versão **5.x** (idealmente 5.6 ou mais nova). Pacotes antigos funcionam, mas podem pedir ajustes.
- **Comece pelos gratuitos.** Há muita coisa útil de graça, incluindo a biblioteca do Quixel Megascans.
- Leia a **licença**: quase sempre dá para usar em jogo comercial, mas confira.
- **Peso em disco:** 16 GB de RAM e o projeto no SSD aguentam bem, mas cuidado com pacotes de 20 GB.
- Prefira pacotes **modulares** (peças que se encaixam) para montar ambientes.

## O que buscar

| Categoria | Termos de busca | Para quê | Prioridade |
|---|---|---|---|
| **Infectados** | "zombie", "infected", "undead character" | Corredor, andarilho, brutamontes. Procure com **animações** (andar, correr, atacar, morrer) | Alta |
| **Saqueadores** | "survivor", "bandit", "raider", "post apocalyptic character" | Inimigos humanos | Alta |
| **Armas realistas** | "weapon pack", "rifle", "pistol", "shotgun", "sniper" | Substituir as armas do template | Alta |
| **Ambiente de estrada** | "abandoned road", "wrecked cars", "highway", "post apocalyptic environment" | Missão 1 | Alta |
| **Posto de gasolina** | "gas station", "petrol station" | Missão 1 | Média |
| **Hospital** | "abandoned hospital", "medical", "hospital interior" | Missão 2 | Média |
| **Depósito e acampamento** | "warehouse", "industrial", "camp", "barricade" | Missões 3 e 4 | Média |
| **Ponte** | "bridge", "concrete barrier" | Missão 5 | Média |
| **Vegetação e natureza** | "overgrown", "foliage", "grass", "trees" | Dar o clima de abandono | Média |
| **Materiais e texturas** | "rust", "concrete", "decal", "dirt" | Realismo | Média |
| **Efeitos** | "muzzle flash", "blood", "impact", "smoke" | Tiro, acerto, ambientação | Média |
| **Sons** | "gunshot", "zombie sounds", "ambience post apocalyptic" | Tiros, criaturas, trilha | Alta |

## Sons e vozes

- **Fab e outras bibliotecas** têm pacotes de áudio de armas, passos e ambiente.
- **Vozes de briefing:** o Claude tem um conector com o ElevenLabs que gera voz a partir de texto. Isso consome créditos da sua conta. Podemos usá-lo para
  narrar os briefings das missões, se você quiser.
- **Música:** busque "post apocalyptic music" em bibliotecas de música livre e confira a licença.

## O que NÃO esperar de pacotes prontos

- Personagens de pacotes diferentes podem ter **esqueletos diferentes**. Animações de um não funcionam em outro sem **retargeting** (feito no editor, mas é trabalho).
- Armas de pacotes costumam trazer as próprias animações e sockets. Integrar com o código do template pede ajuste, que o Claude faz se você disser quais pacotes escolheu.

## Ordem sugerida

1. **Armas + sons de tiro**, para o combate ficar bom primeiro.
2. **Infectados com animação**, porque dependem de código novo do Claude.
3. **Ambiente da Missão 1** (estrada e posto).
4. Os demais, missão por missão.
