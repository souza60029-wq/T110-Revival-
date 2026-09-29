# Histórico de decisões — T110 Revival

Registro curto, em ordem cronológica, para qualquer IA ou pessoa que
assumir o projeto entender rápido "por que estamos fazendo assim" sem
reler a conversa inteira.

## 27/09/2026 — Levantamento inicial do hardware
Inventário completo feito via root/terminal no próprio tablet (scripts de
coleta). Confirmado: kernel 3.4.5, PMIC 88PM822, MUIC RT8973, GPU Vivante
(driver `galcore` ativo, OpenGL ES 2.0), Wi-Fi/BT Marvell SD8777, ZRAM já
ativo de fábrica. USB host não aparece ativo (sem stack `/sys/bus/usb`).

## 27–28/09/2026 — Android vs. Linux (postmarketOS) como base
Decisão: manter Android como base, não trocar para postmarketOS.
Motivo: o kernel do postmarketOS para este aparelho é o mesmo fork 3.4.5
da Samsung — trocar o que roda por cima não resolve os problemas de
energia/drivers, que são do kernel. (Decisão revisada depois, ver
29/09/2026 — não por causa do Android em si, mas pela descoberta de que
dava para montar um Linux **sob medida**, não o postmarketOS genérico.)

## 28/09/2026 — Diagnóstico definitivo de USB/OTG
Com o kernel-fonte oficial da Samsung em mãos (`SM-T110_NA_JB_Opensource.zip`):
confirmado que o host USB está desligado por decisão de projeto
(`# CONFIG_USB is not set` no defconfig) e que o PMIC não tem circuito de
5V para OTG (`vbus_gpio = PM822_NO_GPIO`, comentário do driver: "OTG5V not
supported"). Testes físicos (5 dispositivos) bateram 100% com essa leitura
do código. Conclusão: é limitação de hardware/firmware de fábrica, não
falta de configuração nossa.

## 28/09/2026 — Bluetooth
Confirmado: Bluetooth clássico funciona (controle antigo pareou e
funcionou como HID). Periféricos BLE-only (controle e mouse mais novos)
não aparecem na busca — o Android desta versão não faz varredura BLE na
tela de pareamento padrão.

## 29/09/2026 — Android 9 descartado como meta
Motivos:
- Compilar o Android inteiro (AOSP) exige ~150–300 GB de disco; inviável
  no tablet, no celular, ou em nuvem gratuita sem cartão de crédito.
- Tecnologia de "trocar só as peças" (Project Treble/GSI) só existe a
  partir do Android 8, e o kernel deste aparelho é de 2014 — não foi
  preparado para isso.
- Não existe nenhuma ROM de terceiros (LineageOS ou similar) já feita
  para este chip (Marvell PXA986/988): o chip teve pouquíssimos aparelhos
  no mercado e nenhuma comunidade relevante se formou em torno dele.
- Um port "na unha" (device tree própria + reaproveitamento das peças
  antigas da Samsung) é tecnicamente possível em tese, mas é considerado
  um dos trabalhos mais difíceis e instáveis da área, e ainda dependeria
  da mesma máquina grande no final.

## 29/09/2026 — Novo caminho: kernel próprio + Buildroot
Decisão: recompilar o kernel aberto da Samsung com ajustes próprios, e
montar um Linux mínimo com Buildroot (não uma distribuição genérica como
o Alpine do postmarketOS) por cima dele. Motivos:
- Buildroot ocupa ~5–15 GB para compilar (compilação cruzada, não precisa
  de máquina ARM) — cabe no GitHub Actions gratuito, sem cartão.
- Reaproveita o trabalho mais difícil (o kernel ajustado ao hardware),
  sem herdar o peso do Alpine/X11 que causou os problemas de energia e
  espaço em disco relatados numa tentativa anterior com postmarketOS.
- Interface: framebuffer direto + biblioteca gráfica leve (LVGL) em vez
  de X11/Weston, para manter o sistema mínimo.

## 29/09/2026 — KOF passa a rodar em Linux, não mais em Android
Consequência direta da decisão acima: o jogo passa a usar SDL2 (biblioteca
de jogos 2D) rodando direto no Linux mínimo, em vez de Android/Java.

## 29/09/2026 — Espelhamento: celular como host, não o tablet
Decisão: em vez de tentar resolver o host USB do tablet (que não tem
energia própria, ver diagnóstico de 28/09), inverter os papéis —
o S24 FE (celular, com bateria própria) atua como host/fonte da imagem,
o T110 continua no único papel que já sabe fazer bem (dispositivo USB),
recebendo a tela. Ainda não implementado; é trabalho de software (um
programa em cada ponta), não uma limitação de hardware.

## 29/09/2026 — Marco: primeiro build do kernel bem-sucedido
Manus (outra IA, via GitHub Actions) compilou o kernel-fonte da Samsung
sem nenhuma alteração (build "baseline"), gerando um `zImage` válido
(confirmado: cabeçalho de zImage ARM correto, 3,6 MB). Prova que o
ambiente de build funciona antes de começar a mexer no código.

## 29/09/2026 — Backups completos feitos
Backup via TWRP das partições pequenas (boot, bootloader, radio, recovery,
efs, custom, preload, dtim, loke1st/2nd, mep2, mrd, mrd1) com MD5
verificado, mais um segundo backup cobrindo `system` e `user` (dados) —
completando a exigência de segurança do documento original antes de
qualquer teste de flash.
