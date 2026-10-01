# Checkpoint — Buildroot minimal-v1

**Data:** 2026-10-01
**Resultado:** build completa concluída com sucesso; nenhum dispositivo foi flasheado.

## Resultado da compilação

- Buildroot: **2025.02.18**.
- Comando executado: `make -C /home/ubuntu/work/t110-buildroot -j6 LINUX_CROSS_COMPILE=/home/ubuntu/work/arm-eabi-4.6/bin/arm-eabi-`.
- Exit code: **0**.
- Duração aproximada: **159 s (2 min 39 s)**, do início ao último registro do log; o monitor externo registrou até 180 s porque verificava o processo em intervalos de 45 s.
- Kernel compilado: `UTS_RELEASE="3.4.5-2679758"`; `UTS_VERSION` começa com `#2 SMP PREEMPT`.
- O `compile.h` gerado registra `gcc version 4.6.x-google 20120106 (prerelease) (GCC)`.
- O `rootfs.tar` foi validado como arquivo tar e contém 386 entradas; o `zImage` foi identificado como imagem ARM little-endian.
- O hash do `.config` antes e depois do build foi idêntico: `4785286069c80e1705cdce5d756fdf14c0400afd78a5099b706497ef680a017a`.

## Parâmetros de alvo preservados

O arquivo `.config` realmente usado foi preservado exatamente para entrega privada; ele não foi incluído no repositório público porque contém configuração de senha de root. Os valores verificados incluem ARM (`BR2_arm=y`), Cortex-A9/ARMv7-A (`BR2_ARM_CPU_ARMV7A=y`), EABIhf (`BR2_ARM_EABIHF=y`), NEON (`BR2_ARM_ENABLE_NEON=y`, `BR2_ARM_FPU_NEON=y`) e musl (`BR2_TOOLCHAIN_BUILDROOT_MUSL=y`). O kernel continua apontando para o commit `473b5fbe34cd2d56f5f52dabba0db9280e8c49ba` e para o defconfig `pxa986_goyawifi_rev02`.

A opção de infraestrutura autorizada ficou registrada como `BR2_LINUX_KERNEL_CUSTOM_REPO_SUBDIR="kernel/src"`. O host Go foi corrigido para `BR2_PACKAGE_HOST_GO_BIN_HOST_ARCH="amd64"`, conforme autorização; isso é arquitetura do host de build, não do alvo.

### Oito entradas novas mantidas em `n`

Comparando o `.config` atual com a cópia aprovada anterior, estes oito símbolos foram adicionados desabilitados:

1. `BR2_PACKAGE_HOST_AMLOGIC_BOOT_FIP=n`
2. `BR2_PACKAGE_HOST_FLUTTER_SDK_BIN=n`
3. `BR2_PACKAGE_HOST_PRU_SOFTWARE_SUPPORT=n`
4. `BR2_PACKAGE_HOST_SAM_BA=n`
5. `BR2_PACKAGE_HOST_TI_CGT_PRU=n`
6. `BR2_PACKAGE_LUAJIT=n`
7. `BR2_PACKAGE_PRU_SOFTWARE_SUPPORT=n`
8. `BR2_TARGET_S500_BOOTLOADER=n`

Os logs disponíveis mostram cinco prompts marcados explicitamente `(NEW)` — `HOST_FLUTTER_SDK_BIN`, `HOST_PRU_SOFTWARE_SUPPORT`, `HOST_SAM_BA`, `HOST_TI_CGT_PRU` e `PRU_SOFTWARE_SUPPORT` — respondidos com `n`. Os outros três aparecem como opções novas desabilitadas no delta sincronizado do `.config`, sem prompt `(NEW)` nos logs recuperados. Não houve alteração dos parâmetros protegidos do alvo.

## Toolchains e mudanças de infraestrutura

- O toolchain interno do Buildroot continua atendendo userland/rootfs com ARM/EABIhf/musl (GCC 13.4 no host de build). Não foi substituído pelo GCC 4.6.
- Somente o pacote Linux recebeu o override `LINUX_CROSS_COMPILE` para o toolchain AOSP legado já fixado pelo workflow: commit `b4ecd7806d8f46cddeacaf9f8de92c191fb266e4`. Binutils 2.21 e um teste de compilação para ARM EABI5 foram verificados.
- Para executar os binários legados de 32 bits no host x86_64, foram instalados `libc6-i386` e `lib32z1`; são dependências do host e não modificam o `.config` do alvo.
- O patch `patches/0001-t110-buildroot-kernel-subdir-and-cross-override.patch` implementa o subdiretório do repositório para kernel e linux-headers e o override isolado do compilador do kernel. A limpeza anterior removeu somente a árvore de build Linux falha.
- `headers_install` foi executado. O erro anterior de ausência de alvo `headers_install` **não persistiu**.

## Artefatos e publicação

| Artefato | Tamanho | SHA-256 | Estado |
|---|---:|---|---|
| `.config` realmente usado | 135.587 bytes | `4785286069c80e1705cdce5d756fdf14c0400afd78a5099b706497ef680a017a` | Mantido local para entrega privada; contém configuração de senha de root; não publicado |
| `zImage` | 3.769.528 bytes | `c7f82b7fa1d1af164e44e7851a273b410e61d8aee31aa86bfd701b6bcd85017b` | Seguro para publicar em `buildroot/builds/minimal-v1/zImage` |
| `rootfs.tar` | 3.788.800 bytes | `24f455e0980d6e69dc695f5ea8b5dfc982ca6a8fc455e736ce05f36f28812412` | Mantido local para entrega privada; contém hash de senha em `/etc/shadow`; não publicado |
| `build.log` | 300.603 bytes | `31f2bc4afda27a3dbd4689ccc1be75ef28005cebe4a2d57a1e944825cf808572` | Mantido local; ecoa a senha configurada; não publicado |

O `.config`, `rootfs.tar` e `build.log` locais foram verificados/copiados byte a byte a partir dos produtos do build. O repositório público recebe somente o `zImage` entre os binários. O checkpoint abaixo resume os warnings sem reproduzir credenciais.

## Warnings observados

O log contém 864 linhas com texto de warning (368 mensagens distintas pela contagem literal; várias se repetem em etapas de configuração/compilação). Não há marcador `error:` nem falha `make: ***`. Os avisos mais relevantes são:

- `-Wno-attribute-alias` não é reconhecido pelo GCC 4.6 (84 ocorrências); o compilador o ignora e a compilação concluiu.
- O Kconfig legado avisa que `ZRAM` seleciona `ZSMALLOC` apesar da dependência `STAGING && X86`. A build concluiu, mas a disponibilidade efetiva de ZRAM neste kernel ARM não deve ser presumida sem teste.
- Avisos de possível não inicialização de `sel_area`/`sel_order` em `mm/page_alloc.c`, além de diagnósticos legados de variáveis/funções não usadas, macros PMIC redefinidas, declarações V4L2 e o TODO de `return_address`/unwind tables.
- Ferramentas host compiladas com GCC 13 emitiram avisos de análise (`dangling-pointer` no Kconfig e possível truncamento em `gen_init_cpio.c`). Também houve avisos de `jobserver unavailable: using -j1` em sub-makes; são não fatais.
- A mensagem de `KCFLAGS` sendo anexado aos `CFLAGS` é emitida pela própria árvore kernel; não interrompeu o build.

## Próximo passo

Inspecionar o `zImage` e o resumo de warnings. **Nenhum flash ou teste físico foi feito**; qualquer teste no tablet permanece separado e requer a autorização correspondente.
