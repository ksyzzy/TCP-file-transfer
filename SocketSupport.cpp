// SocketSupport.cpp : Defines the exported functions for the DLL application.
//

#define PORT_LENGTH 256
#include "stdafx.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

extern "C" {

	__declspec(dllexport) int sendFile() {

	}

	__declspec(dllexport) int sendData() {

	}

	__declspec(dllexport) int receiveFile() {

	}

	__declspec(dllexport) int receiveData() {

	}

	DWORD WINAPI clientThread(LPVOID data) {
		SOCKET client = *static_cast<SOCKET*>(data);
		delete data;
		return 0;
	}

	__declspec(dllexport) int listen() {
		WORD RequiredVersion;
		WSADATA WData;
		SOCKET listenSocket, clientSocket;
		struct sockaddr_in listener, incoming;
		int addressLength;
		long val;
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
			fgets(port, sizeof(int), stdin);
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

		while (!shouldEndListening) {
			addressLength = sizeof(incoming);
			clientSocket = accept(listenSocket, (struct sockaddr*) &listener, &addressLength);

			if (clientSocket < 0) {
				printf("Main process: Failed to accept incoming connection\n");
				continue;
			}
			printf("Main process: Incoming connection from %s:%u\n", inet_ntoa(listener.sin_addr), ntohs(listener.sin_port));

			printf("Main process: Creating new thread to support client connection...\n");

			HANDLE thread = CreateThread(NULL, 0, clientThread, &clientSocket, 0, NULL);

			if (thread) {
				printf("Client process: Thread created successfully. Handling client connection...\n");
			}
		}
		return 0;
	}
}


