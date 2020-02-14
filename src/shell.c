/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#include "Process.h"





int groupe = 0;
CmdProcess cp;
int stop;

void handler1(int sig){
	killAll(groupe,&cp,SIGKILL);
	return;
}

void handler2(int sig){
	killAll(groupe,&cp,SIGSTOP);
	ajouterJob(groupe,&cp);
	stop = groupe;
	printf("\nprocess groupe : %d stoppé\n",stop);
	return;
}




int main()
{
	int fils_vivant = 0;
	int groupefini;

	cp.pids = NULL;
	cp.groupes = NULL;

	Signal(SIGINT, handler1);
	Signal(SIGTSTP, handler2);
	while (1) {
		struct cmdline *l;
		groupe++;

		int i;
		int pid;
		int groupeTmp;
		int job;


		// descripteurs des redirections
		int desc_in = -1;
		int desc_out = -1;

		// tableau de descripteur des pipes
		int fd_out[2] = {-1,-1};
		int fd_in[2];

		printf("shell>(%d) ",fils_vivant);
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) {
			desc_in = Open(l->in,O_RDONLY,(S_IROTH | S_IRGRP | S_IRUSR) | (S_IWOTH | S_IWGRP | S_IWUSR));
		}
		if (l->out){
			desc_out = Open(l->out,O_WRONLY | O_CREAT | O_TRUNC, (S_IROTH | S_IRGRP | S_IRUSR) | (S_IWOTH | S_IWGRP | S_IWUSR));
		}

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {

			// initialisation et configuration des pipes
			fd_in[0] = fd_out[0];
			fd_in[1] = fd_out[1];
			pipe(fd_out);

			char **cmd = l->seq[i];

			// detecteur de fin de shell
			if(!strcmp(cmd[0],"quit"))
				return 0;

			if(!strcmp(cmd[0],"fg")){
				if(stop == (groupeTmp = reductionJob(maxJob(&cp),&cp))){
					printf("stop = groupeTmp");
					killAll(stop,&cp,SIGCONT);
					recupererAll(stop,&cp,&fils_vivant);
				}
				else {
					printf("stop != groupeTmp");
					recupererAll(groupeTmp,&cp,&fils_vivant);
				}
				break;
			}
			if(!strcmp(cmd[0],"bg")){
				killAll(stop,&cp,SIGCONT);
				break;
			}


			// incrémentation du nombre de fils courant
			fils_vivant++;
			

			// Lancement du fils
			if((pid = Fork()) == 0){
				// processus de commande
				if(i == 0 && l->seq[i+1] == 0){ // une seule commande
					if (desc_in != -1){
						dup2(desc_in,0);
					}
					if (desc_out != -1){
						dup2(desc_out,1);
					}
				}
				else { // plusieures commandes
					if(i == 0){ // première commande
						if (desc_in != -1){
							dup2(desc_in,0);
						}
						close(fd_out[0]);
						dup2(fd_out[1],1);
					}
					else { // pas première commande
						if(l->seq[i+1] == 0){ // dernière commande
							if (desc_out != -1){
								dup2(desc_out,1);
							}

							close(fd_in[1]);
							dup2(fd_in[0],0);
						}
						else{   //ni première ni dernière
							close(fd_out[0]);
							dup2(fd_out[1],1);

							close(fd_in[1]);
							dup2(fd_in[0],0);
						}
					}
				}
				execvp(cmd[0],cmd);
				exit(0);
			}
			ajouterPid(pid,groupe,&cp);

			// fermeture du tube pour le père
			if(fd_in[0] != -1){
				close(fd_in[0]);
				close(fd_in[1]);
			}
		}

		// Si il n'y a pas de &, alors on attends la fin des fils
		if(!l->background){
			//printf("premier plan, j'attends\n");
			recupererAll(groupe,&cp,&fils_vivant);
			/*if(l->seq[0]!=0){
				printf("\nfini\n");
			}*/
		}
		else{
			job = ajouterJob(groupe,&cp);
			printf("[%d] groupe %d\n",job,groupe);
		}
		if(fils_vivant > 0){
			while((pid = waitpid(-1, NULL, WNOHANG))>0){
				if((groupefini = retirerPid(pid,&cp)) > 0){
					printf("\ngroupe %d a fini de s'éxécuter\n",groupefini);
				}
				fils_vivant--;
			}
		}
		close(desc_in);
		close(desc_out);
		close(fd_out[0]);
		close(fd_out[1]);
	}
}