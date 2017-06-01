#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

int y=0;
int background=0;

//function used to change directories back to $HOME
int goHome(){
	char cwd[1024];
	chdir(getenv("HOME"));
	printf("you cd'd to home which is: %s\n",getcwd(cwd,1024));

	return 0;
}


//function used to redirect output into a file
int redirectRight(char * entry){

	int input=0;
	int output=0;
	int pipe=0;
	int count=0;

	while (entry[count]!='\0'){

		if (entry[count]=='>'){
			output++;
			if (pipe==0){
				return -1;
			}
		}
		else if (entry[count]=='<'){
			input++;
			if (pipe==1){
				return -1;
			}
		}
		else if (entry[count]=='|'){
			pipe=1;
		}
		count ++;
	}
		return 0;
}





//fucntion used to count the number of pipes in a given command
//seeing as this shell supports single pipe commands
int pipeCount(char * entry){

	int pipes=0;

	if (entry==NULL){
		return -1;
	}
	else{

		int length=strlen(entry);
		for (int i=0; i<length; i++){
			if (entry[i]=='|'){
				pipes++;
			}
		}
	}

	return pipes;
}

//this function takes in the input given by the user and parses
//the command into an array of arguments
char** parseCommand(char* inputLine){

	const int MAX_SIZE = 101;
	char **args;
	args = calloc(MAX_SIZE, sizeof(char*));

	char* temp = strtok(inputLine, "	 ");

	int i = 0;

	while (temp != NULL){

		if (strncmp(temp, "	", 1) && strncmp(temp, "\n", 1)){

			args[i] = temp;
		}

		temp = strtok(NULL, "	 ");

		i++;
	}

	args[i] = NULL;

	return args;
}

//this function takes in two args and pipes the output of argv1 into argv2 as its input
void pipes(char ** argv1, char ** argv2){

	int pid;
	int fd[2];

	if(pipe(fd)==-1){
		perror("something wrong with pipe input\n");
		return;
	}

	if ((pid = fork()) == -1){
		perror("something wrong with fork\n");
		return;
	}

	else if(pid ==0){
		close(fd[1]);
		int dup = dup2(fd[0],0);
		if (dup<0){
			perror("dup2 error\n");
			return;
		}
		execvp(argv2[0],argv2);
	}

	//second child for second half of pipe

	int pid2=fork();
	if (pid2==0){
		close(fd[0]);
		int dup=dup2(fd[1],1);
		if (dup<0){
			perror("dup2 error\n");
			return;
		}
		execvp(argv1[0],argv1);
	}

	int status;
	int status2;
	close(fd[0]);
	close(fd[1]);
	waitpid(pid,&status,0);
	waitpid(pid2,&status2,0);

}

//This function deals with input and output redirection and a pipe in one command
int pipeArrow(char ** argv){

	int oldIn=dup(0);
	int oldOut=dup(1);
	int fdin=0;
	int fdout=1;
	FILE *fpOut=NULL;
	FILE *fpIn=NULL;
	char * temp=calloc(1025,sizeof(char));
	char * temp2=calloc(1025,sizeof(char));
	int used=0;
	int inUsed=0;
	int outUsed=0;
	int count=0;

	while(argv[count]!=NULL){

		if ( (0!=strcmp(argv[count],"|")) && (0!=strcmp(argv[count],">")) && (0!=strcmp(argv[count],"<")) && (used!=1) && (inUsed==0) ){
			temp=strcat(temp,argv[count]);
			temp=strcat(temp," ");
		}
		else if ( (0!=strcmp(argv[count],"|")) && (0!=strcmp(argv[count],">")) && (0!=strcmp(argv[count],"<")) && (used==1) && (outUsed==0) ){
			temp2=strcat(temp2,argv[count]);
			temp2=strcat(temp2," ");
		}

		else if(0==strcmp(argv[count],">") ) {
			outUsed=1;
			if (argv[count+1]!='\0'){

				fpOut=fopen(argv[count+1],"w");
				if (fpOut==NULL){
					fprintf(stderr,"unable to open said file.\n");
					break;
				}
				fdout=fileno(fpOut);
				if (fdout==-1){
					fprintf(stderr,"unable to output to said file. Try again\n");
					break;
				}
				dup2(fdout,1);
			}
			else{
				 printf("error with redirection (to out)\n");
			}

		}


		else if(0==strcmp(argv[count],"<")) {
			inUsed=1;
			if (argv[count+1]!='\0'){
				fpIn=fopen(argv[count+1],"r");
				if (fpIn==NULL){
					fprintf(stderr,"unable to open said file.\n");
					break;
				}
				fdin=fileno(fpIn);
				if (fdin==-1){
					fprintf(stderr,"unable to input to said file. Try again\n");
					break;
				}
				dup2(fdin,0);
			}
			else{
				 printf("Error with redirction (to in)\n");
			}
		}

		else if (0==strcmp(argv[count],"|")){
			used=1;
		}

		count++;
	}




	char ** argv1=parseCommand(temp);
	char ** argv2=parseCommand(temp2);
	pipes(argv1,argv2);
	free(temp);
	free(temp2);
	free(argv1);
	free(argv2);
	if (fpOut!=NULL){
		fclose(fpOut);
	}
	if (fpIn!=NULL){
		fclose(fpIn);
	}
	dup2(oldIn,0);
	dup2(oldOut,1);

	return 0;

}

