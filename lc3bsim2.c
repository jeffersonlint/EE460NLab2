/*
    Name 1: Jefferson Lint
    Name 2: Michael O'Leary
    UTEID 1: jdl3858
    UTEID 2: mso547
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
  int k;

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *
   *    Process one instruction at a time
   *       -Fetch one instruction
   *       -Decode
   *       -Execute
   *       -Update NEXT_LATCHES
   */
   int PC = CURRENT_LATCHES.PC/2;
   int byte2 = MEMORY[PC][0]; //lower byte
   int byte1 = MEMORY[PC][1]; //upper byte
   //byte2byte1 = instruction;
   NEXT_LATCHES.PC = CURRENT_LATCHES.PC+2; //increment PC
   int opcode = byte1>>4; //opcode in decimal

   if(opcode==0)  //BR DONE
   {
     int n = (byte1>>3)&1;
     int z = (byte1>>2)&1;
     int p = (byte1>>1)&1;
     if(n&CURRENT_LATCHES.N==1 || z&CURRENT_LATCHES.Z==1 || p&CURRENT_LATCHES.P==1) //branch taken
     {
       int offset = byte2;
       if(byte1&1==1)
       {
         offset = offset | 0xFFFFFF00;
       }
       NEXT_LATCHES.PC=NEXT_LATCHES.PC+(offset<<1);
     }
   }
   else if(opcode==1) //ADD DONE
   {
     int dr = (byte1>>1)&7;
     int sr1 = (byte2>>6)&3;
     if(byte1&1==1) sr1=sr1+4;
     int arg1 = CURRENT_LATCHES.REGS[sr1];
     if((arg1>>15)&1==1)
     {
       arg1 = arg1 | 0xFFFF0000;
     }
     //immediate mode
     if((byte2>>5)&1==1)
     {
        int imm5=byte2&31;
        if ((imm5>>4)&1==1)
        {
          imm5 = imm5 | 0xFFFFFFE0;
        }

        int sum = (arg1+imm5);
        NEXT_LATCHES.REGS[dr]=Low16bits(sum);
        if(sum>0)
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=1;
        }
        else if(sum<0)
        {
          NEXT_LATCHES.N=1;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=0;
        }
        else
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=1;
          NEXT_LATCHES.P=0;
        }
     }
     //register mode
     else
     {
       int sr2 = byte2&7;
       int arg2 = CURRENT_LATCHES.REGS[sr2];
       if((arg2>>15)&1==1)
       {
         arg2 = arg2 | 0xFFFF0000;
       }
       int sum = arg1+arg2;
       NEXT_LATCHES.REGS[dr]=Low16bits(sum);
       if(sum>0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else if(sum<0)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
     }
   }
   else if(opcode==2) //LDB DONE
   {
      int dr = (byte1>>1)&7;
      int baseR = (byte2>>6)&3;
      if(byte1&1==1) baseR=baseR+4;
      int offset6 = byte2&63;
      if((offset6>>5)&1==1){  //To ensure correct sign extension
        offset6 = offset6 | 0xFFC0;
      }

      int memAccess = MEMORY[(CURRENT_LATCHES.REGS[baseR]/2) + offset6][0];
      printf("%i, %i\n", (CURRENT_LATCHES.REGS[baseR]/2) + offset6, MEMORY[(CURRENT_LATCHES.REGS[baseR]/2) + offset6][0]);
      if((memAccess>>7)&1==1)
      {
        memAccess=memAccess|0xFFFFFF00;
      }
      NEXT_LATCHES.REGS[dr]=Low16bits(memAccess);
      if(memAccess>0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else if(memAccess<0)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
   }
   else if(opcode==3) //STB
   {
      int sr = (byte1>>1)&7;
      int baseR = (byte2>>6)&3;
      if(byte1&1==1) baseR=baseR+4;
      int offset6 = byte2&63;
      if((offset6>>5)&1==1){  //To ensure correct sign extension
        offset6 = offset6 | 0xFFFFFFC0;
      }
      MEMORY[(CURRENT_LATCHES.REGS[baseR]/2) + offset6][0] = CURRENT_LATCHES.REGS[sr]&0x000000FF;
   }
   else if(opcode==4) //JSR(R) ////Michael says this is scary
   {
    int temp = NEXT_LATCHES.PC;
    if(byte1>>3 == 0){  //JSRR
      int baseR = (byte2>>6)&3;
      if(byte1&1 == 1) baseR = baseR + 4;
      NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[baseR];
    }
    else{  //JSR
      int pcoffset = byte2;
      if(byte1&1 == 1) pcoffset = pcoffset + 256;
      if(byte1>>1&1 == 1) pcoffset = pcoffset + 512;
      if(byte1>>2&1 == 1) pcoffset = pcoffset + 1024;
      int x = CURRENT_LATCHES.PC + (pcoffset<<1);
      NEXT_LATCHES.PC = Low16bits(x);
      NEXT_LATCHES.REGS[7] = temp;
    }
   }
   else if(opcode==5) //AND DONE
   {
     int dr = (byte1>>1)&7;
     int sr1 = (byte2>>6)&3;
     if(byte1&1==1) sr1=sr1+4;
     int arg1 = CURRENT_LATCHES.REGS[sr1];
     if((arg1>>15)&1==1)
     {
       arg1 = arg1 | 0xFFFF0000;
     }
     //immediate mode
     if((byte2>>5)&1==1)
     {
        int imm5=byte2&31;
        if ((imm5>>4)&1==1)
        {
          imm5 = imm5 | 0xFFFFFFE0;
        }

        int sum = (arg1&imm5);
        NEXT_LATCHES.REGS[dr]=Low16bits(sum);
        if(sum>0)
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=1;
        }
        else if(sum<0)
        {
          NEXT_LATCHES.N=1;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=0;
        }
        else
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=1;
          NEXT_LATCHES.P=0;
        }
     }
     //register mode
     else
     {
       int sr2 = byte2&7;
       int arg2 = CURRENT_LATCHES.REGS[sr2];
       if((arg2>>15)&1==1)
       {
         arg2 = arg2 | 0xFFFF0000;
       }
       int sum = arg1&arg2;
       NEXT_LATCHES.REGS[dr]=Low16bits(sum);
       if(sum>0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else if(sum<0)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
     }
   }
   else if(opcode==6)  //LDW DONE
   {
    int dr = (byte1>>1)&7;
    int baseR = (byte2>>6)&3;
    if(byte1&1==1) baseR = baseR + 4;
    int offset6 = byte2&63;
    if((offset6>>5)&1==1){  //To ensure correct sign extension
      offset6 = offset6 | 0xFFFFFFC0;
    }
    int memAccess = MEMORY[(CURRENT_LATCHES.REGS[baseR]/4)+offset6<<1][1];
    memAccess = memAccess<<8;
    memAccess = memAccess+MEMORY[(CURRENT_LATCHES.REGS[baseR]/4)+offset6<<1][0];
    NEXT_LATCHES.REGS[dr] = Low16bits(memAccess);

      if((memAccess>>15)&1==1)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else if(memAccess!=0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
   }
   else if(opcode==7) //STW
   {
    int sr = (byte1>>1)&7;
    int baseR = (byte2>>6)&3;
    if(byte1&1==1) baseR = baseR + 4;
    int offset6 = byte2&63;
    if((offset6>>5)&1==1){  //To ensure correct sign extension
      offset6 = offset6 | 0xFFC0;
    }
    int shiftoffset6 = offset6<<1;
    MEMORY[(CURRENT_LATCHES.REGS[baseR] + shiftoffset6)/2][1] = CURRENT_LATCHES.REGS[sr];
   }
   else if(opcode==9) //XOR DONE
   {
     int dr = (byte1>>1)&7;
     int sr1 = (byte2>>6)&3;
     if(byte1&1==1) sr1=sr1+4;
     int arg1 = CURRENT_LATCHES.REGS[sr1];
     if((arg1>>15)&1==1)
     {
       arg1 = arg1 | 0xFFFF0000;
     }
     //immediate mode
     if((byte2>>5)&1==1)
     {
        int imm5=byte2&31;
        if ((imm5>>4)&1==1)
        {
          imm5 = imm5 | 0xFFFFFFE0;
        }

        int sum = (arg1^imm5);
        NEXT_LATCHES.REGS[dr]=Low16bits(sum);
        if(sum>0)
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=1;
        }
        else if(sum<0)
        {
          NEXT_LATCHES.N=1;
          NEXT_LATCHES.Z=0;
          NEXT_LATCHES.P=0;
        }
        else
        {
          NEXT_LATCHES.N=0;
          NEXT_LATCHES.Z=1;
          NEXT_LATCHES.P=0;
        }
     }
     //register mode
     else
     {
       int sr2 = byte2&7;
       int arg2 = CURRENT_LATCHES.REGS[sr2];
       if((arg2>>15)&1==1)
       {
         arg2 = arg2 | 0xFFFF0000;
       }
       int sum = arg1^arg2;
       NEXT_LATCHES.REGS[dr]=Low16bits(sum);
       if(sum>0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else if(sum<0)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
     }
   }
   else if(opcode==12)  //JMP
   {
    int baseR = (byte2>>6)&3;
    if(byte1&1 == 1) baseR = baseR + 4;
    NEXT_LATCHES.PC = baseR;
   }
   else if(opcode==13)  //SHF
   {
    int amount = (byte2&15);
    int dr = (byte1>>1)&7;
    int sr = (byte2>>6)&3;
    int nzp = 0;
    if(byte1&1 == 1) sr = sr + 4;
    if(byte2>>4 == 0)  //LSHF
    {
      NEXT_LATCHES.REGS[dr]=Low16bits(sr<<amount);
      nzp = sr<<amount;
    }
    else if(byte2>>5 == 0)  //RSHFL
    {
      NEXT_LATCHES.REGS[dr]=Low16bits(sr>>amount);
      nzp = sr>>amount;
    }
    else  //RSHFA
    {
      if((byte1>>7)&1 == 1){
        nzp = sr;    
        for (int i = 0; i < amount; ++i)
        {
          sr>>1;
          nzp>>1;
          sr = sr | 32768;  //Set most signifcant bit to 1
          nzp = nzp | 32768;
        }
        NEXT_LATCHES.REGS[dr]=Low16bits(sr); 
      }
      else{
        nzp = sr>>amount;
        NEXT_LATCHES.REGS[dr]=Low16bits(sr>>amount); 
      }
    }
           if(nzp>0)
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=1;
       }
       else if(nzp<0)
       {
         NEXT_LATCHES.N=1;
         NEXT_LATCHES.Z=0;
         NEXT_LATCHES.P=0;
       }
       else
       {
         NEXT_LATCHES.N=0;
         NEXT_LATCHES.Z=1;
         NEXT_LATCHES.P=0;
       }
   }
   else if(opcode==14)  //LEA DONE
   {
    int dr = (byte1>>1)&3;
    int pcoffset = byte2;
    if(byte1&1==1) pcoffset = pcoffset | 0xFFFFFF00;
    NEXT_LATCHES.REGS[dr]=Low16bits(NEXT_LATCHES.PC + (pcoffset<<1));
   }
   else if(opcode==15)  //TRAP DONE
   {
     NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
     int memAccess = MEMORY[byte2/2][1];
     memAccess = memAccess<<8;
     memAccess = memAccess+MEMORY[byte2/2][0];
     NEXT_LATCHES.PC = Low16bits(memAccess);
   }
   else
   {
     exit(1);
   }




}
