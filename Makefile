CC       ?= gcc
CFLAGS   ?=
CFLAGS   += -std=c99 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS  ?=
LDLIBS   ?=

DEBUG    ?= 0
ifeq ($(DEBUG), 1)
  BUILD_DIR := build/debug
  TARGET    := mitm-c-debug
  CFLAGS    += -g -O0 -DDEBUG
else
  BUILD_DIR := build/release
  TARGET    := mitm-c
  CFLAGS    += -O2 -DNDEBUG
endif

BIN_DIR  := $(BUILD_DIR)/bin
OBJ_DIR  := $(BUILD_DIR)/obj
SRC_DIR  := src

ENABLE_UDP ?= 0
ENABLE_TLS ?= 0

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

ifeq ($(ENABLE_UDP), 1)
  CFLAGS    += -DENABLE_UDP
  SRC_FILES += $(SRC_DIR)/udp_module.c
endif

ifeq ($(ENABLE_TLS), 1)
  CFLAGS    += -DENABLE_TLS
  SRC_FILES += $(SRC_DIR)/tls_module.c
  LDLIBS    += -lssl -lcrypto
endif

OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

.PHONY: all debug release clean run

all: $(BIN_DIR)/$(TARGET)

debug:
	$(MAKE) DEBUG=1 all

release:
	$(MAKE) DEBUG=0 all

run: all
	$(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

-include $(OBJ_FILES:.o=.d)

clean:
	rm -rf build/
