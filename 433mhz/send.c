#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <memory.h>

typedef unsigned char byte;

static byte on[]  = {0xF6, 0xF6, 0xEE, 0xEE, 0x6F, 0xED, 0xBB, 0xBE, 0xBE, 0xBD};
static byte off[] = {0xF6, 0xF6, 0xEE, 0xF6, 0x6F, 0xED, 0xBB, 0xBE, 0xBE, 0xBD};

struct timespec HIGH_PULSE = {0, 150000L};
struct timespec SHORT_LOW_PULSE = {0, 160000L};
struct timespec LONG_LOW_PULSE = {0, 1860000L}; 
struct timespec GAP_PULSE = {0, 10500000L};

void set_high (int fd)
{
	int signals;

	// Set RTS to 5v
	ioctl (fd, TIOCMGET, &signals);
	signals &= ~TIOCM_RTS;
	ioctl (fd, TIOCMSET, &signals);
}

void set_low (int fd)
{
	int signals;

	// Set RTS to 0v
	ioctl (fd, TIOCMGET, &signals);
	signals |= TIOCM_RTS;
	ioctl (fd, TIOCMSET, &signals);
}

void send_short (int fd)
{
	set_high (fd);
	nanosleep (&HIGH_PULSE, NULL);
	set_low (fd);
	nanosleep (&SHORT_LOW_PULSE, NULL);
}

void send_long (int fd)
{
	set_high (fd);
	nanosleep (&HIGH_PULSE, NULL);
	set_low (fd);
	nanosleep (&LONG_LOW_PULSE, NULL);
}

void send_gap (int fd)
{
	set_low (fd);
	nanosleep (&GAP_PULSE, NULL);
}

void send_file (int fd, char *pfilename)
{
	FILE *fin;
	int c;

	fin = fopen (pfilename, "rt");
	if (fin == NULL)
	{
		printf ("Error opening input: %s\n", strerror (errno));
		return;
	}

	while ((c = fgetc (fin)) != EOF)
	{
		switch (c)
		{
			case '*':
				send_gap (fd);
				break;

			case 'S':
				send_short (fd);
				break;

			case 'L':
				send_long (fd);
				break;
		}
	}

	fclose (fin);

	printf ("Sending complete\n");
}

void delay (int microseconds)
{
	struct timespec time;
	time.tv_sec = 0;
	time.tv_nsec = (microseconds - 50) * 1000L;
	nanosleep (&time, NULL);
}

void send_bit (int fd, byte b)
{
	delay (25);
	if (b)
		set_high (fd);
	else
		set_low (fd);
	delay (280);
	set_low (fd);
	delay (280);
	if (!b)
		delay (300);
}

void send_byte (int fd, byte b)
{
	byte mask;

	send_bit (fd, 1);
	for (mask = 0x80; mask; mask >>= 1)
		send_bit (fd, b & mask);
}

void send (int fd, byte *pmessage)
{
	int repeat, n;

	for (repeat = 0; repeat < 12; repeat++)
	{
		send_bit (fd, 1);
		for (n = 0; n < 10; n++)
			send_byte (fd, pmessage [n]);
		send_bit (fd, 1);
		delay (10250);
	}
}

void square (int fd)
{
	int n;

	for (n = 0; n < 30000; n++)
	{
		set_high (fd);
		nanosleep (&HIGH_PULSE, NULL);
		set_low (fd);
		nanosleep (&HIGH_PULSE, NULL);
	}
}

int main (int argc, char **ppargv)
{
	char *pline;
	size_t length;
	int fd, done, signals;

	fd = open ("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == -1)
	{
		printf ("Error opening port: %s\n", strerror (errno));
		return -1;
	}
	fcntl (fd, F_SETFL, 0);

	done = 0;
	while (!done)
	{
		pline = NULL;
		getline (&pline, &length, stdin);

		if (!strcmp (pline, "quit\n"))
			done = 1;
		else if (!strcmp (pline, "reset\n"))
		{
			ioctl (fd, TIOCMGET, &signals);
			signals &= ~TIOCM_RTS;
			ioctl (fd, TIOCMSET, &signals);
		}
		else if (!strcmp (pline, "set\n"))
		{
			ioctl (fd, TIOCMGET, &signals);
			signals |= TIOCM_RTS;
			ioctl (fd, TIOCMSET, &signals);
		}
		else if (!strcmp (pline, "square\n"))
		{
			square (fd);
		}
		else if (!strcmp (pline, "on\n"))
		{
			send (fd, on);
		}
		else if (!strcmp (pline, "off\n"))
		{
			send (fd, off);
		}
		else
		{
			pline [strlen (pline) - 1] = '\0';
			send_file (fd, pline);
		}

		free (pline);
	}

	close (fd);
}
