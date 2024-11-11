# Compiler and flags
CC = g++
CFLAGS = -Wall -g

# Target to build the final executable
output: main.o flexoffers/flexoffer.o aggregation/aggregation.o evaluation/evaluation.o scheduled_flexoffer/scheduled_flexoffer.o
	$(CC) $(CFLAGS) -o output main.o flexoffers/flexoffer.o aggregation/aggregation.o evaluation/evaluation.o scheduled_flexoffer/scheduled_flexoffer.o

# Rule to compile main.o
main.o: main.cpp flexoffers/flexoffer.h aggregation/aggregation.h evaluation/evaluation.h scheduled_flexoffer/scheduled_flexoffer.h
	$(CC) $(CFLAGS) -c main.cpp -o main.o 

# Rule to compile flexoffer.o
flexoffers/flexoffer.o: flexoffers/flexoffer.cpp flexoffers/flexoffer.h
	$(CC) $(CFLAGS) -c flexoffers/flexoffer.cpp -o flexoffers/flexoffer.o
 
# Rule to compile aggregation.o
aggregation/aggregation.o: aggregation/aggregation.cpp aggregation/aggregation.h
	$(CC) $(CFLAGS) -c aggregation/aggregation.cpp -o aggregation/aggregation.o

# Rule to compile evaluation.o
evaluation/evaluation.o: evaluation/evaluation.cpp evaluation/evaluation.h
	$(CC) $(CFLAGS) -c evaluation/evaluation.cpp -o evaluation/evaluation.o

# Rule to compile scheduled_flexoffer
scheduled_flexoffer/scheduled_flexoffer.o: scheduled_flexoffer/scheduled_flexoffer.cpp scheduled_flexoffer/scheduled_flexoffer.h
	$(CC) $(CFLAGS) -c scheduled_flexoffer/scheduled_flexoffer.cpp -o scheduled_flexoffer/scheduled_flexoffer.o


# Clean up build files
.PHONY: clean
clean:
	rm -f output main.o flexoffers/flexoffer.o aggregation/aggregation.o evaluation/evaluation.o scheduled_flexoffer/scheduled_flexoffer.o

