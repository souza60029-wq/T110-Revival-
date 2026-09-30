# T110 Revival

Um Linux mínimo e sob medida para o Samsung Galaxy Tab 3 Lite
(SM-T110 / Goyawifi).

O objetivo do T110 Revival é transformar o hardware original do
SM-T110 em uma plataforma Linux extremamente leve, mantendo o
máximo possível do hardware utilizável e eliminando a camada
desnecessária de um sistema Android completo.

## Objetivo

O projeto não pretende simplesmente portar uma distribuição Linux
genérica para o tablet.

A ideia é construir um sistema especificamente adaptado ao T110:

- kernel próprio baseado no código-fonte oficial da Samsung;
- sistema Linux mínimo usando Buildroot;
- baixo consumo de RAM e armazenamento;
- ajustes específicos para o hardware do T110;
- investigação e otimização de CPU, GPU, memória e energia;
- suporte USB conforme as capacidades reais do hardware;
- interface gráfica própria e leve;
- ambiente com aparência e comportamento de desktop.

## Arquitetura

O sistema será construído em camadas:

    Samsung Kernel
          ↓
    Kernel modificado
          ↓
       Buildroot
          ↓
      Linux mínimo
          ↓
    Interface gráfica
          ↓
      T110 Revival

## Estado atual

### Kernel

[✓] Código-fonte oficial da Samsung localizado

[✓] Configuração do T110 identificada

[✓] Kernel compilando no GitHub Actions

[✓] `zImage` gerado

[ ] Primeiro boot no hardware real

[ ] Ajustes de energia/governor

[ ] Investigação de USB host/OTG

### Sistema

[ ] Buildroot

[ ] Root filesystem mínimo

[ ] Primeiro Linux funcional

[ ] Framebuffer

[ ] Interface gráfica

[ ] Desktop próprio

## Hardware

O projeto parte do hardware original do SM-T110.

Antes de alterar qualquer componente crítico, o projeto prioriza
testes somente de leitura e documentação do hardware.

## Desenvolvimento

O projeto utiliza GitHub Actions para realizar as compilações,
permitindo desenvolver sem depender de um computador local.

Os artefatos gerados pelas Actions incluem as imagens necessárias
para os testes.

## Segurança

Nenhuma imagem é considerada pronta para instalação apenas porque
a compilação foi concluída.

Cada kernel deve ser:

1. compilado;
2. empacotado corretamente;
3. comparado com o boot original;
4. testado com backup disponível;
5. instalado somente após definir uma forma de recuperação.

## Roadmap

1. Kernel original reproduzível
2. Primeiro boot
3. Ajustes do kernel
4. Buildroot mínimo
5. Linux inicializável
6. Framebuffer
7. Interface gráfica própria
8. Otimizações específicas do T110
9. USB e periféricos
10. Sistema final

## Filosofia

O T110 Revival não tenta transformar o SM-T110 em um dispositivo
moderno através de camadas cada vez maiores.

A proposta é o contrário:

**usar somente aquilo que o hardware realmente precisa.**
