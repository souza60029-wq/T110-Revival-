# Tentativa experimental de USB host — SM-T110

**Estado:** compilada e empacotada; ainda não testada no tablet físico. Nenhuma imagem foi flashada por esta execução.

## Build

- Workflow: [Build SM-T110 kernel](https://github.com/souza60029-wq/T110-Revival-/actions/runs/36668626755)
- Commit do código compilado: `9c86ec6b97e1c7ceb807281a85f75d873a64ddcd`
- `UTS_RELEASE`: `3.4.5-2679758`
- `UTS_VERSION`: `#2 SMP PREEMPT ...`
- Vermagic do módulo: `3.4.5-2679758 SMP preempt mod_unload ARMv7`
- Símbolos adicionados ao defconfig: `CONFIG_USB=y`, `CONFIG_USB_EHCI_HCD=y`, `CONFIG_USB_EHCI_MV=y` e `CONFIG_USB_EHCI_MV_U2O=y`.

A compilação mantém as alterações anteriores de `ondemand`; portanto, este artefato não é um teste isolado de USB em cima do baseline intocado.

## Mudanças exatas no board/USB

Em `kernel/src/arch/arm/mach-mmp/board-goya.c`:

1. O campo `.mode` de `emeidkb_usb_pdata` mudou de `MV_USB_MODE_DEVICE` para `MV_USB_MODE_HOST` (modo host fixo; não `OTG`). O caminho OTG do driver precisa de transceiver e `CONFIG_USB_OTG_UTILS`; o host fixo é a opção adequada para este teste com controlador alimentado externamente.
2. Quando `CONFIG_USB_EHCI_MV_U2O` está ativo, o board associa o mesmo platform data a `pxa988_device_u2oehci` e registra o EHCI. O caminho alternativo que registra `pxa988_device_udc` só é usado sem essa opção. Assim, o mesmo controlador U2O não é registrado simultaneamente como host e gadget.
3. O bloco do platform data é compilado quando UDC ou EHCI U2O está habilitado.

Também foi acrescentado `pxa988_device_u2oehci` em `pxa988.c` e sua declaração em `include/mach/pxa988.h`, reutilizando os recursos `capregs`, `phyregs` e IRQ existentes do PXA988. A declaração genérica PXA168 não é incluída para CPU PXA988 nesta árvore; por isso o dispositivo host precisa ser específico do PXA988.

## Alimentação e limitações

O board mantém `vbus_gpio = PM822_NO_GPIO`. No driver `88pm822_usb.c`, `pm822_set_vbus()` retorna sem habilitar uma fonte OTG5V. Portanto, esta imagem **não faz o tablet fornecer 5 V**. Use somente hub/periférico com alimentação externa e não conte com enumeração: o software pode ativar o host, mas a detecção VBUS/ID e o roteamento físico ainda podem impedir o funcionamento.

Como o UDC não é registrado no modo host, MTP/ADB pela porta USB do tablet não estará disponível enquanto esta imagem estiver ativa.

## Integridade do artefato

- Arquivo: `boot.img` (16 MiB)
- SHA-256: `1f4725b98b8886cd50699b4b4de182745ef07ed1dc7f479288c50eef7a79a036`
- zImage do Actions: SHA-256 `d4c81dda566249209227df821711b27f56c44217405e2ab6f6e9a4f39d55af79`
- ID SHA-1 Android do cabeçalho: `3f04306fe1427254146da84d3823d2cd07d30140`
- O ramdisk foi extraído de `kernel/builds/governor-tuning/boot.img` e conferido byte a byte após recriar a imagem. O ID SHA-1 foi computado com a fórmula de `mkbootimg` e validado contra esse boot de referência.

Teste com cautela via TWRP e mantenha o backup TWRP completo disponível para restaurar a partição Boot se o tablet não iniciar. Esta imagem é experimental, não uma garantia de suporte host.
