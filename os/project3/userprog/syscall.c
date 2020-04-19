#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

typedef int pid_t;

//TODO Rename
struct proc_file 
{
  struct file* ptr;
  int fd;
  struct list_elem elem;
};

void (*syscall_handlers[20])(struct intr_frame *);
void* check_addr(const void *vaddr);
struct proc_file* list_search(struct list* files, int fd);

static void syscall_handler (struct intr_frame *f);

void sys_halt(struct intr_frame* f);
void sys_exit(struct intr_frame* f);
void sys_exec(struct intr_frame* f);
void sys_wait(struct intr_frame* f);
void sys_create(struct intr_frame* f);
void sys_remove(struct intr_frame* f);
void sys_open(struct intr_frame* f);
void sys_filesize(struct intr_frame* f);
void sys_read(struct intr_frame* f);
void sys_write(struct intr_frame* f);
void sys_seek(struct intr_frame* f);
void sys_tell(struct intr_frame* f);
void sys_close(struct intr_frame* f);

void halt();
void exit(int status);
pid_t exec(const char *file);
int wait(pid_t pid);

bool create(const char *file, unsigned size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, void* buffer, unsigned size);
void seek(int fd, int position);
unsigned tell(int fd);
void close(int fd);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall"); //registering handler for TRAP

  syscall_handlers[SYS_HALT] = &sys_halt;
  syscall_handlers[SYS_EXIT] = &sys_exit;
  syscall_handlers[SYS_EXEC] = &sys_exec;
  syscall_handlers[SYS_WAIT] = &sys_wait;
  syscall_handlers[SYS_CREATE] = &sys_create;
  syscall_handlers[SYS_REMOVE] = &sys_remove;
  syscall_handlers[SYS_OPEN] = &sys_open;
  syscall_handlers[SYS_FILESIZE] = &sys_filesize;
  syscall_handlers[SYS_READ] = &sys_read;
  syscall_handlers[SYS_WRITE] = &sys_write;
  syscall_handlers[SYS_SEEK] = &sys_seek;
  syscall_handlers[SYS_TELL] = &sys_tell;
  syscall_handlers[SYS_CLOSE] = &sys_close;
}

void sys_halt(struct intr_frame* f)
{
  shutdown_power_off();
};
void sys_exit(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  exit(arg1);
};

void sys_exec(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  f->eax = process_execute(arg1);
};

void sys_wait(struct intr_frame* f){};

void sys_create(struct intr_frame* f)
{
  uint32_t arg1, arg2;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  user_esp++;
  check_addr(user_esp);
  arg2 = (uint32_t)(*user_esp);

  check_addr(arg1);
  
  f->eax = filesys_create(arg1,arg2);
};

void sys_remove(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  f->eax = (filesys_remove(arg1) != NULL);
};

void sys_open(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);

  check_addr(arg1);

  struct file* fptr;
  fptr = filesys_open(arg1);

  if (fptr == NULL)
  {
    f->eax = -1;
  }
  else 
  {
    struct proc_file *pfile = malloc(sizeof(*pfile));
    pfile->ptr = fptr;
    pfile->fd = thread_current()->fd_count;
    thread_current()->fd_count++;
    list_push_back(&thread_current()->files, &pfile->elem);
    f->eax = pfile->fd;
  }

};
void sys_filesize(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);

  struct proc_file* file_result = list_search(&thread_current()->files, arg1);
  f->eax = file_length(file_result->ptr);

};
void sys_read(struct intr_frame* f)
{
  uint32_t arg1, arg2, arg3;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  user_esp++;
  check_addr(user_esp);
  arg2 = (uint32_t)(*user_esp);

  check_addr(arg2);
  user_esp++;
  check_addr(user_esp);
  arg3 = (uint32_t)(*user_esp);


  f->eax = read((int)arg1, (char *)arg2, (unsigned)arg3);
};

