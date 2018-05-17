/*
 * Example of client using TCP protocol.
 */

#include	<stdio.h>
#include	"inet.h"


/*
 * Read the contents of the FILE *fp, write each line to the
 * stream socket (to the server process), then read a line back from
 * the socket and write it to the standard output.
 *
 * Return to caller when an EOF is encountered on the input file.
 */

#define	MAXLINE	512

str_cli(fp, sockfd)
register FILE	*fp;
register int	sockfd;
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (fgets(sendline, MAXLINE, fp) != NULL) {
		n = strlen(sendline);
		if (writen(sockfd, sendline, n) != n)
			perror("str_cli: writen error on socket");

		/*
		 * Now read a line from the socket and write it to
		 * our standard output.
		 */

		n = readline(sockfd, recvline, MAXLINE);
		if (n < 0)
			perror("str_cli: readline error");
		recvline[n] = 0;	/* null terminate */
		fputs(recvline, stdout);
	}

	if (ferror(fp))
		perror("str_cli: error reading file");
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int			sockfd;
	struct sockaddr_in	serv_addr;

	pname = argv[0];

	/*
	 * Fill in the structure "serv_addr" with the address of the
	 * server that we want to connect with.
	 */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port        = htons(SERV_TCP_PORT);

	/*
	 * Open a TCP socket (an Internet stream socket).
	 */

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("client: can't open stream socket");

	/*
	 * Connect to the server.
	 */

	if (connect(sockfd, (struct sockaddr *) &serv_addr,
							sizeof(serv_addr)) < 0)
		perror("client: can't connect to server");

	str_cli(stdin, sockfd);		/* do it all */

	close(sockfd);
	exit(0);
}
