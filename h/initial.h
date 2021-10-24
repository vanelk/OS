#ifndef INITIAL
#define INITIAL
extern int processCount;
extern int softBlockCount;
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int semDevices[DEVNUM];
int *clockSem;
cpu_t startTOD;

#endif 
