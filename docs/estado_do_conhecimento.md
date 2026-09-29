# T110 Revival: estado do conhecimento (v0.3)

Data: 28/09/2026 · Alvo: Samsung Galaxy Tab 3 Lite 7.0 Wi-Fi, SM-T110 (goyawifi)

Legenda: **[CONFIRMADO]** = visto nos logs · **[INFERIDO]** = conclusão provável, não provada · **[ABERTO]** = ainda não sabemos.

Nada foi gravado, flashado ou modificado nas partições durante este levantamento. Os scripts só leem informação e escrevem em `/storage/extSdCard/logs/`.

---

## 1. Identificação

- **[CONFIRMADO]** Modelo `SM-T110`, device `goyawifi`, produto `goyawifiue`, firmware `T110UEUANE4`.
- **[CONFIRMADO]** Android 4.2.2 (JDQ39, SDK 17), compilado em 21/05/2014.
- **[CONFIRMADO]** Kernel `3.4.5-2679758 #2 SMP PREEMPT` (armv7l, GCC 4.6, compilado em 21/05/2014).
- **[CONFIRMADO]** Root ativo (`uid=0`), `ro.secure=0`, `ro.debuggable=1`.
- **[CONFIRMADO]** Não é uma custom ROM no sentido de base trocada (como LineageOS): o `fingerprint` continua `samsung/goyawifiue/goyawifi:4.2.2/JDQ39/T110UEUANE4:user/release-keys`, idêntico ao original. É a **stock rooteada** com ajustes de `build.prop` (dithering, scrollingcache, storage_preload) e pacotes extras.
- **[CONFIRMADO]** Root via **SuperSU 2.35** + **BusyBox 1.22.0**, ambos instalados por recovery em **25/11/2025** (data dos arquivos em `/system/xbin`). Existe um `last_log.gz` de recovery da mesma data, mas vazio (0 bytes), então não dá pra ler o que foi flashado.
- **[ABERTO]** Se algum pacote de "tweaks" específico (com nome próprio) foi instalado além do root, ou se os ajustes do `build.prop` já vinham de algum guia genérico de otimização.

## 2. Processador e memória

| Item | Valor |
|---|---|
| SoC | Marvell. `ro.product.board=PXA986`, mas `/proc/cpuinfo` diz `PXA988` **[divergência a investigar]** |
| CPU | 2 núcleos ARMv7 Cortex-A9 (part 0xc09), NEON, VFPv3 |
| Frequência | 312 MHz a 1,205 GHz, governor `ondemand` |
| RAM visível | 835.876 kB (~816 MiB) |
| Compressão de RAM | `zram0` de ~300 MB ativo, mais `kcompcached` (visto no dmesg) |
| Térmico | `thermal_zone0` lê sempre **80000**, sem variar |
| RAM reservada para vídeo/GPU | `reserve_gpu=64M` + `ioncarv=80M` no `/proc/cmdline`: **~144 MB** ficam fora do que o Android enxerga |

- **[CONFIRMADO]** `thermal_zone0` é um nó **morto/não calibrado**: em 12 amostras seguidas, com a CPU alternando entre 312 MHz e 1,2 GHz, o valor não mudou nenhuma vez. Não é uma leitura real de temperatura; ignorar esse sensor.
- **[CONFIRMADO]** `swappiness = 100` (o máximo). O kernel prefere agressivamente mandar páginas para o zram em vez de descartar cache. Em amostragem de 1 minuto em repouso, o zram estava majoritariamente livre (~276 MB livres de 300 MB), bem diferente do primeiro inventário (que pegou o zram quase cheio) — o uso do swap varia bastante com o que está rodando.
- **[CONFIRMADO]** O `zram` comprime bem na prática: ~230 MB de dados originais ocupando ~77 MB comprimidos (razão ~3:1).
- **[ABERTO/ponto de otimização]** Na amostragem de 1 minuto em repouso quase total (load médio 0.08–0.18), a CPU ficou em 1,205 GHz (frequência máxima) em 11 das 12 amostras, e só 1 vez em 312 MHz. O governor `ondemand` parece não estar descendo de frequência como seria esperado em ócio — vale investigar os parâmetros dele (`up_threshold`, `sampling_rate`) como alvo de otimização de bateria.

## 3. Armazenamento

- **eMMC** `mmcblk0` (~7,3 GiB), partições p1 a p16, mais `mmcblk0boot0/1` (2 MB cada).
  - p15 `/system` ext4, 1,46 GB (652 MB usados), montado `ro`
  - p16 `/data` ext4, 5,04 GB (355 MB usados)
  - p14 `/cache`, 338 MB · p4 `/efs`, 19,6 MB · p12 `/NVM`, 15,7 MB
