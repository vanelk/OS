HIDDEN * semd_h;
int insertBlocked( int *semAdd, pcb_PTR p){
 semd_ t * currentS = semd_h;
 do{
  int found = (currentS->s_semAdd == semAdd);
  int notInList = (currentS->s_semAdd > semAdd);
  if(found){
   insertProcQ(&(currentS->s_procQ), p);
  }
  currentS = currentS->s_next;
 }while(!found || notInList);
 // do something
}
