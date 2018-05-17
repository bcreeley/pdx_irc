/*
 * Example of server using UDP protocol.
 */

#include	"inet.h"
#include	<sys/types.h>
#include	<sys/socket.h>

/*
 * Read a datagram from a connectionless socket and write it back to
 * the sender.
 *
 * We never return, as we never know when a datagram client is done.
 */

#define	MAXMESG	2048

dg_echo(sockfd, pcli_addr, maxclilen)
int		sockfd;
struct sockaddr	*pcli_addr;	/* ptr to appropriate sockaddr_XX structure */
int		maxclilen;	/* sizeof(*pcli_addr) */
{
	int	n, clilen;
	char	mesg[MAXMESG];

	for ( ; ; ) {
		clilen = maxclilen;
		n = recvfrom(sockfd, mesg, MAXMESG, 0, pcli_addr, &clilen);
		if (n < 0)
			perror("dg_echo: recvfrom error");

		if (sendto(sockfd, mesg, n, 0, pcli_addr, clilen) != n)
			perror("dg_echo: sendto error");
	}
}
main(argc, argv)
int	argc;
char	*argv[];
{
	int			sockfd;
	struct sockaddr_in	serv_addr, cli_addr;

	pname = argv[0];

	/*
	 * Open a UDP socket (an Internet datagram socket).
	 */

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("server: can't open datagram socket");

	/*
	 * Bind our local address so that the client can send to us.
	 */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(SERV_UDP_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		perror("server: can't bind local address");

	dg_echo(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr));

		/* NOTREACHED */
}
