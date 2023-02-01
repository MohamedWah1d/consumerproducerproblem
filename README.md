# CONSUMER PRODUCER PROBLEM
## Goal:
#### SOLVE THE CONSUMER PRODUCER PROBLEM USING SEMAPHORES FOR ACCESSING THE SHARED MEMORY BUFFER
## Structure Used
- Semaphores from System V library
- Shared memory from System V library
<br>

## Description:
#### Shared memory is represented by a consecutive amount of bytes decided and created by the consumer on its call `./consumer (size)`. We access these bytes as a string where we save only characters in it.
#### Shared memory string is in the form of (commodity,price) each in a separate line in the same string to prevent multiple pointers.
#### The producer uses this memory by tokenizing the first line and removing that line from the whole string.
#### The consumer uses this memory by concatenating a new line to the string.
<br>

## CODE: 
### Consumer
> Consumer is responsible in this code to generate both the semaphores and the shared memory
- We have a map in the form of:

|    Commodity      |   INDEX  	|
| :-----------: | :----------:  |
| 	ALUMINIUM	| 0  |
| COPPER | 1  |
| COTTON | 2  |
| CRUDEOIL | 3  |
| GOLD | 4  |
| LEAD | 5  |
| MENTHAOIL | 6  |
| NATURALGAS | 7  |
| NICKEL | 8  |
| SILVER | 9  |
| ZINC	| 10  |
- Then there is the initialization:

```C    
int *out;

    // initializing the shared memory buffer.
    int shmid = shm_id(bs);
    buffer = shm_buffer(shmid);

    // getting the semid of the semaphores
    semid = get_semid();

    // initializing the semaphores.
    sem_init(semid, bs);

    //inializing the indexing of the shared memory buffer.
    shmid1 = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    out = (int *)shmat(shmid1, (void *)0, 0);
    *out = 0;
```
- This is init memory and semaphore function in the previous part:
```C
// function to get the sem id.
int get_semid()
{
    // ftok to generate unique key
    key_t key = ftok("semaphore", 1);

    // creating the semaphore
    // 0666 for the access permissions
    int semid = semget(key, 3, 0666 | IPC_CREAT | IPC_EXCL);

    return semid;
}

// function to initialize the semaphore
void sem_init(int semid, int bs)
{
    union semun value;
    // to get the desired value and pass it to the semctl function
    // Initializing the n semaphore, "Full semaphore".
    value.val = 0;
    int semvaln = semctl(semid, 0, SETVAL, value);
    if (semvaln == -1)
    {
        perror("semctl: error initializing the n semaphore");
        exit(1);
    }

    // Initializing the e semaphore, "Empty semaphore".
    value.val = bs;
    int semvale = semctl(semid, 1, SETVAL, value);
    if (semvale == -1)
    {
        perror("semctl: error initializing the e semaphore");
        exit(1);
    }

    // Initializing the s semaphore, "MUTEX semaphore".
    value.val = 1;
    int semvals = semctl(semid, 2, SETVAL, value);
    if (semvals == -1)
    {
        perror("semctl: error initializing the s semaphore");
        exit(1);
    }
}
```

