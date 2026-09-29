# T110 Revival — Objetivos e Arquitetura do Projeto — v0.1

Alvo: Samsung Galaxy Tab 3 Lite 7.0 Wi-Fi — SM-T110 (goyawifi)

**Visão original:** transformar o T110 num tablet extremamente otimizado,
com experiência de desktop inspirada em Linux e Android 12, voltado
principalmente para leitura, vídeo e jogos 2D.

> Nota de continuidade (29/09/2026): a meta de sistema-base mudou depois
> deste documento. Ver `docs/decisoes.md` para o histórico — o alvo deixou
> de ser Android 9 e passou a ser um kernel próprio + Linux mínimo via
> Buildroot. Os objetivos de uso (leitura, vídeo, KOF, desktop leve) e a
> filosofia de otimização abaixo continuam valendo.

## 1. Objetivo central

Não transformar o T110 num dispositivo moderno por força bruta, e sim
extrair o máximo que o hardware realmente consegue entregar, removendo
tudo que não for necessário para os usos definidos pelo projeto.

## 2. Perfil de uso pretendido

| Prioridade | Uso | Objetivo |
|---|---|---|
| 1 | Leitura de mangás | Interface rápida, leitor leve, baixo consumo |
| 2 | Vídeos | Aceleração de decodificação sempre que o hardware permitir |
| 3 | KOF (jogo 2D) | Alvo otimizado, controles e desempenho adequados |
| 4 | Uso desktop | Janelas, teclado e mouse, multitarefa leve |
| 5 | Espelhamento | USB e/ou protocolo próprio |

## 3. Conceito de interface: Android/Linux Desktop

Aparência própria do projeto, inspirada em desktop Linux e elementos do
Android 12, sem precisar copiar nenhum produto. Elementos planejados:
tela inicial leve, launcher/UI customizado, janelas com mover/redimensionar
quando possível, multitarefa leve, suporte a toque/teclado/mouse.

## 4. Filosofia de otimização

"Super otimizado" é tratado como objetivo técnico mensurável: medir CPU,
RAM, I/O, tempo de boot, GPU, consumo de serviços e comportamento térmico.
Remover/desativar o que não for necessário, minimizar serviços em segundo
plano, reduzir animações sem propósito, otimizar RAM (ZRAM já confirmado
ativo de fábrica), evitar swap tradicional em microSD como padrão, manter
aceleração de GPU/vídeo sempre que for melhor que processar em CPU.

## 5. Hardware e drivers

Mapear e melhorar a integração de CPU, GPU, vídeo, áudio, armazenamento,
USB, Wi-Fi, Bluetooth, sensores e entrada. Ver `docs/estado_do_conhecimento.md`
para o levantamento real já feito (GPU Vivante GC1000 confirmada ativa,
Wi-Fi/BT Marvell SD8777 confirmado, USB host confirmado desligado por
projeto de hardware).

## 6. USB, OTG e periféricos

Meta original: 2 gamepads simultâneos, teclado, mouse, combinações,
camada de input própria no KOF. **Atualização confirmada:** o host USB
está desligado no defconfig do kernel e o PMIC não tem circuito de 5V
para OTG (ver seção 6 do estado do conhecimento). Bluetooth clássico
funciona; periféricos BLE-only não são detectados pela busca do sistema
original. Ideia em andamento: espelhamento via USB usando o celular
(S24 FE) como host, aproveitando que o T110 já funciona bem como
dispositivo USB.

## 7. Espelhamento e Samsung DeX

Investigar o que trafega pelo USB antes de presumir suporte a DeX. Caminho
priorizado atualmente: espelhamento próprio (S24 FE como host/fonte →
transporte USB → T110 como dispositivo/tela), não necessariamente DeX.

## 8. KOF / jogo 2D

Perfil específico para o T110, renderização 2D eficiente, suporte a dois
gamepads, teclado e mouse. **Atualização:** com a troca de rota para
kernel próprio + Buildroot, o KOF passa a rodar nativamente em Linux via
SDL2, não mais em Android/Java.

## 9. Base do sistema

> Superado pela decisão registrada em `docs/decisoes.md`: Android 9 foi
> descartado como base. O texto original abaixo é mantido como registro
> histórico do raciocínio inicial.

Android 9 era o alvo inicial por representar um compromisso entre
compatibilidade de aplicativos e a capacidade limitada do T110, reaproveitando
fontes existentes do T110 (device tree, kernel, vendor) em vez de um
dispositivo genérico.

## 10. MicroSD de 32 GB

SanDisk Ultra, classe UHS-I/U1. Testes de leitura/escrita sequencial e
aleatória ainda pendentes, para comparar com o armazenamento interno.

## 11. Sistema oficial de logs

Todo teste deve ser reproduzível e documentado, com data/hora, comando,
saída completa, código de retorno e identificação do teste, salvo em
`/storage/extSdCard/logs/`. Este padrão já está em uso pelos scripts de
coleta do projeto.

## 12. Regras de segurança do desenvolvimento

- Não executar flash, wipe, format, dd, fastboot, Heimdall ou operações
  destrutivas sem uma etapa explícita de preparação e recuperação.
- Antes de modificar boot, recovery, kernel ou partições, criar e validar
  backups apropriados. **Status:** backup completo (13 partições pequenas +
  system + user) já feito via TWRP em 29/09/2026.
- Não assumir que componentes de outros modelos Galaxy Tab 3 Lite são
  compatíveis com o T110.
- Testar mudanças isoladamente e registrar resultados.
- Sempre que possível, ter um caminho de rollback antes de uma alteração
  de baixo nível.

## 13. Metodologia do projeto

Tutoria prática: cada comando explicado (o que faz, por que é necessário,
o que o resultado significa). Arquitetura de etapas: observar, reproduzir,
modificar, só então instalar. Sem grandes mudanças simultâneas.

## 14. Roadmap (revisado)

> O roadmap original mirava Android 9. Ver `STATUS.md` e
> `docs/decisoes.md` para o roadmap atual (kernel próprio + Buildroot).

## 15. Resultado final esperado

Sistema aberto, reproduzível e documentado, pequeno, responsivo e
especializado em leitura, mídia e jogos 2D, com suporte a periféricos e
espelhamento — dentro dos limites reais do hardware, não da versão mais
nova possível de Android.
