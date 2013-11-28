/* ledctrl.c
 * Jon Green <jon@hosed.org>
 * September 2013
 * 
 * Used to control two LEDs on the BeagleSnort.  Behavior:  Green LED will be lit
 * during normal operation.  When a Snort event is logged, red LED will begin blinking.
 * Blinking will continue until the "acknnowledge" button is pressed.  At that point,
 * the red LED will stop blinking and the green LED will illuminate.
 * 
 * Some parts of this module loosely based on gpio-int-test.c by RidgeRun, as modified by
 * Mark Yoder who split out gpio-utils.c as a separate module.
 *
 * This module will be run in a thread called from main.c.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <signal.h>
#include "gpio-utils.h"
#include "ledctrl.h"
#include "beaglesnort.h"

#define MAX_BUF 64
int running = 1; /* Used by signal handler */

void signal_handler(int sig)
{
	printf( "Signal caught - cleaning up and exiting..\n" );
	running = 0;
}

void *ledControl(void *ptr)
{
	struct pollfd fdset[1];
	int button_fd, rc;
	char buf[MAX_BUF];
	unsigned int green_led, red_led, button;
	int len, *led_color;

	/* Set up signal handler - need to turn LEDs off when program exits */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	if (ptr==0)
		perror("ledControl called with null pointer");

	led_color = (int *) ptr;

	green_led = 49;		/* pin P9-23 */
	red_led = 48;		/* pin P9-15 */
	button = 60;		/* pin P9-12 */

	gpio_export(green_led);
	gpio_export(red_led);
	gpio_export(button);
	gpio_set_dir(green_led, "out");
	gpio_set_dir(red_led, "out");
	gpio_set_dir(button, "in");

	gpio_set_edge(button, "rising");
	button_fd = gpio_fd_open(button, O_RDONLY);


	while (running) {
		memset((void*)fdset, 0, sizeof(fdset));

    	fdset[0].fd = button_fd;
		fdset[0].events = POLLPRI;

		rc = poll(fdset, 1, 500);  /* .5 second timeout */

		if (rc < 0) {
			printf("\npoll() failed!\n");
			exit(-1);
		}
      
		if ((rc == 0) && DEBUG) {
			printf(".");
		}
            
		if (fdset[0].revents & POLLPRI) {
			lseek(fdset[0].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[0].fd, buf, MAX_BUF);
			if(DEBUG)
				printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
				 button, buf[0], len);
			pthread_mutex_lock( &mutex1 );
			*led_color=GREEN;
			pthread_mutex_unlock( &mutex1 );
			gpio_set_value(red_led, 0);
		}

		if (*led_color == GREEN)
			gpio_set_value(green_led, 1);

		if (*led_color == RED)
			gpio_toggle(red_led);

		fflush(stdout);
	}

	gpio_fd_close(button_fd);
	gpio_set_value(red_led, 0);
	gpio_set_value(green_led, 0);
	exit(0);	/* TODO: Can we tell the main thread that we're exiting? */
	return 0;
}

