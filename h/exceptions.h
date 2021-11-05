#ifndef EXCEPTIONS
#define  EXCEPTIONS
extern void SYSCALLHandler();
extern void stateCopy(state_PTR oldState, state_PTR newState);
extern void otherExceptions(int reason);
#endif
