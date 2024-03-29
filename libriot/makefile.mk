.PHONY: libriot libriot-build libriot-test

LIBRIOT_MAJOR	:= 0
LIBRIOT_MINOR	:= 1
LIBRIOT_PATCH	:= 0
LIBRIOT_VERSION	:= $(LIBRIOT_MAJOR).$(LIBRIOT_MINOR).$(LIBRIOT_PATCH)

LIBRIOT_CFLAGS	:= \
		   $(CFLAGS) \
		   $(CPPFLAGS) \
		   -DLIBRIOT_VERSION_MAJOR="\"$(LIBRIOT_MAJOR)"\" \
		   -DLIBRIOT_VERSION_MINOR="\"$(LIBRIOT_MINOR)"\" \
		   -DLIBRIOT_VERSION_PATCH="\"$(LIBRIOT_PATCH)"\" \
		   -DLIBRIOT_VERSION="\"$(LIBRIOT_VERSION)"\" \
		   -Ilibriot/include

LIBRIOT_FLAGS	:= \
		   $(LIBRIOT_CFLAGS) \
		   $(LDFLAGS)

LIBRIOT_SOURCES	:= libriot/src/libriot.c \
		   libriot/src/utils.c \
		   libriot/src/wad.c \
		   libriot/src/wad_reader.c \
		   libriot/src/wad_writer.c \
		   libriot/src/wad_printer.c \
		   libriot/src/inibin.c \
		   libriot/src/inibin_reader.c \
		   libriot/src/inibin_writer.c \
		   libriot/src/inibin_printer.c

LIBRIOT_OBJECTS	:= $(LIBRIOT_SOURCES:%.c=$(OBJ)/%.c.o)
LIBRIOT_OBJDEPS	:= $(LIBRIOT_OBJECTS:%.o=%.d)

-include $(LIBRIOT_OBJDEPS)

$(LIBRIOT_OBJECTS): $(OBJ)/%.c.o: %.c | $(OBJ)
	@mkdir -p $(shell dirname $@)
	$(CC) -MMD -o $@ -c $< $(LIBRIOT_CFLAGS)

$(LIB)/libriot.a: libriot-deps $(LIBRIOT_OBJECTS) | $(LIB)
	@mkdir -p $(shell dirname $@)
	$(AR) -rcs $@ $(wordlist 2,$(words $^),$^)

libriot-deps:

libriot-build: $(LIB)/libriot.a

libriot-test-deps:

libriot-test:

libriot: libriot-build libriot-test
