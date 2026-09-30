# Governor tuning — SM-T110

Imagem de teste do kernel Linux 3.4.5 para o Samsung SM-T110 (`goyawifi`). O baseline corrigido foi informado como funcional no tablet pelo usuário; esta recalibração do `ondemand` ainda aguarda teste físico.

## Alterações

Somente os defaults do governor `ondemand` foram ajustados em `kernel/src/drivers/cpufreq/cpufreq_ondemand.c`:

- `DEF_FREQUENCY_UP_THRESHOLD`: **80 → 90** (fallback de contabilidade por ticks).
- `MICRO_FREQUENCY_UP_THRESHOLD`: **95 → 98** (caminho NO_HZ usado pelo T110).
- `LATENCY_MULTIPLIER`: **1000 → 2000**. Com a latência de transição de 10.000 ns informada pelo driver PXA988 e contabilidade NO_HZ ativa, o sampling_rate efetivo passa de **10.000 µs (10 ms) → 20.000 µs (20 ms)**.

O governor permanece `ondemand`; `CONFIG_LOCALVERSION=-2679758`, `UTS_RELEASE=3.4.5-2679758` e o build number `#2` foram mantidos.

## Imagem

- Arquivo: [`boot.img`](boot.img)
- Conteúdo: zImage compilado no [GitHub Actions, run 36665903655](https://github.com/souza60029-wq/T110-Revival-/actions/runs/36665903655) + ramdisk stock original.
- O ID SHA-1 do cabeçalho Android foi recalculado após substituir o kernel. A extração reversa confirmou que kernel e ramdisk embutidos correspondem aos arquivos esperados.
- SHA-256: `e6159398b30604f7dbc91214928933c14dd792cd4c727188dedd5b0daa95b0d1`

No TWRP, instalar como **Image → Boot**. Manter o backup completo original disponível para restauração. Não é uma ROM completa.
