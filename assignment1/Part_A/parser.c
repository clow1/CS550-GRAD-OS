#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// Defined limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100
#define MAX_PROCESSES 100

size_t MAX_LINE_LEN = 10000;


// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99

// Type definition of signal function with one parameter of type integer and returns void
typedef void Sigfunc(int);


FILE *fp; // File struct for stdin
char **tokens;
char ***pipeTokens;
char *line;

int is_bg; // Background flag
int *processes; // Array to store PIDs
char **commands; // Array to store background process commands
int processes_count = 0; // Int to store the number of background processes

int redirect_in; // Input redirection falg
int redirect_out; // Output redirection flag
int is_piped; // Pipe flag
char *inputFile;
char *outputFile;

void initialize()
{
	// Allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// Allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);
	assert( (pipeTokens = malloc(sizeof(char**)*MAX_TOKENS)) != NULL);

	// Allocate space for process PIDs
	assert( (processes = malloc(sizeof(int)*MAX_PROCESSES)) != NULL);
	assert( (commands = malloc(sizeof(char*)*MAX_PROCESSES)) != NULL);

	// Open stdin as a file pointer
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);
}

void tokenize (char * string)
{
	// Initialize background process to false
	is_bg = 0;
	// Initalize read from input file to false
	redirect_in = 0;
	inputFile = NULL;
	// Initalize write to output file to false
	redirect_out = 0;
	outputFile = NULL;
	// Initalize pipe process to false
	is_piped = 0;
	int token_count = 0;
	int size = MAX_TOKENS;
	char *this_token;

	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

		if (*this_token == '\0'){
			continue;
		}

		/////// Account for & meta shell character ///////
		char *background_Token = "&";
		if (strcmp(this_token, background_Token) == 0) {
			is_bg = 1;
			continue;
		}

		// Check if input recieved contains arguments after &
		if (is_bg == 1) {
			printf("Arugnment %s is Ignored Following &\n", this_token);
			continue;
		}
		/////////////////////////////////////////////////

		// Check if process is reading from input file
		if ((redirect_in == 1) && (inputFile == NULL)) {
			inputFile = this_token;
		}

		// Check if process is writing to output file
		else if ((redirect_out == 1) && (outputFile == NULL)) {
			outputFile = this_token;
		}

		else {
			//////// Account for Input from File ////////
			char *filein = "<";
			if (strcmp(this_token, filein) == 0) {
				redirect_in = 1;
				continue;
			}
			/////////////////////////////////////////////

			//////// Account for Input from File ////////
			char *fileout = ">";
			if (strcmp(this_token, fileout) == 0) {
				redirect_out = 1;
				continue;
			}
			/////////////////////////////////////////////

			///////// Account for Piped Process /////////
			char *pipeToken = "|";
			if (strcmp(this_token, pipeToken) == 0) {

				// Copy contents of tokens into pipeToken array
				char **p = malloc(sizeof(tokens));
				for(int i=0; i < token_count; i++) {
					char *temp = malloc(sizeof(tokens[i]));
					strcpy(temp, tokens[i]);
					p[i] = temp;
				}

				pipeTokens[is_piped] = p;

				is_piped++;
				// If there are more tokens than space, reallocate more space
				if(is_piped >= size){
					size*=2;

					assert ( (pipeTokens = realloc(pipeTokens, sizeof(char**) * size)) != NULL);
				}

				token_count = 0;
				continue;
			}
			////////////////////////////////////////////

			tokens[token_count] = this_token;

			//printf("Token %d: %s\n", token_count, tokens[token_count]);

			token_count++;
			// If there are more tokens than space, reallocate more space
			if(token_count >= size){
				size*=2;

				assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
			}
		}
	}

	// If pipes exist, move everything in tokens to pipeTokens array
	if (is_piped > 0) {

		// Copy contents of tokens into pipeToken array
		char **p = malloc(sizeof(tokens));
		for(int i=0; i < token_count; i++) {
			char *temp = malloc(sizeof(tokens[i]));
			strcpy(temp, tokens[i]);
			p[i] = temp;
		}

		pipeTokens[is_piped] = p;

		// If there are more tokens than space, reallocate more space
		if(is_piped >= size){
			size*=2;

			assert ( (pipeTokens = realloc(pipeTokens, sizeof(char**) * size)) != NULL);
		}

		token_count = 0;
	}

	tokens[token_count] = NULL;
	if(is_piped == 0){
		pipeTokens[0] = NULL;
	} else {
		pipeTokens[is_piped+1] = NULL;
	}

}

