CC = g++
CFLAGS = -Wall -g

# Target to build the final executable
main: main.o flexoffers/flexoffer.o aggregation/aggregation.o
	$(CC) $(CFLAGS) -o main main.o flexoffers/flexoffer.o aggregation/aggregation.o

# Rule to compile main.o
main.o: main.cpp flexoffers/flexoffer.h aggregation/aggregation.h
	$(CC) $(CFLAGS) -c main.cpp -o main.o 

# Rule to compile flexoffer.o
flexoffers/flexoffer.o: flexoffers/flexoffer.cpp flexoffers/flexoffer.h
	$(CC) $(CFLAGS) -c flexoffers/flexoffer.cpp -o flexoffers/flexoffer.o

# Rule to compile aggregation.o
aggregation/aggregation.o: aggregation/aggregation.cpp aggregation/aggregation.h
	$(CC) $(CFLAGS) -c aggregation/aggregation.cpp -o aggregation/aggregation.o

# Clean up build files
clean:
	rm -f main main.o flexoffers/flexoffer.o aggregation/aggregation.o

