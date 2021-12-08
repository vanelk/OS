#ifndef INITPROC
#define INITPROC
swap_t swapPool [POOLSIZE];
int devicesSem[DEVNUM];
int swapSem;
support_t supports [MAXUPROC];
void test();
#endif