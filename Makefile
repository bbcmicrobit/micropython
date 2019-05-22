ECHO = @echo

HEX_SRC = build/bbc-microbit-classic-gcc-nosd/source/microbit-micropython.hex
HEX_FINAL = build/firmware.hex
MBIT_VER_FILE = inc/genhdr/microbitversion.h
VER_ADDR_FILE = build/veraddr.txt

all: $(HEX_FINAL)

# Anything that depends on FORCE will be considered out-of-date
FORCE:
.PHONY: FORCE

$(HEX_FINAL): yotta $(VER_ADDR_FILE)
	tools/adduicr.py $(HEX_SRC) $$(cat $(VER_ADDR_FILE)) -o $(HEX_FINAL)
	@size $(HEX_SRC:.hex=)

yotta: $(MBIT_VER_FILE)
	@yt build

$(MBIT_VER_FILE): FORCE
	python tools/makeversionhdr.py $(MBIT_VER_FILE)

$(VER_ADDR_FILE): yotta
	@echo -n "0x" > $(VER_ADDR_FILE)
	@objdump -x $(HEX_SRC:.hex=) | grep microbit_version_string | cut -f 1 -d' ' >> $(VER_ADDR_FILE)

deploy: $(HEX_FINAL)
	$(ECHO) "Deploying $<"
	@mount /dev/sdb
	@sleep 1s
	@cp $< /mnt/
	@sleep 1s
	@umount /mnt

serial:
	@picocom /dev/ttyACM0 -b 115200