- Finally we have the inifinite loop:
 ```C
while(true) {
	// Check if you can take an item from the buffer
	asem[0].sem_op=-1;
	if (semop (con_sem, asem, 1) == -1) {
   		perror ("semop: con_sem"); exit (1);
	}
	// Get Mutual Execlusion
	asem[0].sem_op=-1;
	if (semop (mutex_sem, asem, 1) == -1) {
   		perror ("semop: mutex_sem"); exit (1);
	}
	// Critical Section Begin
	string tmp = str;
	string tmp2 = tmp.substr(0, tmp.find("\n"));
	tmp.erase(0, tmp.find("\n") + 1);
	strcpy(str, tmp.c_str());
	// Critical Section End
	// Release Mutual Execlusion
	asem[0].sem_op=1;
	if (semop (mutex_sem, asem, 1) == -1) {
   		perror ("semop: mutex_sem"); exit (1);
	}
	// Add empty space on buffer
	asem[0].sem_op=1;
	if (semop (prod_sem, asem, 1) == -1) {
   		perror ("semop: prod_sem"); exit (1);
	}
// Fill and print vector of deques
	fill(v, tmp2);
	printCon(v);
}
```
- This code is for safe exiting and checks if the user wants to delete memory and semaphores:
 ```C
// Handles ctrl C
void handler_function(int sig){
	printf("\nRequest to TERMINATE initiated....\n ");
	printf("Releaseing all Semaphores....\n ");
	end();
}

// Check if user wants to deleter shared memory or semaphores
void end(){
	shmdt(str);
	printf("DELETE SEMAPHORES?? [Y/N] : ");
	char c;
	cin >> c;
	if ( c == 'Y' || c == 'y' ) {
		// remove semaphores
		if (semctl (mutex_sem, 0, IPC_RMID) == -1) {
			perror ("semctl IPC_RMID"); exit (1);
		}
		if (semctl (prod_sem, 0, IPC_RMID) == -1) {
			perror ("semctl IPC_RMID"); exit (1);
		}
		if (semctl (con_sem, 0, IPC_RMID) == -1) {
			perror ("semctl IPC_RMID"); exit (1);
		}
	}
	printf("DELETE SHARED MEMORY?? [Y/N] : ");
	cin >> c;
	if ( c == 'Y' || c == 'y' ) {
		shmctl(shmid,IPC_RMID,0);
	}
	printf("\n TERMINATING...\n");
	exit(0);
}
```
* In the end of each loop  the consumer prints the current prices and average for the last five prices.

### Producer:
- Producer has the same code with small differences:
1. Producer Cannot create semaphores or buffers only calls them
2. Producer has EXITVAL global variables used for same
- Producer while loop:
 ```C
// infinite loop to run the producer unitl ctrl C is hit
while(true) {
	// Check if exit is called
	if ( EXITVAL ) end();
	// get price from distribution
	double price = distribution(generator);
	price = abs(price);
	// Get time in gmt and add 2 hours to be in local time
	clock_gettime( CLOCK_REALTIME ,&ts );
	ts.tv_sec += 7200;
	strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
	fprintf(stderr,"[ %s.%03ld ] %s: generatine new value %.2lf\n",buff, ts.tv_nsec,tmp.c_str(),price);
	// Check if you can take an item from the buffer
	asem[0].sem_op=-1;
	if (semop (prod_sem, asem, 1) == -1) {
   		perror ("semop: con_sem"); exit (1);
	}
	// Try to get mutex time and flag
	clock_gettime( CLOCK_REALTIME ,&ts );
	ts.tv_sec += 7200;
	strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
	fprintf(stderr,"[ %s.%03ld ] %s: Trying to Get MUTEX of share Buffer\n",buff, ts.tv_nsec,tmp.c_str());
	// Get Mutual Execlusion
	asem[0].sem_op=-1;
	if (semop (mutex_sem, asem, 1) == -1) {
   		perror ("semop: mutex_sem"); exit (1);
	}
	// Critical Section Begin
	string s = tmp;
	s = s + ',' + to_string(price);
	s = str + s + '\n';
	strcpy(str, s.c_str());
	// End of Critical Section
	clock_gettime( CLOCK_REALTIME ,&ts );
	ts.tv_sec += 7200;
	strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
	fprintf(stderr,"[ %s.%03ld ] %s: Placing %.2lf on buffer\n",buff, ts.tv_nsec,tmp.c_str(),price);
	// Release Mutual Execlusion
	asem[0].sem_op=1;
	if (semop (mutex_sem, asem, 1) == -1) {
   		perror ("semop: mutex_sem"); exit (1);
	}
	// Add item on buffer
	asem[0].sem_op=1;
	if (semop (con_sem, asem, 1) == -1) {
   		perror ("semop: prod_sem"); exit (1);
	}
	if ( EXITVAL ) end();
	clock_gettime( CLOCK_REALTIME ,&ts );
	ts.tv_sec += 7200;
	strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
	fprintf(stderr,"[ %s.%03ld ] %s: Sleeping for %d ms\n",buff, ts.tv_nsec,tmp.c_str(),(int)slp*1000);
	// Sleep
	sleep(slp);
}
```
## Sample Runs:
> Running Consumer with max buffer of 10 items after running the make file
- Picture one shows consumer and one producer
![pic 1](assets/1.png)
- Picture shows consumer with 11 producers
![pic 2](assets/2.png)
- Picture shows ending consumer and deleting semaphores and shared memory (make veryClean cannot find shared memory as it is alread deleted by consumer)
![pic 3](assets/3.png)
