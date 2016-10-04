#include <pthread.h>
#include <stdio.h>

struct Product
{
    int num;
    int sleepTime;
};

const BUFFER_SIZE = 32;
pthread_mutex_t *mutex;
int cap=0;

void insert_product(struct Product *buffer);
struct Product * remove_product(struct Product *buffer);
int generate_random_num(int a, int b);

void *Produce(struct Product *buffer){
    for(;;){
        printf("producing..\n");
        sleep(generate_random_num(3,7));
        printf("finished producing..\n");
        pthread_mutex_lock(mutex);
        insert_product(buffer);
        pthread_mutex_unlock(mutex);
    }

}

void *Consume(struct Product *buffer){
    for(;;){

        pthread_mutex_lock(mutex);
        struct Product * prod = remove_product(buffer);
        pthread_mutex_unlock(mutex);
        if(prod != 0){
            printf("Consumed product %d..",prod->num);
            printf("Sleeping for %d\n", prod->sleepTime);
            sleep(prod->sleepTime);
            free(prod);
        }

    }
}

int main(int argc, char *argv[])
{
srand(time(NULL));
    if(argc !=3){
        printf("Usage: %s [producers] [consumers]\n", argv[0]);
        exit(-1);
    }
    int num_prod = atoi(argv[1]);
    int num_con = atoi(argv[2]);
    pthread_t prods[num_prod];
    pthread_t cons[num_con];
    mutex=(pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    struct Product * products =(struct Product*) malloc(sizeof(struct Product)*BUFFER_SIZE);
    int rc;
    int i;

    for(i=0; i < BUFFER_SIZE; i++){
        products[i].num= 0;
        products[i].sleepTime=0;
    }
    for(i=0;i< num_prod; i++){
        printf("Creating producer %d\n",i);
        rc = pthread_create(&prods[i], NULL, Produce,products);
        if(rc){
            printf("Error: unable to create thread, %d\n",rc);
            exit(-1);
        }
    }
    for(i=0;i< num_con; i++){
        printf("Creating consumer %d\n",i+num_prod);
        rc = pthread_create(&cons[i], NULL, Consume, products);
        if(rc){
            printf("Error: unable to create thread, %d\n",rc);
            exit(-1);
        }
    }
    pthread_exit(NULL);
}

void insert_product(struct Product *buffer){
    int i;
    for(i=0; i<BUFFER_SIZE;i++){
        if(buffer[i].num == 0){
            buffer[i].num = generate_random_num(1,200);
            buffer[i].sleepTime = generate_random_num(2,9);
            return;
        }
    }
}
struct Product * remove_product(struct Product *buffer){
    int i;
    for(i=0;i<BUFFER_SIZE;i++){
        if(buffer[i].num !=0){
            struct Product * prod = malloc(sizeof(struct Product));
            prod->num=buffer[i].num;
            prod->sleepTime=buffer[i].sleepTime;
            buffer[i].num=0;
            return prod;

        }
    }
    return 0;
}

int generate_random_num(int a,int b){
    return rand() % b + a;
}
