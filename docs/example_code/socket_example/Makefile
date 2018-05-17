#

LIBS	= -lsocket -lnsl
CFLAGS	= -O

all:	tcp udp

#
# Internet stream version (TCP protocol).
#

tcp:	tcpserv tcpcli

tcpcli.o tcpserv.o: inet.h

tcpserv:	tcpserv.o readn.o writen.o readline.o
		cc $(CFLAGS) -o $@ tcpserv.o readn.o writen.o readline.o $(LIBS)

tcpcli:		tcpcli.o readn.o writen.o readline.o
		cc $(CFLAGS) -o $@ tcpcli.o readn.o writen.o readline.o $(LIBS)

#
# Internet datagram version (UDP protocol).
#

udp:	udpserv udpcli

udpcli.o udpserv.o: inet.h

udpserv:	udpserv.o readn.o writen.o readline.o
		cc $(CFLAGS) -o $@ udpserv.o readn.o writen.o readline.o $(LIBS)

udpcli:		udpcli.o readn.o writen.o readline.o
		cc $(CFLAGS) -o $@ udpcli.o readn.o writen.o readline.o $(LIBS)

#
clean:
	-rm -f *.o core a.out temp*.* \
		tcpserv tcpcli udpserv udpcli
