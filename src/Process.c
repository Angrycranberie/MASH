#include <stdlib.h>
#include "csapp.h"

#include "Process.h"

int reductionJob(int job,CmdProcess *cp){
	if(job < 1){
		return -1;
	}
	int tmpG;
	jobslist *tmpCJ,*tmpCJ2;
    jobslist *curseurJ = cp->jobs;
    if(cp->jobs != NULL){
		if(cp->jobs->job == job){
			tmpG = cp->jobs->groupe;
			tmpCJ = cp->jobs;
		}
		if(curseurJ->suivant != NULL){
			if(curseurJ->suivant->job == job){
				tmpG = curseurJ->suivant->groupe;
				tmpCJ = curseurJ;
			}
			if(curseurJ->suivant->job > job){
				curseurJ->suivant->job -= 1;
			}
			while(curseurJ->suivant != NULL){
				if(curseurJ->suivant->job == job){
					tmpG = curseurJ->suivant->groupe;
					tmpCJ = curseurJ->suivant;
				}
				if(curseurJ->suivant->job > job){
					curseurJ->suivant->job -= 1;
				}
				curseurJ = curseurJ->suivant;
			}
		}
    }
	else{
		return -1;
	}
	if(tmpCJ == cp->jobs){
		cp->jobs = cp->jobs->suivant;
		free(tmpCJ);
	}
	else{
		tmpCJ2 = tmpCJ->suivant;
		tmpCJ->suivant = tmpCJ->suivant->suivant;
		free(tmpCJ2);
	}
	return tmpG;
}

int maxJob(CmdProcess *cp){
	jobslist *curseurJ = cp->jobs;
	if(cp->jobs != NULL){
		while(curseurJ->suivant != NULL){
			curseurJ = curseurJ->suivant;
		}
		return curseurJ->job;
	}
	else{
		return -1;
	}
}


int numJob(int groupe,CmdProcess *cp){
        jobslist *curseurJ = cp->jobs;
        if(cp->jobs != NULL){
                while(curseurJ->suivant != NULL && curseurJ->groupe != groupe){
                        curseurJ = curseurJ->suivant;
                }
                if(curseurJ->groupe == groupe){
                        return curseurJ->job;
                }
        }
        return -1;
}

int ajouterJob(int groupe,CmdProcess *cp){
    int job = 1;
    jobslist *curseurJ = cp->jobs;
    if(cp->jobs != NULL){
        if(curseurJ->groupe != groupe){
            job++;
        }
        else{
            return curseurJ->job;
        }
        while(curseurJ->suivant != NULL){
                curseurJ = curseurJ->suivant;
            if(curseurJ->groupe != groupe){
	            job++;
            }
            else{
                return curseurJ->job;
            }
        }
		if(curseurJ->groupe != groupe){
	            job++;
        }
        else{
            return curseurJ->job;
        }
        jobslist *j = (jobslist *)malloc(sizeof(jobslist));
        j->job = job;
        j->groupe = groupe;
        j->suivant = NULL;
        curseurJ->suivant = j;
        return job;
    }
    else{
        jobslist *j = (jobslist *)malloc(sizeof(jobslist));
        j->job = job;
        j->groupe = groupe;
        j->suivant = NULL;
        cp->jobs = j;
		return job;
    }
}

void recupererAll(int groupe,CmdProcess *cp,int *fils_vivant){
    pidlist *curseurP = cp->pids;
	int status;
    int pid;
    if(cp->pids == NULL){
        return;
    }
    else{
        if(curseurP->groupe == groupe){
            pid = curseurP->pid;
            while(waitpid(pid, &status, WNOHANG|WUNTRACED) <= 0){
				if(WIFSTOPPED(status)){
					break;
				}
			}
			if(!WIFSTOPPED(status)){
                retirerPid(curseurP->pid,cp);
                *fils_vivant -= 1;
			}
        }
        while(curseurP->suivant != NULL){
            curseurP = curseurP->suivant;
            if(curseurP->groupe == groupe){
                pid = curseurP->pid;
                while(waitpid(pid, &status, WNOHANG|WUNTRACED) <= 0){
					if(WIFSTOPPED(status)){
						break;
					}
				}
				if(!WIFSTOPPED(status)){
                	retirerPid(curseurP->pid,cp);
                	*fils_vivant -= 1;
				}
            }
        }
    }
}

void killAll(int groupe,CmdProcess *cp,int sig){
        pidlist *curseurP = cp->pids;
        int pid;
        if(groupe == -1){
                if(curseurP->groupe == groupe){
                        pid = curseurP->pid;
                        kill(pid,sig);
                        retirerPid(pid,cp);
                }
                while(curseurP->suivant != NULL){
                        curseurP = curseurP->suivant;
                        pid = curseurP->pid;
                        if(curseurP->groupe == groupe){
                                kill(pid,sig);
                                retirerPid(pid,cp);
                        }
                }
        }
        else{
                if(cp->pids == NULL){
                        return;
                }
                else{
                        if(curseurP->groupe == groupe){
                                pid = curseurP->pid;
                                kill(pid,sig);
                                retirerPid(pid,cp);
                        }
                        while(curseurP->suivant != NULL){
                                curseurP = curseurP->suivant;
                                if(curseurP->groupe == groupe){
                                        pid = curseurP->pid;
                                        kill(pid,sig);
                                        retirerPid(pid,cp);
                                }
                        }
                }
        }
}

