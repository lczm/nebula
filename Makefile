CC = gcc
CCFLAGS = -g -Wall -Wextra -Wpedantic -Wfloat-equal -Wno-unused-function -O0
# CCFLAGS = -g -Wall -Wextra -Wpedantic -Wfloat-equal -Wno-unused-function -O0 -DDEBUGGING

SOURCE_DIR := .
BUILD_DIR := ./build

HEADERS := $(wildcard $(SOURCE_DIR)/*.h)
SOURCES := $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))
OBJECTS_NEBULA := $(filter-out $(BUILD_DIR)/test.o, $(OBJECTS))
OBJECTS_TEST   := $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS))

.PHONY: all nebula clean

all: nebula

# link interpreter
nebula: $(OBJECTS_NEBULA)
	@ printf "%8s %-40s %s\n" $(CC) $@ "$(CCFLAGS)"
	@ $(CC) $(CCFLAGS) $^ -o $@

test: $(OBJECTS_TEST)
	@ printf "%8s %-40s %s\n" $(CC) $@ "$(CCFLAGS)"
	@ $(CC) $(CCFLAGS) $^ -o $@
	@ ./test

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CCFLAGS)"
	@ mkdir -p $(BUILD_DIR)
	@ $(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BUILD_DIR) nebula test
