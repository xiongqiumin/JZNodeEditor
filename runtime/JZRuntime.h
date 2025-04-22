#ifndef JZ_RUNTIME_H_
#define JZ_RUNTIME_H_


#define JMP(addr)   do{ goto Line##addr; }while(0);
#define JE(addr)    do{ if(Reg_Cmp){ goto Line##addr; } }while(0);
#define JNE(addr)   do{ if(!Reg_Cmp){ goto Line##addr; } }while(0);





#endif