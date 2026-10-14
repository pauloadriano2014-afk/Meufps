# MeuFPS — Documento de Design (rascunho 1)

> Rascunho vivo. Tudo que está marcado como **[PROPOSTA]** foi sugerido pelo Claude e ainda não foi aprovado.
> Decisões já tomadas pelo Paulo estão marcadas como **[DECIDIDO]**.

## 1. Resumo

- **Gênero:** FPS single player, visual realista, PC (Windows).
- **Engine:** Unreal Engine 5.8.3, código em C++.
- **Estrutura [DECIDIDO]:** campanha em missões, cada uma em uma fase separada.
- **Ambientação [DECIDIDO]:** pós-apocalíptico.
- **História [DECIDIDO]:** simples, contada por missão (briefing curto + objetivo + final), com uma trama leve ligando tudo.
- **Título provisório [PROPOSTA]:** MeuFPS (trocar depois).

## 2. Pilares de design [PROPOSTA]

1. **Escassez:** munição e recursos são limitados. Cada tiro importa. (Reserva de munição já implementada.)
2. **Combate direto e legível:** tiro instantâneo, recuo controlável, mira (ADS), dano por parte do corpo. O jogador morre rápido e os inimigos também.
3. **Missões curtas e densas:** 15 a 25 minutos cada, com objetivo claro e um final marcante.
4. **História leve:** poucas linhas de texto/áudio por missão, sem cenas longas.

## 3. Premissa e história [PROPOSTA — para discutir]

- **Mundo:** anos depois de um colapso. Cidades abandonadas, estradas bloqueadas, sobreviventes em pequenos assentamentos.
- **Protagonista:** um batedor de um assentamento que sai em missões para buscar suprimentos e informações.
- **Antagonistas:** um grupo de saqueadores organizados que controla as rotas e os recursos da região.
- **Arco geral (5 missões):**
  1. **Estrada:** o batedor deixa o assentamento e abre caminho até um posto de combustível tomado por saqueadores.
  2. **Hospital:** recuperar remédios em um hospital abandonado.
  3. **Depósito:** infiltrar-se em um armazém de munição dos saqueadores.
  4. **Resgate:** resgatar um morador capturado.
  5. **Ponte:** destruir o ponto de controle principal e liberar a rota.
- Cada missão abre com um briefing de 3 a 5 linhas e fecha com uma linha de resultado.

## 4. Sistemas de jogo

### 4.1 Combate
| Item | Estado |
|---|---|
| Movimento, pulo | Pronto (template) |
| Armas, troca, carregador | Pronto (template) |
| Tiro instantâneo (hitscan), dano por distância, headshot | Feito, aguardando teste |
| Recarga manual (R) e munição de reserva | Feito, aguardando teste |
| Mira (ADS) com zoom e menos dispersão | Próximo |
| Correr (Shift) e agachar (Ctrl) | Próximo |
| Marcador de acerto e HUD com reserva de munição | Planejado |
| Efeitos de impacto, sons de tiro e recarga | Planejado (precisa de assets) |

### 4.2 Armas [PROPOSTA]
Pistola, fuzil de assalto, escopeta, rifle de precisão. O lança-granadas do template pode virar item raro.

### 4.3 Inimigos [PROPOSTA]
- **Saqueador:** inimigo padrão, ataca de frente.
- **Atirador:** posição elevada, tiro de longa distância, dano alto.
- **Brutamontes:** mais vida, avança e usa escopeta.
A IA usa a base de StateTree que o template já traz.

### 4.4 Sistema de missões [PROPOSTA]
Estrutura em C++ para que cada fase seja só configuração no editor:
- **Objetivos:** ir até um ponto, eliminar inimigos, coletar item, sobreviver a um tempo, defender uma área.
- **Gerenciador de missão:** controla a lista de objetivos, avança etapas, dispara vitória e derrota.
- **Checkpoints:** o jogador reaparece no último checkpoint.
- **HUD de objetivo:** texto na tela com o objetivo atual.

### 4.5 Vida e recursos [PROPOSTA]
Sem regeneração total: kits médicos escassos. Munição só em caixas e em inimigos derrotados.

## 5. Plano de implementação

| Etapa | Conteúdo | Quem faz |
|---|---|---|
| M1 | Combate sólido: mira, correr, agachar, HUD de reserva, marcador de acerto | Claude (código), Paulo (testa) |
| M2 | Sistema de missões, objetivos, checkpoints, HUD de objetivo | Claude (código) |
| M3 | Tipos de inimigo, patrulha e comportamento | Claude (código), Paulo (ajustes) |
| M4 | **Missão 1 jogável** (mapa, inimigos, objetivo) | Paulo (editor e level design), Claude (gatilhos e spawners) |
| M5 | Menu principal, pausa, salvar/carregar, empacotar `.exe` | Claude e Paulo |
| M6 | Missões 2 a 5, arte, som, polimento | Os dois |

## 6. Arte e áudio

O Claude não gera modelos 3D, animações nem sons. Fontes previstas:
- **Fab / Epic Launcher:** armas, personagens, ambientes pós-apocalípticos, Megascans.
- **Assets de áudio:** pacotes gratuitos e, para vozes de briefing, o conector ElevenLabs.

## 7. Perguntas em aberto

1. Nome do jogo.
2. O protagonista tem nome, voz e rosto, ou é anônimo?
3. Os inimigos são só humanos (saqueadores) ou também criaturas?
4. Quantas missões na primeira versão? (a proposta é 5, com foco em entregar a 1 bem feita primeiro)
5. Jogos de referência para o clima e a dificuldade.
