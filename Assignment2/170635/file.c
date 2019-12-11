// Name: Sarthak Singhal     Roll No. : 170635


#include<types.h>
#include<context.h>
#include<file.h>
#include<lib.h>
#include<serial.h>
#include<entry.h>
#include<memory.h>
#include<fs.h>
#include<kbd.h>
#include<pipe.h>


/************************************************************************************/
/***************************Do Not Modify below Functions****************************/
/************************************************************************************/
void free_file_object(struct file *filep)
{
    if(filep)
    {
       os_page_free(OS_DS_REG ,filep);
       stats->file_objects--;
    }
}

struct file *alloc_file()
{
  
  struct file *file = (struct file *) os_page_alloc(OS_DS_REG); 
  file->fops = (struct fileops *) (file + sizeof(struct file)); 
  bzero((char *)file->fops, sizeof(struct fileops));
  stats->file_objects++;
  return file; 
}

static int do_read_kbd(struct file* filep, char * buff, u32 count)
{
  kbd_read(buff);
  return 1;
}

static int do_write_console(struct file* filep, char * buff, u32 count)
{
  struct exec_context *current = get_current_ctx();
  return do_write(current, (u64)buff, (u64)count);
}

struct file *create_standard_IO(int type)
{
  struct file *filep = alloc_file();
  filep->type = type;
  if(type == STDIN)
     filep->mode = O_READ;
  else
      filep->mode = O_WRITE;
  if(type == STDIN){
        filep->fops->read = do_read_kbd;
  }else{
        filep->fops->write = do_write_console;
  }
  filep->fops->close = generic_close;
  filep->ref_count = 1;
  return filep;
}