//this function takes in argv of type char** and uses its elements
//to call pipes(char ***argv1, char **argv2)
int pipeOne(char ** argv){

	char * temp=calloc(1025,sizeof(char));
	char * temp2=calloc(1025,sizeof(char));
	int used=0; //if your single pipe has been encoutered or not
	int count=0;

	while(argv[count]!='\0'){

		if ( (0!=strcmp(argv[count],"|")) && (used!=1) ){
			temp=strcat(temp,argv[count]);
			temp=strcat(temp," ");
		}
		else if ( (0!=strcmp(argv[count],"|")) && (used==1) ){
			temp2=strcat(temp2,argv[count]);
			temp2=strcat(temp2," ");
		}
		else if (0==strcmp(argv[count],"|")){
			used=1;
		}
		count++;
	}

	char ** argv1=parseCommand(temp);
	char ** argv2=parseCommand(temp2);
	pipes(argv1,argv2);
	free(argv1);
	free(argv2);
	free(temp);
	free(temp2);

return 0;

}

//takes in arguments array and utilizes execvp to run multi word commands
int multWords(char * argv[]){

	pid_t forker;
	int status;
	forker=fork();

	if (forker==-1){
		fprintf(stderr,"fork problem\n");
	}

	else if (forker==0){
		execvp(argv[0],argv);
		fprintf(stderr,"%s: invalid command\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	else{
		if (background==0){
			waitpid(forker,&status,0);
		}
	}

	return 0;
}


//redirect takes in a char ** of arguments and deals with redirection of input and output
void redirect(char ** argv){

	char *temp=calloc(1025,sizeof(char));
	int count=0;
	int oldIn=dup(0);
	int oldOut=dup(1);
	int fdin=0;
	int fdout=1;
	FILE *fpOut=NULL;
	FILE *fpIn=NULL;
	int used =0;
	int inUsed=0;
	int outUsed=0;

	while(argv[count]!='\0'){

		if ( (0!=strcmp(argv[count],">")) && (0!=strcmp(argv[count],"<")) && (used!=1) ){ 
			temp=strcat(temp,argv[count]);
			temp=strcat(temp," ");
		}
		else if( (0==strcmp(argv[count],">")) && (outUsed==0) ) {
			used=1;
			outUsed=1;
			if (argv[count+1]!='\0'){

				fpOut=fopen(argv[count+1],"w");
				if (fpOut==NULL){
					fprintf(stderr,"unable to open said file.\n");
					break;
				}
				fdout=fileno(fpOut);
				if (fdout==-1){
					fprintf(stderr,"unable to output to said file. Try again\n");
					break;
				}
				dup2(fdout,1);
			}
			else{
				 printf("you need another argument after >\n");
			}

		}
		else if( (0==strcmp(argv[count],"<")) && (inUsed==0) ) {
			used=1;
			inUsed=1;
			if (argv[count+1]!='\0'){
				fpIn=fopen(argv[count+1],"r");
				if (fpIn==NULL){
					fprintf(stderr,"unable to open said file.\n");
					break;
				}
				fdin=fileno(fpIn);
				if (fdin==-1){
					fprintf(stderr,"unable to input to said file. Try again\n");
					break;
				}
				dup2(fdin,0);
			}
			else{
				 printf("you need another argument after <\n");
			}
		}
	count ++;
	}


	char ** newArgv=parseCommand(temp);
	multWords(newArgv);
	free(temp);
	if (fpOut!=NULL){
		fclose(fpOut);
	}
	if (fpIn!=NULL){
		fclose(fpIn);
	}
	dup2(oldIn,0);
	dup2(oldOut,1);
	free(newArgv);

}

//this function utilizes fork and execlp to execute and one word command in a new process.
int oneWord(char * command){

	pid_t forker;
	int status;
	forker=fork();

	if (forker==0){
		execlp(command,command, NULL);
		fprintf(stderr,"%s: invalid command\n",command);
		exit(EXIT_FAILURE);
	}
	else{
		waitpid(forker,&status,0);
	}
	return 0;
}


//this function is called when the user inputs myinfo
//it prints out the current process's PID and PPID
void myInfo(){

	pid_t mine;
	pid_t parent;

	parent=getppid();
	mine=getpid();

	printf("My PID is : %d and my PPID is: %d\n", mine,parent);

}

//This function deals with signal handling.
//It ensures the current process will not end when it recieves SIGINT
void sig_handler(int sig){
	signal(sig,sig_handler);
	switch(sig){
		case SIGINT:
			printf("\n");
			y=1;
			break;
		default:
			y=1;
			printf("\n");
	}

}




int main(){

char *entry=calloc(1025,sizeof(char));


while (1){

	signal(SIGINT, sig_handler);

	pid_t childs=1;
	background=0;

	//main while loop for shell input
	while( childs >0){

		//deals with background processes
		childs=waitpid((pid_t)(-1),0,WNOHANG);
		if (childs==0){
			if (!y){
			printf("(bg process running)\n");
			}
		}
		if (childs>0){
			//if (test){
				printf("(process %d completed or was terminated by ^C)\n",childs);
		//	}
		}
	}

	printf("< fakeShell@clyde >");
	fgets(entry,1025,stdin); //each command must be equal to or less than 1024 chars in length
	if (entry==NULL){
		printf("Don't give me a null entry silly!! Shell will be exiting. Bye!\n");
		break;
	}

	if (y==1){
		entry[0]='\n';
		entry[1]='\0';
		y=0;
	}

	//run a process in the background by adding a & to the end of the command
	if ( strpbrk(entry,"&")!=NULL ){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		entry[strlen(entry)-1]='\n';
		background=1;
	}

	//exit the shell
	if (0== strcmp(entry,"exit\n")){
		break;
	}

	else if ((entry[0]=='\n') && (entry[1]=='\0') ){
		;
	}

	//if the user inputs "myinfo"
	else if (0== strcmp(entry,"myinfo\n")){
		myInfo();
	}

	//if the user inputs "cd", call to goHome() function
	else if (0== strcmp(entry,"cd\n")){
		goHome();
	}

	//if the command is to cd into a specific directory
	else if( (entry[0]=='c') && (entry[1]=='d') && (entry[2]==' ') && (5<=strlen(entry)) ){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		int change = chdir((const char *)(entry+3));
		if (change==-1){
			perror((entry+3));
		}

		char cwd[1024];
		if (change!=-1){
			printf("you cd'd to: %s\n",getcwd(cwd,1024));
		}
	}


	//if the command contains no redirection or piping
	else if( ( strpbrk(entry,"<>|&")==NULL ) && (entry[0]!='\0')){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		char ** argv;
		argv=parseCommand(entry);
		multWords(argv);
		free(argv);
	}

	//if the commmand contains I.O. redirection
	else if( (strpbrk(entry,"<>")!=NULL) && (strpbrk(entry,"|")==NULL)){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		char ** argv=parseCommand(entry);
		redirect(argv);
		free(argv);
		entry[0]='\n';
		entry[1]='\0';
	}

	//if your redirection was not done correctly
	//say for instance you had something like < arg1 arg2
	else if(redirectRight(entry)==-1){
		fprintf(stderr,"your pipe and redirection were not done correctly\n");
	}

	//command ocntains one pipe only
	else if( (strpbrk(entry,"<>")==NULL) && (pipeCount(entry)==1) ){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		char ** argv=parseCommand(entry);
		pipeOne(argv);
		free(argv);
	}

	//command contains redirection and a pipe
	else if( (strpbrk(entry,"<>")!=NULL) && (pipeCount(entry)==1) ){
		if (entry[strlen(entry)-1]=='\n'){
			entry[strlen(entry)-1]='\0';
		}
		char ** argv=parseCommand(entry);
		pipeArrow(argv);
		free(argv);
	}

	//if the user tries to do multi-piping
	else if (pipeCount(entry)>1){

		printf("This shell does not support multipipe commands\n");
		continue;
	}

	else{
		entry[strlen(entry)-1]='\0';
		printf("%s: invalid command\n",entry);
	}
}

free(entry);
return 0;

}
