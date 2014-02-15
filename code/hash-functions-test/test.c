#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void timeval_subtract (struct timeval *result, struct timeval *start, struct timeval *end);
long int timeval_to_milliseconds (struct timeval *time);

#include "xxHash.h"
#include "md5.h"

int main()
{
	char *tag = "1234567890";
	unsigned int hash1 = XXH_small(tag, strlen(tag), 1);
	printf("xxHash of \"1234567890\" = 0x%04x\n", hash1);

	unsigned *hash2 = md5(tag, strlen(tag));
	WBunion u;
	int i, j;
    printf("md5 hash of \"1234567890\" = 0x");
    for (i=0;i<4; i++){
        u.w = hash2[i];
        for (j=0;j<4;j++) printf("%02x",u.b[j]);
    }
    printf("\n");



    struct timeval timeBefore, timeAfter, elapsedTime;
			
	gettimeofday(&timeBefore, NULL);

	for (i = 0; i < 1000000; i++)
	{
		unsigned int hash = XXH_small(tag, strlen(tag), 1);
	}

	gettimeofday(&timeAfter, NULL);
	
	timeval_subtract(&elapsedTime, &timeAfter, &timeBefore);

	printf("Time for 1000000 xxHash runs = %ld milliseconds.\n", timeval_to_milliseconds(&elapsedTime));


	gettimeofday(&timeBefore, NULL);

	for (i = 0; i < 1000000; i++)
	{
		unsigned *hash = md5(tag, strlen(tag));
	}

	gettimeofday(&timeAfter, NULL);
	
	timeval_subtract(&elapsedTime, &timeAfter, &timeBefore);

	printf("Time for 1000000 md5 runs = %ld milliseconds.\n", timeval_to_milliseconds(&elapsedTime));
}

void timeval_subtract (struct timeval *result, struct timeval *start, struct timeval *end) {
	// perform the carry for the later subtraction by updating end
	if (start->tv_usec < end->tv_usec) {
		int nsec = (end->tv_usec - start->tv_usec) / 1000000 + 1;
		end->tv_usec -= 1000000 * nsec;
		end->tv_sec += nsec;
		
	}
	if (start->tv_usec - end->tv_usec > 1000000) {
		int nsec = (start->tv_usec - end->tv_usec) / 1000000;
		end->tv_usec += 1000000 * nsec;
		end->tv_sec -= nsec;
		
	}
	
	// do the subtraction
	result->tv_sec = start->tv_sec - end->tv_sec;
	result->tv_usec = start->tv_usec - end->tv_usec;
}

long int timeval_to_milliseconds (struct timeval *time) {
	return (long int) (time->tv_sec * 1000) + (long int) (time->tv_usec / 1000);
}