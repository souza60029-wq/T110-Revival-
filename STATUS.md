# Status do projeto — T110 Revival

Última atualização: 29/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build do kernel: baseline concluído com sucesso no GitHub Actions, sem alterações funcionais no código do kernel. `zImage` gerado e salvo em `kernel/builds/baseline/zImage`.
- Teste físico: o `zImage` baseline ainda **não** foi testado no tablet.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição dos originais foram comparados com os `.md5`: 15/15 conferiram.
- O repositório público contém apenas derivados **sanitizados e parciais**: do primeiro, 7 imagens de partição; do segundo, uma imagem `system` filtrada. EFS, `user/data` e partições adicionais de identidade/fábrica foram excluídas. Esses ZIPs não são conjuntos completos nem devem ser usados para restauração integral; detalhes em `backups/*/LEIA-ME.md`.

## Próximo passo imediato
- Testar o `zImage` baseline no tablet físico via TWRP, antes de começar qualquer modificação no kernel; registrar o resultado.

## Não fazer ainda
- Não começar modificações no kernel antes de validar o baseline no tablet.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes do teste do `zImage` baseline.
