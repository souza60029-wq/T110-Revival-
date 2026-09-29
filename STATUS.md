# Status do projeto — T110 Revival

Última atualização: 29/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build do kernel: baseline concluído com sucesso no GitHub Actions, sem alterações funcionais no código do kernel. `zImage` gerado e salvo em `kernel/builds/baseline/zImage`.
- Teste físico: o `zImage` baseline ainda **não** foi testado no tablet.
- Backups TWRP completos: primeiro backup com 13 partições pequenas (boot, bootloader, radio, recovery, efs, custom, preload, dtim, loke1st, loke2nd, mep2, mrd e mrd1); segundo cobrindo `system` e `user/data`. Os 15 arquivos de partição foram comparados com seus `.md5` e todos conferiram.
- Os ZIPs dos backups ainda não foram publicados aqui: este repositório é público, um deles contém `user/data` e o ZIP maior tem 505 MB (acima do limite normal de arquivo do GitHub). Ver os `LEIA-ME.md` em `backups/`.

## Próximo passo imediato
- Testar o `zImage` baseline no tablet físico via TWRP, antes de começar qualquer modificação no kernel; registrar o resultado.

## Não fazer ainda
- Não começar modificações no kernel antes de validar o baseline no tablet.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes do teste do `zImage` baseline.
