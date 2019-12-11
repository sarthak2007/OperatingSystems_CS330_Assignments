// Name: Sarthak Singhal    Roll No.: 170635

/* To run:

- gcc part1.c
- ./a.out pattern_to_search path_to_file_or_dir 

*/

#include<stdio.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

//checks if loc is regular file
int is_file(char* loc){     
	struct stat tmp;
	stat(loc, &tmp);
	return S_ISREG(tmp.st_mode);
}

//checks if loc is directory
int is_dir(char* loc){		
	struct stat tmp;
	stat(loc, &tmp);
	return S_ISDIR(tmp.st_mode);
}

//print those lines in a file which contain the given pattern as a substring
void grep(char* pattern, char* name, char* loc, int fileordir){   // fileordir=1 if original path was of file , 0 if it was of directory
	int fd = open(name, O_RDONLY);			//open file
	if(fd < 0){
		perror("Error in opening file");
		exit(1);
	}
	char tmp,line[1000009];				//line[] stores a complete line from the file 
	int len=0;							//len stores the current length of the line
	while(read(fd, &tmp, 1)!=0){
		if(tmp=='\n'){					//read till next line
			line[len]=0;				//marks the end of string
			if(strstr(line, pattern))	//checks whether this line contains the given pattern or not
				if(!fileordir)	printf("%s:%s\n",loc,line);	//if original path was directory then print location of the file also
				else	printf("%s\n",line);
			len=0;continue;				//len becomes back to 0 as we are moving to next line
		}
		line[len++]=tmp;				//stores the line from the file character by character
	}
	line[len]=0;						//last line of the file is not covered above as we break at the end of the file
	if(len && strstr(line, pattern))	//if last line is not blank and it contains the pattern
		if(!fileordir)	printf("%s:%s\n",loc,line);
		else	printf("%s\n",line);
	close(fd);							//close file
}

//recursively check every file in sub-directories
void recurse(char* dir, char* complete, char* pattern){
	chdir(dir);	//move into the given directory

	DIR* curr = opendir(".");	//open current directory
	struct dirent* tmp;

	while((tmp=readdir(curr))!=NULL){
		if(strcmp(tmp->d_name,".") && strcmp(tmp->d_name,"..")){	//don't enter if we are on "." or ".."
			char loc[5000];			//array to append new path for subdirectories
			loc[0]=0;			
			strcpy(loc, complete);	//copy given path to loc
			if(loc[strlen(loc)-1]!='/')	//if end of loc doesn't have '/' then append it
				strcat(loc, "/");
			strcat(loc, tmp->d_name);	//append current directory name to the path
			if(is_file(tmp->d_name))	//if regular file then call grep
				grep(pattern, tmp->d_name, loc, 0);
			if(is_dir(tmp->d_name))		//if directory then recurse again
				recurse(tmp->d_name, loc, pattern);
		}
	}
	closedir(curr);		
	chdir("..");	//move back to the previous directory
}

int main(int argc,char** argv){
	if(argc!=3){
		printf("You should enter 2 arguments");
		exit(1);
	}
	if(is_dir(argv[2]))			//if path is of a directory
		recurse(argv[2], argv[2], argv[1]);
	if(is_file(argv[2]))		//if path is of a regular file
		grep(argv[1], argv[2], argv[2], 1);
	return 0;
}