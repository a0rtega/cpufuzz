
/*
cpufuzz - https://github.com/a0rtega/cpufuzz

MIT License
Copyright (c) 2019 Alberto Ortega
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* Needed if we use SYS_getrandom syscall
#include <sys/syscall.h>
#include <linux/random.h>
*/

#define TESTCASES_SIZE 16
#define CHILD_MAXTIME   5 /* seconds */

static inline void send_testcase(unsigned char * test_buffer, int sock, char * hex) {
	unsigned int i;
	for (i = 0; i < TESTCASES_SIZE; i++) {
		sprintf(hex+(i*4), "\\x%02X", test_buffer[i]);
	}
	send(sock, hex, TESTCASES_SIZE * 4, 0);
	send(sock, "\r\n", 2, 0);
}

int main(int argc, char * argv[]) {
	void * test_buffer;
	char * test_hex = malloc(TESTCASES_SIZE * 4);
	pid_t pid;
	int pid_status;
	int t_seconds;
	char * output_server;
	long int output_server_port = 0;
	struct timespec tim;
	FILE * dev_urandom;
	size_t read_f;
	tim.tv_sec = 0;
	tim.tv_nsec = 10000000L;

	struct sockaddr_in server;
	int sock;

	if ((argc == 3) && (output_server_port = strtol(argv[2], NULL, 10))) {
		output_server = argv[1];
	}
	else {
		fprintf(stdout, "%s <output_ip_addr> <port>\n", argv[0]);
		return 1;
	}

	/* Connect output server socket */
	sock = socket(AF_INET , SOCK_STREAM , 0);
	server.sin_addr.s_addr = inet_addr(output_server);
	server.sin_family = AF_INET;
	server.sin_port = htons(output_server_port);
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "Connect to output server failed, abort!\n");
		return 1;
	}

	dev_urandom = fopen("/dev/urandom", "rb");

	fprintf(stderr, "Mapping memory for testcases ... ");
	test_buffer = mmap(NULL, TESTCASES_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	fprintf(stderr, "Done! Mapped at %p\n", test_buffer);

	for (;;) {
		/* Original method to get random data (SYS_getrandom),
			replaced by /dev/urandom for portability.
		syscall(SYS_getrandom, test_buffer, TESTCASES_SIZE, NULL);
		*/
		read_f = fread(test_buffer, TESTCASES_SIZE, 1, dev_urandom);
		if (!read_f)
			continue;
		/* Send buffer to output server */
		send_testcase((unsigned char *)test_buffer, sock, test_hex);
		/* Fork process */
		pid = fork();
		if (pid == 0) {
			/* Execute random data buffer */
			((void(*)())test_buffer)();
		}
		else {
			/* Monitor child process */
			t_seconds = (int)time(NULL);
			while (!waitpid(pid, &pid_status, WNOHANG)) {
				if ((int)time(NULL) >= (t_seconds + CHILD_MAXTIME)) {
					printf("Killing %i ...\n", pid);
					kill(pid, SIGKILL);
				}
				nanosleep(&tim , NULL);
			}
		}
	}
	return 0;
}