void sys_write(struct intr_frame* f)
{
  uint32_t arg1, arg2, arg3;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  user_esp++;
  check_addr(user_esp);
  arg2 = (uint32_t)(*user_esp);
  check_addr(arg2);
  user_esp++;
  check_addr(user_esp);
  arg3 = (uint32_t)(*user_esp);
  f->eax = write((int)arg1, (char *)arg2, (unsigned)arg3);

};
void sys_seek(struct intr_frame* f){};
void sys_tell(struct intr_frame* f){};
void sys_close(struct intr_frame* f)
{
  uint32_t arg1;
  uint32_t *user_esp = f->esp;
  user_esp++;
  check_addr(user_esp);
  arg1 = (uint32_t)(*user_esp);
  close(arg1);
};

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  check_addr(f->esp);

  int callNo = * (int *)f->esp;
  
  syscall_handlers[callNo](f);

}

int write (int fd, void *buffer, unsigned size)
{
  if (fd == 1)
  {
    putbuf(buffer, size);
    return size;
  }
  else
  {
    struct proc_file* fptr = list_search(&thread_current()->files, fd);

    if (fptr == NULL)
    {
      return -1;
    }
    else 
    {
      int ret = file_write(fptr->ptr, buffer, size);

      return ret;
    }
  }
}

int read(int fd, void*buffer, unsigned size)
{
  if (fd == 0)
  {
    for (int i = 0; i < size; i++)
    {
      buffer = input_getc();
    }
    return size;
  }
  else
  {
    struct proc_file* fptr = list_search(&thread_current()->files, fd);
    if (fptr == NULL)
    {
      return -1;
    }
    else 
    {
      return file_read_at(fptr->ptr, buffer, size, 0);
    }

  }
  
}

void close(int fd)
{
  struct list_elem *e;

  struct list* files = &thread_current()->files;

  for (e = list_begin(files); e != list_end(files); e = list_next(e))
  {
    struct proc_file *f = list_entry(e, struct proc_file, elem);
    
    if(f->fd == fd)
    {
      file_close(f->ptr);
      list_remove(e);
    }

  }
}

void exit(int status)
{
  struct thread *curthread = thread_current();
  curthread->ex = true;
  curthread->exit_status = status;
  
  printf("%s: exit(%d)\n", curthread->name, status);
  thread_exit();
}



//TODO
void* check_addr(const void *vaddr)
{
	if (!is_user_vaddr(vaddr))
	{
		exit(-1);
		return 0;
	}
	void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
	if (!ptr)
	{
		exit(-1);
		return 0;
	}
	return ptr;
}

//TODO
struct proc_file* list_search(struct list* files, int fd)
{

	struct list_elem *e;

      for (e = list_begin (files); e != list_end (files);
           e = list_next (e))
        {
          struct proc_file *f = list_entry (e, struct proc_file, elem);
          if(f->fd == fd)
          	return f;
        }
   return NULL;
}

void close_all_files(struct list* files)
{
	struct list_elem *e;

  for (e = list_begin (files); e != list_end (files); e = list_next (e))
  {
    struct proc_file *f = list_entry (e, struct proc_file, elem);

    file_close(f->ptr);
    list_remove(e);
  }
}

