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
#include "ctrl.h"
#include <string.h> //strcpy
#include <random>

struct item *buffer;

// the semaphore id to be initialized
int semid;

// function to get the sem id.
int get_semid()
{
    // ftok to generate unique key
    key_t key = ftok("semaphore", 1);

    // creating the semaphore
    // 0666 for the access permissions
    int semid = semget(key, 3, 0666); // in producer we only ask for premssion not creating

    return semid;
}

// To add a product to the buffer.
void append(struct item product, int *in, struct item *buffer, int buf_size)
{
    // increasing the index of the buffer.
    //  std::cout<<*in<<"\n";

    // adding the new product to the buffer.
    buffer[*in] = product;
    *in = (*in + 1) % buf_size;
    //printf("Producer produces name %s , price = %lf stored in posn %d  \n", product.comodity_name, product.today_price, *in);
}

//initializing the producer index buffer.
int* get_syncro()
{
    // ftok to generate unique key
    key_t key = ftok("semaphore", 2);

    //opening the shared location containing the syncro
    //0666 for the access permissions
    int shmid5=  shmget(key,sizeof(int),IPC_CREAT | 0666);
    int *in =(int *)shmat(shmid5, (void*)0, 0);
    return in;
}

//getting the time of teh producers operations
void get_time_stamp(struct timespec current,std::string task,char * comodity_name)
{

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    clock_gettime(CLOCK_REALTIME, &current);
    char buff[100];
    strftime(buff, sizeof(buff), "%Y/%m/%d %H:%M:%S.", &tm_now);
    std::cout << "\033[;31m[" << buff;
    std::cout << current.tv_nsec;
    std::cout << "]"<<" "<<comodity_name<<":"<<" "<<task<<"\033[0m\n";
}

// The producer function
void producer(int *in, struct item *b, int semid, char *comodity_name, double comodity_price_mean, double comodity_price_stddiv, int sleepinterval, int bs)
{
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
}

using namespace std;

//will be used for initializing the index of the producr buffer.
int shmid1;

int main(int argc, char **argv)
{

    // init of parametsers
    char comodity_name[11];
    double comodity_price_mean;
    double comodity_price_stddiv;
    int sleepinterval;
    int bs;

    // taking the input parameters
    if (argc == 6)
    {
        strcpy(comodity_name, argv[1]);
        comodity_price_mean = stod(argv[2]);
        comodity_price_stddiv = stod(argv[3]);
        sleepinterval = stoi(argv[4]);
        bs = stoi(argv[5]);
    }
    else
    {
        cout << "Number of parameters is less than excpected...try again";
        return 0;
    }

    // initializing the shared memory buffer.
    int shmid = shm_id(bs);
    buffer = shm_buffer(shmid);

    // getting the semid of the semaphores
    int semid = get_semid();

    //initializing the index of the product buffer.
    int *in;
    in = get_syncro();
    
    //calling the producer function.
    producer(in, buffer, semid, comodity_name, comodity_price_mean, comodity_price_stddiv, sleepinterval, bs);

    return 0;
}
