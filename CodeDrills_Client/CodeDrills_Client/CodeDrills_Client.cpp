/*
** client.c -- a stream socket client demo
*/

#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sys/types.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 256 // max number of bytes we can get at once 

using namespace std;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

	WSADATA wsa;
	int error = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (error != 0)
	{
		printf("An error in startup %d\n", WSAGetLastError());
		system("pause");
	}

	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	string host;
	string username;

	cout << "Enter username: ";
	getline(cin, username);

	cout << "Enter host: ";
	getline(cin, host);

	if (argc != 2) {
		fprintf(stderr, "usage: client hostname\n");
		system("pause");
		exit(1);
	}

	Restart:

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host.c_str(), PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		system("pause");
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			closesocket(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		char i;
		cout << "Would you like to try again?" << '\n';
		cin >> i;
		if (i == 'y') {
			goto Restart;
		}
		else if (i == 'n') {
			return 0;
		}
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if (send(sockfd, username.c_str(), 30, 0) == -1)perror("send");

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
		perror("recv");
		system("pause");
		exit(1);
	}

	while (1) {
		RCV_LOOP:
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
			perror("recv");
			system("pause");
			exit(1);
		}

		buf[numbytes] = '\0';

		printf("%s \n", buf);
		goto RCV_LOOP;
	}
	buf[numbytes] = '\0';

	printf("%s \n", buf);

	closesocket(sockfd);
	system("pause");
	return 0;
}