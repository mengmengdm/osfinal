//
// Created by fhr on 12/22/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "datamgr.h"
#include <stdarg.h>
static int log_pipe[2];
static pid_t logger_pid = -1;
static int sequence_number = 0;
FILE *log_file = NULL;

int create_log_process();
int write_to_log_process(const char *format, ...);
int end_log();
char *get_timestamp();
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Please provide the right arguments: first the port, then the max nb of clients");
		return -1;
	}
	sbuffer_t **shared_buffer = malloc(sizeof(sbuffer_t *));
	sbuffer_init(shared_buffer);

	int MAX_CONN = atoi(argv[2]);
	int PORT = atoi(argv[1]);
    printf("calling to start server on port: %d\n",PORT);

	serverArg_t *serverArg = (serverArg_t *)malloc(sizeof(serverArg_t));
	if (serverArg == NULL) {
		printf("Failed to allocate memory for serverArg\n");
		return -1;
	}
	serverArg->PORT = PORT;
	serverArg->MAX_CONN = MAX_CONN;
	serverArg->buffer = *shared_buffer;

	storeArg_t *storeArg = (storeArg_t *)malloc(sizeof(storeArg_t));
	if (storeArg == NULL) {
		printf("Failed to allocate memory for storeArg\n");
		return -1;
	}
	storeArg->buffer = *shared_buffer;

	dataArg_t *dataArg = (dataArg_t *)malloc(sizeof(dataArg_t));
	if (storeArg == NULL) {
		printf("Failed to allocate memory for dataArg\n");
		return -1;
	}
	dataArg->buffer = *shared_buffer;

	if (create_log_process() != 0) {
		printf("Failed to create log process\n");
		return -1;
	}
	//write_to_log_process("test");
	// Create the manager threads
	pthread_t connmgr_thread, datamgr_thread, storagemgr_thread;
	pthread_create(&connmgr_thread, NULL, (void*)start_server, serverArg);
	pthread_create(&datamgr_thread, NULL, (void*)datamgr, dataArg);
	pthread_create(&storagemgr_thread, NULL, (void*)storemgr, storeArg);


	pthread_join(connmgr_thread, NULL);
	pthread_join(storagemgr_thread, NULL);
	pthread_join(datamgr_thread, NULL);


	free(serverArg);
	free(dataArg);
	free(storeArg);
	sbuffer_free(shared_buffer);
	end_log();
	return 0;

}
int create_log_process() {
	sequence_number = 0;
	if (pipe(log_pipe) == -1) {
		perror("Failed to create pipe");
		return -1;
	}

	logger_pid = fork();

	if (logger_pid == -1) {
		perror("Failed to fork");
		return -1;
	}

	if (logger_pid > 0) {
		// Parent process: Close the read end
		close(log_pipe[0]);
		return 0;
	}

	// Child process: Logger
	close(log_pipe[1]);

	log_file = fopen("gateway.log", "w"); // Overwrite file each time server starts
	if (!log_file) {
		perror("Failed to open log file");
		exit(EXIT_FAILURE);
	}

	char buffer[256];
	while (read(log_pipe[0], buffer, sizeof(buffer)) > 0) {
		fprintf(log_file, "%s\n", buffer);
		fflush(log_file);
	}

	fclose(log_file);
	printf("log file created\n");
	exit(EXIT_SUCCESS);
}

int write_to_log_process(const char *format, ...) {
	if (logger_pid <= 0) {
		fprintf(stderr, "Logger process not initialized\n");
		return -1;
	}

	// lock for log (mutiple thread entering log)
	pthread_mutex_lock(&log_mutex);

	char log_entry[100];
	char *timestamp = get_timestamp();

	va_list args;
	va_start(args, format);
	char message[90];
	vsnprintf(message, sizeof(message), format, args);
	va_end(args);

	snprintf(log_entry, sizeof(log_entry), "%d %s %s", sequence_number++, timestamp, message);
	free(timestamp);

	if (write(log_pipe[1], log_entry, strlen(log_entry) + 1) == -1) {
		perror("Failed to write to log pipe");
		pthread_mutex_unlock(&log_mutex);  // unlock when failed
		return -1;
	}

	// unlock
	pthread_mutex_unlock(&log_mutex);

	return 0;
}

int end_log() {
	sleep(1);
	if (logger_pid > 0) {
		close(log_pipe[1]);

		wait(&logger_pid);
		logger_pid = -1;
	}
	return 0;
}

char *get_timestamp() {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	char *timestamp = malloc(20); // Format: YYYY-MM-DD HH:MM:SS
	if (timestamp) {
		strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", t);
	}
	return timestamp;
}
