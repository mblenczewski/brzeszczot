## toolchain
CC		?= cc
CXX		?= c++
AR		?= ar
RANLIB		?= ranlib

STRIP		?= strip

TAR		?= tar
ZIP		?= gzip

## project build directories
BIN		:= bin
OBJ		:= obj
LIB		:= lib
TST		:= $(BIN)/tests

$(BIN):
	@mkdir -p $(BIN)

$(OBJ):
	@mkdir -p $(OBJ)

$(LIB):
	@mkdir -p $(LIB)

$(TST): $(BIN)
	@mkdir -p $(TST)

## project-wide includes
INC		:= include

## append project-wide toolchain flags
CFLAGS		:= $(CFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS	:= $(CPPFLAGS) -I$(INC) -D_XOPEN_SOURCE=700
LDFLAGS		:= $(LDFLAGS) -L$(LIB)

REGIME		?= DEBUG

DEBUG_CFLAGS	:= $(DEBUG_CFLAGS) -Og -ggdb
DEBUG_CPPFLAGS	:= $(DEBUG_CPPFLAGS) -UNDEBUG
DEBUG_LDFLAGS	:= $(DEBUG_LDFLAGS)

RELEASE_CFLAGS	:= $(RELEASE_CFLAGS) -O3 -flto
RELEASE_CPPFLAGS:= $(RELEASE_CPPFLAGS) -DNDEBUG
RELEASE_LDFLAGS	:= $(RELEASE_LDFLAGS) -flto

## append regime-specific toolchain flags
CFLAGS		:= $(CFLAGS) $($(REGIME)_CFLAGS)
CPPFLAGS	:= $(CPPFLAGS) $($(REGIME)_CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) $($(REGIME)_LDFLAGS)
