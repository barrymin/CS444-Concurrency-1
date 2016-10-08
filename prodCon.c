#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

struct Product
{
    int num;
    int sleepTime;
};

const BUFFER_SIZE = 5;
pthread_mutex_t *mutex;
sem_t * emptysem;
sem_t * fullsem;

void insert_product(struct Product *buffer);
struct Product * remove_product(struct Product *buffer);
int generate_random_num(int a, int b);
int rdrandsupport();
unsigned int rdrand();
void print_buffer(struct Product * buffer);
void init_genrand(unsigned long s);
unsigned long genrand_int32(void);

void *Produce(void *buffer){
    for(;;){
        sem_wait(fullsem);
	int rand =generate_random_num(5,3);
        printf("producing for %d seconds..\n",rand);
        sleep(rand);
        pthread_mutex_lock(mutex);
        insert_product((struct Product *)buffer);
	printf("After Production:\n");
        print_buffer((struct Product *)buffer);
	pthread_mutex_unlock(mutex);
        sem_post(emptysem);
	printf("Finished Production loop\n");
    }
}

void *Consume(void *buffer){
    for(;;){
        sem_wait(emptysem);
        pthread_mutex_lock(mutex);
        struct Product * prod = remove_product((struct Product *)buffer);
	printf("After Consumption:\n");
	print_buffer((struct Product *)buffer);
        pthread_mutex_unlock(mutex);
        if(prod != 0){
            printf("Consumed product %d..",prod->num);
            printf("Sleeping for %d\n", prod->sleepTime);
            sleep(prod->sleepTime);
            free(prod);
	    sem_post(fullsem);
    	}
    printf("finished cosumption loop\n");
    }
}

int main(int argc, char *argv[])
{   
    if(argc !=3){
        printf("Usage: %s [producers] [consumers]\n", argv[0]);
        exit(-1);
    }
    init_genrand(time(NULL));
    int num_prod = atoi(argv[1]);
    int num_con = atoi(argv[2]);
    pthread_t prods[num_prod];
    pthread_t cons[num_con];
    mutex=(pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    emptysem = malloc(sizeof(sem_t));
    sem_init(emptysem,0,-1);
    fullsem = malloc(sizeof(sem_t));
    sem_init(fullsem,0,BUFFER_SIZE+1);
int semval;
sem_getvalue(fullsem,&semval); 
printf("sem value = %d\n",semval);
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
            buffer[i].num = generate_random_num(200,1);
            buffer[i].sleepTime = generate_random_num(8,2);
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
 	if(rdrandsupport()){
		unsigned int r = rdrand(a,b);
		if(r!=-1){
			return (r%a)+b; 		
		}	
	}
	return (genrand_int32()%a)+b;
	   
	
}

int rdrandsupport(){
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	
	if(ecx & 0x40000000){				
		return 1;
	}
	else{
		return 0;
	}
}

unsigned int rdrand(){
	unsigned char sucs;
	unsigned int rand;
	__asm__ __volatile__("rdrand %0; setc %1"
			: "=r" (rand), "=qm" (sucs));
	if(sucs)
		return rand;
	else
		return -1;
	//printf("%d random num = %d\n",ok,(rand%8)+2);

}

void print_buffer(struct Product * buffer){
	int i;	
	for(i=0; i < BUFFER_SIZE; i++){
		printf("| ");
		if(buffer[i].num != 0)
			printf("%d ",buffer[i].num);
	}
	printf("|\n");

}

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* a default initial seed is used */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}
  
	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] = 
			(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}
