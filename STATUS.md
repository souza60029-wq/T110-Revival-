# Status do projeto — T110 Revival

Última atualização: 30/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build baseline corrigido passou no GitHub Actions: `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION` começa com `#2 SMP PREEMPT`, vermagic compilado `3.4.5-2679758 SMP preempt mod_unload ARMv7` (run 36662068883). O usuário relata que esse baseline já funciona no tablet.
- A primeira tentativa anterior travou no logo; o empacotamento inicial preservava o ID SHA-1 antigo do cabeçalho, então a falha não prova incompatibilidade de vermagic. `galcore.ko` não foi encontrado no backup `system`, portanto seu vermagic específico não foi confirmado.
- Governor `ondemand` recalibrado sem trocar de governor; build passou no GitHub Actions (run 36665903655) mantendo release `3.4.5-2679758` e o mesmo vermagic.
- A imagem de teste do governor, com ramdisk stock e ID do cabeçalho recalculado, está em `kernel/builds/governor-tuning/boot.img`; ainda não foi avaliada fisicamente.
- Tentativa experimental de USB host compilada no GitHub Actions (run 36668626755): release `3.4.5-2679758`, `#2 SMP PREEMPT`, vermagic `3.4.5-2679758 SMP preempt mod_unload ARMv7`. A imagem completa está em `kernel/builds/usb-host-attempt/boot.img` (SHA-256 `1f4725b98b8886cd50699b4b4de182745ef07ed1dc7f479288c50eef7a79a036`). Ela acumula a calibração `ondemand` anterior.
- USB host foi selecionado como modo fixo (`MV_USB_MODE_HOST`); o dispositivo UDC não é registrado neste build. A porta não oferecerá MTP/ADB enquanto esta imagem estiver ativa. O PM822 está configurado com `PM822_NO_GPIO` e o callback de VBUS não fornece 5 V; o objetivo depende de hub alimentado externamente e o reconhecimento não está garantido.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição dos originais foram comparados com os `.md5`: 15/15 conferiram.
- O repositório público contém apenas derivados **sanitizados e parciais**: do primeiro, 7 imagens de partição; do segundo, uma imagem `system` filtrada. EFS, `user/data` e partições adicionais de identidade/fábrica foram excluídas. Esses ZIPs não são conjuntos completos nem devem ser usados para restauração integral; detalhes em `backups/*/LEIA-ME.md`.

## Próximo passo imediato
- Testar via TWRP `kernel/builds/usb-host-attempt/boot.img` com um hub USB alimentado externamente. Confirmar primeiro se o Android inicia e depois se enumera algum dispositivo; lembrar que esta imagem também contém o ajuste `ondemand`. Se houver falha de boot, restaurar Boot pelo backup TWRP completo. Registrar o resultado antes de novas alterações.

## Não fazer ainda
- Não acumular outras modificações no kernel até avaliar fisicamente esta tentativa de USB host.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes de avaliar os testes físicos pendentes.
