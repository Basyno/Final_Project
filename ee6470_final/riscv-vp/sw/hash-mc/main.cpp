#include "string"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#define NUM 120
using namespace std;

int hval[NUM];

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

FILE *infp;
FILE *outfp;
int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
  *__sem=count;
  return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}
// Random Forest ACC
static char* const hval0_START_ADDR = reinterpret_cast<char* const>(0x30000000);
static char* const hval0_READ_ADDR  = reinterpret_cast<char* const>(0x30000004);
static char* const hval1_START_ADDR = reinterpret_cast<char* const>(0x40000000);
static char* const hval1_READ_ADDR  = reinterpret_cast<char* const>(0x40000004);
// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x50000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x50000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x50000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x5000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x50000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;
//bool _is_using_dma = false;

void read_data(string infile_name){
    infp = NULL;
    infp = fopen(infile_name.c_str(),"r");
    if (infp == NULL) {
      printf("fopen %s error\n", infile_name.c_str());
    }
    for(int j = 0 ; j < NUM ; j++){

        fscanf(infp, "%c\n", &hval[j]);
       /* printf("%c\n",hval[j]);*/
      }
      printf("\n");
    
    fclose(infp);
}


void write_data(string outfile_name){
  
  outfp = NULL;
	outfp = fopen(outfile_name.c_str(), "wb");
	if (outfp == NULL)
	{
		printf("Couldn't open response.txt for writing.");
	}
  for(int i = 0 ; i < NUM/6 ; i++){
   
      fprintf( outfp, "%d\n", hval[i]);
  
    }
	// Close the results file and end the simulation
	fclose( outfp );
}

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
int read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  word data;
  int x;
  unsigned char buff[4];
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buff);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buff, ADDR, sizeof(unsigned char)*len);
  }
  data.uc[0] = buff[0];
  data.uc[1] = buff[1];
  data.uc[2] = buff[2];
  data.uc[3] = buff[3];
  memcpy(data.uc, buff, 4);
  x = data.sint;
  return x;
}

//Total number of cores
//static const int PROCESSORS = 2;
#define PROCESSORS 2
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS];

int main(unsigned hart_id) {
  if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
		sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< PROCESSORS; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}
  int start = NUM / PROCESSORS * hart_id;
  int end = NUM / PROCESSORS * hart_id + NUM / PROCESSORS;

  if(hart_id == 0){
     read_data("hash_input.txt");
     printf("Start processing...\n");
  }
 
  unsigned char buffer[4];
  for(int t = start; t < end ; t++){
 
          buffer[0] = (unsigned char)hval[t];
          sem_wait(&lock);
          if(hart_id == 0){
            write_data_to_ACC(hval0_START_ADDR, buffer, 4);
          }
          else{
            write_data_to_ACC(hval1_START_ADDR, buffer, 4);
          }
          sem_post(&lock);
          
        }
        
      
      //printf("\n");
   for(int t = start; t < end/6 ; t++){   
      if(hart_id == 0){
        sem_wait(&lock);
        hval[t] = read_data_from_ACC(hval0_READ_ADDR, buffer, 4);
        sem_post(&lock);
      }
      else{
        sem_wait(&lock);
        hval[t] = read_data_from_ACC(hval1_READ_ADDR, buffer, 4);
        sem_post(&lock);
      }
      //printf("Result[%d][%d] = %d \n",r,t,Result[r][t]);
    }
      //printf("\n");
  barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);
  
  
  if(hart_id == 0){
    write_data("hval_output.txt");
  }
}
