#include "ProjectFunctions.h"

/*firstqPASS - Performs the first pass of the assembler over the source file.
 *
 * This function reads the input file line by line and performs the following tasks:
 *
 *  1. Label Detection & Validation:
 *     - Identifies labels (words ending with ':')
 *     - Validates label names against reserved words and previously defined labels
 *     - Stores labels in the labels table with their address and attributes
 *
 *  2. Directive Handling:
 *     - .data   : Parses integer values and stores them in DCTable; adds label with "data" attribute
 *     - .string : Parses a string and stores ASCII values in DCTable; adds label with "data" attribute
 *     - .entry  : Sets the entryFlag (label resolution deferred to second pass)
 *     - .extern : Adds the external label to the labels table with address=0 and "extern" attribute
 *
 *  3. Command Handling:
 *     - Searches for the command in the commands table
 *     - Determines the number of operands and their addressing methods
 *     - Encodes the instruction word and operand words into RAM via changeMemoryFirstCross()
 *     - Advances IC (Instruction Counter) by the length of the command
 *
 *  4. Post-Pass Finalization:
 *     - Updates data label addresses to be relative to ICF (end of code segment)
 *     - Copies DCTable into RAM after the code segment
 *
 * Parameters:
 *   newfile       - Pointer to the input (.am) source file
 *   RAM           - Memory array to write encoded instructions into
 *   labels        - Pointer to the dynamic labels table (may be reallocated)
 *   labelCount    - Pointer to the number of labels currently in the table
 *   entryFlag     - Pointer to flag indicating if any .entry directive was found
 *   commands      - Array of command structs (name, opcode, funct)
 *   extFlag       - Pointer to flag indicating if any .extern directive was found
 *   DCF           - Pointer to the final Data Counter value (number of data words)
 *   registerTable - Table of valid register names (r0-r7)
 *
 * Returns:
 *   ICF  - The final Instruction Counter value (address after last instruction)
 *   FALSE - If any error was encountered during the pass
 */