/*

System Call: void halt (void)
    Terminates Pintos by calling shutdown_power_off() (declared in "threads/init.h"). This should be seldom used, because you lose some information about possible deadlock situations, etc. 

System Call: void exit (int status)
    Terminates the current user program, returning status to the kernel. If the process's parent waits for it (see below), this is the status that will be returned. Conventionally, a status of 0 indicates success and nonzero values indicate errors. 

System Call: pid_t exec (const char *cmd_line)
    Runs the executable whose name is given in cmd_line, passing any given arguments, and returns the new process's program id (pid). Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason. Thus, the parent process cannot return from the exec until it knows whether the child process successfully loaded its executable. You must use appropriate synchronization to ensure this. 

System Call: int wait (pid_t pid)
    Waits for a child process pid and retrieves the child's exit status.

    If pid is still alive, waits until it terminates. Then, returns the status that pid passed to exit. If pid did not call exit(), but was terminated by the kernel (e.g. killed due to an exception), wait(pid) must return -1. It is perfectly legal for a parent process to wait for child processes that have already terminated by the time the parent calls wait, but the kernel must still allow the parent to retrieve its child's exit status, or learn that the child was terminated by the kernel.

    wait must fail and return -1 immediately if any of the following conditions is true:

        pid does not refer to a direct child of the calling process. pid is a direct child of the calling process if and only if the calling process received pid as a return value from a successful call to exec.

        Note that children are not inherited: if A spawns child B and B spawns child process C, then A cannot wait for C, even if B is dead. A call to wait(C) by process A must fail. Similarly, orphaned processes are not assigned to a new parent if their parent process exits before they do.

        The process that calls wait has already called wait on pid. That is, a process may wait for any given child at most once. 

    Processes may spawn any number of children, wait for them in any order, and may even exit without having waited for some or all of their children. Your design should consider all the ways in which waits can occur. All of a process's resources, including its struct thread, must be freed whether its parent ever waits for it or not, and regardless of whether the child exits before or after its parent.

    You must ensure that Pintos does not terminate until the initial process exits. The supplied Pintos code tries to do this by calling process_wait() (in "userprog/process.c") from main() (in "threads/init.c"). We suggest that you implement process_wait() according to the comment at the top of the function and then implement the wait system call in terms of process_wait().

    Implementing this system call requires considerably more work than any of the rest.

System Call: bool create (const char *file, unsigned initial_size)
    Creates a new file called file initially initial_size bytes in size. Returns true if successful, false otherwise. Creating a new file does not open it: opening the new file is a separate operation which would require a open system call. 

System Call: bool remove (const char *file)
    Deletes the file called file. Returns true if successful, false otherwise. A file may be removed regardless of whether it is open or closed, and removing an open file does not close it. See Removing an Open File, for details. 

System Call: int open (const char *file)
    Opens the file called file. Returns a nonnegative integer handle called a "file descriptor" (fd), or -1 if the file could not be opened.

    File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will never return either of these file descriptors, which are valid as system call arguments only as explicitly described below.

    Each process has an independent set of file descriptors. File descriptors are not inherited by child processes.

    When a single file is opened more than once, whether by a single process or different processes, each open returns a new file descriptor. Different file descriptors for a single file are closed independently in separate calls to close and they do not share a file position.

System Call: int filesize (int fd)
    Returns the size, in bytes, of the file open as fd. 

System Call: int read (int fd, void *buffer, unsigned size)
    Reads size bytes from the file open as fd into buffer. Returns the number of bytes actually read (0 at end of file), or -1 if the file could not be read (due to a condition other than end of file). Fd 0 reads from the keyboard using input_getc(). 

System Call: int write (int fd, const void *buffer, unsigned size)
    Writes size bytes from buffer to the open file fd. Returns the number of bytes actually written, which may be less than size if some bytes could not be written.
    Writing past end-of-file would normally extend the file, but file growth is not implemented by the basic file system. The expected behavior is to write as many bytes as possible up to end-of-file and return the actual number written, or 0 if no bytes could be written at all.
    Fd 1 writes to the console. Your code to write to the console should write all of buffer in one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is reasonable to break up larger buffers.) Otherwise, lines of text output by different processes may end up interleaved on the console, confusing both human readers and our grading scripts.

System Call: void seek (int fd, unsigned position)
    Changes the next byte to be read or written in open file fd to position, expressed in bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)

    A seek past the current end of a file is not an error. A later read obtains 0 bytes, indicating end of file. A later write extends the file, filling any unwritten gap with zeros. (However, in Pintos files have a fixed length until project 4 is complete, so writes past end of file will return an error.) These semantics are implemented in the file system and do not require any special effort in system call implementation.

System Call: unsigned tell (int fd)
    Returns the position of the next byte to be read or written in open file fd, expressed in bytes from the beginning of the file. 

System Call: void close (int fd)
    Closes file descriptor fd. Exiting or terminating a process implicitly closes all its open file descriptors, as if by calling this function for each one. 
*/