# Status do projeto — T110 Revival

Última atualização: 30/09/2026

## Onde estamos
- Alvo: Samsung Galaxy Tab 3 Lite 7.0 (SM-T110, "goyawifi").
- Caminho escolhido: kernel próprio baseado no fonte aberto da Samsung + Linux mínimo via Buildroot. Android 9 foi descartado (ver `docs/decisoes.md`).
- Build baseline corrigido passou no GitHub Actions: `UTS_RELEASE=3.4.5-2679758`, `UTS_VERSION` começa com `#2 SMP PREEMPT`, vermagic compilado `3.4.5-2679758 SMP preempt mod_unload ARMv7` (run 36662068883). O usuário relata que esse baseline já funciona no tablet.
- A primeira tentativa anterior travou no logo; o empacotamento inicial preservava o ID SHA-1 antigo do cabeçalho, então a falha não prova incompatibilidade de vermagic. `galcore.ko` não foi encontrado no backup `system`, portanto seu vermagic específico não foi confirmado.
- Governor `ondemand` recalibrado sem trocar de governor; build passou no GitHub Actions (run 36665903655), mantendo release `3.4.5-2679758` e o mesmo vermagic. A imagem `kernel/builds/governor-tuning/boot.img` ainda aguarda teste físico.
- O default global no kernel é `vm_swappiness=60` (`kernel/src/mm/vmscan.c:162` no commit governor-only). O valor efetivo `100` vinha do initramfs stock em `/init.pxa988.rc` (`# RTCC Swappiness`). A imagem `kernel/builds/swappiness-tuning/boot.img` muda somente essa escrita para `60`; reutiliza o zImage governor-only e não inclui host USB. ID SHA-1 Android `497649659185d55a848932ab7429c6c59b02cd39`; SHA-256 `460b30e8f9a2243ccd833a947573d165a487f923593c54807036f971631442e8`.
- A tentativa experimental de USB host também está compilada (Actions run 36668626755), mas **ainda não foi testada no tablet**. Ela habilita `MV_USB_MODE_HOST`, não registra o UDC e não fornece 5 V pelo PM822; detalhes em `kernel/builds/usb-host-attempt/README.md`.
- Os dois backups TWRP originais estão preservados localmente. Os 15 arquivos de partição foram comparados com os `.md5`: 15/15 conferiram. O repositório público contém somente derivados sanitizados e parciais; veja `backups/*/LEIA-ME.md`. Não usar esses ZIPs parciais para restauração integral.

## Próximo passo imediato
- Testar via TWRP `kernel/builds/swappiness-tuning/boot.img`, confirmar que o Android inicia e que `cat /proc/sys/vm/swappiness` retorna `60`. Repetir a medição de `SwapFree` no mesmo repouso de três minutos e registrar o resultado. Se houver falha de boot, restaurar Boot pelo backup TWRP completo.
- Depois desse teste, avaliar separadamente a imagem `kernel/builds/usb-host-attempt/boot.img` com hub alimentado externamente; ela é um build diferente.

## Não fazer ainda
- Não acumular novos ajustes no kernel antes de observar o efeito do swappiness no tablet.
- Não compilar `Platform.tar.gz` (Android) — fora de escopo.
- Não iniciar o Buildroot antes de avaliar os testes físicos pendentes.
