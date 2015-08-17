ECHO = @echo
PYTHON = python

QSTR_DEFS = inc/py/qstrdefs.h inc/microbit/qstrdefsport.h

HEX_SRC = build/bbc-microbit-classic-gcc-nosd/source/microbit-micropython.hex
HEX_FINAL = build/firmware.hex

all: yotta

yotta: inc/genhdr/qstrdefs.generated.h
	@yt build
	@/bin/cp $(HEX_SRC) $(HEX_FINAL)

# Note: we need to protect the qstr names from the preprocessor, so we wrap
# the lines in "" and then unwrap after the preprocessor is finished.
inc/genhdr/qstrdefs.generated.h: $(QSTR_DEFS) tools/makeqstrdata.py inc/microbit/mpconfigport.h inc/py/mpconfig.h
	$(ECHO) "Generating $@"
	@cat $(QSTR_DEFS) | sed 's/^Q(.*)/"&"/' | $(CPP) -Iinc -Iinc/microbit - | sed 's/^"\(Q(.*)\)"/\1/' > build/qstrdefs.preprocessed.h
	@$(PYTHON) tools/makeqstrdata.py build/qstrdefs.preprocessed.h > $@

deploy: $(HEX_FINAL)
	$(ECHO) "Deploying $<"
	@mount /dev/sdb
	@sleep 1s
	@cp $< /mnt/
	@sleep 1s
	@umount /mnt

serial:
	@picocom /dev/ttyACM0 -b 115200
