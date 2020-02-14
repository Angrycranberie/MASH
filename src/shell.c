/*
 * Copyright (C) 2002, Simon Nieuviarts
 * Copyright (C) 2020, Mathias Deplanque
 * Copyright (C) 2020, Alexis Yvon
 */

// MASH : Mathias-Alexis Shell
// If you put a linux shell to your ear, can you ear the C ?

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

/* Debug Mode - 0 OFF, 1 ON */
#define DEBUG 	0

/* Colors constants */
#define C_RST	"\033[0m"
#define C_MSH	"\033[1;32m"
#define C_ERR	"\033[0;31m"

int main()
{

	int nbalive = 0;

	while (1) {
		struct cmdline *l;
		int i, j;
		
		int fd_in[2];
		int fd_out[2] = {-1,-1};
		
		printf("%smash>%s ", C_MSH, C_RST);
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			if(DEBUG) printf("exit\n");
			exit(EXIT_SUCCESS);
		}

		/* Syntax error, read another command */
		if (l->err) {
			printf("%serror:%s %s\n", C_ERR, C_RST, l->err);
			continue;
		}

		if (DEBUG) {
			if (l->in) printf("in: %s\n", l->in);
			if (l->out) printf("out: %s\n", l->out);
		}

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			pid_t pid = getpid();

			fd_in[0] = fd_out[0];
			fd_in[1] = fd_out[1];
			pipe(fd_out);

			/* Display command infos in debug mode */
			if (DEBUG) {
				printf("seq[%d]: ", i);
				for (j=0; cmd[j]!=0; j++) {
					printf("%s ", cmd[j]);
				}
			}

			/* One processus is created per command ; checks if the shell must be quitted before */
			
			/* Quits the shell */
			if (!strcmp(cmd[0],"quit")) exit(EXIT_SUCCESS);		

			nbalive++;

			/* Execute the command if current processus is a child */
			if(pid = Fork() == 0) {

				//pipe and file desc management
				if(i==0 && l->seq[i+1] == 0){
					if (l->in) {
		 				int f_in = Open(l->in, O_RDONLY, 0);
		 				Dup2(f_in, 0);
		 			}

		 			if (l->out) {
		 				int f_out = Open(l->out, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IXUSR);
		 				Dup2(f_out, 1);
		 			}

				}else{
					
					if (i==0){
						if (l->in) {
		 					int f_in = Open(l->in, O_RDONLY, 0);
		 					Dup2(f_in, 0);
		 				}

		 				Close(fd_out[0]);
		 				Dup2(fd_out[1],1);

					
					} else if (l->seq[i+1]==0){
						if (l->out) {
		 					int f_out = Open(l->out, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IXUSR);
		 					Dup2(f_out, 1);
		 				}

		 				Close(fd_in[1]);
		 				Dup2(fd_in[0],0);


					} else {

						Close(fd_out[0]);
		 				Dup2(fd_out[1],1);

						Close(fd_in[1]);
		 				Dup2(fd_in[0],0);
						
					}

				}

				execvp(cmd[0], cmd);
				//Termine le processus si aucune commande n'a pu être exécutée.
				exit(0);
			}
			if(fd_in[0] != -1){
				Close(fd_in[0]);
				Close(fd_in[1]);
			}
		}

		//Check if background or not
		if(l->bg != 1){
			while(waitpid(-1, NULL, WNOHANG) <= 0){
				nbalive--;
			}
		}
		//wait last process
		if(nbalive > 0 ){
			while((waitpid(-1, NULL, WNOHANG|WUNTRACED)){
				nbalive--;
			}

		}
		//closing pipe
		Close(fd_out[0]);
		Close(fd_out[1]);

	}
}