- **microSD** `mmcblk1`, 29,7 GB, vfat, montado em `/storage/extSdCard` com `rw,dirsync,noexec,nosuid`. Tem 5,23 GB usados e 24,5 GB livres.
- **[CONFIRMADO]** Mapa completo das 16 partições da eMMC, via `PARTNAME` do sysfs:

| Partição | Nome |
|---|---|
| p1 | MRD |
| p2 | MRD_BK |
| p3 | MEP2 |
| p4 | EFS |
| p5 | Reserved |
| p6 | LOKE_2ND |
| p7 | LOKE_1ST |
| p8 | PARAM |
| p9 | RECOVERY |
| p10 | KERNEL (boot) |
| p11 | MODEM |
| p12 | NVM |
| p13 | HIDDEN |
| p14 | CACHE |
| p15 | SYSTEM |
| p16 | USER (data) |

- **[ABERTO]** Benchmarks de leitura/escrita (interno vs. microSD) ainda não foram feitos. O `dirsync` provavelmente deixa a escrita no cartão mais lenta.
- **Consequência prática:** já dá para planejar o backup completo (todas as 16 partições) antes de qualquer tentativa de flash futura.

## 4. Tela, GPU e vídeo

- **[CONFIRMADO]** Tela 1024×600. `fb0` (camada gráfica, buffer virtual 1024×1800) e `fb1` (camada de vídeo/overlay separada), driver `pxa168-fb`.
- **[CONFIRMADO]** Módulo `galcore` (driver Vivante) carregado. Registradores em `c0400000`, 64 MiB reservados (`gpu_mem`). OpenGL ES 2.0. Existem nós de frequência de GPU separados para 2D, 3D e shader (`gpu_freq_*_min/max`).
- **[INFERIDO]** O nome "GC1000" vem do documento do projeto, não dos logs.
- **[CONFIRMADO]** Bloco de vídeo `pxa-coda7542` presente (IRQ e memória próprios), o candidato natural à decodificação por hardware. Falta testar na prática.
- **[CONFIRMADO]** HALs proprietárias da Marvell: `gralloc.mrvl`, `hwcomposer.mrvl`, `camera.mrvl`, `sensors.mrvl`, `power.mrvl`, `audio.*.mrvl`. Elas pesam muito em qualquer tentativa de subir de versão do Android.
- Touch: `sec_touchscreen` (Zinitix BT532). Acelerômetro: BMA255. **É o único sensor exposto.**
- Áudio: placa ALSA `emei-dkb-hifi`, codec 88PM805. Há também rádio FM (driver `mrvl8xxx_fm`).

## 5. Wi-Fi e Bluetooth

- **[CONFIRMADO]** Wi-Fi e Bluetooth são o mesmo chip combo Marvell SD8777 (SDIO). Firmware `mrvl/sd8777_uapsta.bin`, módulos `sd8xxx` e `mlan`.
- **[CONFIRMADO]** Wi-Fi conecta normalmente. O modo hotspot (AP) e o Wi-Fi Direct (`p2p0`) também aparecem funcionando.
- **[CONFIRMADO]** Bluetooth: o firmware carrega (`/dev/mbtchar0`), e o tablet pareia com o celular.
- **[CONFIRMADO]** O stack tem serviços com marca Broadcom: HID, A2DP, HFP, PAN, MAP, SAP e **GATT**. Há bibliotecas aptX.
- **Periféricos Bluetooth (testes manuais):**
  - Controle BT novo: não aparece.
  - Mouse BT: não aparece.
  - Controle antigo: aparece (como teclado), mas o pareamento falha com "Unable to communicate with C1:4A:23:80:63:25".
  - Teclado BT: não testado à parte. O modelo do gamepad ainda não foi identificado.
- **[INFERIDO]** O endereço `C1:…` começa com bits `11` e byte ímpar, o padrão de endereço aleatório de dispositivos **BLE**. Se o controle for BLE-only, o tablet não consegue parear com ele.
- **Ressalva:** como o stack inclui GATT, não dá para afirmar que BLE seja totalmente sem suporte. O suporte a HID sobre BLE é **[ABERTO]**.
- **[ABERTO]** Nenhum log de tentativa de pareamento foi capturado ainda. A etapa C do script não pegou nenhuma tentativa.

## 6. USB e OTG (resultado principal até agora)

**Cinco testes físicos** com cabo OTG (funcional em outro celular), cada um com snapshot antes e depois, deram o mesmo resultado:

