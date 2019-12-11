// Name: Sarthak Singhal    Roll No.: 170635

/* To run:

- gcc part2.c

Part1:
	- ./a.out @ pattern_to_search path_to_file_or_dir 

Part2:
	- ./a.out $ pattern_to_search path_to_file_or_dir output_file command	
*/

#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>


//if there is error in pipe system call
void pipeerror(){
	printf("Error in pipe occured\n");
	exit(1);
}

//if there is error in fork
void forkerror(){
	perror("Error in fork occured\n");
	exit(1);
}

int main(int argc,char** argv){

	// ******** Part1 begins ******** //
	if(!strcmp(argv[1], "@")){
		int fd[2];
		if(pipe(fd)<0)	pipeerror();  //create a pipe
		int pid = fork();	//forking
		if(pid<0)	forkerror();
		else if(!pid){		// child process
			dup2(fd[1], 1);	// duping output end of the pipe with STDOUT
			close(fd[0]);	// closing input end of the pipe
			char *list[] = {"grep", "-rF", argv[2], argv[3], NULL};
			execvp("grep", list); // execute grep and throw its output to pipe
		}
		else{				// parent process
			dup2(fd[0], 0);	// duping input end of the pipe with STDIN
			close(fd[1]);	// closing output end of the pipe
			char *list[] = {"wc", "-l", NULL};
			execvp("wc", list); // execute "wc -l" and by taking input from pipe 
		}
	}
	// ******** Part1 ends ******** //


	// ******** Part2 begins ******** //
	else if(!strcmp(argv[1], "$")){
		int fd[2];
		if(pipe(fd)<0)	pipeerror(); //create a pipe
		int pid = fork();   //forking
		if(pid<0)	forkerror();
		else if(!pid){		// child process
			close(fd[0]);	// closing input end of the pipe
			dup2(fd[1], 1);	// duping output end of the pipe with STDOUT
			char *list[] = {"grep", "-rF", argv[2], argv[3], NULL};
			execvp("grep", list);	// execute grep and throw its output to pipe
		}
		else{
			//parent process
			close(fd[1]);		// closing output end of the pipe
			int newfd[2];		
			if(pipe(newfd)<0)	pipeerror();	//create a new pipe
			int pid1 = fork();	//forking again
			if(pid1<0)	forkerror();
			else if(!pid1){		//child due to new fork
				close(newfd[0]); // closing input end of the new pipe
				dup2(newfd[1], 1);	// duping output end of the new pipe with STDOUT	
				char c;
				creat(argv[4], 0664); // create output file
				int file = open(argv[4], O_WRONLY);	//open the output file
				while(read(fd[0], &c, 1)){	//read from old pipe
					write(file, &c, 1);		//write into file
					write(1, &c, 1);		//write into new pipe
					fflush(stdout);
				}
				close(file);	//close the file
				exit(0);		//exit from child
			}
			else{			//parent process
				close(newfd[1]);	//closing output end of new pipe
				dup2(newfd[0], 0);	//duping input end of the new pipe with STDIN
				char *list[argc-4];	//list for arguments of the command to be executed
				int i;
				for(i=0;i<argc-5;i++)	list[i] = argv[i+5];
				list[i] = NULL;
				execvp(list[0], list);	//executing the new command
			}
		}
	}
	// ******** Part2 ends ******** //

	else{
		printf("You should enter correct arguments\n");
		exit(1);
	}
	return 0;
}