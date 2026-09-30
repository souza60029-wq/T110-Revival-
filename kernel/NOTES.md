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
- A imagem de reteste foi montada localmente com o ramdisk stock. Ao atualizar boot images Android antigas com `abootimg -u`, recalcular o ID SHA-1 do cabeçalho: a ferramenta preserva o ID do kernel antigo. A nova imagem foi validada por checksum e extração reversa; não usar o primeiro boot.img de teste.
- O novo `boot.img` não é versionado/publicado: foi derivado do boot original contido no backup TWRP completo. O reteste físico ainda está pendente.
