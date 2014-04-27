/*
  Copyright (c) 2014, FiveMicro LLC
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define DEFAULT_BAUD_RATE 300
#define BUF_SIZE 8192

ssize_t             /* Write "n" bytes to a descriptor */
writen(int fd, const void *ptr, size_t n)
{
  size_t      nleft;
  ssize_t     nwritten;

  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) < 0) {
      if (nleft == n)
	return(-1); /* error, return -1 */
      else
	break;      /* error, return amount written so far */
    } else if (nwritten == 0) {
      break;
    }
    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n - nleft);      /* return >= 0 */
}

void usage() {
  fprintf(stderr, "Usage: tty2out tty [baudrate]\n");
  fprintf(stderr, "example: tty2out /dev/ttyUSB0\n");
  fprintf(stderr, "example: tty2out /dev/ttyUSB0 9600\n");
  fprintf(stderr, "default baud rate is %d if nothing is specified\n", DEFAULT_BAUD_RATE);
}

int main(int argc, char **argv) {

  int fd;
  ssize_t readlen;
  ssize_t writelen;
  unsigned char buf[BUF_SIZE];
  struct termios tio;
  int baudrate = DEFAULT_BAUD_RATE;

  if (argc < 2 || (0 == strcmp(argv[1], "-h"))) {
    usage();
    return -1;
  }

  if (argc > 2) {
    baudrate = atoi(argv[2]);
  }

  fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Failed to open %s. errno = %d (%s).\n", argv[1], errno,
	    strerror(errno));
    return -1;
  }

  cfmakeraw(&tio);

  if (cfsetspeed(&tio, baudrate) < 0) {
    fprintf(stderr, "Failed to set baudrate to %d. errno = %d (%s).\n",
	    baudrate, errno, strerror(errno));
    return -1;
  }

  if (tcsetattr(fd,TCSANOW,&tio) < 0) {
    fprintf(stderr, "tcsetattr failed. errno = %d (%s).\n", errno,
	    strerror(errno));
    return -1;
  }

  for (;;) {
    readlen = read(fd, &buf[0], sizeof(buf));
    if (readlen > 0) {
      writelen = writen(STDOUT_FILENO, buf, readlen);
      if (writelen < 0) {
	fprintf(stderr, "Error writing to stdout. errno = %d (%s).\n",
		errno, strerror(errno));
      }
    }
    (void) usleep(100);
  }

  return 0;
}
