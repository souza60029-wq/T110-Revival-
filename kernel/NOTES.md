# Notas de build — kernel SM-T110

- Fonte: `src/`, Linux 3.4.5 da Samsung para o SM-T110 (goyawifi).
- Defconfig: `pxa986_goyawifi_rev02_defconfig`.
- Toolchain indicado pelo fabricante: Android `arm-eabi-4.6`, prefixo `arm-eabi-`.
- O GitHub Actions fixa o toolchain no commit `b4ecd7806d8f46cddeacaf9f8de92c191fb266e4` do espelho oficial Android.
- Comandos usados no workflow:

```sh
make -C kernel/src ARCH=arm CROSS_COMPILE=/caminho/arm-eabi-4.6/bin/arm-eabi- pxa986_goyawifi_rev02_defconfig
make -C kernel/src ARCH=arm CROSS_COMPILE=/caminho/arm-eabi-4.6/bin/arm-eabi- -j2 zImage modules
```

- Saída compilada: `kernel/src/arch/arm/boot/zImage`; cópia de referência baseline: `kernel/builds/baseline/zImage`.
- O Perl atual rejeita `defined(@val)` no script auxiliar `src/kernel/timeconst.pl`. Para permitir o build, o workflow troca essa expressão por `@val` somente no workspace temporário do runner; nenhuma alteração desse tipo é gravada na fonte versionada.
- Primeiro build pelo GitHub Actions: concluído com sucesso em 29/09/2026. O `zImage` baseline ainda aguarda teste no tablet físico.