int firstqPASS(FILE *newfile, int RAM[RAM_SIZE], label **labels, int *labelCount, int *entryFlag, command commands[], int *extFlag, int *DCF, char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE]) {
    char *wordArray[MAX_LINE_SIZE];   /* Will contain an array of the line divided into words*/
    char line[MAX_LINE_SIZE];     /* Will contain the current line*/
    int wordAmount = ZERO;        /* Amount of words in the line*/
    int lineCounter = ZERO;      /* Will be used to index the lines*/
    int labelFlag = ZERO;        /* Turned on if we came across a label */
    char strLabelWithoutColon[MAX_LINE_SIZE];  /* Will be used to remove the colon at the end of label name*/
    int errorFlag = FALSE;     /* Will be used to Mark if we encountered an error*/
    label *tempLabels;         /* Will be used to make sure a dynamically allocated labels array is not NULL*/
    int DC = ZERO;               /* Indicates the progress in the data set */
    char intLine[MAX_LINE_SIZE];    /* The temp line of number in separated by "," */
    int numOfCommas = ZERO;         /* Will be used to count how many commas are in the line to check if the line is balanced */
    int numInLineCounter = ZERO;     /* Will be used to count how many numbers we have in the line */
    int intArray[MAX_LINE_SIZE];    /* Will be used to count how many numbers we have in the line */
    int dataLineIndex = ZERO;   /* Will be used to run on the numbers in int data entering */
    int DCTable[RAM_SIZE];  /* Will be used to enter the data*/
    int labelStatus = ZERO;  /* Will be used to hold the number returned from the method checkLabelName */
    int IC = RAM_START;     /* Will be used for RAM memory allocation*/
    int lengthOfCommand = ZERO; /* Will be used to hold the variables of the command */
    int commandIndex = ZERO;    /* Will be used to hold the command number */
    int operandsCounter = ZERO; /* Will be used to hold the number of operands in a line  */
    char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE]; /* Will be used to hold the command number */
    int ICF = ZERO;   /* final IC */
    int index = ZERO;   /* Running index for the loop */
    int firstOperandAddressMethod ;
    int secondOperandAddressMethod ;

   /* Initialize integer and data counter arrays to zero */
    for (index = ZERO; index < MAX_LINE_SIZE; index++) intArray[index] = ZERO;
    for (index = ZERO; index < RAM_SIZE; index++) DCTable[index] = ZERO;
    index = ZERO;

    /* Reset file pointer to the beginning of the file */
    fseek(newfile, ZERO, SEEK_SET);

    while (fgets(line, MAX_LINE_SIZE, newfile)) { /* Reads a line from the file */

        wordAmount = splitLineIntoWords(line, wordArray);
        lineCounter++;
        labelFlag = ZERO;
        lengthOfCommand = ZERO;
        firstOperandAddressMethod = ZERO;
        secondOperandAddressMethod = ZERO;
        strLabelWithoutColon[ZERO] = '\0';

        /* Skip empty lines and comments */
        if (wordAmount == ZERO) continue;
        if (wordArray[ZERO][ZERO] == ';') continue;
        if (strcmp(wordArray[ZERO], "\n") == ZERO) continue;
        if (strcmp(wordArray[ZERO], "\t") == ZERO) continue;

        /* Check if the first word is a label (ends with ':') */
        if (wordArray[ZERO][strlen(wordArray[ZERO]) - ONE] == ':') {
            labelWithoutColon(wordArray[ZERO], strLabelWithoutColon);
            removesNewLine(strLabelWithoutColon);

            /* Validate the label name against reserved words and existing labels */
            labelStatus = checkLabelName(strLabelWithoutColon, *labels, *labelCount);
            if (labelStatus == MINUS_TWO) {
                printf("Line %d- Error - error in label name\n", lineCounter);
                errorFlag = TRUE;
                continue;
            } else if (labelStatus >= ZERO) {
                printf("Line %d- Error - label already defined\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }
            labelFlag = ONE;
        }

        /* .data — parse integer values and store in DCTable */
        if (strcmp(wordArray[ZERO + labelFlag], ".data") == ZERO) {

            /* If a label precedes .data, add it to the labels table with "data" attribute */
            if (labelFlag == ONE) {
                if (*labelCount == ZERO) {
                    (*labels) = (label *) malloc(sizeof(label));
                } else {
                    tempLabels = (label *) realloc((*labels), (*labelCount + ONE) * sizeof(label));
                    if (tempLabels == NULL) { puts("realloc failed"); exit(EXIT); }
                    (*labels) = tempLabels;
                }
                (*labelCount)++;
                strcpy((*labels)[*labelCount - ONE].name, strLabelWithoutColon);
                (*labels)[*labelCount - ONE].address = DC;
                (*labels)[*labelCount - ONE].attributesCount = ONE;
                strcpy((*labels)[*labelCount - ONE].attributes[ZERO], "data");
            }

            /* Build a single string from all data values and convert to integer array */
            makeIntLine(wordArray, wordAmount, intLine, labelFlag);
            numOfCommas = countCommas(intLine);
            numInLineCounter = makeItNumArray(intArray, intLine);

            if (numInLineCounter < ZERO) {
                printf("Line %d- Error - Wrong data was entered\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }
            /* Validate that commas and number of values are balanced */
            if (numOfCommas != ZERO && numOfCommas + ONE != numInLineCounter) {
                printf("Line %d- Error - Numbers and commas are not balanced\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }
            /* Store each integer value in DCTable, masking negatives to 12 bits */
            for (dataLineIndex = ZERO; dataLineIndex < numInLineCounter; dataLineIndex++) {
                DCTable[DC] = intArray[dataLineIndex];
                if (DCTable[DC] < ZERO) DCTable[DC] &= TURN_OFF_BITS_FOR_NEGATIVE_NUMBERS;
                DC++;
            }
        }

        /* .string — store ASCII values of each character in DCTable, terminated by 0 */
        else if (strcmp(wordArray[ZERO + labelFlag], ".string") == ZERO) {

            /* If a label precedes .string, add it to the labels table with "data" attribute */
            if (labelFlag == ONE) {
                if (*labelCount == ZERO) {
                    (*labels) = (label *) malloc(sizeof(label));
                } else {
                    tempLabels = (label *) realloc((*labels), (*labelCount + ONE) * sizeof(label));
                    if (tempLabels == NULL) { puts("realloc failed"); exit(EXIT); }
                    (*labels) = tempLabels;
                }
                (*labelCount)++;
                strcpy((*labels)[*labelCount - ONE].name, strLabelWithoutColon);
                (*labels)[*labelCount - ONE].address = DC;
                (*labels)[*labelCount - ONE].attributesCount = ONE;
                strcpy((*labels)[*labelCount - ONE].attributes[ZERO], "data");
            }

            if (wordAmount < THREE) {
                printf("Line %d- Error - missing string argument\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }
            /* Store each character's ASCII value, skipping quotes and newlines */
            for (dataLineIndex = ZERO; wordArray[ONE + labelFlag][dataLineIndex] != '\0'; dataLineIndex++) {
                if (wordArray[ONE + labelFlag][dataLineIndex] != '"' && wordArray[ONE + labelFlag][dataLineIndex] != '\n') {
                    DCTable[DC] = (int) wordArray[ONE + labelFlag][dataLineIndex];
                    DC++;
                }
            }
            /* Null-terminate the string in DCTable */
            DCTable[DC] = ZERO;
            DC++;
        }

        /* .entry — mark that an entry directive exists (label resolved in second pass) */
        else if (strcmp(wordArray[ZERO + labelFlag], ".entry") == ZERO) {
            *entryFlag = TRUE;
        }

        /* .extern — add the external label to the labels table with address=0 */
        else if (strcmp(wordArray[ZERO + labelFlag], ".extern") == ZERO) {
            removesNewLine(wordArray[ONE + labelFlag]);
            labelStatus = checkLabelName(wordArray[ONE + labelFlag], *labels, *labelCount);

            if (labelStatus == MINUS_TWO) {
                printf("Line %d- Error - error in extern label name\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }

            /* Add the extern label only if it hasn't been defined before */
            if (labelStatus == MINUS_ONE) {
                if (*labelCount == ZERO) {
                    (*labels) = (label *) malloc(sizeof(label));
                } else {
                    tempLabels = (label *) realloc((*labels), (*labelCount + ONE) * sizeof(label));
                    if (tempLabels == NULL) { puts("realloc failed"); exit(EXIT); }
                    (*labels) = tempLabels;
                }
                (*labelCount)++;
                strcpy((*labels)[*labelCount - ONE].name, wordArray[ONE + labelFlag]);
                (*labels)[*labelCount - ONE].address = ZERO;
                (*labels)[*labelCount - ONE].attributesCount = ONE;
                strcpy((*labels)[*labelCount - ONE].attributes[ZERO], "extern");
                *extFlag = TRUE;
            }
        }

        /* Regular command — encode into RAM and advance IC */
        else {
            /* If a label precedes the command, add it to the labels table with "code" attribute */
            if (labelFlag == ONE) {
                if (*labelCount == ZERO) {
                    (*labels) = (label *) malloc(sizeof(label));
                } else {
                    tempLabels = (label *) realloc((*labels), (*labelCount + ONE) * sizeof(label));
                    if (tempLabels == NULL) { puts("realloc failed"); exit(EXIT); }
                    (*labels) = tempLabels;
                }
                (*labelCount)++;
                strcpy((*labels)[*labelCount - ONE].name, strLabelWithoutColon);
                (*labels)[*labelCount - ONE].address = IC;
                (*labels)[*labelCount - ONE].attributesCount = ONE;
                strcpy((*labels)[*labelCount - ONE].attributes[ZERO], "code");
                (*labels)[*labelCount - ONE].length = ZERO;
            }

            /* Search for the command in the commands table */
            commandIndex = searchCommand(commands, wordArray[ZERO + labelFlag]);
            if (commandIndex == MINUS_ONE) {
                printf("Line %d- Error - command not found\n", lineCounter);
                errorFlag = TRUE;
                continue;
            }

            /* Handle 0-operand commands (rts, stop) */
            if ((strcmp(wordArray[ZERO + labelFlag], "rts") == ZERO) ||
                (strcmp(wordArray[ZERO + labelFlag], "stop") == ZERO)) {
                operandsCounter = ZERO;
                lengthOfCommand = ONE;
            } else {
                /* Parse and validate operands from the line */
                operandsCounter = lineForCommand(wordArray, labelFlag, wordAmount, commandOperands, lineCounter);
                if (operandsCounter == MINUS_ONE) {
                    errorFlag = TRUE;
                    continue;
                }

                switch (commandIndex) {
                    /* 2-operand commands: validate operand count and addressing methods */
                    case ZERO: /* mov */
                    case ONE:  /* cmp */
                    case TWO:  /* add */
                    case THREE:/* sub */
                    case FOUR: /* lea */
                        if (operandsCounter != TWO) {
                            numOfOperandsNotMatch(lineCounter);
                            errorFlag = TRUE;
                            continue;
                        }
                        firstOperandAddressMethod  = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                        secondOperandAddressMethod = findAddressMethod(commandOperands[ONE],  registerTable, lineCounter);
                        if (firstOperandAddressMethod == MINUS_ONE || secondOperandAddressMethod == MINUS_ONE) {
                            printf("Line %d- Error - bad address method\n", lineCounter);
                            errorFlag = TRUE;
                            continue;
                        }
                        lengthOfCommand = THREE; /* instruction word + 2 operand words */
                        break;

                    /* 1-operand commands: validate operand count and addressing method */
                    case FIVE:    /* clr */
                    case SIX:     /* not */
                    case SEVEN:   /* inc */
                    case EIGHT:   /* dec */
                    case NINE:    /* jmp */
                    case TEN:     /* bne */
                    case ELEVEN:  /* jsr */
                    case TWELVE:  /* red */
                    case THIRTEEN:/* prn */
                        if (operandsCounter != ONE) {
                            numOfOperandsNotMatch(lineCounter);
                            errorFlag = TRUE;
                            continue;
                        }
                        secondOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                        if (secondOperandAddressMethod == MINUS_ONE) {
                            printf("Line %d- Error - bad address method\n", lineCounter);
                            errorFlag = TRUE;
                            continue;
                        }
                        lengthOfCommand = TWO; /* instruction word + 1 operand word */
                        break;

                    /* 0-operand commands */
                    case FOURTEEN: /* rts  */
                    case FIFTEEN:  /* stop */
                        lengthOfCommand = ONE;
                        break;
                }
            }

            /* Encode the instruction and operands into RAM */
            changeMemoryFirstCross(firstOperandAddressMethod, secondOperandAddressMethod,
                                   RAM, IC, commandIndex, commands, operandsCounter,
                                   registerTable, commandOperands);

            /* Update the label's length if it was defined on this line */
            if (labelFlag == ONE) {
                (*labels)[*labelCount - ONE].length = lengthOfCommand;
            }
            /* Advance the instruction counter */
            IC += lengthOfCommand;
        }
    }

    if (errorFlag == TRUE) return FALSE;

    *DCF = DC;
    ICF = IC;

    /* Update data label addresses to be relative to the end of the code segment */
    for (index = ZERO; index < *labelCount; index++) {
        if (strcmp((*labels)[index].attributes[ZERO], "data") == ZERO) {
            (*labels)[index].address = ICF + (*labels)[index].address;
        }
    }

    /* Copy the data segment (DCTable) into RAM after the code segment */
    for (index = ZERO; index < *DCF; index++) {
        RAM[ICF + index] = DCTable[index];
    }

    return ICF;
}
