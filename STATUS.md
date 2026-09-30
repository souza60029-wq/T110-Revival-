# Status do projeto — T110 Revival

Última atualização: 30/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build corrigido concluído no GitHub Actions: `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION` começa com `#2 SMP PREEMPT`, e o módulo de teste gerou vermagic `3.4.5-2679758 SMP preempt mod_unload ARMv7` (run 36662068883).
- O `boot.img` de reteste foi montado localmente com o novo zImage e o ramdisk stock original; o SHA-1 do cabeçalho foi recalculado e a extração reversa confirmou kernel e ramdisk. A imagem não foi publicada no repositório porque deriva do backup stock completo.
- A primeira tentativa travou no logo. O empacotamento inicial também preservava o ID SHA-1 antigo do cabeçalho; portanto não se pode atribuir a falha apenas à vermagic. `galcore.ko` não foi encontrado no backup `system` recebido, então seu vermagic específico não pôde ser confirmado.
- Teste físico da nova imagem ainda pendente; manter o backup completo original para restaurar a partição Boot se necessário.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição dos originais foram comparados com os `.md5`: 15/15 conferiram.
- O repositório público contém apenas derivados **sanitizados e parciais**: do primeiro, 7 imagens de partição; do segundo, uma imagem `system` filtrada. EFS, `user/data` e partições adicionais de identidade/fábrica foram excluídas. Esses ZIPs não são conjuntos completos nem devem ser usados para restauração integral; detalhes em `backups/*/LEIA-ME.md`.

## Próximo passo imediato
- Testar via TWRP a nova imagem `boot.img` com release `3.4.5-2679758`; se o tablet continuar preso no logo, usar o backup TWRP completo para restaurar Boot. Registrar o resultado.

## Não fazer ainda
- Não começar modificações no kernel antes de validar o baseline no tablet.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes do teste do baseline corrigido.
