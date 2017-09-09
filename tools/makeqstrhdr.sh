#!/bin/sh
#
# This script generates the qstrdefs.generated.h file.  You'll need to run
# it if you change any code with "MP_QSTR_xxx" in it, or if you get an
# error from the compiler about undefined MP_QSTR_xxx constants.
#
# Run it from the root directory, like this:
#
# $ ./tools/makeqstrhdr.sh

MKDIR=mkdir
CAT=cat
SED=sed
PYTHON=python

INC="-I. -Iinc -Iinc/lib -Iinc/microbit -Isource -Iyotta_modules/microbit-dal/inc/platform -Iyotta_modules/microbit-dal/inc/types -Iyotta_modules/microbit-dal/inc/core -Iyotta_modules/microbit-dal/inc/drivers -Iyotta_modules/mbed-classic/hal -Iyotta_modules/mbed-classic/targets/cmsis -Iyotta_modules/mbed-classic/api -Iyotta_modules/nrf51-sdk/source/nordic_sdk/components/device -Iyotta_modules/nrf51-sdk/source/nordic_sdk/components/drivers_nrf/hal -I./yotta_modules/mbed-classic/targets/cmsis/TARGET_NORDIC/TARGET_MCU_NRF51822 -Iyotta_modules/mbed-classic/targets/hal/TARGET_NORDIC/TARGET_MCU_NRF51822 -I./yotta_modules/mbed-classic/targets/hal/TARGET_NORDIC/TARGET_MCU_NRF51822/TARGET_NRF51_MICROBIT -Iyotta_modules/ble"
DEF="-DNRF51 -DYOTTA_BUILD_INFO_HEADER=<build/bbc-microbit-classic-gcc-nosd/yotta_build_info.h>"
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++
CFLAGS="$INC $DEF -Wall -Wpointer-arith -Werror -std=c99 -nostdlib"
CXXFLAGS="$INC $DEF -Wall"

TOP=source
GENHDRDIR=./inc/genhdr
SRC_C=$(find source -name '*.c')
SRC_CXX=$(find source -name '*.cpp')
QSTR_EXTRA="inc/py/qstrdefs.h inc/microbit/qstrdefsport.h"

mkdir -p $GENHDRDIR

echo "Preprocessing the source files"
$CC -E -DNO_QSTR $CFLAGS $SRC_C > $GENHDRDIR/qstr.i.last || exit 1
$CXX -E -DNO_QSTR $CXXFLAGS $SRC_CXX >> $GENHDRDIR/qstr.i.last || exit 1

echo "Extracting all qstrs"
$PYTHON $TOP/py/makeqstrdefs.py split $GENHDRDIR/qstr.i.last $GENHDRDIR/qstr $GENHDRDIR/qstrdefs.collected.h || exit 1
$PYTHON $TOP/py/makeqstrdefs.py cat $GENHDRDIR/qstr.i.last $GENHDRDIR/qstr $GENHDRDIR/qstrdefs.collected.h || exit 1

echo "Generating qstr header"
$CAT $QSTR_EXTRA $GENHDRDIR/qstrdefs.collected.h | $SED 's/^Q(.*)/"&"/' | $CC -E $CFLAGS - | $SED 's/^"\(Q(.*)\)"/\1/' > $GENHDRDIR/qstrdefs.preprocessed.h || exit 1
$PYTHON $TOP/py/makeqstrdata.py $GENHDRDIR/qstrdefs.preprocessed.h > $GENHDRDIR/qstrdefs.generated.h || exit 1
