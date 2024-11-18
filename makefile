# Compiler and flags
CC = g++
CFLAGS = -Wall -g

# Directories
SRC_DIR = .
FO_DIR = flexoffers
AGG_DIR = aggregation
EVAL_DIR = evaluation
SFO_DIR = scheduled_flexoffer

# Output executable
TARGET = output

# Source files
SRCS = $(SRC_DIR)/main.cpp \
       $(FO_DIR)/flexoffer.cpp \
       $(AGG_DIR)/aggregation.cpp \
       $(EVAL_DIR)/evaluation.cpp \
       $(SFO_DIR)/scheduled_flexoffer.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Header files for dependencies
HEADERS = $(FO_DIR)/flexoffer.h \
          $(AGG_DIR)/aggregation.h \
          $(EVAL_DIR)/evaluation.h \
          $(SFO_DIR)/scheduled_flexoffer.h

# Default target to build the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Pattern rule to compile object files
%.o: %.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
