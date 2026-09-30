# Status do projeto — T110 Revival

Última atualização: 29/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build do kernel: o primeiro baseline compilou como release `3.4.5`, mas o kernel stock informado pelo tablet é `3.4.5-2679758`. O boot.img inicial travou na tela de logo; incompatibilidade de versão é uma hipótese, não causa comprovada.
- Correção em andamento: defconfig ajustado para `CONFIG_LOCALVERSION="-2679758"`, mantendo `SMP` e `PREEMPT`; o build number `#2` será reproduzido no build. `galcore.ko` não foi encontrado no backup `system` recebido, então seu vermagic não pôde ser lido diretamente.
- Teste físico: primeira tentativa falhou no logo. Nova imagem compatibilizada ainda precisa ser testada; manter o boot stock/restaurá-lo pelo backup completo até lá.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição dos originais foram comparados com os `.md5`: 15/15 conferiram.
- O repositório público contém apenas derivados **sanitizados e parciais**: do primeiro, 7 imagens de partição; do segundo, uma imagem `system` filtrada. EFS, `user/data` e partições adicionais de identidade/fábrica foram excluídas. Esses ZIPs não são conjuntos completos nem devem ser usados para restauração integral; detalhes em `backups/*/LEIA-ME.md`.

## Próximo passo imediato
- Testar via TWRP a nova imagem `boot.img` com release `3.4.5-2679758`; antes disso, restaurar o Boot original se o tablet ainda estiver travado. Registrar o resultado.

## Não fazer ainda
- Não começar modificações no kernel antes de validar o baseline no tablet.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes do teste do `zImage` baseline.
