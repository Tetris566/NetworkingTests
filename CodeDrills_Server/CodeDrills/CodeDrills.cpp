// CodeDrills.cpp : Defines the entry point for the console application.
//
// This is just for practicing.

#include "stdafx.h"
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <ws2tcpip.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "3490"    // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXDATASIZE 256 // max number of bytes we can get at once 

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	//while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
#pragma region WinSock Startup

	WSADATA wsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);					// This starts Winsock. It needs to be run early.
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

#pragma endregion

	int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information

	char buf[MAXDATASIZE];

	socklen_t sin_size;
	//struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	std::vector< int > FD_Arr;
	std::vector< std::string > Client_Names;

	std::string serverName = "SERVER";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, "1",
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			closesocket(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		FD_Arr.push_back(new_fd);

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		closesocket(sockfd); // child doesn't need the listener

		if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
			perror("recv");
			system("pause");
			exit(1);
		}

		buf[numbytes] = '\0';

		Client_Names.push_back(buf);

		printf("Client Name: '%s'\n", buf);

		if (send(new_fd, serverName.c_str(), 30, 0) == -1) {
			perror("send");
		}
		else {
			break;
		}

	}

	while (new_fd) {
	MSG_SEND:
		printf("Message: ");
		std::string message = "SERVER: ";

		std::string input = "";
		std::getline(std::cin, input);

		message.append(input);

		for (int i = 0; i < FD_Arr.size(); i++) {
			if (send(FD_Arr[i], message.c_str(), 30, 0) == -1)perror("send");
		}
		
		goto MSG_SEND;
	}

	closesocket(new_fd);

	printf("User disconnected!\n");
	system("pause");

	return 0;

	WSACleanup();													// Run this when sockets are done being used.
	system("pause");
}

