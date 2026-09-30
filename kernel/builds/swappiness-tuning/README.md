# Swappiness tuning — SM-T110

**Estado:** imagem empacotada; aguarda teste físico. Não foi feito flash no tablet.

## O que foi alterado

O default global do kernel já era `60` no build governor-only: `kernel/src/mm/vmscan.c`, variável `int vm_swappiness = 60;` (linha 162 no commit de referência `47ff25c9eff05688ec6338376307d450890cf760`). Não há um `CONFIG_...` separado para escolher esse valor.

O valor efetivo `100` vinha de um override no ramdisk stock: `/init.pxa988.rc`, no bloco `on boot`, sob `# RTCC Swappiness`, escrevia `write /proc/sys/vm/swappiness 100`. Essa linha foi alterada para `60` no ramdisk do novo boot. Assim, o sistema aplica `60` em todo boot; o usuário não precisa configurar o sysctl manualmente depois.

O patch da linha está salvo em [`ramdisk-init.pxa988.rc.patch`](ramdisk-init.pxa988.rc.patch). Não foi necessário alterar o fonte do kernel: o default em `vmscan.c` já era o valor desejado.

## Base e exclusão da tentativa USB host

Esta imagem foi derivada diretamente de `kernel/builds/governor-tuning/boot.img`, commit governor-only `47ff25c9`, anterior aos commits experimentais de USB host. O `zImage` foi comparado byte a byte com o da imagem governor; portanto, **esta imagem mantém o ajuste ondemand e não contém a tentativa USB host ainda não testada**. Como o código do kernel não mudou, foi reutilizado o kernel compilado no workflow [36665903655](https://github.com/souza60029-wq/T110-Revival-/actions/runs/36665903655); somente o ramdisk foi atualizado e o boot image foi reempacotado.

## Validação

- Release herdado do build governor: `3.4.5-2679758`; vermagic do build permanece o já validado `3.4.5-2679758 SMP preempt mod_unload ARMv7`.
- O entry `/init.pxa988.rc` contém uma única escrita de swappiness, agora com valor `60`.
- Os demais entries e payloads do CPIO do ramdisk foram comparados e permaneceram idênticos.
- Ramdisk stock preservado exceto por essa linha; cabeçalho Android recriado e ID SHA-1 validado pela fórmula de `mkbootimg` contra o boot governor de referência.
- Android boot ID SHA-1: `497649659185d55a848932ab7429c6c59b02cd39`.
- SHA-256 de `boot.img`: `460b30e8f9a2243ccd833a947573d165a487f923593c54807036f971631442e8`.

Depois do teste via TWRP, conferir no Android `cat /proc/sys/vm/swappiness` (esperado: `60`) e comparar `SwapFree` no mesmo intervalo de repouso usado no teste anterior. Manter o backup TWRP completo disponível para recuperação.
