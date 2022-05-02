all: release

TARGET_DIR = build
INSTALL_DIR = `pwd`/install

build:
	meson -Db_sanitize=address,undefined $(TARGET_DIR) --buildtype=debug --prefix=$(INSTALL_DIR)
	ninja -C $(TARGET_DIR)

release:
	# Disable asserts
	meson $(TARGET_DIR) -Db_ndebug=true --buildtype=release --prefix=$(INSTALL_DIR)
	ninja -C $(TARGET_DIR)
	cd $(TARGET_DIR) && ninja install

debug:
	if [ ! -d $(TARGET_DIR) ]; then mkdir $(TARGET_DIR); fi
	meson -Db_sanitize=address,undefined $(TARGET_DIR) --buildtype=debug --prefix=$(INSTALL_DIR)
	ninja -C $(TARGET_DIR)
	cd $(TARGET_DIR) && ninja install

install:
	meson $(TARGET_DIR) --prefix=$(INSTALL_DIR)
	ninja -C $(TARGET_DIR)
	cd $(TARGET_DIR) && ninja install

clean:
	rm -rf $(TARGET_DIR)
	rm -rf $(INSTALL_DIR)

clean-all:
	rm -rf $(TARGET_DIR)
	rm -rf $(INSTALL_DIR)
	rm -rf ./subprojects/freeDiameter
	rm -rf ./subprojects/usrsctp

webui-clean:
	rm -rf ./webui/node_modules/*
	rm -rf ./webui/.next

.PHONY: release debug install clean clean-all webui-clean