// Store PID for all running background processes
void store_bg_pid(int pid, char * token)
{
	int size = MAX_PROCESSES;
	char *cmd = malloc(sizeof(token));
	strcpy(cmd, token);

	// Store current running bg process pid and command
	processes[processes_count] = pid;
	commands[processes_count] = cmd;

	// Incremenet number of background process executed
	processes_count++;

	// If there are more tokens than space, reallocate more space
	if(processes_count >= size){
		size*=2;

		assert ( (processes = realloc(processes, sizeof(int) * size)) != NULL);
		assert ( (commands = realloc(commands, sizeof(char*) * size)) != NULL);
	}
}

// Handle display of all background processes
void list_jobs()
{
	printf("List of background processes:\n");
	pid_t pid_i;
	char* cmd_i;

	for (int i = 0; i < processes_count; i++) {
		pid_i = processes[i];
		cmd_i = commands[i];

		// If process is still running than status will be 0
		if (kill(pid_i, 0) == 0) { 
			printf("Command %s with PID %d  Status: RUNNING \n", cmd_i, pid_i);
		}
		// Otherwise, status is -1 and process has terminated
		else if (kill(pid_i,0) == -1) {
			printf("Command %s with PID %d Status: FINISHED \n", cmd_i, pid_i);
		}
	}
}

// Bring background process to forground
void move_forground(){
	pid_t w;
	int status;

	int in_proccesses_array = 0;

	if (tokens[1] != NULL) {
		w = atoi(tokens[1]);

		// Loop through process PID array to find if background process exists
		for (int i = 0; i < processes_count; i++) {
			if (processes[i] == w) {
				in_proccesses_array = 1;
			}
		}

		// If the backgroun process exists move to forground
		if (in_proccesses_array == 1) {
			printf("PID %d moving to foreground.\n", w);
			waitpid(w, &status, 0);
		}
		else {
			printf("PID %d not a valid background process.\n", w);
		}
	}
}

// Handle I/O redirection for input files
void input_redirection() {
	int fd = open(inputFile, O_RDONLY);
	if(fd < 0) {
		perror("FAILED TO READ FILE");
	}

	dup2(fd, 0);
	close(fd);
}

// Hanlde I/O redirection for output files
void output_recirection() {
	int fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
	if(fd < 0) {
		perror("FAILED TO WRITE TO FILE");
	}

	// Redirect stdout to file
	dup2(fd, 1);
	close(fd);
}

Sigfunc *install_signal_handler(int signo, Sigfunc *handler) {

	struct sigaction act, oact;

	// Populate action structure
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if( signo == SIGALRM ) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;  /* SunOS 4.x */
#endif
	} else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;  /* SVR4, 4.4BSD */
#endif
	}

	if (sigaction(signo, &act, &oact) < 0 ) {
		return (SIG_ERR);
	}
	return(oact.sa_handler);
}

//  Signal handler for SIGINT: ctr-l
void int_handler(int sig) {
	for (int i=0; i < processes_count; i++) {
		kill(processes[i], SIGTERM);
	}
}

// Handle Kill command, cannot use signals since SIGKILL is one of the two commands which can't be caught or ignored
void kill_handler() {
	printf("Kill executed: %d\n", getpgid(atoi(tokens[1])));
	killpg(atoi(tokens[1]), SIGKILL);
	
	// Wait to avoid Zombie processes
	waitpid(-(processes[processes_count-1]), NULL, 0);
}

