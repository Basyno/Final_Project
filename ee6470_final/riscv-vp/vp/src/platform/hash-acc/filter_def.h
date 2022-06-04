#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_
#define HASH_NUM 120
#define CLOCK_PERIOD 5



const int DMA_TRANS = 64;

// Sobel Filter inner transport addresses
// Used between blocking_transport() & do_filter()
const int HASH_R_ADDR = 0x00000000;
const int HASH_RESULT_ADDR = 0x00000004;

const int HASH_RS_R_ADDR   = 0x00000000;
const int HASH_RS_W_WIDTH  = 0x00000004;
const int HASH_RS_W_HEIGHT = 0x00000008;
const int HASH_RS_W_DATA   = 0x0000000C;
const int HASH_RS_RESULT_ADDR = 0x00800000;


union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

#endif
