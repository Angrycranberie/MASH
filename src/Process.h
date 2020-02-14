#ifndef __PROCESS_H__
#define __PROCESS_H__


typedef struct pidlist{
	int pid;
	int groupe;
	struct pidlist *suivant;
}pidlist;

typedef struct groupCmd{
	int groupe;
	int nbProcess;
	struct groupCmd *suivant;
}groupCmd;

typedef struct jobslist{
	int job;
	int groupe;
	struct jobslist *suivant;
}jobslist;

typedef struct CmdProcess{
	groupCmd *groupes;
	pidlist *pids;
	jobslist *jobs;
}CmdProcess;

void killAll(int groupe,CmdProcess *cp,int sig);

void ajouterPid(int pid,int groupe,CmdProcess *cp);

int retirerPid(int pid,CmdProcess *cp);

void recupererAll(int groupe,CmdProcess *cp,int *fils_vivant);

int reductionJob(int groupe,CmdProcess *cp);

int maxJob(CmdProcess *cp);

int numJob(int groupe,CmdProcess *cp);

int ajouterJob(int groupe,CmdProcess *cp);




#endif

