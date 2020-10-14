#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SIZE 16

int main (int argc, char* argv[])
{
  sem_t *sem1;
   sem_t *sem2;
   int status;
   long int i, loop, temp, *shmPtr;
   int shmId;
   pid_t pid;
   unsigned int value;

   if( argc == 2 ) {
     loop =  atoi(argv[1]);
   }
   else if( argc > 2 ) {
      printf("Too many arguments supplied.\n");
   }
   else {
      printf("One argument expected.\n");
   }

   value = loop;

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n");
      exit (1);
   }
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) {
      perror ("can't attach\n");
      exit (1);
   }

   sem1 = sem_open("/semaphore1", O_CREAT, 0644, 0);
   sem2 = sem_open("/semaphore2", O_CREAT, 0644, 1);
   
   shmPtr[0] = 0;
   shmPtr[1] = 1;

   if (!(pid = fork())) {
      for (i=0; i<loop; i++) {
	sem_wait(sem1);
	temp = shmPtr[0];
	shmPtr[0] = shmPtr[1];
	shmPtr[1] = temp;
	sem_post(sem2);
      }
      if (shmdt (shmPtr) < 0) {
         perror ("just can't let go\n");
	 sem_close(sem1);
	 sem_unlink("/semaphore1");
	 sem_close(sem2);
	 sem_unlink("/semaphore2");
         exit (1);
      }
      exit(0);
      
   }
   else
      for (i=0; i<loop; i++) {
	sem_wait(sem2);
	temp = shmPtr[0];
	shmPtr[0] = shmPtr[1];
	shmPtr[1] = temp;
	sem_post(sem1);
      }

   wait (&status);
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) {
      perror ("just can't let go\n");
      exit (1);
   }
   if (shmctl (shmId, IPC_RMID, 0) < 0) {
      perror ("can't deallocate\n");
      exit(1);
   }
   sem_close(sem1);
   sem_unlink("/semaphore1");
   sem_close(sem2);
   sem_unlink("/semaphore2");
   return 0;
}