1. **[CONFIRMADO]** O chip MUIC **RT8973** detecta o cabo: `Cable change to MUIC_RT8973_CABLE_TYPE_OTG`, `OTG attached`, `ID value = 0x0`. O contador de IRQ do `rt8973` sobe +1 por conexão.
2. **[CONFIRMADO]** Depois disso não acontece nada: nenhum regulador muda, nenhum dispositivo de entrada aparece, `/sys/bus/usb` não existe e o `lsmod` não muda. O mouse USB não liga, ou seja, **o tablet não entrega 5 V**.
3. **[CONFIRMADO]** O kernel não tem driver de host USB (nenhum símbolo `ehci`, `ohci` ou `xhci`). A única entidade USB da plataforma é o `mv-udc`, em modo dispositivo.
4. **[CONFIRMADO]** O kernel tem código para tratar OTG (`rt8973_otg_attach_handler`, `pm822_set_vbus`, `smb327_charger_otg_control`).
5. **[INFERIDO]** O mesmo bloco de registradores (`capregs`/`phyregs`) pertence ao `mv-udc`. Controladores dessa família costumam suportar host, mas isso não está provado para este SoC.
6. **[CONFIRMADO]** Os itens `msm_hsusb_host` no `vold.fstab` são um modelo genérico copiado de outra plataforma (Qualcomm). Não indicam suporte real.

### Confirmado pelo código-fonte do kernel (SM-T110_NA, aberto pela Samsung)

O kernel oficial aberto da placa **confirma, ponto a ponto, o que os logs indicavam**:

- **[CONFIRMADO]** O defconfig da placa (`pxa986_goyawifi_rev02_defconfig`) tem `# CONFIG_USB is not set`: o suporte a host USB do kernel **está desligado desde a compilação**. Só existe o modo dispositivo (`CONFIG_USB_MV_UDC=y`).
- **[CONFIRMADO]** Em `board-goya.c`, o controlador USB é registrado com `.mode = MV_USB_MODE_DEVICE` e `.extern_attr = MV_USB_HAS_VBUS_DETECTION` — a placa só **detecta** tensão no USB, não a **fornece**.
- **[CONFIRMADO]** O RT8973 chama `chip->pdata->otg_callback(1)` quando detecta o cabo OTG, mas a placa define `.otg_callback = NULL`. É exatamente o "OTG attached" que aparece no dmesg sem nenhum efeito depois.
- **[CONFIRMADO]** No driver do PMIC 88PM822, a struct `pm822_usb.vbus_gpio = PM822_NO_GPIO`, e o comentário do próprio driver diz: *"OTG5V not supported - Do nothing"*. É a explicação mais provável para o mouse USB não ligar.
- **[ABERTO]** O carregador SMB328 tem um registrador "OTG power and LDO" com um bit `CFG_OTG_ENABLE`, mas o driver só mexe nele em situações de carga/desligamento, não em resposta ao cabo OTG. Não está confirmado se esse bit alimenta algum circuito de 5 V utilizável.

### Confirmado nos 5 testes físicos (pendrive, mouse 2.4G, controle 2.4G, mouse USB, teclado USB)

Todos deram o mesmo resultado, e depois repetimos com um sexto dispositivo (nomeado "controlebluethoot2_4Ghz" no log) com captura mais detalhada do dmesg:

1. O RT8973 detecta o cabo corretamente: `Cable change to MUIC_RT8973_CABLE_TYPE_OTG`, `ID value = 0x0`, `otg = 1`. A contagem de IRQ do `rt8973` sobe.
2. **Nada mais muda**: os reguladores de tensão, o registrador do carregador SMB328 (lido via debugfs) e a lista de dispositivos USB **não mudam em nenhum teste**, confirmando que o software realmente nunca liga o 5 V (bate com o `otg_callback = NULL` do código).
3. Ao remover o cabo, o RT8973 volta a `ID value = 0x1f` ("sem cabo"), normalmente.
4. A única diferença notada num teste (nível de um pino `scl` de I²C) foi só o instante de leitura pegando o barramento I²C em uso — não é um sinal relacionado a OTG.

**Conclusão sobre USB/OTG:** a limitação é de **projeto de hardware/firmware da Samsung**, não uma falha ou desconhecimento nosso. O caminho de software (recompilar o kernel com `CONFIG_USB_EHCI_MV`, que existe na árvore) é viável, mas mesmo que o host funcione, o PMIC declara explicitamente que não tem linha de 5 V para isso. Um hub USB com **alimentação externa própria** contornaria a falta de 5 V, mas só depois de o host existir no software.

Consequência prática: dongles 2.4 GHz, mouse, teclado e pendrive USB não funcionam por causa de **uma só** limitação de origem (sem host USB, sem 5 V). Controle, mouse e teclado Bluetooth são um problema à parte. Por ora, o Bluetooth é o caminho mais realista para periféricos, e o espelhamento por USB fica dependente de resolver o host — e possivelmente de um hub USB energizado.

