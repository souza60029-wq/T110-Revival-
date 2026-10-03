GOYA_SHELL_VERSION = 1.0
GOYA_SHELL_SITE = $(BR2_EXTERNAL_GOYAOS_V2_PATH)/package/goya-shell/src
GOYA_SHELL_SITE_METHOD = local
GOYA_SHELL_DEPENDENCIES = lvgl

GOYA_SHELL_CFLAGS = $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include

define GOYA_SHELL_BUILD_CMDS
	$(TARGET_CC) $(GOYA_SHELL_CFLAGS) -c $(@D)/main.c -o $(@D)/main.o
	$(TARGET_CC) $(TARGET_LDFLAGS) -o $(@D)/goya-shell $(@D)/main.o \
		-L$(STAGING_DIR)/usr/lib -Wl,-Bstatic -llvgl -Wl,-Bdynamic -lm
endef

define GOYA_SHELL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/goya-shell $(TARGET_DIR)/usr/bin/goya-shell
endef

$(eval $(generic-package))
