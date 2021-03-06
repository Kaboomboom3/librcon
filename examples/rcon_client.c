#include <rcon/rcon.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void _print_error(RCONerror error) {
	switch (error) {
	case RCON_ERROR_NONE:
		fprintf(stderr, "[RCON]: No error occured.\n");
		break;
	case RCON_ERROR_AUTHENTICATION_FAILURE:
		fprintf(stderr, "[RCON]: Failed to authenticate with the server.\n");
		break;
	case RCON_ERROR_CONNECTION_FAILURE:
		fprintf(stderr, "[RCON]: Failed to connect to server.\n");
		break;
	case RCON_ERROR_OUT_OF_MEMORY:
		fprintf(stderr, "[RCON]: Ran out of memory.\n");
		break;
	case RCON_ERROR_PLATFORM:
		fprintf(stderr, "[RCON]: A platform-specific error occured.\n");
		break;
	case RCON_ERROR_UNRESOLVED_HOSTNAME:
		fprintf(stderr, "[RCON]: Failed to resove hostname.\n");
		break;
	default:
		fprintf(stderr, "[RCON]: An unknown error occured.\n");
		break;
	}
	getc(stdin);
}

/**
 * \brief Reads a line from stdin.
 * 
 * \note Returned value must be free'd.
 * 
 * \returns The read line
 */
static const char *_read_line(void) {
	char *buffer = malloc(sizeof(char)), *tempBuffer;
	size_t bufferLength = sizeof(char);
	size_t filledBuffer = 0;
	
	if (buffer == NULL) {
		return NULL;
	}
	
	for (;;) {
		int c = fgetc(stdin);
		if (c == EOF || c == '\n') {
			break;
		}
		buffer[filledBuffer++] = (char)c;
		
		if (filledBuffer + 1 > bufferLength) {
			tempBuffer = realloc(buffer, bufferLength *= 2);
			if (tempBuffer == NULL) {
				free(buffer);
				return NULL;
			}
			
			buffer = tempBuffer;
		}
	}
	buffer[filledBuffer] = '\0';
	
	return buffer;
}

/**
 * \brief Prints out the usage of the command.
 */
static void _usage(void) {
	printf("Usage: RCON_example_client <HOSTNAME> <PORT> <PASSWORD>\n");
	exit(0);
}

int main(int argc, const char **argv) {
	int i;
	RCONclient *client;
	const char *input;
	char *response;
	
	if (argc != 4) {
		_usage();
	}

	printf("[RCON]: Initializing RCON.\n");
	if (!rconInitialize()) {
		_print_error(rconGetLastError());
		return -1;
	}

	printf("[RCON]: Connecting to server.\n");
	client = rconClientConnect(argv[1], argv[2]);
	if (!client) {
		_print_error(rconGetLastError());
		rconTerminate();
		return -1;
	}
	printf("[RCON]: Connected to '%s:%s'.\n", argv[1], argv[2]);

	printf("[RCON]: Authenticating with server.\n");
	if (!rconClientAuthenticate(client, argv[3])) {
		_print_error(rconGetLastError());
		rconClientDisconnect(client);
		rconTerminate();
		return -1;
	}
	printf("[RCON]: Authenticated with '%s'.\n", argv[1]);

	printf("[RCON]: Type a command or '!quit' to exit.\n");
	do {
		input = _read_line();
		if (input == NULL) {
			printf("[RCON/Error]: Failed to read input.\n");
			rconClientDisconnect(client);
			rconTerminate();
			return -1;
		}
		
		if (strcmp("!quit", input) == 0) {
			break;
		}

		if (!rconClientCommand(client, input, &response)) {
			_print_error(rconGetLastError());
			rconTerminate();
			return -1;
		}
		if (strlen(response) > 0) {
			printf("[Server]: %s\n", response);
		}
		free(response);

	} while (1);

	rconClientDisconnect(client);

	rconTerminate();
}
