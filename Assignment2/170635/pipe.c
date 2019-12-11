// Name: Sarthak Singhal     Roll No. : 170635


#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>
/***********************************************************************
 * Use this function to allocate pipe info && Don't Modify below function
 ***********************************************************************/
struct pipe_info* alloc_pipe_info()
{
    struct pipe_info *pipe = (struct pipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);
    pipe ->pipe_buff = buffer;
    return pipe;
}


void free_pipe_info(struct pipe_info *p_info)
{
    if(p_info)
    {
        os_page_free(OS_DS_REG ,p_info->pipe_buff);
        os_page_free(OS_DS_REG ,p_info);
    }
}
/*************************************************************************/
/*************************************************************************/


int pipe_read(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Read the contect from buff (pipe_info -> pipe_buff) and write to the buff(argument 2);
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */

    if(!filep || !(filep->pipe))
        return -EINVAL;

    //check whether read end of pipe is open or not and the given file object is pointing to read end of pipe or not
    if(filep->pipe->is_ropen != 1  || filep->mode != O_READ)
        return -EACCES;

    int read_start = filep->pipe->read_pos;
    if(read_start + count > filep->pipe->buffer_offset)
        return -EOTHERS;

    int i=read_start,j=0; // i iterate over pipe starting from read_position and j iterate over buff;

    while(i<read_start+(filep->pipe->buffer_offset)){
        if(i-read_start == count)   break;
        
        buff[j] = filep->pipe->pipe_buff[i];
        i++;j++;
    }

    //shifting the remaining characters to starting of the pipe
    while(i<read_start+(filep->pipe->buffer_offset)){
        filep->pipe->pipe_buff[i-j] = filep->pipe->pipe_buff[i];
        i++;
    }

    filep->pipe->buffer_offset -= j;   //update buffer_offset of pipe
    filep->pipe->write_pos -= j; //update write end of the pipe
    return j;
}


int pipe_write(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Write the contect from   the buff(argument 2);  and write to buff(pipe_info -> pipe_buff)
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */

    if(!filep || !(filep->pipe))
        return -EINVAL;

    //check whether write end of pipe is open or not and the given file object is pointing to write end of pipe or not
    if(filep->pipe->is_wopen != 1 || filep->mode != O_WRITE)
        return -EACCES;

    int write_start = filep->pipe->write_pos;
    if(write_start + count > PIPE_MAX_SIZE)
        return -EOTHERS;

    int i=write_start,j=0; // i iterate over pipe starting from starting position of write and j iterate over buff;

    while(i<PIPE_MAX_SIZE){
        if(j == count)  break;  //if count many characters are written then stop
        filep->pipe->pipe_buff[i] = buff[j];
        i++;j++;
    }

    filep->pipe->buffer_offset += j;   //update buffer_offset of pipe
    filep->pipe->write_pos = i; //update write end of the pipe
    return j;
}

int create_pipe(struct exec_context *current, int *fd)
{
    /**
    *  TODO:: Implementation of Pipe Create
    *  Create file struct by invoking the alloc_file() function, 
    *  Create pipe_info struct by invoking the alloc_pipe_info() function
    *  fill the valid file descriptor in *fd param
    *  Incase of Error return valid Error code 
    */
    if(!current)
        return -EINVAL;

    //find first two unused file descriptors
    int fdes = 3;
    while(fdes<32 && current->files[fdes])
      fdes++;
    if(fdes==32)
      return -EINVAL;
    fd[0] = fdes;
    fdes++;
    while(fdes<32 && current->files[fdes])
      fdes++;
    if(fdes==32)
      return -EINVAL;
    fd[1] = fdes;
    
    //call alloc_file
    current->files[fd[0]] = alloc_file();
    current->files[fd[1]] = alloc_file();
    if(current->files[fd[0]] == NULL)
        return -ENOMEM;
    if(current->files[fd[1]] == NULL)
        return -ENOMEM;

    struct file *filep1 = current->files[fd[0]];
    struct file *filep2 = current->files[fd[1]];
    
    //set inode
    filep1->inode = NULL;
    filep2->inode = NULL;

    //set type
    filep1->type = PIPE;
    filep2->type = PIPE;
    
    //set mode
    filep1->mode = O_READ;
    filep2->mode = O_WRITE;
    
    //set reference count
    filep1->ref_count = 1;
    filep2->ref_count = 1;
    
    //alloc pipe
    struct pipe_info *temp;
    temp = alloc_pipe_info();
    if(temp == NULL)
        return -ENOMEM;
    filep1->pipe = temp; 
    filep2->pipe = temp;
    
    //set parameters of pipe
    temp->read_pos = 0;
    temp->write_pos = 0;
    temp->buffer_offset = 0;
    temp->is_ropen = 1;
    temp->is_wopen = 1;

    //set functions of file objects
    filep1->fops->read = pipe_read;
    filep2->fops->write = pipe_write;
    filep1->fops->close = generic_close;
    filep2->fops->close = generic_close;
    return 0;
}

