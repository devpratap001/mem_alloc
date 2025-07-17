CC := gcc
CFLAGS := -g -Wall -std=c99 -MMD -MP
BUILD := ./build
DEP_DIR := $(BUILD)/deps
OBJ_DIR := $(BUILD)/objs
SRCDIR = ./src
INCLUDE := ./include
TARGET := $(BUILD)/mem_alloc
SRCS := $(filter $(SRCDIR)/%.c, $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*.cpp))
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS  := $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(OBJS))

-include $(DEPS)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS): $(OBJ_DIR)/%.o : $(SRCDIR)/%.c | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -c -MF $(DEP_DIR)/$*.d -o $@ $<

$(BUILD):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(DEP_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -r $(TARGET) $(DEPS) $(OBJS)