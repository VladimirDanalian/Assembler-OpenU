#include "ProjectFunctions.h"

/*
 * Helper: fill one RAM word for a direct (method=1) or relative (method=2) operand.
 *
 * Direct  (1): writes label.address into RAM[RAMIndex], flagged R.
 *              Advances RAMIndex by 1.
 * Relative(2): writes (label.address - RAMIndex - 1), flagged A.
 *              Advances RAMIndex by 1.
 * Extern  (1 but base==0): writes 0|E into RAM[RAMIndex].
 *              Advances RAMIndex by 1.
 */
static void fillLabelWord(int method, int labelIndex,
                           int *RAM, int *RAMIndex,
                           label **labels, label **externs, int *externsCount,
                           char *labelToken) {
    label *tempExterns;


    if (method == ONE) { /* direct */
        if ((*labels)[labelIndex].address != ZERO) {
            RAM[*RAMIndex]  = (*labels)[labelIndex].address;
           /* RAM[*RAMIndex] |= (ONE << ARE_R);*/
            (*RAMIndex)++;
        }else {
            /* extern label */
            tempExterns = (label *) realloc(*externs, ((*externsCount) + ONE) * sizeof(label));
            if (!tempExterns) { puts("realloc failed"); exit(EXIT); }
            *externs = tempExterns;
            strcpy((*externs)[*externsCount].name, labelToken);
            (*externs)[*externsCount].address = *RAMIndex;
            (*externsCount)++;
            (*RAMIndex)++;
        }
    } else if (method == TWO) { /* relative */
        int distance = (*labels)[labelIndex].address - (*RAMIndex) - ONE;


        if (distance < ZERO) distance &= 0xFFF;
        RAM[*RAMIndex]  = distance;
        RAM[*RAMIndex] |= (ONE << ARE_A);
        (*RAMIndex)++;
    }
}

