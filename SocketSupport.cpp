// SocketSupport.cpp : Defines the exported functions for the DLL application.
//

#define PORT_LENGTH 256
#define BUFFER_SIZE 1024
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdafx.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#pragma comment(lib, "Ws2_32.lib")

extern "C" {

	__declspec(dllexport) int GetFileList(char host[], int port) {
		WORD RequiredVersion;
		WSADATA WData;
		SOCKET s;
		struct sockaddr_in addr;
		struct hostent *he;
		long instruction;
		FILE* file;
		long fileLength, bytesRead, totalRead;
		char buffer[BUFFER_SIZE+1];

		RequiredVersion = MAKEWORD(2, 0);

		if (WSAStartup(RequiredVersion, &WData) != 0) {
			printf("Failed to initialize WinSock2\n");
			return -1;
		}

		he = gethostbyname(host);

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = *((unsigned long*)he->h_addr);

		printf("Structure prepared successfully\n");

		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		connect(s, (struct sockaddr*) &addr, sizeof(addr));

		printf("Connected successfully\n");
		
		instruction = 1; // Request for file list transfer
		instruction = htonl(instruction);
		send(s, (char*)&instruction, sizeof(long), 0); // Send instruction
		printf("Instruction sent successfully\n");

		if (recv(s, (char*)&fileLength, sizeof(long), 0) != sizeof(long)) { // Receiving file list size
			closesocket(s);
			WSACleanup();
			return -2;
		}

		printf("File length received successfully\n");

		fileLength = ntohl(fileLength);
		totalRead = 0;
		file = fopen("catlist.txt", "wb");

		while (totalRead < fileLength) {
			memset(buffer, 0, BUFFER_SIZE + 1);
			bytesRead = recv(s, buffer, BUFFER_SIZE, 0);
			if (bytesRead < 0)
				break;
			totalRead += bytesRead;
			fprintf(file, buffer);
		}
		fclose(file);
		closesocket(s);
		return 1;
	}

	__declspec(dllexport) int DownloadImage() {
		return 1;
	}

	__declspec(dllexport) int SendImage(char text[]) {
		return 1;
	}

	__declspec(dllexport) void HandleClientConnection(SOCKET clientsock) {
		printf("Hello from HandleClientConnection\n");
		printf("client is %d in HandleClientConnection\n", (int)clientsock);
		long instruction;
		bool handleConnection = TRUE;
		char filename[512] = "catlist.txt"; // Preparing for file transfer
		long fileLength, bytesSent, totalSent, bytesRead;
		FILE* file;
		struct stat fileinfo;
		unsigned char buffer[BUFFER_SIZE];

		while (handleConnection) { // Keep handling the client until given a termination signal
			instruction = 0;
			recv(clientsock, (char*)&instruction, sizeof(long), 0); // Awaiting instructions from the client
			printf("Thread: Received instruction, analyzing...\n");

			instruction = ntohl(instruction);

			switch (instruction) {
			case 1: // Request for file list transfer
				printf("Thread: Received request for file list transfer, beginning...\n");

				if (stat(filename, &fileinfo) < 0)
				{
					printf("Thread: cannot acquire file information\n");
					return;
				}

				if (fileinfo.st_size == 0)
				{
					printf("Thread: file is empty\n");
					return;
				}

				printf("Thread: length of file: %d\n", fileinfo.st_size);

				fileLength = htonl((long)fileinfo.st_size);

				if (send(clientsock, (char*)&fileLength, sizeof(long), 0) != sizeof(long)) {
					printf("Thread: error while sending file size information\n");
					return;
				}

				fileLength = fileinfo.st_size;
				totalSent = 0;
				file = fopen(filename, "rb");

				while (totalSent < fileLength) {
					bytesRead = fread(buffer, 1, 1024, file);
					bytesSent = send(clientsock, (char*)buffer, bytesRead, 0);
					if (bytesRead != bytesSent) {
						break;
					}
					totalSent += bytesSent;
					printf("Thread: Sent %d bytes\n", totalSent);
				}

				if (totalSent == fileLength) {
					printf("Thread: file has been sent successfully\n");
				}
				else {
					printf("Thread: error while sending file\n");
				}
				fclose(file);
				handleConnection = FALSE;
				break;
			case 2: // Request for image transfer

				break;
			}
		}
	}

	__declspec(dllexport) DWORD WINAPI ClientThread(LPVOID data) {
		SOCKET client = *static_cast<SOCKET*>(data);
		printf("Hello from ClientThread\n");
		printf("client in ClientThread is %d\n", (int)client);
		HandleClientConnection(client);
		closesocket(client);
		return 0;
	}

	__declspec(dllexport) int BeginServerOperation() {
		WORD RequiredVersion;
		WSADATA WData;
		SOCKET listenSocket, clientSocket;
		struct sockaddr_in listener, incoming;
		int addressLength;
		char port[PORT_LENGTH];
		bool isOkay = FALSE, shouldEndListening = FALSE;

		RequiredVersion = MAKEWORD(2, 0); // Deciding on required version

		if (WSAStartup(RequiredVersion, &WData) != 0) {
			printf("Main process: Failed to initialize Winsock2\n");
			return -1;
		}

		while (!isOkay) { // Acquiring port number
			port[0] = 0;
			fflush(stdout);
			fgets(port, PORT_LENGTH, stdin);
			if ((port[0] != 0) && (atoi(port) != 0)) {
				isOkay = TRUE;
			}
		}

		listener.sin_family = AF_INET; // Preparing structure for listening process
		listener.sin_port = htons(atoi(port));
		listener.sin_addr.s_addr = INADDR_ANY;

		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Creating socket for listening
		if (bind(listenSocket, (struct sockaddr*) &listener, sizeof(listener)) < 0) { // Binding the listen socket
			printf("Main process: Binding the listen socket failed\n");
			return -1;
		}

		listen(listenSocket, 10); // Active listening on listen socket
		printf("listenSocket is %d on listen()\n", (int)listenSocket);
		addressLength = sizeof(incoming);

		int i = 0;

		while (!shouldEndListening) {
			printf("Beginning %d iteration\n", i);
			clientSocket = accept(listenSocket, (struct sockaddr*) &listener, &addressLength);
			printf("clientSocket is %d on accept()\n", (int)clientSocket);

			if (clientSocket < 0) {
				printf("Main process: Failed to accept incoming connection\n");
				continue;
			}
			printf("Main process: Incoming connection from %s:%u\n", inet_ntoa(listener.sin_addr), ntohs(listener.sin_port));

			printf("Main process: Creating new thread to support client connection...\n");

			HANDLE thread = CreateThread(NULL, 0, ClientThread, &clientSocket, 0, NULL);

			if (thread) {
				printf("Client process: Thread created successfully. Handling client connection until termination...\n");
			}
		}
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
}