## 7. Energia

- **[CONFIRMADO]** Bateria Li-ion com fuel gauge funcionando, saúde "Good". Medidas: 3,72 V e 44% às 15:37.
- **[CONFIRMADO]** PMIC 88PM822 (I²C 0x30), codec 88PM805, carregador `sec-charger` (I²C 0x34) e MUIC RT8973 (I²C 0x14).
- **[ABERTO]** O comportamento de suspensão profunda e o consumo em espera **ainda não foram medidos de verdade**: o teste feito durou só 32 segundos (o ideal são 20+ minutos, sem carregador), então não deu para calcular consumo real. Repetir esse teste é a pendência mais simples da lista.

## 8. Software atual

- 127 pacotes instalados e 154 processos no snapshot de 15:37.
- Só 5 módulos de kernel carregados: `sd8xxx`, `mlan`, `citty`, `geu`, `galcore`.
- `/proc/config.gz` não existe, então a configuração do kernel não pode ser lida do sistema em execução.

## 9. Diferenças em relação ao documento v0.1

| Documento dizia | O que os logs e o código-fonte mostram |
|---|---|
| SoC PXA986 | Board diz PXA986, cpuinfo diz PXA988 |
| "Estudar ZRAM" | Já ativo de fábrica (~300 MB), com `swappiness=100` |
| Sem `/sys/class/usb` talvez signifique sem OTG | Confirmado pelo código: host USB desligado no defconfig e sem circuito de 5 V no PMIC. Não é uma incerteza, é uma decisão de projeto da Samsung |
| 1 GB de RAM | ~816 MiB visíveis ao sistema, e mais ~144 MB reservados à parte para GPU/vídeo |
| "ROM custom" (relato do usuário) | É a stock original (fingerprint idêntico) + root SuperSU/BusyBox instalado em 25/11/2025 |

## 10. Registro de testes

| Pasta de logs | Dispositivo | Resultado |
|---|---|---|
| `inventario_20260927_153706` | Inventário completo, sem periféricos | Base de todos os dados acima |
| `otg_bt_20260927_222135` | Pendrive | OTG detectado, sem host |
| `otg_bt_20260927_223125` | Mouse 2.4 GHz | OTG detectado, sem host |
| `otg_bt_20260927_223512` | Controle 2.4 GHz | OTG detectado, sem host |
| `otg_bt_20260927_224250` | Mouse USB | OTG detectado, sem host, mouse não liga |
| `otg_bt_20260927_224902` | Teclado USB | OTG detectado, sem host |
| `coleta02_20260928_223545` | "controlebluethoot2_4Ghz" (OTG) | Mesmo resultado, com dmesg mais detalhado |
| `coleta02_20260928_223107` / `224520` / `224651` / `224839` / `224904` | Hardware estático, partições, ROM, baseline | Ver seções 2, 3, 6, 8 e 9 |
| `coleta02_20260928_225051` | Bateria | Só 32 s de teste — refazer com 20+ min |
| Fontes: `SM-T110_NA_JB_Opensource.zip` | Kernel aberto (Samsung, 2016) | Ver seção 6 |

## 11. Decisões tomadas

- Base do sistema: **Android**. Linux (postmarketOS) só se houver segurança de que fica melhor. O kernel do pmOS para este aparelho é um fork do mesmo 3.4.5, então os drivers e a gestão de energia não melhoram automaticamente.
- O KOF será desenvolvido para rodar nativamente no Android do T110.
- Espelhamento: preferir USB (sem atraso), não necessariamente DeX. Depende de resolver o host USB.

## 12. Próximos passos sugeridos

1. **Capturar o log real de uma tentativa de pareamento Bluetooth** (script `t110_coleta_02.sh`, opção 3) — ainda não foi feito. Essa é a maior lacuna atual.
2. **Refazer o teste de bateria** com pelo menos 20 minutos sem carregador (script, opção 7), com tela ligada e depois com tela apagada, para ter uma linha de base real de consumo.
3. Investigar os parâmetros do governor `ondemand` (`up_threshold`, `sampling_rate`): a CPU ficou quase sempre em frequência máxima mesmo em repouso.
4. Ler o registrador do SMB328 e o código do `sec-charger` com mais profundidade para decidir se vale tentar (com cautela) habilitar o bit `CFG_OTG_ENABLE` como experimento controlado, ou se é melhor já partir para um hub USB energizado como solução prática.
5. Fazer benchmarks de leitura/escrita do microSD e do eMMC (ainda pendente).
6. Com o mapa de partições em mãos, planejar o **backup completo das 16 partições** antes de qualquer tentativa futura de flash.
