# T110 Revival — Samsung SM-T110

Este repositório contém o código-fonte do kernel publicado pela Samsung para o **SM-T110 (goyawifi)**, versão Linux **3.4.5**, em `kernel/src/`. O código foi extraído sem alterações de `Kernel.tar.gz`, incluído em `SM-T110_NA_JB_Opensource.zip`. As instruções originais do fabricante estão em [`kernel/src/README_Kernel_vendor.txt`](kernel/src/README_Kernel_vendor.txt).

## Compilar pelo GitHub

1. Abra a aba **Actions** deste repositório.
2. Escolha **Build SM-T110 kernel** e clique em **Run workflow**.
3. Quando o processo terminar, abra a execução e baixe o artefato **SM-T110-zImage**.

O workflow compila o `pxa986_goyawifi_rev02_defconfig` com o compilador **arm-eabi-4.6**, indicado pelo fabricante. Ele é manual (`workflow_dispatch`): não começa automaticamente a cada envio de código.

Como o runner atual do GitHub usa Perl que rejeita uma expressão obsoleta no script auxiliar de Linux 3.4, o workflow adapta essa verificação para sua forma equivalente apenas no espaço temporário da execução. A árvore versionada do kernel permanece inalterada.

## Compilação local (Linux)

O procedimento original requer o toolchain `arm-eabi-4.6`:

```sh
cd kernel/src
export ARCH=arm
export CROSS_COMPILE=/caminho/para/arm-eabi-4.6/bin/arm-eabi-
make pxa986_goyawifi_rev02_defconfig
make -j2
```

A imagem do kernel baseline validada pelo build fica em `kernel/builds/baseline/zImage`. Ela ainda aguarda teste no tablet físico.

> O arquivo compactado original tem mais de 100 MB e excede o limite por arquivo do GitHub. Por isso, este repositório guarda a árvore de fontes extraída, em vez do `Kernel.tar.gz` como um único arquivo.
