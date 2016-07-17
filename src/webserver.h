/**
 * webserver.h
 *
 * Author: Mark Royer
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

/**
 * Max number of threads for the program.
 */
static const int NUM_THREADS = 20;

/**
 * The current number of active threads in the program.
 */
static int threadCount;

/**
 * Lock for accessing the thread count.
 */
static pthread_mutex_t countLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * The directory storing the web site files.
 */
const char* WEBDIRECTORY = "WebContent/";

// Type Declarations And Definitions

/**
 * Holds parameters for the function processRequest.
 */
typedef struct {

	/**
	 * The active socket
	 */
	int socket;

	/**
	 * The current thread number.
	 */
	int threadNumber;

} ThreadArgs;


// Function Declarations

const char* readLine(int socket);
const char* contentType(const char* fileName);
bool endsWith(const char* string, const char* ending);
void writeBytes(int socket, const char* string);
void sendBytes(FILE* fis, int os);
const char* append(const char* str1, const char* str2);
const char* getFileName(const char* requestLine);
void *processRequest(ThreadArgs* socket);


#endif /* WEBSERVER_H_ */
