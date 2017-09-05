ECHO = @echo

HEX_SRC = build/bbc-microbit-classic-gcc-nosd/source/microbit-micropython.hex
HEX_FINAL = build/firmware.hex

all: yotta

yotta:
	@yt build
	@size $(HEX_SRC:.hex=)
	@/bin/cp $(HEX_SRC) $(HEX_FINAL)

deploy: $(HEX_FINAL)
	$(ECHO) "Deploying $<"
	@mount /dev/sdb
	@sleep 1s
	@cp $< /mnt/
	@sleep 1s
	@umount /mnt

serial:
	@picocom /dev/ttyACM0 -b 115200
