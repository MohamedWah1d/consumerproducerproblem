#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include "ctrl.h"
#include <string.h>
#include <cmath>
#include <map>
#include <numeric> // to use accumlate
#define all(v) ((v).begin()), ((v).end())
#define sz(v) ((int)((v).size()))

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

// To remove a product from the buffer.
struct item take(int *out, struct item *buffer, int bs)
{
    struct item product;
    // dummy variable to empty the consumed spot
    struct item dummy;
    strcpy(dummy.comodity_name, "EMPTY");
    dummy.today_price = 0;
    /// consume
    product = buffer[*out];
    /// empty
    buffer[*out] = dummy;
    /// increment shared syncro index between producers
    *out = (*out + 1) % bs;
    return product;
}

std::map<std::string, std::vector<double>> map_init()
{
    // initilizaing a map full of all avaliable comodities
    // GOLD, SILVER, CRUDEOIL, NATURALGAS, ALUMINIUM,COPPER, NICKEL, LEAD, ZINC, MENTHAOIL, and COTTON
    std::map<std::string, std::vector<double>> m;
    std::vector<double> dummy(1, 0.0); // empty vector to be appended on later
    m["GOLD"] = dummy;
    m["SILVER"] = dummy;
    m["CRUDEOIL"] = dummy;
    m["NATURALGAS"] = dummy;
    m["ALUMINIUM"] = dummy;
    m["COPPER"] = dummy;
    m["NICKEL"] = dummy;
    m["LEAD"] = dummy;
    m["ZINC"] = dummy;
    m["MENTHAOIL"] = dummy;
    m["COTTON"] = dummy;
    return m;
}

std::map<std::string, std::vector<double>> map1_init()
{
    // initilizaing a map full of all avaliable comodities
    // GOLD, SILVER, CRUDEOIL, NATURALGAS, ALUMINIUM,COPPER, NICKEL, LEAD, ZINC, MENTHAOIL, and COTTON
    std::map<std::string, std::vector<double>> m;
    std::vector<double> dummy(4, 0.0); // empty vector to be appended on later (dummy[0] is the prev price  dummy[1] is the prev avg dummy [2] =1 means price increased dummy [3]=1 means avg increased)
    m["GOLD"] = dummy;
    m["SILVER"] = dummy;
    m["CRUDEOIL"] = dummy;
    m["NATURALGAS"] = dummy;
    m["ALUMINIUM"] = dummy;
    m["COPPER"] = dummy;
    m["NICKEL"] = dummy;
    m["LEAD"] = dummy;
    m["ZINC"] = dummy;
    m["MENTHAOIL"] = dummy;
    m["COTTON"] = dummy;
    return m;
}

//fit the spaces at the output.
std::string spaces_naive(int n)
{
    std::string empty = "";
    for (int i = 0; i < n; i++)
    {
        empty = empty + " ";
    }
    return empty;
}

//printing the products.
std::map<std::string, std::vector<double>> customer_reciept(std::map<std::string, std::vector<double>> m, std::map<std::string, std::vector<double>> m1)
{
     system("clear");
    // calculating the avg price of each comoditiy and storing them in m1 to compare
    for (auto i : m)
    {
        std::string ptemp = i.first;
        std::vector<double> ptempt = i.second;
        double avg = accumulate(all(ptempt), 0.0) / sz(ptempt);
        if (m1[ptemp][1] < avg)
        {
            m1[ptemp][3] = 1.0; // avg price increased
            m1[ptemp][1] = avg;
        }
        else if (m1[ptemp][1] > avg)
        {
            m1[ptemp][3] = -1.0; // avg price decreased
            m1[ptemp][1] = avg;
        }

        if (m1[ptemp][0] < ptempt.back()) // if the last price is bigger than the prev that means there was increase in prices
        {
            m1[ptemp][2] = 1.0; // avg price icreased

            m1[ptemp][0] = ptempt.back();
        }
        else if (m1[ptemp][0] > ptempt.back())
        {
            m1[ptemp][2] = -1.0; // avg price decreased
            
            m1[ptemp][0] = ptempt.back();
        }
    }
    /*
    +-------------------------------------+
    | Currency | Price | AvgPrice |
    +-------------------------------------+
    */
    std::cout << "+-------------------------------------+"
              << "\n";
    std::cout << "| Currency      |  Price   | AvgPrice |"
              << "\n";
    std::cout << "+-------------------------------------+"
              << "\n";

    for (auto i : m1)
    {
        std::string ptempmap = i.first;
        std::vector<double> ptempmapt = i.second;

        int offset;
        std::string comoditiy_name;
        comoditiy_name=ptempmap;
        // name of comodity format length is 10 max
        if(ptempmap.length() < 10)
        {
            offset=10-ptempmap.length();
            comoditiy_name=ptempmap+spaces_naive(offset);
        }



        double price=ptempmapt[0];
        double avg_price=ptempmapt[1];
        std::string price_arrow;
        std::string avg_price_arrow;
        if (ptempmapt[2] == 1.0)
        {
            price_arrow = "↑";
        }
        else if (ptempmapt[2] == -1.0)
        {
            price_arrow = "↓";
        }
        else
        {
            price_arrow = " ";
        }

        if (ptempmapt[3] == 1.0)
        {
            avg_price_arrow = "↑";
        }
        else if (ptempmapt[3] == -1.0)
        {
            avg_price_arrow = "↓";
        }
        else
        {
            avg_price_arrow = " ";
        }

        std::cout<<"| "+comoditiy_name+"    | ";
        if(price_arrow.compare(" ")==0)
        {
            printf("\033[;26m%7.2lf \033[0m",price);
        }
        else if (price_arrow.compare("↑")==0)
        {
            printf("\033[;32m%7.2lf↑\033[0m",price);
        }
        else
        {
            printf("\033[;31m%7.2lf↓\033[0m",price);
        }

        std::cout<<" | ";


            if(avg_price_arrow.compare(" ")==0)
        {
            printf("\033[;26m%7.2lf \033[0m",avg_price);
        }
        else if (avg_price_arrow.compare("↑")==0)
        {
            printf("\033[;32m%7.2lf↑\033[0m",avg_price);
        }
        else
        {
            printf("\033[;31m%7.2lf↓\033[0m",avg_price);
        }
        std::cout<<" |\n";
    }
    std::cout << "+-------------------------------------+"
              << "\n";
    return m1;
}
// the consumer function
void consumer(int *out, struct item *buffer, int semid, int bs, std::map<std::string, std::vector<double>> m, std::map<std::string, std::vector<double>> m1)
{
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
        sem_signal(semid, 1);
    }
}

using namespace std;

int shmid1;
std::map<std::string, std::vector<double>> m = map_init();   // hashmap to store the values of the commodities
std::map<std::string, std::vector<double>> m1 = map1_init(); // hashmap to keep track of the prev price and the prev avg

int main(int argc, char **argv)
{

    // init of buffer
    int bs;
    // taking the size of buffer

    if (argc == 2)
    {
        bs = stoi(argv[1]);
    }
    else
    {
        cout << "Number of parameters is less than excpected...try again";
        return 0;
    }

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
    *out = 0;
    // initilizing hashmaps to keep track of price

    //
    // to syncronize between producers we share a var specifing the empty location 4th semaphore

    for (int i = 0; i < bs; i++)
    {
        struct item dummy;
        buffer[i] = dummy;
    }
    
    //calling the consumer function.
    consumer(out, buffer, semid, bs,m,m1);

    return 0;
}
