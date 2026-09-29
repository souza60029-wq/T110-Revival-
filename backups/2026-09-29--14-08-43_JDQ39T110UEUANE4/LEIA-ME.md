# Backup TWRP — 2026-09-29--14-08-43_JDQ39T110UEUANE4

Arquivo recebido: `2026-09-29--14-08-43_JDQ39T110UEUANE4.zip`.

## Partições incluídas

- `system` (`system.ext4.win`), em versão sanitizada.

## Integridade

Antes da sanitização, os MD5 das duas imagens originais conferiram (**2/2**). Como a imagem `system` foi alterada para remover configurações potencialmente identificadoras, seu MD5 foi recalculado; o novo `.md5` foi verificado contra o arquivo incluído (**1/1**).

## Cópia publicada e exclusões

O arquivo `2026-09-29--14-08-43_JDQ39T110UEUANE4_SANITIZADO_PARCIAL.zip` contém apenas a imagem `system` filtrada. A partição `user/data` (`data.ext4.win`) foi excluída por inteiro; ela contém estado de contas/apps e arquivos pessoais. Também foram removidos da imagem `system` arquivos de identidade/configuração de Wi‑Fi e MAC; valores identificadores em arquivos de configuração de texto foram mascarados. O MD5 original do `system` conferia antes da sanitização.

O ZIP original completo permanece com o usuário. Esta cópia é parcial, não deve ser tratada como backup restaurável integral e não inclui `user/data`. Por exceder 100 MiB, o ZIP sanitizado será armazenado via Git LFS se a quota da conta permitir.