int open_standard_IO(struct exec_context *ctx, int type)
{
   int fd = type;
   struct file *filep = ctx->files[type];
   if(!filep){
        filep = create_standard_IO(type);
   }else{
         filep->ref_count++;
         fd = 3;
         while(ctx->files[fd])
             fd++; 
   }
   ctx->files[fd] = filep;
   return fd;
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/



void do_file_fork(struct exec_context *child)
{
   /*TODO the child fds are a copy of the parent. Adjust the refcount*/
    for(int i=0;i<MAX_OPEN_FILES;i++){
      if(child->files[i]){    //if file descriptor is not unused
        child->files[i]->ref_count++;
      }
    }
}

void do_file_exit(struct exec_context *ctx)
{
   /*TODO the process is exiting. Adjust the ref_count
     of files*/
    for(int i=0;i<MAX_OPEN_FILES;i++){
      if(ctx->files[i]){      //if file descriptor is not unused
        generic_close(ctx->files[i]);
        ctx->files[i] = NULL;
      }
    }
}

long generic_close(struct file *filep)
{
  /** TODO Implementation of close (pipe, file) based on the type 
   * Adjust the ref_count, free file object
   * Incase of Error return valid Error code 
   *
   */
    if(filep == NULL)
      return -EINVAL;

    if(filep->ref_count == 1){    //if only file descriptor is pointing to the given file object
      if(filep->type == PIPE){          //if pipe is not NULL

        if(filep->mode == O_READ)   //read end of pipe
          filep->pipe->is_ropen = 0;  //close read end

        if(filep->mode == O_WRITE)   //write end of pipe
          filep->pipe->is_wopen = 0;  //close write end

        if(filep->pipe->is_wopen==0 && filep->pipe->is_ropen==0) //if both ends of pipe are closed
          free_pipe_info(filep->pipe);      //free pipe's memory
      }

      free_file_object(filep);
      filep = NULL;
    }
    else{     //if reference count is greater than 1
      filep->ref_count--;  
      filep = NULL;
    }
    return 0;
}

static int do_read_regular(struct file *filep, char * buff, u32 count)
{
   /** TODO Implementation of File Read, 
    *  You should be reading the content from File using file system read function call and fill the buf
    *  Validate the permission, file existence, Max length etc
    *  Incase of Error return valid Error code 
    * */
    if(filep==NULL || filep->inode==NULL)
      return -EINVAL;
    struct inode *temp = filep->inode;  //get the inode pointed by the file object

    if(filep->offp + count > temp->file_size)
        return -EOTHERS;

    //checking the permissions and flags
    if((filep->mode & O_READ) && !((temp->mode) & O_READ))
      return -EACCES;
    //checking if file is open in read mode or not
    if(!(filep->mode & O_READ))      
      return -EACCES;    

    if(temp->file_size > FILE_SIZE)     //checking file size
      return -EOTHERS;

    //read function call
    int size = flat_read(filep->inode, buff, count, &(filep->offp));
    
    //increment the offset
    filep->offp += size;
    return size;  //return number of bytes read
}


static int do_write_regular(struct file *filep, char * buff, u32 count)
{
    /** TODO Implementation of File write, 
    *   You should be writing the content from buff to File by using File system write function
    *   Validate the permission, file existence, Max length etc
    *   Incase of Error return valid Error code 
    * */
    if(filep==NULL || filep->inode==NULL)
      return -EINVAL;
    struct inode *temp = filep->inode; //get inode pointed by the file object

    if(filep->offp + count > FILE_SIZE)
        return -EOTHERS;

    //checking the permissions and flags
    if((filep->mode & O_WRITE) && !((temp->mode) & O_WRITE))
      return -EACCES;
    //checking if file is open in write mode or not
    if(!(filep->mode & O_WRITE))      
      return -EACCES;
    
    if(temp->file_size > FILE_SIZE)   //checking file size
      return -EOTHERS;
    
    //write function call
    int size = flat_write(filep->inode, buff, count, &(filep->offp));
    if(size==-1) //if it returns error
      return -EOTHERS;
    filep->offp += size; //update the file offset
    return size; //return number of bytes written
}

static long do_lseek_regular(struct file *filep, long offset, int whence)
{
    /** TODO Implementation of lseek 
    *   Set, Adjust the ofset based on the whence
    *   Incase of Error return valid Error code 
    * */

    int newoffset = filep->offp;
    
    if(!filep)
      return -EINVAL;
    
    if(whence == SEEK_CUR)
      newoffset += offset;  //increment offset from current position
    else if(whence == SEEK_END) //increment offset from end position
      newoffset = filep->inode->file_size + offset;
    else if(whence == SEEK_SET)  //set offset same as the given offset 
      newoffset = offset;

    if(newoffset >= FILE_SIZE)  //if newoffset is greater than maximum file size
      return -EOTHERS;

    filep->offp = newoffset;  //update the file offset
    
    return filep->offp;  
}

extern int do_regular_file_open(struct exec_context *ctx, char* filename, u64 flags, u64 mode)
{ 
  /**  TODO Implementation of file open, 
    *  You should be creating file(use the alloc_file function to creat file), 
    *  To create or Get inode use File system function calls, 
    *  Handle mode and flags 
    *  Validate file existence, Max File count is 32, Max Size is 4KB, etc
    *  Incase of Error return valid Error code 
    * */
    if(ctx == NULL)
      return -EINVAL;

    if(strlen(filename)>=256)      //checking length of filename
      return -EOTHERS;

    struct inode *temp;
    temp = lookup_inode(filename);  //lookup inode
    if(flags & O_CREAT){            //if opened in create mode
      if(temp==NULL){                //check file already present
        temp = create_inode(filename, mode);  //if not present then create inode  
        if(temp == NULL)
          return -ENOMEM;
      }
      else{
        return -EOTHERS;
      }
    }
    else{                 
      if(temp==NULL)              //if file is not opened in create mode and file is not present
        return -EINVAL;
    }

    if(temp->file_size > FILE_SIZE)   //if filesize is greater than maximum file size
      return -EOTHERS;

    //checking whether flags match with permissions or not
    if(((flags & O_READ) && !((temp->mode) & O_READ)) || ((flags & O_WRITE) && !((temp->mode) & O_WRITE)))
      return -EACCES;

    //search for first unused file descriptor
    int fd = 3;
    while(fd<MAX_OPEN_FILES && ctx->files[fd])
      fd++;
    if(fd==MAX_OPEN_FILES)
      return -EINVAL;
    
    //allocate and set the corresponding parameters
    ctx->files[fd] = alloc_file();
    if(ctx->files[fd] == NULL)
      return -ENOMEM;
    ctx->files[fd]->inode = temp;
    ctx->files[fd]->type = REGULAR;
    ctx->files[fd]->mode = flags;
    ctx->files[fd]->offp = 0;
    ctx->files[fd]->ref_count = 1;
    ctx->files[fd]->pipe = NULL;
    ctx->files[fd]->fops->read = do_read_regular;
    ctx->files[fd]->fops->write = do_write_regular;
    ctx->files[fd]->fops->close = generic_close;
    ctx->files[fd]->fops->lseek = do_lseek_regular;
    return fd; //return the first unused file descriptor
}

int fd_dup(struct exec_context *current, int oldfd)
{
     /** TODO Implementation of dup 
      *  Read the man page of dup and implement accordingly 
      *  return the file descriptor,
      *  Incase of Error return valid Error code 
      * */

    //check if oldfd is valid or not
    if(!(oldfd >=0 && oldfd<MAX_OPEN_FILES))
      return -EINVAL;

    if(current == NULL || current->files[oldfd]==NULL)
      return -EINVAL;
    
    //find first unused file descriptor
    int fd = 0;
    while(fd<MAX_OPEN_FILES && current->files[fd])
      fd++;
    if(fd==MAX_OPEN_FILES)
      return -EINVAL;

    //point the new file descriptor to oldfd's file object
    current->files[fd] = current->files[oldfd];
    
    //increment the reference count of the file object
    current->files[oldfd]->ref_count++;
    return fd;
}


int fd_dup2(struct exec_context *current, int oldfd, int newfd)
{
  /** TODO Implementation of the dup2 
    *  Read the man page of dup2 and implement accordingly 
    *  return the file descriptor,
    *  Incase of Error return valid Error code 
    * */

  //check if oldfd and newfd are valid or not
  if(!(oldfd >=0 && oldfd<MAX_OPEN_FILES))
      return -EINVAL;
  if(!(newfd >=0 && newfd<MAX_OPEN_FILES))
      return -EINVAL;    

  if(current == NULL || current->files[oldfd]==NULL)
    return -EINVAL;

  //if both are same then do nothing
  if(oldfd == newfd)
      return newfd;

  //if newfd is pointing to a file object the close it first  
  if(current->files[newfd])
    generic_close(current->files[newfd]);

  //point the new file descriptor to oldfd's file object
  current->files[newfd] = current->files[oldfd];

  //increment the reference count of the file object
  current->files[oldfd]->ref_count++;
  return newfd;
}
