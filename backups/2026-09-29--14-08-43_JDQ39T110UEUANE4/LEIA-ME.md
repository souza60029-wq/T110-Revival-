# Backup TWRP — 2026-09-29--14-08-43_JDQ39T110UEUANE4

Arquivo recebido: `2026-09-29--14-08-43_JDQ39T110UEUANE4.zip`.

## Partições incluídas

- `system` (`system.ext4.win`), em versão sanitizada.

## Integridade

Antes da sanitização, os MD5 das duas imagens originais conferiram (**2/2**). Como a imagem `system` foi alterada para remover configurações potencialmente identificadoras, seu MD5 foi recalculado; o novo `.md5` foi verificado contra o arquivo incluído (**1/1**).

## Cópia publicada e exclusões

O arquivo `2026-09-29--14-08-43_JDQ39T110UEUANE4_SANITIZADO_PARCIAL.zip` contém apenas a imagem `system` filtrada. A partição `user/data` (`data.ext4.win`) foi excluída por inteiro; ela contém estado de contas/apps e arquivos pessoais. Da imagem `system` foram removidos `bin/wpa_supplicant`, `etc/secure_storage/.system.bin.wpa_supplicant/ss_id`, `etc/wifi/wpa_supplicant.conf` e `etc/bluetooth/iop_bt.db`. Nos arquivos de configuração de texto, foram mascarados 1.064 endereços IPv4, 14 e-mails e uma sequência numérica longa. O MD5 original do `system` conferia antes da sanitização; o novo MD5 do arquivo alterado foi conferido.

O ZIP original completo permanece com o usuário. Esta cópia é parcial, não deve ser tratada como backup restaurável integral e não inclui `user/data`. Componentes binários e certificados do Android stock ainda podem conter strings técnicas genéricas; a sanitização não é uma certificação de que cada byte do firmware está livre de qualquer identificador. O ZIP sanitizado foi publicado via Git LFS.
