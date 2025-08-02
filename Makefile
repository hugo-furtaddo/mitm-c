CC       ?= gcc
CFLAGS   ?= -std=c99 -Wall -Wextra -Iinclude
LDFLAGS  ?=
LDLIBS   ?=

DEBUG ?= 0
ifeq ($(DEBUG), 1)
  BUILD_DIR := build/debug
  TARGET    := mitm-c-debug
  CFLAGS    += -g -O0 -DDEBUG
else
  BUILD_DIR := build/release
  TARGET    := mitm-c
  CFLAGS    += -O2 -DNDEBUG
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

SRC_DIR   := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

ENABLE_UDP ?= 0
ENABLE_TLS ?= 0
ifeq ($(ENABLE_UDP), 1)
  CFLAGS    += -DENABLE_UDP
  SRC_FILES += $(SRC_DIR)/udp_module.c
endif
ifeq ($(ENABLE_TLS), 1)
  CFLAGS    += -DENABLE_TLS
  SRC_FILES += $(SRC_DIR)/tls_module.c
  # LDLIBS   += -lssl -lcrypto
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Alvos padrão
.PHONY: all debug release clean
all: $(BUILD_DIR)/$(TARGET)

debug: clean
	@$(MAKE) DEBUG=1 all

release: clean
	@$(MAKE) DEBUG=0 all

clean:
	rm -rf build/*
