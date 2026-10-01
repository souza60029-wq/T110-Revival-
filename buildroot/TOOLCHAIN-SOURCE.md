# Toolchain do kernel T110 no Buildroot

O `.config` do Buildroot continua usando o toolchain de destino ARM/EABIhf/NEON/musl. Para compilar **somente o pacote do kernel Linux 3.4.5**, esta tarefa usa `LINUX_CROSS_COMPILE` apontando ao toolchain legado já especificado no workflow do kernel; os demais pacotes seguem usando `TARGET_CROSS` do Buildroot.

- Repositório AOSP: <https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/>
- Branch definida no workflow: `jb-release`
- Commit esperado pelo workflow: `b4ecd7806d8f46cddeacaf9f8de92c191fb266e4`
- Workflow que fixa e valida o commit: <https://github.com/souza60029-wq/T110-Revival-/blob/main/.github/workflows/build-kernel.yml>
- Prefixo aplicado exclusivamente ao pacote Linux: `/home/ubuntu/work/arm-eabi-4.6/bin/arm-eabi-`

## Dependências e verificação no host

O executável legado é ELF de 32 bits. Neste host Ubuntu x86_64 foram instalados `libc6-i386` (runtime ELF de 32 bits) e `lib32z1` (para `libz.so.1`, exigida por Binutils). Verificado: GCC reporta `4.6.x-google 20120106 (prerelease)`, GNU assembler/linker reportam Binutils 2.21, e um teste de compilação gerou um objeto ARM EABI5.

A referência do compilador é passada por `LINUX_CROSS_COMPILE` à invocação de make do pacote `linux`. Ela não altera `.config`, `TARGET_CROSS`, GCC do userland/rootfs, ABI, arquitetura ou commit do kernel.
