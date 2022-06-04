#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#define NUM 120

int hash[NUM];

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// hash ACC
static char* const HASH_START_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const HASH_READ_ADDR  = reinterpret_cast<char* const>(0x73000004);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;

void read_data(std::string infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "r");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name.c_str());

  }
  for(int i=0;i<NUM;i++){
        fscanf(fp_s, "%c\n",&hash[i]);
          
    }
    printf("read data finish\n");
    fclose(fp_s); 
}

void write_data(std::string outfile_name) {
  FILE *fp_t = NULL; // target file handler

  fp_t = fopen(outfile_name.c_str(), "w");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
  }
  for(int i=0;i<NUM/6;i++){
        fprintf(fp_t, "%d\n",hash[i]);
        printf("result:%d\n",i); 
        printf("%d\n",hash[i]); 
    }
    

  fclose(fp_t);
  
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

void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

int main(int argc, char *argv[]) {

  read_data("hash_input.txt");
  printf("======================================\n");

  unsigned char  buffer[4] = {0};
  word data;

  for(int i=0;i<NUM;i++){

          buffer[0] =(unsigned char)hash[i];
          buffer[1] = 0;
          buffer[2] = 0;
          buffer[3] = 0;
          write_data_to_ACC(HASH_START_ADDR, buffer, 4);
        
  }
  
  for(int i=0;i<NUM/6;i++){
  read_data_from_ACC(HASH_READ_ADDR, buffer, 4);
      memcpy(data.uc, buffer, 4);
      hash[i] = (data).sint;
  }
    
  write_data("hash_out.txt");
  
}


