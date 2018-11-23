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
	SOCKET clientSocket;

	__declspec(dllexport) int CheckConnectionStatus() {
		if (connectionEstablished) {
			return 1;
		}
		else {
			return -1;
		}
	}

	__declspec(dllexport) int GetFileList() {
		int instruction, received;
		FILE* file;
		long fileLength, bytesRead, totalRead;
		char buffer[BUFFER_SIZE+1];

		/*Request for file list transfer*/
		instruction = 1; 
		instruction = htonl(instruction);

		/*Send instruction to the server*/
		send(clientSocket, (char*)&instruction, sizeof(long), 0);
		printf("Instruction sent successfully\n");

		/*Wait for confirmation*/
		recv(clientSocket, (char*)&received, sizeof(long), 0);
		received = ntohl(received);
		
		switch (received) {
		case 1:
			printf("No errors encountered. Beginning file transfer...\n");
			break;
		case 2:
			printf("Server could not acquire filelist information. Aborting...\n");
			return -1;
			break;
		case 3:
			printf("File is empty. Aborting operation...\n");
			return -2;
			break;
		default:
			printf("Unknown error has occured while receiving confirmation from the server. Aborting operation...\n");
			return -9;
			break;
		}

		buffer[0] = 0;

		if (recv(clientSocket, (char*)&fileLength, sizeof(long), 0) != sizeof(long)) {
			printf("Error has occured while receiving filelist size\n");
			return -3;
		}

		printf("File length received successfully\n");

		fileLength = ntohl(fileLength);
		totalRead = 0;

		/*Creating file or updating already existing one*/
		file = fopen("catlist.txt", "wb");

		while (totalRead < fileLength) {
			memset(buffer, 0, BUFFER_SIZE + 1);
			bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
			if (bytesRead < 0)
				break;
			totalRead += bytesRead;
			fprintf(file, buffer);
		}
		printf("File received successfully from the server\n");
		fclose(file);
		return 1;
	}

	__declspec(dllexport) void TerminateClientConnection() {
		long instruction;

		instruction = 99;
		instruction = htonl(instruction);
		send(clientSocket, (char*)&instruction, sizeof(long), 0);
		shouldTerminateConnection = TRUE;
		WakeByAddressSingle(&shouldTerminateConnection);
		return;
	}

	__declspec(dllexport) int DownloadImage() {
		return 1;
	}

	__declspec(dllexport) int UploadImage(char text[]) {
		return 1;
	}

	__declspec(dllexport) void HandleClientConnection(SOCKET clientsock) {
		long instruction;
		bool handleConnection = TRUE;
		char filename[512];
		long fileLength, bytesSent, totalSent, bytesRead;
		FILE* file;
		struct stat fileinfo;
		unsigned char buffer[BUFFER_SIZE];

		/*Keep handling client connection on separate thread until given termination signal*/
		while (handleConnection) {
			instruction = 0;

			/*Awaiting instruction from client*/
			recv(clientsock, (char*)&instruction, sizeof(long), 0);
			printf("Thread: Received instruction, analyzing...\n");

			instruction = ntohl(instruction);

			switch (instruction) {
			case 1:
				/*Request for file list transfer*/
				printf("Thread: Received request for file list transfer, beginning...\n");		

				filename[0] = 0;
				sprintf(filename, "%s", "catlist.txt");

				if (stat(filename, &fileinfo) < 0)
				{
					printf("Thread: cannot acquire file information\n");
					instruction = 2;
					instruction = htonl(instruction);
					send(clientsock, (char*)&instruction, sizeof(long), 0);
					continue;
				}

				if (fileinfo.st_size == 0)
				{
					printf("Thread: file is empty\n");
					instruction = 3;
					instruction = htonl(instruction);
					send(clientsock, (char*)&instruction, sizeof(long), 0);
					continue;
				}

				/*Send information about that no errors were encountered*/
				instruction = 1;
				instruction = htonl(instruction);
				send(clientsock, (char*)&instruction, sizeof(long), 0);

				printf("Thread: length of file: %d\n", fileinfo.st_size);

				fileLength = htonl((long)fileinfo.st_size);

				if (send(clientsock, (char*)&fileLength, sizeof(long), 0) != sizeof(long)) {
					printf("Thread: error while sending file size information\n");
					continue;
				}

				fileLength = fileinfo.st_size;
				totalSent = 0;
				file = fopen(filename, "rb");
				buffer[0] = 0;

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
				break;
			case 2:
				/*Request for image transfer*/

				break;
			case 3:
				/*Request for image upload*/

				break;
			case 99:
				/*Termination signal*/
				printf("Thread: Termination signal from client received, closing connection\n");
				return;
				break;
			}
		}
	}

	__declspec(dllexport) DWORD WINAPI ClientThread(LPVOID data) {
		SOCKET client = *static_cast<SOCKET*>(data);
		HandleClientConnection(client);
		closesocket(client);
		return 0;
	}

	__declspec(dllexport) void BeginServerOperation() {
		WORD RequiredVersion;
		WSADATA WData;
		SOCKET listenSocket, receiveSocket;
		struct sockaddr_in listener;
		int addressLength;
		char port[PORT_LENGTH];
		bool isOkay = FALSE;

		RequiredVersion = MAKEWORD(2, 0);

		/*Try initializing WinsSock2*/
		if (WSAStartup(RequiredVersion, &WData) != 0) {
			printf("Main process: Failed to initialize Winsock2\n");
			return;
		}

		printf("Input a port number");
		/*Acquire port number*/
		while (!isOkay) {
			port[0] = 0;
			fflush(stdout);
			fgets(port, PORT_LENGTH, stdin);
			if ((port[0] != 0) && (atoi(port) != 0)) {
				isOkay = TRUE;
			}
		}

		/*Preparing structure for listening process*/
		listener.sin_family = AF_INET;
		listener.sin_port = htons(atoi(port));
		listener.sin_addr.s_addr = INADDR_ANY;

		/*Create and bind socket for listening*/
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (bind(listenSocket, (struct sockaddr*) &listener, sizeof(listener)) < 0) {
			printf("Main process: Binding the listen socket failed\n");
			return;
		}

		listen(listenSocket, 10);

		addressLength = sizeof(listener);

		while (TRUE) {
			receiveSocket = accept(listenSocket, (struct sockaddr*) &listener, &addressLength);

			if (receiveSocket < 0) {
				printf("Main process: Failed to accept incoming connection\n");
				continue;
			}
			printf("Main process: Incoming connection from %s:%u\n", inet_ntoa(listener.sin_addr), ntohs(listener.sin_port));

			printf("Main process: Creating new thread to support client connection...\n");

			HANDLE thread = CreateThread(NULL, 0, ClientThread, &receiveSocket, 0, NULL);

			if (thread) {
				printf("Client process: Thread created successfully. Handling client connection until termination...\n");
			}
		}
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	__declspec(dllexport) int BeginClientOperation(char host[], int port) {
		WORD RequiredVersion;
		WSADATA WData;
		struct sockaddr_in addr;
		struct hostent *he;
		bool undesiredState = FALSE;

		RequiredVersion = MAKEWORD(2, 0);

		/*Try initializing WinSock2*/
		if (WSAStartup(RequiredVersion, &WData) != 0) {
			printf("Failed to initialize WinSock2\n");
			return -1;
		}

		/*Find host destination*/
		he = gethostbyname(host);

		/*Preparing sockaddr_in structure for socket*/
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = *((unsigned long*)he->h_addr);

		/*Try to establish a connection with the server*/
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET) {
			printf("Failed to create a socket\n");
			WSACleanup();
			return -2;
		}

		if (connect(clientSocket, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR) {
			printf("Failed to connect\n");
			WSACleanup();
			return -3;
		}

		connectionEstablished = TRUE;

		/*Wait for termination signal from function TerminateClientConnection()*/
		WaitOnAddress(&shouldTerminateConnection, &undesiredState, sizeof(bool), INFINITE);
		printf("Received termination signal. Terminating...\n");
		closesocket(clientSocket);
		connectionEstablished = FALSE;
		shouldTerminateConnection = FALSE;
		WSACleanup();

		return 1;
	}
}


