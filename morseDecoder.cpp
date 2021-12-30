#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>
#include <stdbool.h>

#include "morseCodeDecoder.h"

#include <iostream>
#include <fstream>

using namespace std;

#define GPIO 	17

struct timespec gpioLevelTime;
bool gotChar, gotWord, gotDitOrDah, match;
static double startTime, time1, time2;
double timeUnit = 240; // 240 mS is one time unit for 5WPM using PARIS
unsigned int charArray[8], elementIndex;


int main(int argc, char *argv[])
{
	char str[256];
//   	struct timeval tv;
   	struct pollfd pfd;
   	static int fp, gpio, timeout, i, j;
   	char buf[8];
   	char value;

   	if (argc > 1)
        	sprintf(buf, "%d", atoi(argv[1]));
   	else          
		sprintf(buf, "%d", GPIO);

   	fp = open("/sys/class/gpio/export", O_WRONLY);
   	write(fp, buf, sizeof(buf));
   	close(fp);

   	sprintf(str, "/sys/class/gpio/gpio%d/direction",  atoi(argv[1]));
//   	fp = open("/sys/class/gpio/gpio17/direction", O_WRONLY);
   	fp = open(str, O_WRONLY);
   	write(fp, "in", 2);
   	close(fp);

   	sprintf(str, "/sys/class/gpio/gpio%d/edge",  atoi(argv[1]));
//   	fp = open("/sys/class/gpio/gpio17/edge", O_WRONLY);
   	fp = open(str, O_WRONLY);
   	write(fp, "both", 4);
   	close(fp);

   	sprintf(str, "/sys/class/gpio/gpio%d/value",   atoi(argv[1]));
//   	fp = open("/sys/class/gpio/gpio17/value", O_RDONLY);

//   	fp = open(str, O_RDONLY);
//   	pfd.fd = fp;
//   	pfd.events = POLLPRI | POLLERR;

//   	lseek(fp, 0, SEEK_SET);    /* consume any prior interrupt */
//   	read(fp, &value, 1);

// 	Start looking for dits and dahs
   	gotChar = false;
   	gotWord = false;

//        clock_gettime(CLOCK_MONOTONIC, &gpioLevelTime);
//        startTime = (gpioLevelTime.tv_sec * 1000) + (gpioLevelTime.tv_nsec / 1000000);

	elementIndex = 0;
	gotDitOrDah = false;
	gotChar = false;

  	while (!gotChar)
  	{
 
   		cout << "Waiting for interrupt\r\n" << endl;

	        fp = open(str, O_RDONLY);
        	pfd.fd = fp;
        	pfd.events = POLLPRI | POLLERR;

        	lseek(fp, 0, SEEK_SET);    /* consume any prior interrupt */
        	read(fp, &value, 1);

		timeout = 0;
   		timeout = poll(&pfd, 1, timeUnit*4);         /* wait for interrupt */

   		lseek(fp, 0, SEEK_SET);    /* consume interrupt */
   		read(fp, &value, 1);
   		close(fp);

		cout << "Got Interrupt: Value1: " << value << endl;

		if (timeout > 0)
		{
	                cout << "Got Interrupt: Value2: " << value << endl;

        	        clock_gettime(CLOCK_MONOTONIC, &gpioLevelTime);
                	time1 = gpioLevelTime.tv_sec + (gpioLevelTime.tv_nsec / 1000000);

  	                fp = open(str, O_RDONLY);
          	      	pfd.fd = fp;
                	pfd.events = POLLPRI | POLLERR;

                	lseek(fp, 0, SEEK_SET);    /* consume any prior interrupt */
               	 	read(fp, &value, 1);

			timeout = 0;
                	timeout = poll(&pfd, 1, -1);         /* wait for interrupt */

                	lseek(fp, 0, SEEK_SET);    /* consume interrupt */
                	read(fp, &value, 1);
                	close(fp);

			cout << "Got Interrupt: Value3: " << value << endl;

                        clock_gettime(CLOCK_MONOTONIC, &gpioLevelTime);
               		time2 = gpioLevelTime.tv_sec + (gpioLevelTime.tv_nsec / 1000000);

			if ( (time2 - time1) < timeUnit*3) // it's a DIT
				charArray[elementIndex] = DIT;
			else if ( ( (time2 - time1) >= timeUnit*3 ) && ( (time2 - time1) < timeUnit*4) ) // it's a DAH
				charArray[elementIndex] = DAH;

			elementIndex++;
                        gotDitOrDah = true;
		}


		else if ( (timeout == 0 ) && gotDitOrDah )  // We got a character, look it up
		{

			gotChar = true;
			match = false;
			int foundChar = 0;
			i = 0;
			j = 0;

			do
			{
				j = 0;

				do
				{
					if (charArray[j] == morseCodeTable[i][j])
						match = true;
					else
						match = false;

					j++;

				} while (match && morseCodeTable[i][j] != END);

				if (match && morseCodeTable[i][j] == END)
					foundChar = 1;
				else
					i++;

			} while (!foundChar);
		
			printf ("The character is: %c\r\n", morseCodeTable[i][j+1]);
		}

		else
		{
		}
	}

   exit(0);	
}
