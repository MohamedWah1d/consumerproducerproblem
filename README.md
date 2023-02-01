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
struct item product;
    while (true)
    {
        // To decrease the value of the buffer size by 1, whenever the consumer consumes the buffer
        sem_wait(semid, 0);

        // To ensure no two processes enter the critical section at the same time
        sem_wait(semid, 2);

        // To get the data form the buffer
        product = take(out, buffer, bs);

        // appending to the vector in the hash table to keep track of the prev prices and the newest price
        std::vector<double> temp;
        temp = m[product.comodity_name];
        if (temp[0] == 0.0)
        {
            temp.erase(temp.begin()); // deleting dummy element before insertion (we don't count price 0.0 as a price)
        }

        if (sz(temp) < 5)
        {
            temp.push_back(product.today_price);
            m[product.comodity_name] = temp;
        }
        else
        {
            temp.erase(temp.begin());
            temp.push_back(product.today_price);
            m[product.comodity_name] = temp;
        }

        m1=customer_reciept(m, m1);
        // add 1 to the s semaphore after getting out from the critical section
        sem_signal(semid, 2);

        // decrease e by 1 when a value is taken from the buffer
        sem_signal(semid, 1);
    }
```
* In the end of each loop  the consumer prints the current prices and average for the last five prices.

### Producer:
- Producer has the same code with small differences:
1. Producer Cannot create semaphores or buffers only calls them
2. Producer has EXITVAL global variables used for same
- Producer while loop:
 ```C
struct item t;
    double randprice;
    struct timespec current;
    while (true)
    {
        std::random_device mch;
        std::default_random_engine generator(mch());
        std::normal_distribution<double> distribution(comodity_price_mean, comodity_price_stddiv);
        double random_current_value = distribution(generator);

        // t.today_price = randprice;
        t.today_price = random_current_value;
        get_time_stamp(current,"generating a new value  "+std::to_string(t.today_price),comodity_name);

        // will reduce the value of semaphore e by 1.
        sem_wait(semid, 1);
        get_time_stamp(current,"trying to get mutex on shared buffer",comodity_name);
        // To make sure that no two processes can enter the critical section at the same time.
        sem_wait(semid, 2);
        get_time_stamp(current,"placing "+std::to_string(t.today_price)+" on shared buffer",comodity_name);
        // put the new product into the buffer.
        //randprice = rand() % 1000;
        strcpy(t.comodity_name, comodity_name);
        append(t, in, b, bs);


        // allow other semaphores to enter the critical section, when it's empty of processes
        sem_signal(semid, 2);

        // To add one to the semaphore variable n, whenever a value is added to the buffer.
        sem_signal(semid, 0);
        get_time_stamp(current,"sleeping for "+std::to_string(sleepinterval)+"ms",comodity_name);

        sleep(sleepinterval/1000);
    }
```
