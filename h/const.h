#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define RAMBASESIZE		0x10000004
#define TODLOADDR		  0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024


/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'

#define NULL 			    ((void *)0xFFFFFFFF)

/* device interrupts */
#define DISKINT			  3
#define FLASHINT 		  4
#define NETWINT 		  5
#define PRNTINT 		  6
#define TERMINT			  7

#define DEVNUM            49
#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */	
#define DEVREGSIZE	  16 		/* device register size in bytes */

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			    2
#define DATA1			    3

/* device register field number for terminal devices */
#define RECVSTATUS  	0
#define RECVCOMMAND 	1
#define TRANSTATUS  	2
#define TRANCOMMAND 	3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			    1
#define BUSY			    3

/* device common COMMAND codes */
#define RESET			    0
#define ACK				    1

/* Memory related constants */
#define KSEG0           0x00000000
#define KSEG1           0x20000000
#define KSEG2           0x40000000
#define KUSEG           0x80000000
#define RAMSTART        0x20000000
#define BIOSDATAPAGE    0x0FFFF000
#define	PASSUPVECTOR	  0x0FFFF900

/* Exceptions related constants */
#define	PGFAULTEXCEPT	  0
#define GENERALEXCEPT	  1


/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define MAXPROC 20
#define IOCLOCK 100000
#define QUANTUM 5000
#define INTERVAL 

/* syscalls */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define PASSEREN 3
#define VERHOGEN 4
#define WAITIO 5
#define GETCPUTIME 6
#define WAITCLOCK 7
#define GETSUPPORTPTR 8

/* important places */
#define NUKE 0x20001000
#define STATUSREG 0x10400000

/* bit operations */
#define ALLOFF 0x00000000
#define IEON 0x00000004
#define IECON	0x00000001
#define IMON 0x0000FF00
#define TEBITON 0x08000000
#define UMOFF 0x00000002
#define KUON 0x00000008

/* masks */
#define EXCODEMASK 0x0000007c
#define IPMASK 0x00FF00
#define LINE0INTON 1
#define LINE1INTON 2
#define LINE2INTON 4
#define LINE3INTON 8
#define LINE4INTON 16
#define LINE5INTON 32
#define LINE6INTON 64
#define LINE7INTON 128
#define TRANSBITS 15

/*extra useful numbers */
#define PCINC 4
#define ZERO 0
#define ONE 1
#define ON 1
#define OFF 0


/* phase 3 global variables */
#define MAXUPROC 8
#define POOLSIZE 32
#define TERMINATE 9
#define GetTOD 10 
#define WRITETOPRINTER 11 
#define WRITETOTERMINAL 12 
#define READFROMTERMINAL 13
#define	TERM0ADDR 0x10000254
#define TERMSTATMASK 0xff
#define RECVD 5
#define DIRTYON 0x00000400
#define VPNSHIFT 12
#define ASIDSHIFT 5
#define GETPAGENO 0x00007000

#define PRINTCHR	2
#define BYTELEN	8
#endif
