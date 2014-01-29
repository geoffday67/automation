#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <memory.h>

#define COMMAND_OFF		0
#define COMMAND_ON		1

int Channel, Command;

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
	signals |= TIOCM_RTS;
	ioctl (fd, TIOCMSET, &signals);
}

void set_low (int fd)
{
	int signals;

	// Set RTS to 0v
	ioctl (fd, TIOCMGET, &signals);
	signals &= ~TIOCM_RTS;
	ioctl (fd, TIOCMSET, &signals);
}

void delay (int microseconds)
{
	static int offset = 50;
	struct timespec time;

	if (microseconds <= offset)
		return;

	time.tv_sec = 0;
	time.tv_nsec = (microseconds - offset) * 1000L;
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

	delay (10250);

	for (repeat = 0; repeat < 12; repeat++)
	{
		send_bit (fd, 1);
		for (n = 0; n < 10; n++)
			send_byte (fd, pmessage [n]);
		send_bit (fd, 1);
		delay (10250);
	}

	set_low (fd);
}

byte encode_value (int value)
{
	static byte table [] = {0xF6, 0xEE, 0xED, 0xEB, 0xDE, 0xDD, 0xDB, 0xBE, 0xBD, 0xBB, 0xB7, 0x7E, 0x7D, 0x7B, 0x77, 0x6F};

	if (value < 0 || value > 15)
		return 0;

	return table [value];
}

void send_command (int fd, int channel, int command)
{
	static byte TRANSMITTER_ID [6] = {0x6F, 0xED, 0xBB, 0xBE, 0xBE, 0xBD};
	byte message [10];

	// Set final 6 bytes to our transmitter's ID
	memcpy (message + 4, TRANSMITTER_ID, 6);

	// Set level to zero as we're not using it here
	message [0] = 0xF6;
	message [1] = 0xF6;

	// Set channel byte
	message [2] = encode_value (channel);

	// Set command byte
	message [3] = encode_value (command);

	// Send completed command
	send (fd, message);
}

int command (int fd, char *pcommand)
{
	int signals;

	if (!strcmp (pcommand, "quit"))
		return -1;

	if (!strcmp (pcommand, "reset"))
	{
		ioctl (fd, TIOCMGET, &signals);
		signals &= ~TIOCM_RTS;
		ioctl (fd, TIOCMSET, &signals);
	}
	else if (!strcmp (pcommand, "set"))
	{
		ioctl (fd, TIOCMGET, &signals);
		signals |= TIOCM_RTS;
		ioctl (fd, TIOCMSET, &signals);
	}
	else if (!strcmp (pcommand, "on"))
	{
		send (fd, on);
	}
	else if (!strcmp (pcommand, "off"))
	{
		send (fd, off);
	}
	else if (!strcmp (pcommand, "test1"))
	{
		send_command (fd, 0, COMMAND_ON);
	}
	else if (!strcmp (pcommand, "test2"))
	{
		send_command (fd, 0, COMMAND_OFF);
	}

	return 0;
}

void parse_args (int argc, char **ppargv)
{
	int index = 1;

	while (index < argc)
	{
		if (!strcmp (ppargv [index], "--channel"))
			Channel = atoi (ppargv [++index]) - 1;
		else if (!strcmp (ppargv [index], "--command"))
		{
			index++;
			if (!strcmp (ppargv [index], "on"))
				Command = COMMAND_ON;
			else if (!strcmp (ppargv [index], "off"))
				Command = COMMAND_OFF;
		}

		index++;
	}
}

int main (int argc, char **ppargv)
{
	char *pline;
	size_t length;
	int fd, done;

	fd = open ("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == -1)
		return -1;
	fcntl (fd, F_SETFL, 0);

	set_low (fd);

	if (argc > 1)
	{
		parse_args (argc, ppargv);
		send_command (fd, Channel, Command);
	}
	else
	{
		done = 0;
		while (!done)
		{
			pline = NULL;
			getline (&pline, &length, stdin);
			pline [strlen (pline) - 1] = '\0';
			done = (command (fd, pline) == -1);
			free (pline);
		}
	}

	close (fd);
}
