.PHONY: all build clean test

all: build test

include config.mk

clean:
	rm -fr $(BIN) $(LIB) $(OBJ)

build: libriot-build brzeszczot-build

test: libriot-test brzeszczot-test

include libriot/makefile.mk
include brzeszczot/makefile.mk
