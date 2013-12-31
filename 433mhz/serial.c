#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <memory.h>

FILE *fout = NULL;

void sighandler (int sig)
{
	if (fout != NULL)
	{
		fclose (fout);
		fout = NULL;
	}

	exit (0);
}

long timediff_mus (struct timespec *pend, struct timespec * pstart)
{
	long sec_diff = pend->tv_sec - pstart->tv_sec;
	return (sec_diff * 1000000L) + ((pend->tv_nsec - pstart->tv_nsec) / 1000L);
}

void wait_high_cts (int fd)
{
	int signals;

	while (1)
	{
		// Wait for a CTS transition	
		ioctl (fd, TIOCMIWAIT, TIOCM_CTS);

		// Quit if it's now high
		ioctl (fd, TIOCMGET, &signals);
		if (signals & TIOCM_CTS)
			break;
	}
}

int process_serial ()
{
	int fd, pulse;
	struct timespec start, now;

	fd = open ("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == -1)
	{
		printf ("Error opening port: %s\n", strerror (errno));
		return -1;
	}
	fcntl (fd, F_SETFL, 0);

	fout = fopen ("cts.log", "wt");
	if (fout == NULL)
	{
		printf ("Error opening output: %s\n", strerror (errno));
		return -1;
	}
	
	signal (SIGINT, sighandler);

	// Wait for initial low/high transition of CTS
	wait_high_cts (fd);

	// Record first timestamp
	clock_gettime (CLOCK_REALTIME, &start);

	while (1)
	{
		// Wait for low/high transition of CTS
		wait_high_cts (fd);

		// Get current timestamp, log pulse duration allowing for overhead time
		clock_gettime (CLOCK_REALTIME, &now);
		pulse = timediff_mus (&now, &start) - 3;
		fprintf (fout, "%08ld\n", pulse);

		// Set current timestamp as start of new pulse
		memcpy (&start, &now, sizeof (struct timespec));
	}

	return 0;
}

int process_file (char *pfilename)
{
	FILE *fin;
	int pulse, in_data, line;
	char data [64];

	fin = fopen (pfilename, "rt");
	if (fin == NULL)
	{
		printf ("Error opening input: %s\n", strerror (errno));
		return -1;
	}

	line = 0;
	while (!feof (fin))
	{
		fscanf (fin, "%d\n", &pulse);
		line++;

		if (pulse > 10000)
			sprintf (data, "*\n*");
		else if (pulse > 500 && pulse < 600)
			strcpy (data, "S");
		else if (pulse > 1500 && pulse < 1600)
			strcpy (data, "L");
		else
			sprintf (data, "[%d]", pulse);

		printf (data);
	}
	printf ("*\n");

	fclose (fin);
	return 0;
}

int main (int argc, char **ppargv)
{
	if (argc == 2)
		return process_file (ppargv [1]);
	else
		return process_serial ();
}
