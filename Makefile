CFLAGS = -Wall -o
CC = g++

morseDecoder: morseDecoder2.cpp
	$(CC) $(CFLAGS) morseDecoder2 morseDecoder2.cpp

.PHONY clean:
	rm -f *.o
	rm -f morseDecoder2
