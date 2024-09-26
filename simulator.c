/*
 * EECS 370, University of Michigan, Fall 2023
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec: https://eecs370.github.io/project_3_spec/
 * Make sure NOT to modify printState or any of the associated functions
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8 // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

const char* opcode_to_str_map[] = {
    "add",
    "nor",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop"
};

#define NOOPINSTR (NOOP << 22)
#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
	int pcPlus1;
	int instr;
} IFIDType;

typedef struct IDEXStruct {
	int pcPlus1;
	int valA;
	int valB;
	int offset;
	int instr;
} IDEXType;

typedef struct EXMEMStruct {
	int branchTarget;
    int eq;
	int aluResult;
	int valB;
	int instr;
} EXMEMType;

typedef struct MEMWBStruct {
	int writeData;
    int instr;
} MEMWBType;

typedef struct WBENDStruct {
	int writeData;
	int instr;
} WBENDType;

typedef struct stateStruct {
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	unsigned int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	unsigned int cycles; // number of cycles run so far
} stateType;

static inline int opcode(int instruction) {
    return instruction>>22;
}

static inline int field0(int instruction) {
    return (instruction>>19) & 0x7;
}

static inline int field1(int instruction) {
    return (instruction>>16) & 0x7;
}

static inline int field2(int instruction) {
    return instruction & 0xFFFF;
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

void printState(stateType*);
void printInstruction(int);
void readMachineCode(stateType*, char*);


int main(int argc, char *argv[]) {

    /* Declare state and newState.
       Note these have static lifetime so that instrMem and
       dataMem are not allocated on the stack. */

    static stateType state, newState;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    readMachineCode(&state, argv[1]);

    // Initialize state here

    for (int i = 0; i < NUMREGS; ++i){
        state.reg[i] = 0;
    }
    for (int i = 0; i < state.numMemory; ++i){
        state.dataMem[i] = state.instrMem[i];
    }

    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;
    state.cycles = 0;
    state.pc = 0;

    newState = state;

    // int valinRegA = 0;
    // int valinRegB = 0;
    while (opcode(state.MEMWB.instr) != HALT) {
        printState(&state);
        
        newState.cycles += 1;
        /* ---------------------- IF stage --------------------- */
        //fetch
        newState.IFID.instr = state.instrMem[state.pc];
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc++;

        /* ---------------------- ID stage --------------------- */
        //decode
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;

        if((opcode(state.IDEX.instr) == 2) && (field1(state.IDEX.instr) == field0(newState.IDEX.instr) || field1(state.IDEX.instr) == field1(newState.IDEX.instr))) {
            if((opcode(newState.IDEX.instr) == 2) && (field1(state.IDEX.instr) != field0(newState.IDEX.instr) && field1(state.IDEX.instr) == field1(newState.IDEX.instr))) {
                //new state is lw and field1's match but not field 1 and field0
                newState.IDEX.valA = state.reg[field0(state.IFID.instr)];
                newState.IDEX.valB = state.reg[field1(state.IFID.instr)];
                newState.IDEX.offset = convertNum(field2(state.IFID.instr));
            } else {
                newState.pc = state.pc;
                newState.IFID = state.IFID;
                newState.IDEX.instr = NOOPINSTRUCTION;
                
            }
            
        } else {
            newState.IDEX.valA = state.reg[field0(state.IFID.instr)];
            newState.IDEX.valB = state.reg[field1(state.IFID.instr)];
            newState.IDEX.offset = convertNum(field2(state.IFID.instr));
        }
        


        /* ---------------------- EX stage --------------------- */
        //execution
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
        int psuA = state.IDEX.valA;
        int psuB = state.IDEX.valB;
        //newState.EXMEM.valB = state.IDEX.valB;
        newState.EXMEM.eq = 0;

        int WriteBack = -1;
        int MemoryBack = -1;
        int ExecBack = -1;
        //need to check for both values of add, nor, sw, beq. Lw just care about field0 
        //but need to compare these values with add, nor's dest reg and lw's valb

        //if the write back instruction is LW then assign writeback to field1
        //if the write back instruction is Add or Nor then assign writeback to field2
        //assign write back values to values A and/or B to state.IDEX.valA and B if they same

        if((opcode(state.WBEND.instr) == 0) || (opcode(state.WBEND.instr) == 1)) {
            //add or nor reg
            
            WriteBack = field2(state.WBEND.instr);
        } else if (opcode(state.WBEND.instr) == 2) {
            //lw reg
            WriteBack = field1(state.WBEND.instr);
            printf("PotentialDataDependacy here\n");
            printf("%d\n", WriteBack);
        }

        if(opcode(newState.EXMEM.instr) == 2) {
            //lw is diff cuz we should only edit field0
           
            if(WriteBack == field0(newState.EXMEM.instr)) {
                
                
                //if new instruction uses same reg as dest reg then overwrite it
                psuA = state.WBEND.writeData;
            }
        } else {
            printf("correct field here\n");
            printf("%d\n", field1(newState.EXMEM.instr));
            printf("%d\n", state.WBEND.instr);
            if(WriteBack == field0(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                printf("Write Value here\n");
                printf("%d\n", state.WBEND.writeData);
                psuA = state.WBEND.writeData;
            
            }
            if(WriteBack == field1(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                printf("Write Value here\n");
                printf("%d\n", state.WBEND.writeData);
                psuB = state.WBEND.writeData;
            }
        }
       
        //if the Memory back instruction is LW then assign Memoryback to field1
        //if the Memory back instruction is Add or Nor then assign Memoryback to field2
        //assign Memory back values to values A and/or B to psuA and psuB if they same
        //this might be wrong
        if((opcode(state.MEMWB.instr) == 0) || (opcode(state.MEMWB.instr) == 1)) {
            //add or nor reg
            MemoryBack = field2(state.MEMWB.instr);
        } else if (opcode(state.MEMWB.instr) == 2) {
            //lw reg
            MemoryBack = field1(state.MEMWB.instr);
        }
        if(opcode(newState.EXMEM.instr) == 2) {
            //lw is diff cuz we should only edit field0
            if(MemoryBack == field0(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuA = state.MEMWB.writeData;
            }
        } else {
            if(MemoryBack == field0(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuA = state.MEMWB.writeData;
            }
            if(MemoryBack == field1(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuB = state.MEMWB.writeData;
            }
        }


        //if the Exec Back instruction is LW then assign ExecBack to field1
        //if the Exec Back instruction is Add or Nor then assign ExecBack to field2
        //assign Exec Back alu result to values A and/or B to pusA and B if they same
        if((opcode(state.EXMEM.instr) == 0) || (opcode(state.EXMEM.instr) == 1)) {
            //add or nor reg
            ExecBack = field2(state.EXMEM.instr);
        } else if (opcode(state.EXMEM.instr) == 2) {
            //lw reg
            ExecBack = field1(state.EXMEM.instr);
        }
        if(opcode(newState.EXMEM.instr) == 2) {
            //lw is diff cuz we should only edit field0
            if(ExecBack == field0(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuA = state.EXMEM.aluResult;
            }
        } else {
            if(ExecBack == field0(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuA = state.EXMEM.aluResult;
            }
            if(ExecBack == field1(newState.EXMEM.instr)) {
                //if new instruction uses same reg as dest reg then overwrite it
                psuB = state.EXMEM.aluResult;
            }
        }


        //printf("%d\n", field0(newState.EXMEM.instr));
        //use if statement to check what value of mux to use (valB or offset)
        //should use offset if the instr is LW or SW
        //put that result into the ALU, use if statement for either nor or add
        //do an if check for equality, store that into EXMEM.eq
        
        if(opcode(newState.EXMEM.instr) == 2 || opcode(newState.EXMEM.instr) == 3) {
            //lw or sw instruction
            newState.EXMEM.aluResult = psuA + state.IDEX.offset;
        } else {
            if(opcode(newState.EXMEM.instr) == 1) {
                //nor instruction
                newState.EXMEM.aluResult = ~(psuA | psuB);
            } else {
                //add a and b
                newState.EXMEM.aluResult = psuA + psuB;
            }
        }
        if(opcode(newState.EXMEM.instr) == 4) {
            if(psuA == psuB) {
                newState.EXMEM.eq = 1;
            }
        }

        newState.EXMEM.valB = psuB;
        
        
        /* --------------------- MEM stage --------------------- */
        //memory
        newState.MEMWB.instr = state.EXMEM.instr;
        // printf("%s\n", "987state.dataMem[state.EXMEM.aluResult]");
        // printf("%d\n", newState.MEMWB.instr);
        if(opcode(newState.MEMWB.instr) == 2) {
            //LW instruction
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
            // printf("%s\n", "state.dataMem[state.EXMEM.aluResult]");
            // printf("%d\n", state.dataMem[state.EXMEM.aluResult]);
        }
        else if (opcode(newState.MEMWB.instr) == 3) {
            //SW instruction
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.valB;
        }
        else if (opcode(newState.MEMWB.instr) == 4) {
            //BEQ instruction
            if (state.EXMEM.eq == 1) {
                newState.pc = state.EXMEM.branchTarget;
                newState.IFID.instr = NOOPINSTRUCTION;
                newState.IDEX.instr = NOOPINSTRUCTION;
                newState.EXMEM.instr = NOOPINSTRUCTION;
            }
        }
        else if (opcode(newState.MEMWB.instr) != 6 && opcode(newState.MEMWB.instr) != 7) {
            //add, nor not noop/halt
            newState.MEMWB.writeData = state.EXMEM.aluResult;
        }

        /* ---------------------- WB stage --------------------- */
        //writeback to registers
        newState.WBEND.instr = state.MEMWB.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;
        if(opcode(state.MEMWB.instr) == 0 || opcode(state.MEMWB.instr) == 1) {
            newState.reg[field2(newState.WBEND.instr)] = state.MEMWB.writeData;
        }
        if(opcode(state.MEMWB.instr) == 2) {
            newState.reg[field1(newState.WBEND.instr)] = state.MEMWB.writeData;
            //printf("%d\n", newState.reg[field1(newState.WBEND.instr)] + 21);
        }

        /* ------------------------ END ------------------------ */
        state = newState; /* this is the last statement before end of the loop. It marks the end
        of the cycle and updates the current state with the values calculated in this cycle */
    }
    printf("Machine halted\n");
    printf("Total of %d cycles executed\n", state.cycles);
    printf("Final state of machine:\n");
    printState(&state);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW.
*/

void printInstruction(int instr) {
    const char* instr_opcode_str;
    int instr_opcode = opcode(instr);
    if(ADD <= instr_opcode && instr_opcode <= NOOP) {
        instr_opcode_str = opcode_to_str_map[instr_opcode];
    }

    switch (instr_opcode) {
        case ADD:
        case NOR:
        case LW:
        case SW:
        case BEQ:
            printf("%s %d %d %d", instr_opcode_str, field0(instr), field1(instr), convertNum(field2(instr)));
            break;
        case JALR:
            printf("%s %d %d", instr_opcode_str, field0(instr), field1(instr));
            break;
        case HALT:
        case NOOP:
            printf("%s", instr_opcode_str);
            break;
        default:
            printf(".fill %d", instr);
            return;
    }
}

void printState(stateType *statePtr) {
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i=0; i<statePtr->numMemory; ++i) {
        printf("\t\tdataMem[ %d ] = %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i=0; i<NUMREGS; ++i) {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if(opcode(statePtr->IFID.instr) == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");

    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if(idexOp == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegA = %d", statePtr->IDEX.valA);
    if (idexOp >= HALT || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->IDEX.valB);
    if(idexOp == LW || idexOp > BEQ || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int exmemOp = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (exmemOp > SW || exmemOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->EXMEM.valB);
    if (exmemOp != SW) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
	int memwbOp = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (memwbOp >= SW || memwbOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // WB/END
	int wbendOp = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (wbendOp >= SW || wbendOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
    fflush(stdout);
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char* filename) {
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory) {
        if (sscanf(line, "%d", state->instrMem+state->numMemory) != 1) {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ]\t= 0x%08x\t= %d\t= ", state->numMemory, 
            state->instrMem[state->numMemory], state->instrMem[state->numMemory]);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf("\n");
    }
}
