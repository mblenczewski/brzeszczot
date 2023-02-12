.PHONY: brzeszczot brzeszczot-build brzeszczot-test

BRZESZCZOT_MAJOR	:= 0
BRZESZCZOT_MINOR	:= 1
BRZESZCZOT_PATCH	:= 0
BRZESZCZOT_VERSION	:= $(BRZESZCZOT_MAJOR).$(BRZESZCZOT_MINOR).$(BRZESZCZOT_PATCH)

BRZESZCZOT_CFLAGS	:= \
			   $(CFLAGS) \
			   $(CPPFLAGS) \
			   -DBRZESZCZOT_VERSION_MAJOR="\"$(BRZESZCZOT_MAJOR)"\" \
			   -DBRZESZCZOT_VERSION_MINOR="\"$(BRZESZCZOT_MINOR)"\" \
			   -DBRZESZCZOT_VERSION_PATCH="\"$(BRZESZCZOT_PATCH)"\" \
			   -DBRZESZCZOT_VERSION="\"$(BRZESZCZOT_VERSION)"\" \
			   -Ibrzeszczot/include -Ilibriot/include

BRZESZCZOT_FLAGS	:= \
			   $(BRZESZCZOT_CFLAGS) \
			   $(LDFLAGS) -lriot

BRZESZCZOT_SOURCES	:= brzeszczot/src/brzeszczot.c \
			   brzeszczot/src/argparse.c

BRZESZCZOT_OBJECTS	:= $(BRZESZCZOT_SOURCES:%.c=$(OBJ)/%.c.o)
BRZESZCZOT_OBJDEPS	:= $(BRZESZCZOT_OBJECTS:%.o=%.d)

-include $(BRZESZCZOT_OBJDEPS)

$(BRZESZCZOT_OBJECTS): $(OBJ)/%.c.o: %.c | $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) -MMD -o $@ -c $< $(BRZESZCZOT_CFLAGS)

$(BIN)/brzeszczot: brzeszczot-deps $(BRZESZCZOT_OBJECTS) | $(BIN)
	@mkdir -p $(dir $@)
	$(CC) -static -o $@ $(wordlist 2,$(words $^),$^) $(BRZESZCZOT_FLAGS)

brzeszczot-deps: $(LIB)/libriot.a

brzeszczot-build: $(BIN)/brzeszczot

brzeszczot-test-deps:

brzeszczot-test:

brzeszczot: brzeszczot-build brzeszczot-test