// ajoute le pid donné avec son groupe
void ajouterPid(int pid,int groupe,CmdProcess *cp){
        pidlist *p = (pidlist *)malloc(sizeof(pidlist));
        p->pid = pid;
        p->groupe = groupe;
        p->suivant = NULL;
        pidlist *curseurP = cp->pids;
        groupCmd *curseurG = cp->groupes;
        if(curseurP == NULL){
                cp->pids = p;
        }
        else{
                while(curseurP->suivant != NULL){
                        curseurP = curseurP->suivant;
                }
                curseurP->suivant = p;
        }
        if(curseurG == NULL){
                groupCmd *g = (groupCmd *)malloc(sizeof(groupCmd));
                g->groupe = groupe;
                g->nbProcess = 1;
                g->suivant = NULL;
                cp->groupes = g;
        }
        else{
                while(curseurG->suivant != NULL && curseurG->groupe != groupe){
                        curseurG = curseurG->suivant;
                }
                if(curseurG->groupe == groupe){
                        curseurG->nbProcess += 1;
                }
                else{
                        groupCmd *g = (groupCmd *)malloc(sizeof(groupCmd));
                        g->groupe = groupe;
                        g->nbProcess = 1;
                        g->suivant = NULL;
                        curseurG->suivant = g;

                }
        }
}


// retirerPid(pid,cp) enlève pid de la liste des processus de cp
// retourne 0 si le groupe de processus n'est pas fini, un nombre positif si oui
// et négatif si pid pas trouvé (-1 pas d'élément,-2 un seul élément différent de pid, -3 pid pas trouvé dans l'ensemble, -4 si le groupe n'a pas été trouvé)
int retirerPid(int pid,CmdProcess *cp){
        int g;
        pidlist *tmp;
        pidlist *curseurP = cp->pids;

        if(curseurP == NULL){// il n'y a pas d'élément dans la liste
                return -1;
        }

        //recherche du processus
        if(curseurP->pid == pid){// on trouve le pid au premiers élément
                g = curseurP->groupe;
                tmp = cp->pids;
                cp->pids = cp->pids->suivant;
                free(tmp);
        }
        else{
                if(curseurP->suivant == NULL){// la liste ne contient qu'un élément qui n'est pas le pid cherché
                        return -2;
                }
                while(curseurP->suivant->suivant != NULL && curseurP->suivant->pid != pid){ // on cherche dans les autres éléments
                        curseurP = curseurP->suivant;
                }
                if(curseurP->suivant->pid == pid){
                        g = curseurP->suivant->groupe;
                        tmp = curseurP->suivant;
                        curseurP->suivant = curseurP->suivant->suivant;
                        free(tmp);
                }
                else{
                        return -3;
                }
        }

        //recherche du groupe du processus trouvé
        groupCmd *tmpG;
        groupCmd *curseurG = cp->groupes;
        if(curseurG == NULL){
                return -5;
        }

        // qu'un élément dans la liste
        if(curseurG->suivant == NULL){
                if(curseurG->groupe == g){
                        curseurG->nbProcess -= 1;
                        if(curseurG->nbProcess == 0){
                                tmpG = cp->groupes;
                                cp->groupes = curseurG->suivant;
                                free(tmpG);
                                reductionJob(g,cp);
                                return g;
                        }
                }
                else{
                        return -4;
                }
        }
        else{ // plus de 1 groupe
                // vérification du premier groupe
                if(curseurG->groupe == g){
                        curseurG->nbProcess -= 1;
                        if(curseurG->nbProcess == 0){
                                tmpG = cp->groupes;
                                cp->groupes = curseurG->suivant;
                                free(tmpG);
                                reductionJob(g,cp);
                                return g;
                        }
                }
                else{// le premier groupe n'est pas le bon
                        while(curseurG->suivant->suivant != NULL && curseurG->suivant->groupe != g){
                                curseurG = curseurG->suivant;
                        }
                        if(curseurG->suivant->groupe == g){ // le groupe trouvé est le bon
                                curseurG->suivant->nbProcess -= 1;
                                if(curseurG->suivant->nbProcess == 0){ // suppression du groupe car plus aucun process en cours
                                        tmpG = curseurG->suivant;
                                        curseurG->suivant = curseurG->suivant->suivant;
                                        free(tmpG);
                                        reductionJob(g,cp);
                                        return g;
                                }
                        }
                        else{ // aucun groupe pour pid (impossible)
                                return -4;
                        }
                }
        }
        return 0;
}