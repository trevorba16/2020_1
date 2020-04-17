#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);


struct file_descriptor{
  int fd;
  struct list_elem elem; 
  struct file* file;
};

struct process {
  struct list_elem elem;
  struct thread *thread;  
}

#endif /* userprog/process.h */
