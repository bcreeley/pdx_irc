/*
 * Example of client using UDP protocol.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	"inet.h"

/*
 * Read the contents of the FILE *fp, write each line to the
 * datagram socket, then read a line back from the datagram
 * socket and write it to the standard output.
 *
 * Return to caller when an EOF is encountered on the input file.
 */

#define	MAXLINE	512

dg_cli(fp, sockfd, pserv_addr, servlen)
FILE		*fp;
int		sockfd;
struct sockaddr	*pserv_addr;	/* ptr to appropriate sockaddr_XX structure */
int		servlen;	/* actual sizeof(*pserv_addr) */
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (fgets(sendline, MAXLINE, fp) != NULL) {
		n = strlen(sendline);
		if (sendto(sockfd, sendline, n, 0, pserv_addr, servlen) != n)
			perror("dg_cli: sendto error on socket");

		/*
		 * Now read a message from the socket and write it to
		 * our standard output.
		 */

		n = recvfrom(sockfd, recvline, MAXLINE, 0,
				(struct sockaddr *) 0, (int *) 0);
		if (n < 0)
			perror("dg_cli: recvfrom error");
		recvline[n] = 0;	/* null terminate */
		fputs(recvline, stdout);
	}

	if (ferror(fp))
		perror("dg_cli: error reading file");
}
main(argc, argv)
int	argc;
char	*argv[];
{
	int			sockfd;
	struct sockaddr_in	cli_addr, serv_addr;

	pname = argv[0];

	/*
	 * Fill in the structure "serv_addr" with the address of the
	 * server that we want to send to.
	 */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port        = htons(SERV_UDP_PORT);

	/*
	 * Open a UDP socket (an Internet datagram socket).
	 */

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("client: can't open datagram socket");

	/*
	 * Bind any local address for us.
	 */

	bzero((char *) &cli_addr, sizeof(cli_addr));	/* zero out */
	cli_addr.sin_family      = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port        = htons(0);
	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
		perror("client: can't bind local address");

	dg_cli(stdin, sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	close(sockfd);
	exit(0);
}
