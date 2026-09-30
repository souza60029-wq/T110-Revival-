# Status do projeto — T110 Revival

Última atualização: 30/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build baseline corrigido passou no GitHub Actions: `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION` começa com `#2 SMP PREEMPT`, vermagic compilado `3.4.5-2679758 SMP preempt mod_unload ARMv7` (run 36662068883). O usuário relata que esse baseline já funciona no tablet.
- A primeira tentativa anterior travou no logo; o empacotamento inicial preservava o ID SHA-1 antigo do cabeçalho, então a falha não prova incompatibilidade de vermagic. `galcore.ko` não foi encontrado no backup `system`, portanto seu vermagic específico não foi confirmado.
- Governor `ondemand` recalibrado sem trocar de governor; build passou no GitHub Actions (run 36665903655) mantendo release `3.4.5-2679758` e o mesmo vermagic.
- Imagem completa de teste, com ramdisk stock e ID SHA-1 do cabeçalho recalculado, salva em `kernel/builds/governor-tuning/boot.img`. Teste físico desta nova tuning ainda pendente.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição dos originais foram comparados com os `.md5`: 15/15 conferiram.
- O repositório público contém apenas derivados **sanitizados e parciais**: do primeiro, 7 imagens de partição; do segundo, uma imagem `system` filtrada. EFS, `user/data` e partições adicionais de identidade/fábrica foram excluídas. Esses ZIPs não são conjuntos completos nem devem ser usados para restauração integral; detalhes em `backups/*/LEIA-ME.md`.

## Próximo passo imediato
- Testar via TWRP `kernel/builds/governor-tuning/boot.img` e repetir a amostragem de frequência em repouso; se houver falha de boot, restaurar Boot pelo backup TWRP completo. Registrar o resultado.

## Não fazer ainda
- Não acumular outras mudanças no kernel antes de avaliar fisicamente este ajuste do `ondemand`.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes de avaliar o baseline com governor recalibrado.
