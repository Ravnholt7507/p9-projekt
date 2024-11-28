# Binary File
OBJ := output

# Source Files
SRC_DIR := src/
SOURCE_FILES := $(wildcard $(SRC_DIR)*.cpp)

# Object Files
OBJ_DIR := bin/
OBJECT_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SOURCE_FILES))
DEPEND_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.d,$(SOURCE_FILES))

# Header Files
HEADER_DIR := include/
INCLUDE := -I$(HEADER_DIR)


# Compiler
CC := g++

# Compilation Flags
CFLAGS = -Wall -g -MMD -MP

.PHONY: all clean

all: $(OBJ)

# Ensure obj dir exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) -o $(OBJ)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CC) -c $< $(CFLAGS) $(INCLUDE) -o $@

# Include generated dependency files
-include $(DEPEND_FILES)

clean:
	rm -f $(OBJ_DIR)*.o
	rm -f $(OBJ_DIR)*.d
	rmdir $(OBJ_DIR)
	rm -f $(OBJ)
