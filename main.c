#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LIMIT 10
#define MAX_INPUT 260

bool in_background = false;

// split the given string into words.
char** split(char* c)
{
	
	int WORDS_NUM = MAX_LIMIT, WORD_LENGTH = MAX_LIMIT;
	char** words = malloc(WORDS_NUM * sizeof(char*));
	for(int i = 0; i < WORDS_NUM; i++){
		words[i] = malloc(WORD_LENGTH*sizeof(char));
	}
	int current_char_index = 0, current_word_index = 0;
	while(*c){ 
		if(*c == '&')  in_background = true;
		// fgets take the new line as a char.
		if((*c == ' ' && *(c+1) != '&') || *c == '\n'){
			words[current_word_index][current_char_index] = '\0';
			current_word_index++;
			current_char_index = 0;
			WORD_LENGTH = MAX_LIMIT;
			if(current_word_index == WORDS_NUM){ // change the words size to double.
				WORDS_NUM *= 2;
				words = realloc(words,WORDS_NUM*sizeof(char*));
				for(int i = current_word_index; i < WORD_LENGTH; i++){
					words[i] = malloc(WORD_LENGTH*sizeof(char));
				}
			}
		}else if(*c != ' ' && *c != '&'){
			words[current_word_index][current_char_index] = *c;
			current_char_index++; 
			if(current_char_index == WORD_LENGTH){ // change the word length to double.
				WORD_LENGTH *= 2;
				words[current_word_index] = realloc(words[current_word_index],WORD_LENGTH*sizeof(char));
			}
		}
		c++;
	}
	
	// execvp needs the argumens vector to be end with null value.
	words[current_word_index] = NULL;
	
	return words;
}



FILE *f;
	
	
// handle the signal SIGCHILD.
void handler()
{	
	// clean the zombie processes.
	waitpid(-1,NULL,WNOHANG);
	if (f != NULL) {
		fprintf(f,"Child process was terminated\n");
		fflush(f);
	}
	
}




void execute(char** words){
	/**
	* status is a variable where unix stores the return value
	* of the child process.
	*/
	int status;
	
	pid_t pid;
	// fork a child process
	pid = fork();
	
	// check if the input is exit.
	if(strcmp(words[0],"exit") == 0) exit(0);
	
	if(pid >= 0){// fork was successful	
		if(pid == 0){// child process
			if(execvp(words[0],words) < 0){
				if(strcmp(words[0],"\0")) fprintf(stderr,"syntax error!\n");
				exit(1);
			}
		}
		else { // parent process
			if(!in_background)
				while (wait(&status) != pid);
			in_background = false;// reset the value to false.
			/*
			 * for debugging.
			 * printf("Child Complete\n");
			*/
				
		}
	}
	else{ // fork failed
		
		/*
		 * for debugging.
		 * fprintf(stderr,"Fork Failed\n");
		*/
		exit(1);
	}
}


int main()
{
	f = fopen("logger.log", "w");

	while(true){		
		
		// is called when the child process terminates.
		signal(SIGCHLD, handler);
	
		printf("> ");
		char string [MAX_INPUT];
		fgets(string,MAX_INPUT,stdin);
		char** words = split(string);
		
		execute(words);
		
	}
	
	fclose(f);

    return 0;
}
