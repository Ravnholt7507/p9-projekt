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


# ============================
# CPLEX Configuration
# ============================

# Allow CPLEX_DIR to be set via environment variable or passed as a make argument
CPLEX_DIR ?= $(CPLEX_STUDIO_DIR2211)

# CPLEX Include Directories
CPLEX_CONCERT_INCLUDE := $(CPLEX_DIR)/concert/include
CPLEX_CPLEX_INCLUDE   := $(CPLEX_DIR)/cplex/include

# Add CPLEX includes to INCLUDE
INCLUDE += -I$(CPLEX_CONCERT_INCLUDE) -I$(CPLEX_CPLEX_INCLUDE)

# CPLEX Library Directories
CPLEX_CONCERT_LIB := $(CPLEX_DIR)/concert/lib/x86-64_linux/static_pic
CPLEX_CPLEX_LIB   := $(CPLEX_DIR)/cplex/lib/x86-64_linux/static_pic

# Linker Flags
LDFLAGS = -L$(CPLEX_CONCERT_LIB) -L$(CPLEX_CPLEX_LIB) \
          -lconcert -lilocplex -lcplex -ldl -lpthread -lm

# ============================
# End of CPLEX Configuration
# ============================

.PHONY: all clean

all: $(OBJ)

# Ensure obj dir exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) -o $(OBJ) $(LDFLAGS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CC) -c $< $(CFLAGS) $(INCLUDE) -o $@

# Include generated dependency files
-include $(DEPEND_FILES)

clean:
	rm -f $(OBJ_DIR)*.o
	rm -f $(OBJ_DIR)*.d
	rmdir $(OBJ_DIR) || true
	rm -f $(OBJ)