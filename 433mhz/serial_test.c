#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>

int main ()
{
	int fd = -1, count, n;
	struct termios options;
	char buffer [64];

	// Open port
	fd = open ("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == -1)
	{
		printf ("Failed to open port\n");
		goto Exit;
	}
	fcntl (fd, F_SETFL, 0);

	// Set port parameters
	tcgetattr (fd, &options);
	cfsetispeed (&options, B115200);
	cfsetospeed (&options, B115200);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	options.c_oflag &= ~OPOST;

	options.c_cc [VMIN] = 0;
	options.c_cc [VTIME] = 10;

	tcsetattr (fd, TCSANOW, &options);

	// Send some data
	write (fd, "AT\r", 3);

	// Wait for incoming data
	while (1)
	{
		count = read (fd, buffer, 64);
		if (count == 0)
			break;

		for (n = 0; n < count; n++)
		{
			printf ("Byte %d: %02X", n, buffer [n]);
			if (buffer [n] >= 32)
				printf (" (%c)\n", buffer [n]);
			else
				printf ("\n");
		}
	}

	int signals;
	ioctl (fd, TIOCMGET, &signals);
	if (signals & TIOCM_CAR)
		printf ("DCD set\n");
	else
		printf ("DCD not set\n");

	struct timespec clock;
	clock_gettime (CLOCK_REALTIME, &clock);
	printf ("Clock is %ld seconds and %ld nanoseconds\n", clock.tv_sec, clock.tv_nsec);

	size_t length;
	char *pline;
	int done = 0;
	while (!done)
	{
		pline = NULL;
		getline (&pline, &length, stdin);

		if (!strcmp (pline, "quit\n"))
			done = 1;

		free (pline);
	}

Exit:
	if (fd != -1)
		close (fd);

	return 0;
}
