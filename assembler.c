/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
int checkLabel(char* string, char addresstoLabel[1000][7], int size);
void findDuplicates(char addressToLabel[MAXLINELENGTH][7], int arrSize);

int
main(int argc, char **argv)
{
   
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    // if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //     /* reached end of file */
    // }
    char labelArray[MAXLINELENGTH][7];
    int PC = 0;
    int arrSize = 0;
    //int address = 0;
    //PC = 5, arrSize = 10, line 10;
    
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) == 1 ) {
        //check if first char of label is num
        char temp = label[0]; 
        //printf("%s\n", &temp);
        if(isNumber(&temp)) {
            exit(1);
        }
        
        //labels can have a max of 6 char
        if(strlen(label) > 6) {
            exit(1);
        }
        
        if (strlen(label)) {
            strcpy(labelArray[arrSize], label);
            //printf("%s\n", label);
        }
        arrSize++;
        //printf("%d\n", PC);
    
    }
    //printf("findDuplicates\n");
    //printf("%d\n", arrSize);
    //printf("%s\n", addressToLabel[5]);

    findDuplicates(labelArray, arrSize);
    //printf("afterDuplicates\n");
    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    /* after doing a readAndParse, you may want to do the following to test the
        opcode */
    if (!strcmp(opcode, "add")) {
        /* do whatever you need to do for opcode "add" */
    }

    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) == 1) {
        
        if(!strcmp(opcode, "add")) {
            //printf("work\n");
            int op = 0;
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);
            //printf("%s\n", arg2);
            if(!isNumber(arg2)) {
                exit(1);
            }
            int destReg = atoi(arg2);
          
            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7) || (destReg < 0) || (destReg > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;
            
            fprintf(outFilePtr, "%d\n", op+regA+regB+destReg);
            //printf("add\n");
        } else if (!strcmp(opcode, "nor")) {
            int op = 1;
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);
            if(!isNumber(arg2)) {
                exit(1);
            }
            int destReg = atoi(arg2);

            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7) || (destReg < 0) || (destReg > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;

            fprintf(outFilePtr, "%d\n", op + regA + regB + destReg);
            //printf("nor\n");
        } else if (!strcmp(opcode, "lw")) {
            int op = 2;
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);
            
            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;

            int offset = 0;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
                //printf("%d\n", offset);
                if(offset >= 32768 || offset <= -32769) {
                    exit(1);
                }
                if(offset < 0) {
                    offset &= 0xFFFF;
                    offset |= 0X8000;
                }
                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
            } else {
                offset = checkLabel(arg2, labelArray, arrSize);
                // printf("%d\n", offset);
                // printf("%s\n", arg2);
                if(offset >= 32768 || offset <= -32769) {
                    exit(1);
                }
                if(offset < 0) {
                    offset &= 0xFFFF;
                    offset |= 0X8000;
                }
                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
            }
            
            
            //printf("lw\n");
        } else if (!strcmp(opcode, "sw")) {
            int op = 3; 
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);
            

            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;

            int offset = 0;

            if (isNumber(arg2)) {
                offset = atoi(arg2);
                if (offset >= 32768 || offset <= -32769) {
                    exit(1);
                }

                if (offset < 0) {
                    offset &= 0xFFFF;
                    //offset |= 0X8000;
                }

                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
            } else {
                offset = checkLabel(arg2, labelArray, arrSize);
                if (offset >= 32768 || offset <= -32769) {
                    exit(1);
                }

                if (offset < 0) {
                    offset &= 0xFFFF;
                    //offset |= 0X8000;
                }

                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
            }

            
            //printf("sw\n");
        } else if (!strcmp(opcode, "beq")) {
            int op = 4; 
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);
        
            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;

            int offset = 0;
            if (isNumber(arg2)) {
                offset = atoi(arg2);

                if(offset >= 32768 || offset <= -32769) {
                    exit(1);
                }

                if (offset < 0) {
                    offset &= 0xFFFF;
                    //offset |= 0X8000;
                }
                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
            } else {

                offset = checkLabel(arg2, labelArray, arrSize) - PC;
                offset -= 1;


                if(offset >= 32768 || offset <= -32769) {
                    exit(1);
                }
                if (offset < 0) {
                    offset &= 0xFFFF;
                    //offset |= 0X8000;
                }
                //printf("%s\n", arg2);
                fprintf(outFilePtr, "%d\n", op + regA + regB + offset);
                
            }
            
            //printf("beq\n");
        } else if (!strcmp(opcode, "jalr")) {
            int op = 5; 
            op <<= 22;

            int regA = atoi(arg0);
            int regB = atoi(arg1);

            if((regA < 0) || (regA > 7) || (regB < 0) || (regB > 7)) {
                exit(1);
            }

            regA <<= 19;
            regB <<= 16;

            fprintf(outFilePtr, "%d\n", op + regA + regB);
            //printf("jalr\n");

        
            //PC = regA;
        } else if (!strcmp(opcode, "halt")) {
            int op = 6;  
            op <<= 22;

            fprintf(outFilePtr, "%d\n", op);
            //printf("halt\n");

        } else if (!strcmp(opcode, "noop")) {
            int op = 7;
            op <<= 22;

            fprintf(outFilePtr, "%d\n", op);
            //printf("noop\n");
        } else if (!strcmp(opcode, ".fill")) {
            int ans = 0;
            if (isNumber(arg0)) {
                ans = atoi(arg0);
            } else {
                ans = checkLabel(arg0, labelArray, arrSize);
            }
            
            fprintf(outFilePtr, "%d\n", ans);
            //printf("fill\n");
        } else {
            printf("%s\n", opcode);
            printf("error: wrong label \n");
            exit(1);
        }
        PC++;
        continue;
    } 
    return(0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}

int checkLabel(char* string, char addresstoLabel[1000][7], int size) {
    int search = -1;
    for(int i = 0; i < size; i++) {
        if(!strcmp(string, addresstoLabel[i])) {
            search = i;
            return search;
        }
    }
    if (search == -1) {
        exit(1);
    }
    return -1;
}

void findDuplicates(char addressToLabel[MAXLINELENGTH][7], int arrSize){
    for (int i = 0; i < arrSize; i++){
        for (int j = i + 1; j < arrSize; j++){
            if(strlen(addressToLabel[i]) && strlen(addressToLabel[j])) {
                if(!strcmp(addressToLabel[i], addressToLabel[j])){
                    exit(1);
                }
            }
            
        }
    }
}

