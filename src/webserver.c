/**
 * A simple multi-threaded web server.  The number of threads allowed is fixed.
 *
 * @author Mark Royer
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h> // for struct sockaddr_in
#include <arpa/inet.h> // for inet_pton
#include <stdbool.h> // for true, false
#include <unistd.h> // for close
#include <pthread.h> // for multi-threading
#include "webserver.h"

/**
 * Entrance to the program.
 *
 * @param argc The number of program parameters including the program name.
 * @param argv Points to
 */
int main(int argc, char *argv[]) {

	// Set the port number.
	int port = 1024;
	char* ipaddress = "127.0.0.1";

	if (argc < 2) {
		printf("usage: webserver ipaddress [port]\n");
		printf("By default the webserver runs ");
		printf("with ip-address %s and port %d.\n", ipaddress, port);
	}
	if (argc >= 2) {
		ipaddress = argv[1];
		printf("IP Address is %s\n", argv[1]);
	}

	if (argc >= 3) {
		port = atoi(argv[2]);
		printf("Port is %s\n", argv[2]);
	}

	int err;

	struct sockaddr_in sa;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	inet_pton(AF_INET, ipaddress, &(sa.sin_addr));

	int my_socket = socket(AF_INET,SOCK_STREAM, 0);

	err = bind(my_socket, (struct sockaddr *) &sa, sizeof(sa));

	/*
	 * Check to see if the port is open.
	 */
	if (err != 0) {
		perror("BIND PROBLEM");
		exit(0);
	}

	listen(my_socket, 1);

	pthread_t thread;

	printf("Listening for HTTP request on port %d.\n", port);
	fflush(stdout);

	/*
	 * Process HTTP service requests in an infinite loop.
	 */
	while (true) {

		socklen_t len = sizeof(sa);

		int my_socket1 = accept(my_socket, (struct sockaddr*) &sa, &len);

		/*
		 * Check for a valid socket connection.
		 */
		if (my_socket1 > 0) {

			//			printf("Received request.\n");
			//			fflush(stdout);

			int rc;

			pthread_mutex_lock(&countLock);

			/*
			 * Only serve the request if there are not too many threads.
			 */
			if (threadCount <= NUM_THREADS) {

				ThreadArgs* args = calloc(1, sizeof(ThreadArgs));
				args->socket = my_socket1;
				args->threadNumber = threadCount;

				// process the request
				rc = pthread_create(&thread, NULL,(void*) processRequest, args);
				if (rc) {
					printf("ERROR; return code from pthread_create() is %d\n",
							rc);
					exit(-1);
				}

				threadCount++;
			}
			pthread_mutex_unlock(&countLock);

		}
	}

	// The following lines will never get executed.

	/*
	 * Close the listening socket
	 */
	close(my_socket);

	return EXIT_SUCCESS;
}

/**
 * Try to service the http request on the given socket.
 *
 * @param args The arguments to the method.  A socket and a thread number.
 */
void *processRequest(ThreadArgs* args) {
	pthread_detach(pthread_self());

	int new_socket = args->socket;

	const char* CRLF = "\r\n";

	// Get the request line of the HTTP request message.
	const char* requestLine = readLine(new_socket);

	// Get the header lines.
	const char* tmp = calloc(1, sizeof(char));
	while (strcmp(CRLF, tmp)) {
		free((char*) tmp);
		tmp = readLine(new_socket);
	}
	free((char*) tmp);

	const char* fileName = getFileName(requestLine);

	tmp = append(WEBDIRECTORY, fileName);

	FILE* inputFilePtr = fopen(tmp, "r");
	free((char*) tmp);

	// Construct the response message.
	const char* statusLine = NULL;
	const char* contentTypeLine = NULL;
	const char* entityBody = NULL;
	bool fileExists = inputFilePtr != NULL;

	// Don't serve directories
	if (endsWith(fileName, "/")) {
		fileExists = false;
	}

	if (fileExists) {
		statusLine = append("HTTP/1.0 200 OK", CRLF);
		const char* tmp = append("Content-type: ", contentType(fileName));
		contentTypeLine = append(tmp, CRLF);
		free((char*) tmp);
	} else {
		statusLine = append("HTTP/1.0 404 Not Found", CRLF);
		contentTypeLine = append("Content-type: text/html", CRLF);
		entityBody
				= "<HTML><HEAD><TITLE>Not Found</TITLE></HEAD><BODY>\
								Not Found</BODY></HTML>";
	}

	// Send the status line.
	writeBytes(new_socket, statusLine);

	// Send the content type line.
	writeBytes(new_socket, contentTypeLine);

	// Send a blank line to indicate the end of the header lines.
	writeBytes(new_socket, CRLF);

	// Send the entity body.
	if (fileExists) {
		sendBytes(inputFilePtr, new_socket);
		fclose(inputFilePtr);
	} else {
		writeBytes(new_socket, entityBody);
	}

	// Close the socket.
	close(new_socket);
	free((char*) requestLine);
	free((char*) fileName);
	free((char*) statusLine);
	free((char*) contentTypeLine);
	free((ThreadArgs*) args);

	pthread_mutex_lock(&countLock);
	threadCount--;
	pthread_mutex_unlock(&countLock);

	return (NULL);
}