int secondPass(FILE *newfp, int RAM[RAM_SIZE], label **labels, int labelCount, command commands[],
               char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], label **externs, int *externsCount) {

    char line[MAX_LINE_SIZE];
    int  wordAmount = ZERO;
    char *wordArray[MAX_LINE_SIZE];
    int  lineCounter = ZERO;
    int  labelFlag = ZERO;
    int  entryLabelIndex = ZERO;
    int  errorFlag = FALSE;
    int  commandIndex = ZERO;
    int  firstOperandAddressMethod  = ZERO;
    int  secondOperandAddressMethod = ZERO;
    int  RAMIndex = RAM_START;
    char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE];
    int  labelIndex = ZERO;
    char labelToken[MAX_LINE_SIZE];

    fseek(newfp, ZERO, SEEK_SET);

    while (fgets(line, MAX_LINE_SIZE, newfp)) {
        labelFlag = ZERO;
        wordAmount = splitLineIntoWords(line, wordArray);/* Divides the line into array containing each word*/
        lineCounter++;

        printf("line %d: wordArray[0]='%s' labelFlag=%d word='%s'\n",
           lineCounter, wordArray[0], labelFlag, wordArray[labelFlag]);

        if (wordArray[ZERO][strlen(wordArray[ZERO]) - ONE] == ':') {
            labelFlag = ONE;
        }


        /* Skip comments and blank lines */

        if ((strcmp(wordArray[ZERO + labelFlag], ".data")   == ZERO) ||
            (strcmp(wordArray[ZERO + labelFlag], ".string") == ZERO) ||
            (strcmp(wordArray[ZERO + labelFlag], ".extern") == ZERO)) {
            printf("checking skip: '%s'\n", wordArray[ZERO + labelFlag]);
            continue;
        }

        /* Detect label */
        if (wordArray[ZERO][strlen(wordArray[ZERO]) - ONE] == ':') {
            labelFlag = ONE;
        }

        /* Skip .data / .string / .extern */
        if ((strcmp(wordArray[ZERO + labelFlag], ".data")   == ZERO) ||
            (strcmp(wordArray[ZERO + labelFlag], ".string") == ZERO) ||
            (strcmp(wordArray[ZERO + labelFlag], ".extern") == ZERO)) {
            continue;
        }

        /* .entry — mark the label */
        if (strcmp(wordArray[ZERO + labelFlag], ".entry") == ZERO) {
            entryLabelIndex = labelNum(wordArray[ONE + labelFlag], *labels, labelCount);
            if (entryLabelIndex == MINUS_ONE) {
                errorFlag = TRUE;
                continue;
            }
            (*labels)[entryLabelIndex].attributesCount++;
            strcpy((*labels)[entryLabelIndex].attributes[(*labels)[entryLabelIndex].attributesCount - ONE],
                   "entry");
            continue;
        }

        /* Command line */
        commandIndex = searchCommand(commands, wordArray[ZERO + labelFlag]);

        /* 0-operand commands */
        if ((strcmp(wordArray[ZERO + labelFlag], "rts")  == ZERO) ||
            (strcmp(wordArray[ZERO + labelFlag], "stop") == ZERO)) {
            RAMIndex++;
            continue;
        }
        if (wordAmount <= labelFlag + ONE) {
            RAMIndex++;
            continue;
        }


        lineForCommand(wordArray, labelFlag, wordAmount, commandOperands, lineCounter);

        switch (commandIndex) {

            /* ── 2-operand commands ── */
            case ZERO:  /* mov */
            case ONE:   /* cmp */
            case TWO:   /* add */
            case THREE: /* sub */
            case FOUR:  /* lea */

                firstOperandAddressMethod  = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                secondOperandAddressMethod = findAddressMethod(commandOperands[ONE],  registerTable, lineCounter);

                RAMIndex++; /* skip instruction word */

                /* source operand */
                if (firstOperandAddressMethod == ONE || firstOperandAddressMethod == TWO) {
                    if (firstOperandAddressMethod == ONE) {
                        strcpy(labelToken, commandOperands[ZERO]);
                    } else { /* relative — strip '%' */
                        strcpy(labelToken, commandOperands[ZERO] + ONE);
                    }
                    labelIndex = labelNum(labelToken, *labels, labelCount);
                    if (labelIndex == MINUS_ONE) {
                        printf("Line %d- Error - Label wasn't found (src)\n", lineCounter);
                        errorFlag = TRUE; continue;
                    }
                    fillLabelWord(firstOperandAddressMethod, labelIndex,
                                  RAM, &RAMIndex, labels, externs, externsCount, labelToken);
                } else {
                    /* register or immediate — already written in pass 1, just advance */
                    RAMIndex++;
                }

                /* destination operand */
                if (secondOperandAddressMethod == ONE || secondOperandAddressMethod == TWO) {
                    if (secondOperandAddressMethod == ONE) {
                        strcpy(labelToken, commandOperands[ONE]);
                    } else {
                        strcpy(labelToken, commandOperands[ONE] + ONE);
                    }
                    labelIndex = labelNum(labelToken, *labels, labelCount);
                    if (labelIndex == MINUS_ONE) {
                        printf("Line %d- Error - Label wasn't found (dst)\n", lineCounter);
                        errorFlag = TRUE; continue;
                    }
                    fillLabelWord(secondOperandAddressMethod, labelIndex,
                                  RAM, &RAMIndex, labels, externs, externsCount, labelToken);
                } else {
                    /* register or immediate — already written in pass 1, just advance */
                    RAMIndex++;
                }
                break;

            /* ── 1-operand commands ── */
            case FIVE:    /* clr */
            case SIX:     /* not */
            case SEVEN:   /* inc */
            case EIGHT:   /* dec */
            case NINE:    /* jmp */
            case TEN:     /* bne */
            case ELEVEN:  /* jsr */
            case TWELVE:  /* red */
            case THIRTEEN:/* prn */

                secondOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);

                RAMIndex++; /* skip instruction word */

                if (secondOperandAddressMethod == ONE || secondOperandAddressMethod == TWO) {
                    if (secondOperandAddressMethod == ONE) {
                        strcpy(labelToken, commandOperands[ZERO]);
                    } else { /* relative — strip '%' */
                        strcpy(labelToken, commandOperands[ZERO] + ONE);
                    }
                    labelIndex = labelNum(labelToken, *labels, labelCount);
                    if (labelIndex == MINUS_ONE) {
                        printf("Line %d- Error - Label wasn't found\n", lineCounter);
                        errorFlag = TRUE; continue;
                    }
                    fillLabelWord(secondOperandAddressMethod, labelIndex,
                                  RAM, &RAMIndex, labels, externs, externsCount, labelToken);
                } else {
                    /* register or immediate — already written in pass 1, just advance */
                    RAMIndex++;
                }
                break;

            /* ── 0-operand commands ── */
            case FOURTEEN: /* rts  */
            case FIFTEEN:  /* stop */
                RAMIndex++;
                break;
        }
    }

    if (errorFlag == TRUE)
    {
        printf("secondPass failed at line %d\n", lineCounter);
        return MINUS_ONE;
    }
    return ONE;
}
