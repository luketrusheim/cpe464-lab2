/******************************************************************************
 * myServer.c
 *
 * Writen by Prof. Smith, updated Jan 2023
 * Use at your own risk.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "recvPDU.h"
#include "sendPDU.h"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void addNewSocket(int mainServerSocket);
void processClient(int polledSocket);
void serverControl(int mainServerSocket);
void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0; // socket descriptor for the server socket
	int portNumber = 0;

	portNumber = checkArgs(argc, argv);

	// create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	serverControl(mainServerSocket);

	/* close the socket */
	close(mainServerSocket);

	return 0;
}

void addNewSocket(int mainServerSocket) {
	int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	addToPollSet(clientSocket);
	printf("New client connected | Socket: %d\n", clientSocket);
}

void processClient(int clientSocket) {
	recvFromClient(clientSocket);
}

void serverControl(int mainServerSocket)
{
	int readySocket = 0;
	setupPollSet();
	addToPollSet(mainServerSocket);
	
	while (1) {
		readySocket = pollCall(POLL_WAIT_FOREVER);
		if (readySocket == mainServerSocket) {
			addNewSocket(readySocket);
		} else{
			processClient(readySocket);
		}
	}
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;

	// now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		//this should never happen
		perror("recv from client negative return");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Socket %d | Bytes recv: %d | Message: %s\n", clientSocket, messageLen, dataBuffer);

		// send it back to client (just to test sending is working... e.g. debugging)
		messageLen = sendPDU(clientSocket, dataBuffer, messageLen);
		printf("Socket: %d | Bytes sent: %d | Message: %s\n", clientSocket, messageLen, dataBuffer);
	}
	else
	{
		removeFromPollSet(clientSocket);
		printf("Socket: %d | Connection closed by other side\n", clientSocket);
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}

	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}

	return portNumber;
}
