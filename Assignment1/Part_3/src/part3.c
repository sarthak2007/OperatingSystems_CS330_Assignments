// Name: Sarthak Singhal    Roll No.: 170635

/* To run:

- gcc part3.c
- ./a.out path_of_dir	
*/


/* ``````````````````Explanation``````````````````

In a while loop in int main() I am checking for contents of the given directory and updating a variable which stores the size of the
given root directory.
Case1 :If I encounter a file in while loop then simply find its size and update the variable storing size.

Case2 :If it a directory then I create a pipe and then fork the process. In child process I call a function which finds the size of
this sub-directory recursively and returns it. Then I print the name of the sub-directory and its size on STDOUT. Then I dup the output
end of the pipe with STDOUT and throw the size of this sub-directory in the pipe. And exit from child process.
In parent process I take the input from the pipe and update the variable storing the size of the root directory by adding the size of
the sub-directory for which we called recursive function.  

Then finally I output the name of the root directory by extracting the name of the root directory from the given path.
And also output the size of the root directory.

*/

#include<stdio.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>


//checks if the path is of directory or not
int is_dir(char* loc){
	struct stat tmp;
	stat(loc, &tmp);
	return S_ISDIR(tmp.st_mode);
}


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

//extract name of the root directory from the given path
char* extract(char* path){
	int len = strlen(path);	//length of path
	char tmp[len+1];	//stores the reverse of the name of the directory	
	int i=len-1,cnt=0;
	if(path[len-1]=='/')	i--;	//case when path has / in the end
	for( ;i>=0;i--){
		if(path[i]=='/')	break;
		tmp[cnt++]=path[i];
	}
	char* ans = malloc((cnt+1)*sizeof(char)); //stores the name of the directory
	for(i=0;i<cnt;i++){
		ans[i] = tmp[cnt-i-1];
	}
	ans[i] = 0;	// mark the end of the string
	return ans;
}


//recursively searches into the sub-directories of the given directory to find sizes of files and returns the size of the directory
int recurse(char* dir){
	chdir(dir);		//move inside the given directory
	long long int size = 0; //variable to store the size of the given directory
	DIR* curr = opendir(".");	//open the current directory
	struct dirent* tmp;
	while((tmp=readdir(curr))!=NULL){
		if(strcmp(tmp->d_name,".") && strcmp(tmp->d_name,"..")){	//don't enter if we are on "." or ".."
			if(is_dir(tmp->d_name))		//if directory then recurse into it to find size of it
				size += recurse(tmp->d_name);	//update the size variable
			else{						//if file then find its size
				int fd = open(tmp->d_name, O_RDONLY);
				size += lseek(fd, 0, SEEK_END);	//finds the size of the file and update the size variable
				close(fd);	//closes file
			}
		}
	}
	closedir(curr);
	chdir("..");	//return back to the previous directory
	return size;	//returns size of the given directory
}

int main(int argc,char** argv){
	if(argc!=2){
		printf("You should enter only 1 argument");
		exit(1);
	}
	long long int ans = 0;	//stores the size of the root directory
	chdir(argv[1]);			//move inside the given directory
	DIR* curr = opendir(".");
	struct dirent* tmp;

	while((tmp=readdir(curr))!=NULL){
		if(strcmp(tmp->d_name,".") && strcmp(tmp->d_name,"..")){	//don't enter if we are on "." or ".."
			if(is_dir(tmp->d_name)){	//if diirectory then we have to do fork and all
				int fd[2];
				if(pipe(fd)<0)	pipeerror();	// create pipe
				int pid = fork();		//forking
				if(pid<0)	forkerror();
				else if(!pid){
					//child process
					long long int this_size = recurse(tmp->d_name); //find size of this sub-directory recursively
					printf("%s %lld\n", tmp->d_name, this_size);	//output name of this sub-directory and its size on STDOUT
					fflush(stdout);
					close(fd[0]);		//close input end of the pipe
					write(fd[1], &this_size, sizeof(this_size));	//write into pipe
					exit(0);	//exit from child
				}
				else{
					//parent process
					close(fd[1]);	//close output end of the pipe
					long long int tp;
					read(fd[0], &tp, sizeof(tp));	//read from the pipe the size of the sub-directory
					ans += tp;	//update ans with size of sub-directory
				}
			}
			else{						//if file then directly find its size
				int fd = open(tmp->d_name, O_RDONLY);
				ans += lseek(fd, 0, SEEK_END);	//returns size of file and update ans
				close(fd);
			}
		}
	}
	printf("%s %lld\n", extract(argv[1]), ans);	//Print root and its size
	fflush(stdout);
	closedir(curr);

	return 0;
}