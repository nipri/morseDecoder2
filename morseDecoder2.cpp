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
#include <math.h>

#include "morseCodeDecoder.h"

#define INTERVAL	1 // mS interval to execute timerProc()
//#define GPIO		17
static unsigned int GPIO = 17;



struct timespec timer;
bool gotChar, rxChar, done, init, gotWord, calibrate, firstMark, secondMark, thirdMark, fourthMark;
bool fifthMark, sixthMark, seventhMark, eighthMark;
long long int highTime, lowTime, currentTime;
unsigned long long int lastHandledTime;

double wpm = 5;

// Base time unit
static double initialTimeUnit = 240;
static double timeUnit = 240; // mS time unit for 5 WPM

// Time tolerance is the amount of leeway or slop in mS  to add or subtract on either end of 
// a time unit to account for a wide (how wide?) variation of the time unit when a human
// is sending code. 
// Will set to 10% of 5 WPM for now
static double timeUnitTolerance = 24; // mS


unsigned int charArray[8], elementIndex;
unsigned int gpioValue;
char buffer[64];

FILE *fp;

void timerProc(void);
void getTimeInterval(void);
void lookUpChar(void);
unsigned int readGPIO(unsigned int);

void getSpeed(void);

void getSpeed(void)
{

	static double highTimeArray[8], lowTimeArray[8], wpm, highTime, lowTime, aveLowTime;
	static unsigned int aveLowTimeCount, i;
	static unsigned int minute = 60000;
	static unsigned int paris = 50; // This is the number of time units in the word "paris "

	fflush(stdout);

	gpioValue =  readGPIO(GPIO);

//	if (gpioValue == 1)
//		highTime++;

       // We're receiving a char and are in-between DITS and DAHS or between words
      if ( (gpioValue == 0) && rxChar)
      {
              lowTime++;

                // Timed out after 3 seconds
                if (lowTime >= 3000) 
                {
                      rxChar = false;
                        printf(".");
                        fflush(stdout);
                        lowTime = 0;
                } 
      }

	
	// Didn't receive anythng yet
        if ( (gpioValue == 0) && !rxChar) 
        {
		lowTime++;
		memset(highTimeArray, 0, sizeof(highTimeArray));
		memset (lowTimeArray, 0, sizeof(lowTimeArray));
		firstMark = false;
		secondMark = false;
		thirdMark = false;
		fourthMark = false;
		fifthMark = false;
		sixthMark = false;
		seventhMark = false;
		eighthMark = false;

                if (lowTime >= 1000)
                {
                        printf(".");
                        fflush(stdout);
			lowTime = 0;
                }
        }

	// We're receiving our first DIT or DAH
        else if  ( (gpioValue == 1) && !firstMark && !secondMark && !thirdMark && !fourthMark &&\
		    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
                rxChar = true;
		lowTime = 0;
		highTime++;
		printf("1 ");
        }

	// Got our first DIT or DAH
	else if ( (gpioValue == 0) && (highTime > 0) && !firstMark && !secondMark && !thirdMark && !fourthMark &&\
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{
		firstMark = true;
		highTimeArray[0] = highTime;
		highTime = 0;
		printf("2 ");
	}


	// Getting our 2nd DIT or DAH
	else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && !secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{       
		lowTimeArray[0] = lowTime;
		lowTime = 0;
		highTime++;
		printf("3 ");

		if (lowTimeArray[0] >= highTimeArray[0])
		{
			firstMark = false;
			rxChar = false;
		}

	}

	// Got our 2nd mark
	else if ( (gpioValue == 0) && (highTime > 0) && firstMark && !secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{
		secondMark = true;
		highTimeArray[1] = highTime;
		highTime = 0;
		printf("4 ");
	}

        // Getting our 3rd DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
                lowTimeArray[1] = lowTime;
                lowTime = 0;
		highTime++;
		printf("5 ");
        }

        // Got our 3nd mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
                thirdMark = true;
                highTimeArray[2] = highTime;
                highTime = 0;
		printf("6 ");
        }

        // Getting our 4rd DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && secondMark && thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
               	lowTimeArray[2] = lowTime;
                lowTime = 0;
		highTime++;
		printf("7 ");
        }

       // Got our 4th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
                fourthMark = true;
                highTimeArray[3] = highTime;
                highTime = 0;
		printf("8 ");
        }

        // Getting our 5th DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0 ) && firstMark && secondMark && thirdMark && fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
               	lowTimeArray[3] = lowTime;
                lowTime = 0;
		highTime++;
		printf("9 ");
        }

      // Got our 5th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
                fifthMark = true;
                highTimeArray[4] = highTime;
                highTime = 0;
		printf("10 ");
        }

        // Getting our 6th DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
               	lowTimeArray[4] = lowTime;
                lowTime = 0;
		highTime++;
		printf("11 ");
        }

      // Got our 6th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
                sixthMark = true;
                highTimeArray[5] = highTime;
                highTime = 0;
		printf("12 ");
        }

       // Getting our 7th DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && !seventhMark && !eighthMark)
        {       
               	lowTimeArray[5] = lowTime;
                lowTime = 0;
		highTime++;
		printf("13 ");
        }

     // Got our 7th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && !seventhMark && !eighthMark)
        {
                seventhMark = true;
                highTimeArray[6] = highTime;
                highTime = 0;
		printf("14 ");

        }

       // Getting our 8th DIT or DAH
        else if ( (gpioValue == 1) && (lowTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && seventhMark && !eighthMark)
        {       
               	lowTimeArray[6] = lowTime;
                lowTime = 0;
		highTime++;
		printf("15 ");
        }

     // Got our 8th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && seventhMark && !eighthMark)
        {
                eighthMark = true;
                highTimeArray[7] = highTime;
                highTime = 0;
		printf("16 ");
        }

	// We should have all the data we need, lets figure out a timeUnit
        else if (eighthMark) 
        {
		printf("17 ");

		// Find the low times that arn't a space between chars and average them
		aveLowTime = 0;
		aveLowTimeCount = 0;

		initialTimeUnit = lowTimeArray[0];

//		for (i=1; i<=6; i++)
		for (i=0; i<=6; i++)		
		{
			if (lowTimeArray[i] <= initialTimeUnit)
			{
				aveLowTime += lowTimeArray[i];
				aveLowTimeCount++;
			}
		}

		aveLowTime /= (double)aveLowTimeCount;

		timeUnit = aveLowTime;
		printf("ave Low Time: %f	%d\r\n\r\n", aveLowTime, aveLowTimeCount);

		for (i=0; i<=6; i++)
			printf("%.0f\r\n", lowTimeArray[i]);


		calibrate = false;

		printf("minute: %d	timeUnit: %f	paris: %d\r\n", minute, timeUnit, paris);

                wpm = (minute / timeUnit) / paris;
                printf("\r\nGot Speed: %.0f WPM!\r\n", wpm);


		printf("\r\nGot Speed: %.0f WPM!\r\n", wpm);
		usleep(5000);
//		lowTime = 0;
		rxChar = false;
	}

}


