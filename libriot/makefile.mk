.PHONY: libriot libriot-build libriot-test

LIBRIOT_MAJOR		:= 0
LIBRIOT_MINOR		:= 1
LIBRIOT_PATCH		:= 0
LIBRIOT_VERSION		:= $(LIBRIOT_MAJOR).$(LIBRIOT_MINOR).$(LIBRIOT_PATCH)

LIBRIOT_FLAGS		:= \
			   $(CFLAGS)						\
			   $(CPPFLAGS)						\
			   -DLIBRIOT_VERSION_MAJOR="\"$(LIBRIOT_MAJOR)"\"	\
			   -DLIBRIOT_VERSION_MINOR="\"$(LIBRIOT_MINOR)"\"	\
			   -DLIBRIOT_VERSION_PATCH="\"$(LIBRIOT_PATCH)"\"	\
			   -DLIBRIOT_VERSION="\"$(LIBRIOT_VERSION)"\"		\
			   -Ilibriot/include					\
			   $(LDFLAGS)

LIBRIOT_SOURCES		:= libriot/src/io.c \
			   libriot/src/bin_reader.c \
			   libriot/src/bin_writer.c
LIBRIOT_OBJECTS		:= $(LIBRIOT_SOURCES:%.c=$(OBJ)/%.c.o)
LIBRIOT_OBJDEPS		:= $(LIBRIOT_OBJECTS:%.o=%.d)

-include $(LIBRIOT_OBJDEPS)

$(LIBRIOT_OBJECTS): $(OBJ)/%.c.o: %.c | $(OBJ)
	@mkdir -p $(shell dirname $@)
	$(CC) -MMD -o $@ -c $< $(LIBRIOT_FLAGS)

$(LIB)/libriot.$(LIBRIOT_VERSION).a: libriot-deps $(LIBRIOT_OBJECTS) | $(LIB)
	@mkdir -p $(shell dirname $@)
	$(AR) -rcs $@ $(wordlist 2,$(words $^),$^)

$(LIB)/libriot.$(LIBRIOT_MAJOR).a: $(LIB)/libriot.$(LIBRIOT_VERSION).a | $(LIB)
	ln -sf $(notdir $<) $@

$(LIB)/libriot.a: $(LIB)/libriot.$(LIBRIOT_MAJOR).a | $(LIB)
	ln -sf $(notdir $<) $@

libriot-deps:

libriot-build: $(LIB)/libriot.a

libriot-test-deps:

libriot-test:

libriot: libriot-build libriot-test
