# Notas de build — kernel SM-T110

- Fonte: `src/`, Linux 3.4.5 da Samsung para o SM-T110 (goyawifi).
- Defconfig: `pxa986_goyawifi_rev02_defconfig`.
- Release alinhado ao stock: `CONFIG_LOCALVERSION="-2679758"` com `CONFIG_LOCALVERSION_AUTO` desativado; o workflow fixa `KBUILD_BUILD_VERSION=2`.
- Toolchain indicado pelo fabricante: Android `arm-eabi-4.6`, prefixo `arm-eabi-`.
- O GitHub Actions fixa o toolchain no commit `b4ecd7806d8f46cddeacaf9f8de92c191fb266e4` do espelho oficial Android.
- Comandos usados no workflow:

```sh
make -C kernel/src ARCH=arm CROSS_COMPILE=/caminho/arm-eabi-4.6/bin/arm-eabi- pxa986_goyawifi_rev02_defconfig
make -C kernel/src ARCH=arm CROSS_COMPILE=/caminho/arm-eabi-4.6/bin/arm-eabi- -j2 zImage modules
```

- Saída compilada: `kernel/src/arch/arm/boot/zImage`; cópia de referência baseline: `kernel/builds/baseline/zImage`.
- O Perl atual rejeita `defined(@val)` no script auxiliar `src/kernel/timeconst.pl`. Para permitir o build, o workflow troca essa expressão por `@val` somente no workspace temporário do runner; nenhuma alteração desse tipo é gravada na fonte versionada.
- Build corrigido do GitHub Actions: `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION=#2 SMP PREEMPT ...`; vermagic observado em módulo compilado: `3.4.5-2679758 SMP preempt mod_unload ARMv7`. O backup `system` recebido não contém `galcore.ko`, então não foi possível confirmar o vermagic desse módulo proprietário.
- Ajuste exclusivo de `ondemand` (commit `473b5fbe`): `DEF_FREQUENCY_UP_THRESHOLD` 80 → 90 (fallback por ticks); `MICRO_FREQUENCY_UP_THRESHOLD` 95 → 98 (caminho NO_HZ usado pelo T110); `LATENCY_MULTIPLIER` 1000 → 2000. No PXA988, a latência configurada é 10.000 ns; com NO_HZ ativo isso eleva o sampling_rate efetivo de 10.000 µs para 20.000 µs. Nenhum outro governor ou opção de versão foi alterado.
- A imagem de reteste foi montada localmente com o ramdisk stock. Ao atualizar boot images Android antigas com `abootimg -u`, recalcular o ID SHA-1 do cabeçalho: a ferramenta preserva o ID do kernel antigo. A nova imagem foi validada por checksum e extração reversa; não usar o primeiro boot.img de teste.
- O build ajustado passou no GitHub Actions (run 36665903655). O `boot.img` completo, com o zImage ajustado e ramdisk original, está em `kernel/builds/governor-tuning/boot.img`; o cabeçalho foi recalculado e a imagem ainda aguarda teste físico.

- Tentativa USB host (Actions run `36668626755`, commit `9c86ec6b`): `CONFIG_USB=y`, `CONFIG_USB_EHCI_HCD=y`, `CONFIG_USB_EHCI_MV=y`, `CONFIG_USB_EHCI_MV_U2O=y`; `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION=#2 SMP PREEMPT`, vermagic inalterado. Em `board-goya.c`, modo alterado de `MV_USB_MODE_DEVICE` para `MV_USB_MODE_HOST`; o ramo host registra `pxa988_device_u2oehci` e não registra o UDC. O device PXA988 foi definido em `pxa988.c` com os recursos USB cap/PHY/IRQ já existentes e declarado em `pxa988.h`.
- O PM822 permanece sem fonte OTG5V (`vbus_gpio=PM822_NO_GPIO`; callback `set_vbus` não energiza o conector). Hub alimentado externamente é requisito do experimento; MTP/ADB ficam indisponíveis nesta imagem e o teste físico ainda está pendente.
- `kernel/builds/usb-host-attempt/boot.img` foi empacotado com ramdisk byte a byte preservado; Android boot ID SHA-1 validado: `3f04306fe1427254146da84d3823d2cd07d30140`; SHA-256 da imagem: `1f4725b98b8886cd50699b4b4de182745ef07ed1dc7f479288c50eef7a79a036`.
