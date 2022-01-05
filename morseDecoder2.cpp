

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
#define GPIO		17

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

	static double highTimeArray[8], lowTimeArray[8], wpm;
	static unsigned int highTime, lowTime, aveLowTime, aveLowTimeCount, i;
	static unsigned int minute = 60000;
	static unsigned int paris = 50; // This is the number of time units in the word "paris "
	static bool setLowTime = true;

	fflush(stdout);

	gpioValue =  readGPIO(GPIO);

       // We're receiving a char and are in-between DITS and DAHS
      if ( (gpioValue == 0) && rxChar)
      {
              lowTime++;

/*              printf("2 ");
                fflush(stdout);

                // Timed out after 1 second
                if (lowTime >= 1000) 
                {
                      rxChar = false;
                        printf(".");
                        fflush(stdout);
                        lowTime = 0;
                } */
      }

	
	// Didn't receive anythng yet
        if ( (gpioValue == 0) && !rxChar) 
        {
//		printf("1 ");
//		fflush(stdout);
		lowTime++;
		memset(highTimeArray, 0, sizeof(highTimeArray));
		memset (highTimeArray, 0, sizeof(highTimeArray));
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

//               printf("3 ");
//                fflush(stdout);

                rxChar = true;
                highTime++;
		lowTime = 0;
        }

	// Got our first DIT or DAH
	else if ( (gpioValue == 0) && (highTime > 0) && !firstMark && !secondMark && !thirdMark && !fourthMark &&\
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{
  //             printf("4 ");
  //              fflush(stdout);

		firstMark = true;
		highTimeArray[0] = highTime;
		highTime = 0;
	}


	// Getting our 2nd DIT or DAH
	else if ( (gpioValue == 1) && firstMark && !secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{       
//               printf("5 ");
//                fflush(stdout);

		if (setLowTime)
		{
			lowTimeArray[0] = lowTime;
			setLowTime = false;
		}

		lowTime = 0;
		highTime++;
	}

	// Got our 2nd mark
	else if ( (gpioValue == 0) && (highTime > 0) && firstMark && !secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
	{
//               printf("6 ");
//                fflush(stdout);

		secondMark = true;
		highTimeArray[1] = highTime;
		highTime = 0;
		setLowTime = true;
	}

        // Getting our 3rd DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
//               printf("7 ");
//                fflush(stdout);

		if (setLowTime)
		{
	                lowTimeArray[1] = lowTime;
			setLowTime = false;
		}

                lowTime = 0;
                highTime++;
        }

        // Got our 3nd mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && !thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
 //              printf("8 ");
 //               fflush(stdout);

                thirdMark = true;
                highTimeArray[2] = highTime;
                highTime = 0;
		setLowTime = true;
        }

        // Getting our 4rd DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
//               printf("9 ");
//                fflush(stdout);

		if (setLowTime)
		{
                	lowTimeArray[2] = lowTime;
			setLowTime = false;
		}

                lowTime = 0;
                highTime++;
        }

       // Got our 4th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && !fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
//               printf("10 ");
//                fflush(stdout);

                fourthMark = true;
                highTimeArray[3] = highTime;
                highTime = 0;
		setLowTime = true;
        }

        // Getting our 5th DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && thirdMark && fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
//               printf("11 ");
//                fflush(stdout);

		if (setLowTime)
		{
                	lowTimeArray[3] = lowTime;
			setLowTime = false;
		}
                lowTime = 0;
                highTime++;
        }

      // Got our 5th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    !fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
//               printf("12 ");
//                fflush(stdout);

                fifthMark = true;
                highTimeArray[4] = highTime;
                highTime = 0;
		setLowTime = true;
        }

        // Getting our 6th DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {       
