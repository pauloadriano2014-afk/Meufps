# MeuFPS — Documento de Design (rascunho 1)

> Rascunho vivo. Tudo que está marcado como **[PROPOSTA]** foi sugerido pelo Claude e ainda não foi aprovado.
> Decisões já tomadas pelo Paulo estão marcadas como **[DECIDIDO]**.

## 1. Resumo

- **Gênero:** FPS single player, visual realista, PC (Windows).
- **Engine:** Unreal Engine 5.8.3, código em C++.
- **Estrutura [DECIDIDO]:** campanha em missões, cada uma em uma fase separada.
- **Ambientação [DECIDIDO]:** pós-apocalíptico.
- **História [DECIDIDO]:** simples, contada por missão (briefing curto + objetivo + final), com uma trama leve ligando tudo.
- **Título provisório [DECIDIDO]:** No Choices (pode mudar depois).
- **Protagonista [DECIDIDO]:** Paul, homem.
- **Inimigos [DECIDIDO]:** humanos e criaturas.
- **Primeira versão [DECIDIDO]:** 5 missões.
- **Referências [DECIDIDO]:** The Last of Us, State of Decay, Dead Rising, Dead Island 2.
  - Do que tomamos como direção: tensão e escassez (The Last of Us), sensação de comunidade e risco de perder recursos (State of Decay), improviso com o que está à mão e horda de inimigos (Dead Rising, Dead Island 2).

## 2. Pilares de design [PROPOSTA]

1. **Escassez:** munição e recursos são limitados. Cada tiro importa. (Reserva de munição já implementada.)
2. **Combate direto e legível:** tiro instantâneo, recuo controlável, mira (ADS), dano por parte do corpo. O jogador morre rápido e os inimigos também.
3. **Missões curtas e densas:** 15 a 25 minutos cada, com objetivo claro e um final marcante.
4. **História leve:** poucas linhas de texto/áudio por missão, sem cenas longas.

## 3. Premissa e história [PROPOSTA — para discutir]

- **Mundo:** anos depois de um colapso. Cidades abandonadas, estradas bloqueadas, sobreviventes em pequenos assentamentos.
- **Protagonista:** Paul, um batedor de um assentamento que sai em missões para buscar suprimentos e informações.
- **Ameaças:** dois tipos de inimigo.
  - **Humanos:** um grupo de saqueadores organizados que controla as rotas e os recursos da região.
  - **Criaturas:** infectados que vagam pelas cidades. A causa do colapso fica em aberto para ser revelada aos poucos, ou nunca. (Decidir depois.)
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
| Tiro instantâneo (hitscan), dano por distância, headshot | Feito e testado |
| Recarga manual (R) e munição de reserva | Feito, recarga automática testada (R ainda por confirmar) |
| Mira (ADS, botão direito) com zoom e menos dispersão | Feito, aguardando teste |
| Correr (Shift) e agachar (Ctrl) | Feito, aguardando teste |
| Marcador de acerto e HUD com reserva de munição | Feito, aguardando teste |
| Efeitos de impacto, sons de tiro e recarga | Planejado (precisa de assets) |
| Sistema de missões (objetivos, checkpoints, HUD de objetivo e mensagens) | Feito, aguardando teste. Ver `docs/MISSION_SETUP.md` |
| Infectados (andarilho, corredor, brutamontes) que ouvem tiros e perseguem | Feito, aguardando teste |
| Objetivo de interagir (segurar E) com barulho que atrai a horda | Feito, aguardando teste |
| Construtor automático da Missão 1 | Feito, aguardando teste. Ver `docs/BUILD_MISSION_01.md` |

### 4.2 Armas [PROPOSTA]
- **De fogo:** pistola, fuzil de assalto, escopeta, rifle de precisão. O lança-granadas do template pode virar item raro.
- **Corpo a corpo:** com as referências escolhidas (Dead Rising, Dead Island 2), faz sentido ter armas brancas improvisadas (taco, facão, barra de ferro) como alternativa sem munição. Elas combinam com a escassez e com enfrentar criaturas sem chamar atenção. **Isso é uma decisão nova: confirmar.** O template não traz sistema de combate corpo a corpo, então seria código novo (uma etapa própria).

### 4.3 Inimigos [PROPOSTA]
**Humanos (saqueadores):**
- **Saqueador:** inimigo padrão, ataca de frente.
- **Atirador:** posição elevada, tiro de longa distância, dano alto.

**Criaturas (infectados):**
- **Corredor:** rápido e frágil, ataca em grupo.
- **Andarilho:** lento, resistente, reage a barulho.
- **Brutamontes:** muita vida, avança em linha reta e causa dano alto de perto.

A IA humana usa a base de StateTree que o template já traz. As criaturas precisam de comportamento próprio (perseguir pelo som e pela visão, atacar de perto) e de modelos e animações de infectado, que vêm de assets da Fab. O barulho dos tiros já alerta inimigos no template (sistema de percepção por ruído), o que combina com a ideia de que atirar atrai a horda.

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

1. Armas corpo a corpo entram na primeira versão? (ver 4.2)
2. De onde vem a infecção? Pode ficar sem explicação.
3. Paul tem voz (dublagem) ou fala só por texto de briefing?
4. Haverá um assentamento jogável entre missões (estilo State of Decay) ou só uma tela de menu?
5. Que nível de gore e violência?

Respondidas: nome (No Choices), protagonista (Paul), inimigos (humanos e criaturas), número de missões (5), referências (ver seção 1).
