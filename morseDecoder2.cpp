#include <iostream>
#include <fstream>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <errno.h>

#include "morseCodeDecoder.h"

#define INTERVAL	1 // mS interval to execute timerProc()
//#define INTERVAL	100
#define GPIO		17

struct timespec timer;
bool gotChar, rxChar, done, init, gotWord;
long long int highTime, lowTime, currentTime;
unsigned long long int lastHandledTime;

double wpm = 5;

static double timeUnit = 240; // 5 WPM
//static double timeUnit = 60; // 20 WPM


unsigned int charArray[8], elementIndex;
unsigned int gpioValue;
char buffer[64];


FILE *fp;


void timerProc(void);
void getTimeInterval(void);
void lookUpChar(void);
unsigned int readGPIO(unsigned int);


void timerProc(void)
{
	if (init)
	{
		init = false;
        	std::cout << "TIMER!!" << std::endl;
		printf("\r\n\r\n");
	}

	gpioValue =  readGPIO(GPIO);

	if (gpioValue == 1)
	{
		rxChar = true;
		gotWord = true;
		highTime++;
		lowTime = 0;
	}

	else if (gpioValue == 0)
	{
		lowTime++;

		if (lowTime >= timeUnit*7) // We havent received anything yet or  
		{			   // may just be in-between words
			lowTime = 0;

			if (gotWord)
			{
				gotWord = false;
				printf(" ");
			}
		}
	}

	else
	{
	}

	if ( rxChar && (gpioValue == 0) && (highTime > 0)) // We have a dir or a dah
	{

		if (highTime < timeUnit*3)
		{
			charArray[elementIndex] = DIT;
			elementIndex++;
		}

		else if ( (highTime >= timeUnit*3) && (highTime < timeUnit*4) )
		{
			charArray[elementIndex] = DAH;
			elementIndex++;
		}

		else
		{
		}

		highTime = 0;
	}

	if (rxChar && (gpioValue == 0) && (lowTime > timeUnit*4)) // We received a complete character
	{

		charArray[elementIndex] = END;
		rxChar = false;
		elementIndex = 0;
		lowTime = 0;
		highTime = 0;
		lookUpChar();
	}
}

void getTimeInterval(void)
{

	clock_gettime(CLOCK_MONOTONIC, &timer);
        currentTime = (timer.tv_sec * 1000) + (timer.tv_nsec / 1000000);

	if ( (currentTime - lastHandledTime) >= INTERVAL) 
	{
		timerProc();
	        clock_gettime(CLOCK_MONOTONIC, &timer);
	        lastHandledTime = (timer.tv_sec * 1000) + (timer.tv_nsec / 1000000);
	}
}


unsigned int readGPIO(unsigned int whichOne)
{

//    	char buffer[64];
    	unsigned int value;

	sprintf(buffer, "/sys/class/gpio/gpio%d/value", whichOne);

//    	fp = fopen("/sys/class/gpio/gpio17/value", "r");
	fp = fopen(buffer, "r");

    	if (fp == NULL)
    	{
        	perror("GPIO ERROR");
        	printf("Can't read GPIO %d\r\n", whichOne);
        	return(-1);
    	}
    	else
    	{
        	fscanf(fp, "%u", &value);
        	fclose(fp);
        	return value;
    	}
}

void lookUpChar(void)
{
	bool match, foundChar;
        int i, j;
        

	i = 0;
	match = 0;
	foundChar = 0;

        do
        {
        	j = 0;

                do
                {
 	               if (charArray[j] == morseCodeTable[i][j])
       		                match = true;
                       else
                                match = false;

		       if (match && (morseCodeTable[i][j] == END) && (charArray[j] == END) )
				foundChar = true;

                	j++;

                } while (match && !foundChar);

		i++;

          } while (!foundChar);

        printf("%c", morseCodeTable[i-1][j]);
	fflush(stdout);
}

int main(int argc, char *argv[])
{

	double newSpeedFactor;

	std::cout << "Hello..." << std::endl;

	// Set up the input GPIO
	sprintf(buffer, "/sys/class/gpio/export");
//        fp = fopen("/sys/class/gpio/export", "w");
	fp = fopen(buffer, "w");
//        fprintf(fp, "17");
	fprintf(fp, "%d", GPIO);
        fclose(fp);

	sprintf(buffer, "/sys/class/gpio/gpio%d/direction", GPIO);
//        fp = fopen("/sys/class/gpio/gpio17/direction", "w");
	fp = fopen(buffer, "w");
        fprintf(fp, "in");
        fclose(fp);

	lowTime = 0;
	highTime = 0;
	elementIndex = 0;

        //initialize timer 
        clock_gettime(CLOCK_MONOTONIC, &timer);
        lastHandledTime = (timer.tv_sec * 1000) + (timer.tv_nsec * 1000000);

	usleep(1000);
	memset(charArray, 0, sizeof(charArray));

	init = true;
	gotWord = false;

	newSpeedFactor = wpm / atoi(argv[1]);
	timeUnit *= newSpeedFactor;

	if (readGPIO(GPIO) == 1)
	{
		printf("Waiting for GPIO to go LOW before proceeding\r\n");

		do
		{
			printf("|\r");
			fflush(stdout);
			usleep(500000);
			printf("/\r");
                        fflush(stdout);
			usleep(500000);
			printf("-\r");
                        fflush(stdout);

			usleep(500000);
			printf("\\");
			printf("\r");
                        fflush(stdout);

			usleep(500000);
			printf("|\r");

                        fflush(stdout);
			usleep(500000);
			printf("/\r");
                        fflush(stdout);

			usleep(500000);
			printf("-\r");
                        fflush(stdout);

			usleep(500000);
			printf("\\");
			printf("\r");
                        fflush(stdout);

			usleep(500000);

		} while (readGPIO(GPIO) == 1);
	}	


	while(!done)
	{
		getTimeInterval();
//		usleep(200);
	}
}




