/*
 * main.c
 *
 * Jon Green <jon@hosed.org>
 * September 2013
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "ledctrl.h"
#include "beaglesnort.h"

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	pthread_t ledctrl;
	int led_ret;
	int led_color=GREEN;
	const char *path = LOGFILE;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	/* Spawn a thread to manage the LEDs and the reset button.  This lets the LED keep on
	 * blinking correctly even when the main thread blocks.  Plus, I really wanted to write
	 * something multi-threaded!
	 */
	led_ret = pthread_create( &ledctrl, NULL, ledControl, (void *)&led_color);
	if (led_ret != 0)
		perror("Error spawning thread");

	/* Monitor logfile; if anything changes, trigger the LED color to RED */
	/* TODO: How do we handle if the file is closed and then reopened - e.g. wraps? */

	fp = fopen(path, "r");
	if(fp == NULL) {
		perror("Error opening logfile " LOGFILE);
		return(-1);
	}

	/* Move position indicator to end of file - we don't care about what was already
	 * logged, only what comes in the future. */
	fseek(fp, 0L, SEEK_END);

	next_line:
	while ((read = getline(&line, &len, fp)) != -1) {
		if (DEBUG) {
			printf("DEBUG: Retrieved line of length %zu :\n", read);
			printf("DEBUG: %s", line);
		}
		pthread_mutex_lock( &mutex1 );
		led_color=RED;
		pthread_mutex_unlock( &mutex1 );
	}

	if (feof(fp)) {
		/* getline() failed because of EOF.. wait a little bit and try again */
		sleep(3);
		goto next_line;
	}

	/* Should only get here if getline() fails for some reason other than EOF */
	pthread_join( ledctrl, NULL);
	free(line);

	return 0;
}