// Handle piped processes
void pipe_handle()
{
	int status;
	pid_t cpid;
	int fd_total = 2*is_piped;

	// Create file descriptor
	int fds[fd_total];

	for(int i=0; i < is_piped; i++) {
		// Create a pipe
		if(pipe(fds + i*2) < 0) {
			perror ("PIPE CREATION FAILED");
			exit(1);
		}
	}

	// Loop through each process, fork, generate pipe communication, and execute
	for (int j=0; j < (is_piped+1); j++) {
		cpid = fork();

		if(cpid < 0) {
			perror ("FORK FAILED");
			exit(1);
		}
		else if (cpid == 0) {
			if(is_bg == 1) {
				// Set first child process to it's own group
				if(j == 0) {
					setpgid(0,0);
				}
				//Set all other processes to first process id stored in processes array
				else{
					setpgid(0,processes[processes_count-1]);
				}
			}

			// Check if processing last fork process
			if(j != (is_piped)) {
				// Make output end of each pipe equal to stdout
				dup2(fds[(2*j)+1], 1);

			}
			// Check if processing first fork process
			if(j != 0) {
				// Make input end of pipe equal to stdin
				dup2(fds[(2*j)-2], 0);
			}

			for(int i=0; i<fd_total; i++){
				close(fds[i]);
			}

			// Check if command arguments are passed in through input file
			if(j == (is_piped) && redirect_in) {
				input_redirection();
			}

			// Check if command output is writen to file
			if(j == (is_piped) && redirect_out) {
				output_recirection();
			}

			if(execvp(pipeTokens[j][0],pipeTokens[j]) < 0) {
				perror ("INVALID CMD");
				exit(1);
			}
		}

		// Store background process if background flag is set and first command in filter is being processed
		if(is_bg == 1 && j==0){
			store_bg_pid(cpid, pipeTokens[j][0]);
		}
	}

	//// Parent Process ////
	// Close parent file descriptors
	for(int i=0; i < fd_total; i++) {
		close (fds[i]);
	}

	if(is_bg == 0){
		for(int i=0; i < is_piped; i++) {
			if(wait(&status) < 0 ){
				perror ("WAITPID FAILED");
				exit(2);
			}

		}
	}
}

void read_command()
{
	// Getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1);

	printf("Shell read this line: %s\n", line);

	tokenize(line);
}

int run_command() {

	// Check for any children of background proccess that have finished
	waitpid(-1, NULL, WNOHANG);

	// Check if no command was entered
	if (tokens[0] == NULL && pipeTokens[0] == NULL) {
		perror("Please Enter a Command");
	}

	// Check if pipe process
	else if (is_piped > 0) {
		pipe_handle();
	}

	// Check if exit command was entered
	else if (strcmp( tokens[0], EXIT_STR ) == 0) {
		return EXIT_CMD;
	}

	//Check if kill command was executed
	else if(strcmp( tokens[0], "kill") == 0) {
		kill_handler();
	}

	// Check if user requested a list of running jobs
	else if (strcmp( tokens[0], "listjobs") == 0 ) {
		list_jobs();
	}

	// Check if user requested to move a process to foreground
	else if (strcmp( tokens[0], "fg") == 0) {
		move_forground();
	}

	else {
		pid_t cpid; // Child pid
		int status; // Status of child pid

		cpid = fork();

		if(cpid < 0) {
			perror ("FORK FAILED");
			exit(1);
		}

		///// Child Process /////
		else if(cpid == 0) {

			// If background process assign pid group
			if(is_bg ==  1){
				setpgid(0,0);
			}

			// Check if command arguments are passed in through input file
			if(redirect_in) {
				input_redirection();
			}

			// Check if command output is writen to file
			if(redirect_out) {
				output_recirection();
			}

			if(execvp(tokens[0],tokens) < 0) {
				perror ("INVALID CMD");
				exit(1);
			}
			exit(0);
		}

		///// Parent Process /////
		else {
			if(is_bg == 0){
				if(waitpid(cpid, &status, 0) < 0 ){
					perror ("WAITPID FAILED");
					exit(2);
				}
			}
			else {
				store_bg_pid(cpid, tokens[0]);
			}
		}
	}
	return UNKNOWN_CMD;
}

int main()
{
	// Install signal handler for ctrl-c
	install_signal_handler(SIGINT, int_handler);

	initialize();

	do {
		printf("sh550> ");
		read_command();

	} while( run_command() != EXIT_CMD );

	return 0;
}