void timerProc(void)
{

	if (calibrate && init)
	{
		std::cout << "Getting Speed: Please send me some code!" << std::endl;
		init = false;
	}

	if (calibrate)
	{
		getSpeed();
	}

	else
	{
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

		if (rxChar && (gpioValue == 0) && (lowTime >= timeUnit*3)) // We received a complete character
		{
			charArray[elementIndex] = END;
			rxChar = false;
			elementIndex = 0;
			highTime = 0;
			lookUpChar();
		}
	} // end else
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

    	unsigned int value;

	sprintf(buffer, "/sys/class/gpio/gpio%d/value", whichOne);

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
	bool match, foundChar, error;
        int i, j;
        

	i = 0;
	match = 0;
	foundChar = 0;
	error = false;

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

		if (i >= 37)
			error = true;

          } while ( (!foundChar) && (error == false) );


	if (error)
		printf("*");
	else
        	printf("%c", morseCodeTable[i-1][j]);

	fflush(stdout);
}

int main(int argc, char *argv[])
{

//	double newSpeedFactor;

	std::cout << "Hello..." << std::endl;

	if (argc == 1)
	{
		std::cout << "Please specify a GPIO to use... usually it's 17" << std::endl;
		return 0;
	}

	else
	{

		GPIO = atoi(argv[1]);

		 // Set up the input GPIO
		sprintf(buffer, "/sys/class/gpio/export");
		fp = fopen(buffer, "w");
		fprintf(fp, "%d", GPIO);
        	fclose(fp);

		sprintf(buffer, "/sys/class/gpio/gpio%d/direction", GPIO);
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
		firstMark = false;
		secondMark = false;
		thirdMark = false;
		fourthMark = false;
		fifthMark = false;
		sixthMark = false;
		seventhMark = false;
		eighthMark = false;
	
		if (readGPIO(GPIO) == 1)
		{
			printf("Waiting for GPIO %d to go LOW before proceeding...\r\n", GPIO);
			printf("Please either initialize the GPIO or hit CTRL-C to terminate this program\r\n\r\n");

			do // Do the clocky looking thing while waiting :)
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

			} while (readGPIO(GPIO) == 1);
		}	
	
		calibrate = true;

		while(!done)
		{
			getTimeInterval();
//			usleep(2);
		}
	}
}