/**
 * Read a line that ends with '\r\n' from the given socket.
 *
 * @param my_socket
 * @return The line minus the '\r\n'
 */
const char* readLine(int socket) {
	int pos = 0;
	int tmp = 1;
	int done = 0;
	char* my_buf = calloc(1024, sizeof(char));

	while (tmp > 0 && done != 1) {
		tmp = recv(socket, my_buf + pos, 1, 0);
		if (pos > 0 && my_buf[pos - 1] == '\r' && my_buf[pos] == '\n') {
			done = 1;
		}
		pos = pos + 1;
	}

	char* resultString = strdup(my_buf);

	free(my_buf);

	return resultString;
}

/**
 * Returns the content type string for the given file name.
 *
 * @param fileName The name of the file
 * @return The content type string, eg, "image/gif" for the file myImage.gif
 */
const char* contentType(const char* fileName) {
	if (endsWith(fileName, ".htm") || endsWith(fileName, ".html")) {
		return "text/html";
	}
	if (endsWith(fileName, ".css")) {
		return "text/css";
	}
	if (endsWith(fileName, ".js")) {
		return "text/javascript";
	}
	if (endsWith(fileName, ".jpg") || endsWith(fileName, ".jpeg")) {
		return "image/jpeg";
	}
	if (endsWith(fileName, ".gif")) {
		return "image/gif";
	}

	return "application/octet-stream";
}

/**
 * @param string The string to check the end of
 * @param ending The ending to match against the string
 * @return True iff the string ends with the given ending
 */
bool endsWith(const char* string, const char* ending) {

	int endLength = strlen(ending);
	int strOffset = strlen(string) - endLength;
	int count = 0;
	bool sameCharacter = true;

	// If the ending is longer than the string itself it can't end with
	// the ending!
	if (strOffset < 0) {
		return false;
	}

	while (sameCharacter && count < endLength) {
		sameCharacter = string[strOffset + count] == ending[count];
		count++;
	}

	return sameCharacter;
}

/**
 * Writes the string to the given socket.
 *
 * @param socket The socket to write to
 * @param string The string that will be written to the socket
 */
void writeBytes(int socket, const char* string) {

	int numBytes = strlen(string);
	int sentBytes = 0;
	int tmp = 0;

	while (sentBytes < numBytes && tmp >= 0) {
		//		printf("%s", string + sentBytes);
		//		fflush(stdout);
		tmp = send(socket, string + sentBytes, numBytes - sentBytes, 0);
		sentBytes += tmp;
	}

}

/**
 * Writes the bytes from the input stream to the output stream.
 *
 * @param fis The input stream
 * @param os The output stream
 */
void sendBytes(FILE* fis, int os) {

	int amnt;
	rewind(fis);

	char buff[1024];

	int sent;
	int tmp = 1;
	while ((amnt = fread(buff, 1, 1024, fis)) > 0 && tmp >= 0) {
		sent = 0;
		while (sent < amnt && tmp >= 0) {
			tmp = send(os, buff + sent, amnt - sent, 0);

			sent += tmp;
		}
	}
}

/**
 * Creates a new string that is the two strings concatenated. The user is
 * responsible for freeing the memory of the returned string.
 *
 *@param str1 Beginning of the returned string
 *@param str2 End of the returned string
 *@return New string that is the two given strings combined
 */
const char* append(const char* str1, const char* str2) {

	int str1Length = strlen(str1);
	int str2Length = strlen(str2);

	char* result = calloc((str1Length + str2Length + 1), sizeof(char));

	strcpy(result, str1);
	strcpy(result + str1Length, str2);

	return result;
}

/**
 * Get the filename out of the request line.  The caller is responsible for
 * freeing the memory of the returned string.
 *
 * @param requestLine The request line, eg, GET /index.html HTTP/1.1
 * @return The file name, eg, index.html
 */
const char* getFileName(const char* requestLine) {

	int lastSpot = 4; // "GET " skip over this much
	int done = 0;
	// Find out how long the file name is
	while (done != 1) {
		if (requestLine[lastSpot] == ' ') {
			done = 1;
		} else {
			lastSpot = lastSpot + 1;
		}
	}

	char* fileName = calloc(strlen(requestLine), sizeof(char));

	strcpy(fileName, requestLine + 5); // Skip the / character
	fileName[lastSpot - 5] = '\0';

	return fileName;
}

