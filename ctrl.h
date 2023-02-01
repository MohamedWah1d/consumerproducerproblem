#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <signal.h>
#include <vector>


//used with the semctl function and depends on the control operation.
union semun
{
    int val; /* val for SETVAL */
    struct semid_ds *buf; /* Buffer for IPC_STAT and IPC_SET */
    unsigned short *array; /* Buffer for GETALL and SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO and SEM_INFO*/
};

//taking name and price of the product from the producer.
struct item
{
    char comodity_name[11];
    double today_price;
};

//initiazling the shared memory buffer.
int shm_id(int buf_size)
{
    // ftok to generate unique key
    key_t key = ftok("sharedmemory", 1);

    // shmget returns an identifier in shmid
    int shmid = shmget(key,buf_size*sizeof(struct item), IPC_CREAT | 0666);

    return shmid;
}

//To create a shared memory buffer.
struct item *shm_buffer(int shmid)
{
    // shmat to attach to shared memory
    struct item *shmaddress = (struct item*) shmat(shmid,(void*)0,0);

    return shmaddress;
}

//To detach semaphores from shared memmory.
template <class T> int shm_detach(int shmid, T *b )
{
    shmdt(b);

    //to release the semaphores
    int semrel = semctl(shmid, IPC_RMID, 0);
    return semrel;
}

// The wait function
void sem_wait(int semid, int id)
{
    // setting the values of the sembuf struct to be used in the semop function.
    struct sembuf sem_lock = {0, -1, 0};
    sem_lock.sem_num = id;

    // updating the semaphore value and perform the wait operation
    semop(semid, &sem_lock, 1);
}

// the signal function.
void sem_signal(int semid, int id)
{
    // setting the values of the sembuf struct to be used in the semop function.
    struct sembuf sem_unlock = {0, 1, 0};
    sem_unlock.sem_num = id;

    // updating the semaphore value and perform the signal operation
    semop(semid, &sem_unlock, 1);
}

// Get the value of the semaphore used for debugging.
int sem_getval(int semid, int id)
{
    // to get the desired value and pass it to the semctl function
    union semun value;

    // initializing the semaphore
    int semval = semctl(semid, id, GETVAL, value);
    return semval;
}
