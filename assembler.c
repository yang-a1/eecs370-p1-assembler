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
static inline void printHexToFile(FILE *, int);
static int endsWith(char *, char *);

struct labelStorage
{
    char label[7];
    int address;
};

int findLabel(const char* instr, struct labelStorage labelsList[], int index) 
{
    for (int i = 0; i < index; i++)
    {
        if (strcmp(labelsList[i].label, instr) == 0) return labelsList[i].address;
    }

    return -1;
}

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

    if (!endsWith(inFileString, ".as") &&
        !endsWith(inFileString, ".s") &&
        !endsWith(inFileString, ".lc2k")
    ) {
        printf("warning: assembly code file does not end with .as, .s, or .lc2k\n");
    }

    if (!endsWith(outFileString, ".mc")) {
        printf("error: machine code file must end with .mc\n");
        exit(1);
    }

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

    struct labelStorage labelsList[1000];
    int PC = 0;
    int index = 0;

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (label[0] != '\0')
        {
            for (int i = 0; i < index; i++)
            {
                if (strcmp(label, labelsList[i].label) == 0)
                {
                    exit(1);
                }
            }
           
            strcpy(labelsList[index].label, label);
            labelsList[index].address = PC;
            index++;
        }

        PC++;
    }

    
    rewind(inFilePtr);
    PC = 0;

    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int mc = 0;
        int opcodeVal = 0;
        int reg1Val = 0;
        int reg2Val = 0;
        int offsetField = 0;
        int destination = 0;

        if (strcmp("add", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1) || 
                !isNumber(arg2)) exit(1);

            opcodeVal = 0;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);
            destination = atoi(arg2);

            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7) ||
                (destination < 0 || destination > 7)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16) | (destination & 0x7);
        }
        else if (strcmp("nor", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1) || 
                !isNumber(arg2)) exit(1);

            opcodeVal = 1;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);
            destination = atoi(arg2);

            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7) ||
                (destination < 0 || destination > 7)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16) | (destination & 0x7);
        }
        else if (strcmp("lw", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1)) exit(1);

            opcodeVal = 2;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);
            if(isNumber(arg2)) offsetField = atoi(arg2);
            else offsetField = findLabel(arg2, labelsList, index);

            if (!isNumber(arg2) && offsetField == -1) exit(1);

            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7) ||
                (offsetField < -32768 || offsetField > 32767)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16) | (offsetField & 0xFFFF);
        }
        else if (strcmp("sw", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1)) exit(1);

            opcodeVal = 3;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);
            if(isNumber(arg2)) offsetField = atoi(arg2);
            else offsetField = findLabel(arg2, labelsList, index);

            if (!isNumber(arg2) && offsetField == -1) exit(1);

            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7) ||
                (offsetField < -32768 || offsetField > 32767)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16) | (offsetField & 0xFFFF);
        }
        else if (strcmp("beq", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1)) exit(1);

            opcodeVal = 4;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);
            if(isNumber(arg2)) offsetField = atoi(arg2);
            else
            {
                int defLoc = findLabel(arg2, labelsList, index);
                if (defLoc == -1) exit(1);
                offsetField = defLoc - (PC + 1);
            } 

            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7) ||
                (offsetField < -32768 || offsetField > 32767)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16) | (offsetField & 0xFFFF);
        }
        else if (strcmp("jalr", opcode) == 0)
        {
            if (!isNumber(arg0) || !isNumber(arg1)) exit(1);

            opcodeVal = 5;
            reg1Val = atoi(arg0);
            reg2Val = atoi(arg1);   
            if ((reg1Val < 0 || reg1Val > 7) ||
                (reg2Val < 0 || reg2Val > 7)) exit(1);
            
            mc = (opcodeVal << 22) | (reg1Val << 19) | (reg2Val << 16);
        }
        else if (strcmp("halt", opcode) == 0)
        {
            opcodeVal = 6;
            mc = (opcodeVal << 22);
        }
        else if (strcmp("noop", opcode) == 0)
        {
            opcodeVal = 7;
            mc = (opcodeVal << 22);
        }
        else if (!strcmp(opcode, ".fill")) 
        {
            if (isNumber(arg0)) mc = atoi(arg0);
            else 
            {
                int defLoc = findLabel(arg0, labelsList, index);
                if (defLoc == -1) exit(1);
                mc = defLoc;
            }
        }
        else 
        {
            exit(1);
        }

        PC++;
        printHexToFile(outFilePtr, mc);
    }

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


// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}

// Returns 1 if string ends with substr, 0 otherwise
static int
endsWith(char *string, char *substr) {
    size_t stringLen = strlen(string);
    size_t substrLen = strlen(substr);
    if (stringLen < substrLen) {
        return 0; // string too short
    }
    char *stringEnd = string + stringLen - substrLen;
    if (strcmp(stringEnd, substr) == 0) {
        return 1;
    }
    return 0;
}
