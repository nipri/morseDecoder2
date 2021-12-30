#ifndef MORSECODEDECODER_H_
#define MORSECODEDECODER_H_

#define DIT 	0xd1
#define DAH	0xda
#define END	0xa5

unsigned int morseCodeTable[26][8] = 
		{
		{DIT, DAH, END, 'a'},
		{DAH, DIT, DIT, DIT, END, 'b'},
		{DAH, DIT, DAH, DIT, END, 'c'},
		{DAH, DIT, DIT, END, 'd'},
		{DIT, END, 'e'},
		{DIT, DIT, DAH, DIT, END, 'f'},
		{DAH, DAH, DIT, END, 'g'},
		{DIT, DIT, DIT, DIT, END, 'h'},
		{DIT, DIT, END, 'i'},
		{DIT, DAH, DAH, DAH, END, 'j'},
		{DAH, DIT, DAH, END, 'k'},
		{DIT, DAH, DIT, DIT, END, 'l'},
		{DAH, DAH, END, 'm'},
		{DAH, DIT, END, 'n'},
		{DAH, DAH, DAH, END, 'o'},
		{DIT, DAH, DAH, DIT, END, 'p'},
		{DAH, DAH, DIT, DAH, END, 'q'},
		{DIT, DAH, DIT, END, 'r'},
		{DIT, DIT, DIT, END, 's'},
		{DAH, END, 't'},
		{DIT, DIT, DAH, END, 'u'},
		{DIT, DIT, DIT, DAH, END, 'v'},
		{DIT, DAH, DAH, END, 'w'},
		{DAH, DIT, DIT, DAH, END, 'x'},
		{DAH, DIT, DAH, DAH, END, 'y'},
		{DAH, DAH, DIT, DIT, END, 'z'}
		};
		
		 




#endif 