//               printf("13 ");
//                fflush(stdout);

		if (setLowTime)
		{
                	lowTimeArray[4] = lowTime;
			setLowTime = false;
		}
                lowTime = 0;
                highTime++;
        }

      // Got our 6th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && !sixthMark && !seventhMark && !eighthMark)
        {
//               printf("14 ");
//                fflush(stdout);

                sixthMark = true;
                highTimeArray[5] = highTime;
                highTime = 0;
		setLowTime = true;
        }

       // Getting our 7th DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && !seventhMark && !eighthMark)
        {       
//               printf("15 ");
//                fflush(stdout);

		if (setLowTime)
		{
                	lowTimeArray[5] = lowTime;
			setLowTime = false;
		}

                lowTime = 0;
                highTime++;
        }

     // Got our 7th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && !seventhMark && !eighthMark)
        {
//               printf("16 ");
//                fflush(stdout);

                seventhMark = true;
                highTimeArray[6] = highTime;
                highTime = 0;
		setLowTime = true;

        }

       // Getting our 8th DIT or DAH
        else if ( (gpioValue == 1) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && seventhMark && !eighthMark)
        {       
//               printf("17 ");
//                fflush(stdout);
		
		if (setLowTime)
		{
                	lowTimeArray[6] = lowTime;
			setLowTime = false;
		}
                lowTime = 0;
                highTime++;
        }

     // Got our 8th mark
        else if ( (gpioValue == 0) && (highTime > 0) && firstMark && secondMark && thirdMark && fourthMark && \
                    fifthMark && sixthMark && seventhMark && !eighthMark)
        {
//               printf("18 ");
//                fflush(stdout);

                eighthMark = true;
                highTimeArray[7] = highTime;
                highTime = 0;
        }

	// We should have all the data we need, lets figure out a timeUnit
        else if (eighthMark) 
        {
//               printf("19 ");
//                fflush(stdout);


		i = 0;

		// Find the first low time that isnt a space between chars
//		do
//		{
//			printf("%d\r\n", lowTimeArray[i]);
//			i++;

//		} while (lowTimeArray[i-1] > initialTimeUnit*3);

		// Find the low times that arn't a space between chars and average them
		aveLowTime = 0;
		aveLowTimeCount = 0;
		
		for (i=0; i<=6; i++)
		{
			if (lowTimeArray[i] <= initialTimeUnit)
			{
				aveLowTime += lowTimeArray[i];
				aveLowTimeCount++;
			}
		}

		aveLowTime /= (double)aveLowTimeCount;

//		timeUnit = lowTimeArray[i-1];
		timeUnit = aveLowTime;
		printf("ave Low Time: %d	%d\r\n", aveLowTime, aveLowTimeCount);

		for (i=0; i<=6; i++)
			printf("%.0f\r\n", lowTimeArray[i]);

		calibrate = false;
        }

	if (!calibrate)
	{

                wpm = (minute / timeUnit) / paris;

//		printf("Time Unit 1: %.2f\r\n", timeUnit);
//		printf("WPM1: %.2f\r\n", wpm);
		

		if ( (wpm - int(wpm) ) != 0)
		{
			if ( (wpm - int(wpm) ) > (float)0.5)
				wpm = ceil(wpm);
			else if ( (wpm - int(wpm) ) <= 0.5)
				wpm = floor(wpm);

			timeUnit = minute / (paris * wpm);

//	                printf("Time Unit 2: %.2f\r\n", timeUnit);
//        	        printf("WPM2: %.2f\r\n", wpm);

			if ( (timeUnit - int(timeUnit)) != 0)
			{
//				printf("Calc new time unit\r\n");
//				fflush(stdout);

				timeUnit = (int)timeUnit;
				
//				if ( (timeUnit - int(timeUnit)) > (float)0.5)
		//			timeUnit = ceil(timeUnit);
//					timeUnit = (int)timeUnit;
//				else if ( (timeUnit - int(timeUnit) ) <= (float)0.5 )
//					timeUnit = ceil(timeUnit);
			}

			else
			{

			}

//			printf("Calculated!\r\n");
//        	        printf("Time Unit 3: %.2f\r\n", timeUnit);
//	                printf("WPM3: %.2f\r\n", wpm);

		}

		else
		{
//			printf("Straight!\r\n");
		}


		printf("\r\nGot Speed: %.0f WPM!\r\n", wpm);
//		printf("time Unit 4: %f %f %f %f %f\r\n", timeUnit, lowTimeArray[0], lowTimeArray[1], lowTimeArray[2], lowTimeArray[3] );
//                printf("%f %f %f %f\r\n",  highTimeArray[0], highTimeArray[1], highTimeArray[2], highTimeArray[3] );

	}
}


void timerProc(void)
{

	if (calibrate && init)
	{
		std::cout << "Getting Speed: Please send me a DIT and a DAH or a DAH and a DIT!" << std::endl;
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

		if (rxChar && (gpioValue == 0) && (lowTime > timeUnit*4)) // We received a complete character
		{

			charArray[elementIndex] = END;
			rxChar = false;
			elementIndex = 0;
			lowTime = 0;
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
	firstMark = false;
	secondMark = false;
	thirdMark = false;
	fourthMark = false;
	fifthMark = false;
	sixthMark = false;
	seventhMark = false;
	eighthMark = false;
	

	newSpeedFactor = wpm / atoi(argv[1]);
	timeUnit *= newSpeedFactor;

	if (readGPIO(GPIO) == 1)
	{
		printf("Waiting for GPIO to go LOW before proceeding\r\n");

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
		usleep(200);
	}
}




