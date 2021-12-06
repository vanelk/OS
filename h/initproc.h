#ifndef INITPROC
#define INITPROC
swap_t swapPool [POOLSIZE];
int devicesSem[DEVNUM];
int swapSem;
support_t supports [MAXUPROC+1];
void test();
#endif