CC       ?= gcc
CFLAGS   ?=
INCLUDE_DIR := include
CFLAGS   += -std=c99 -Wall -Wextra -I$(INCLUDE_DIR) -MMD -MP
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
SRC_DIR := src
MODULES_DIR := modules
CONFIG_DIR := config

SRC_FILES := \
    $(wildcard $(SRC_DIR)/*.c) \
    $(wildcard $(MODULES_DIR)/*.c)

FORMAT_FILES := $(shell find src include modules -name '*.c' -o -name '*.h')
LINT_FILES := $(shell find src modules -name '*.c')
TIDY_FLAGS := -std=c99 -I$(INCLUDE_DIR)

ifeq ($(ENABLE_UDP), 1)
  SRC_FILES += $(SRC_DIR)/udp_module.c
  CFLAGS    += -DENABLE_UDP
endif

ifeq ($(ENABLE_TLS), 1)
  SRC_FILES += $(SRC_DIR)/tls_module.c
  CFLAGS    += -DENABLE_TLS
  LDLIBS    += -lssl -lcrypto
endif

LDLIBS    += -pthread
OBJ_FILES := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_FILES)))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(MODULES_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all debug release clean run

.PHONY: format lint cppcheck

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

format:
	clang-format -i $(FORMAT_FILES)

lint:
	clang-tidy $(LINT_FILES) -extra-arg=-w -- $(TIDY_FLAGS)

cppcheck:
	cppcheck --enable=warning,performance,portability --std=c99 --error-exitcode=1 --suppress=missingIncludeSystem -I$(INCLUDE_DIR) src modules
