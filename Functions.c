#include "ProjectFunctions.h"


/*
 * This function splits an input string into words
 * using a delimiter and stores pointers to each word in an array,
 * returning the total number of words
 */

int splitLineIntoWords(char inputLine[MAX_LINE_SIZE], char **wordsArray)
{
    int wordCount = ZERO;
    char *currentWord;

    currentWord = strtok(inputLine, " \t");
    while (currentWord != NULL) {
        wordsArray[wordCount] = currentWord;
        wordCount++;
        currentWord = strtok(NULL, " \t");
    }

    return wordCount;
}

/*
 * This function copies a string into another string while removing the newline character (\n)
 */
void removeNewLine(char inputLine[MAX_LINE_SIZE], char cleanedLine[MAX_LINE_SIZE]) {
    int index = ZERO;

    while (inputLine[index] != '\n') {
        cleanedLine[index] = inputLine[index];
        index++;
    }

    cleanedLine[index] = '\0';
}

/*
 *This function checks whether a given label name is valid by
 *comparing it with reserved words and previously defined labels.
 */

int checkLabelName(char word[], label labels[], int labelCount) {
    int savedWordIndex = ZERO;
    int labelsIndex = ZERO;

    /* An array containing the reserved words of the program */
    char savedWord[SAVED_WORDS_AMOUNT][TEN] =
    {
        {"mov"}, {"cmp"}, {"add"}, {"sub"}, {"lea"},
        {"clr"}, {"not"}, {"inc"}, {"dec"}, {"jmp"},
        {"bne"}, {"jsr"}, {"red"}, {"prn"}, {"rts"},
        {"stop"},
        {"r0"}, {"r1"}, {"r2"}, {"r3"},
        {"r4"}, {"r5"}, {"r6"}, {"r7"},
        {".data"}, {".string"}, {".entry"}, {".extern"}
    };

    /* Compares the given label name with all reserved words
       to make sure it is not identical to any of them */
    for (savedWordIndex = ZERO; savedWordIndex < SAVED_WORDS_AMOUNT; savedWordIndex++) {
        if (strcmp(word, savedWord[savedWordIndex]) == ZERO) {
            return MINUS_TWO;
        }
    }

    /* Compares the given label name with all previously defined labels
       to make sure it is not identical to any of them */
    for (labelsIndex = ZERO; labelsIndex < labelCount; labelsIndex++) {
        if (strcmp(word, labels[labelsIndex].name) == ZERO) {
            return labelsIndex; /* Returns the index of the label with the same name */
        }
    }

    return MINUS_ONE;
}

/*
 * Removes the colon of the label
 */

void labelWithoutColon(char labelBefore[MAX_LINE_SIZE], char labelWithoutColon[MAX_LINE_SIZE]) {
    char *token;
    token = strtok(labelBefore, ":");
    strcpy(labelWithoutColon, token);
}
/*
 * Enter the array to a string of numbers
 */

void makeIntLine(char *wordArray[MAX_LINE_SIZE], int wordAmount, char tempLine[MAX_LINE_SIZE], int labelFlag) {
    int ind;
    strcpy(tempLine, "");
    for (ind = ONE + labelFlag; ind < wordAmount; ind++) {
        strcat(tempLine, wordArray[ind]);
    }
    removesNewLine(tempLine);
}
int countCommas(char line[MAX_LINE_SIZE]) {
    int ind;
    int count = ZERO;
    /*  Counts how many commas are and returns the count*/
    for (ind = START; ind < strlen(line); ind++) {
        if (line[ind] == ',') {
            count++;
        }
    }
    return count;
}

/*
 * makeItNumArray - Converts a comma-separated string of numbers into an int array.
 *
 * Parameters:
 *   IntArray - Output array to be populated with parsed integers
 *   intLine  - Input string in the format "1,2,3"
 *
 * Returns:
 *   Number of elements successfully parsed, or MINUS_ONE if an invalid value is found.
 */

int makeItNumArray(int IntArray[MAX_LINE_SIZE], char intLine[MAX_LINE_SIZE]) {
    int index = ZERO;
    char *token;
    char *endPtr;
    char temp[MAX_LINE_SIZE];

    strcpy(temp, intLine);

    token = strtok(temp, ",");
    while (token != NULL) {
        removesNewLine(token);
        token = remove_white_spaces(token);
        if (*token == '\0') {
            return MINUS_ONE;
        }
        IntArray[index] = (int)strtol(token, &endPtr, 10);
        if (*endPtr != '\0' && *endPtr != '\n') {
            return MINUS_ONE;
        }
        index++;
        token = strtok(NULL, ",");
    }

    return index;
}

/*
 * searchCommand - Searches for a command by name in a commands array.
 *
 * Parameters:
 *   commands - Array of command structs to search through
 *   name     - The command name to look for
 *
 * Returns:
 *   Index of the matching command in the array, or MINUS_ONE if not found.
 */

int searchCommand(command commands[], char *name) {
    int i = ZERO;
    removesNewLine(name);
    for (i = ZERO; i < AMOUNT_OF_COMMANDS; i++) {
        if ((strcmp(name, commands[i].name)) ==
            ZERO) {   /* The name sent has the same name as one of the commands in commands array*/
            return i;
            }
    }
    return MINUS_ONE;

}

/*
 * This method does 2 tasks:
 * 1. Check if the commas and the operands are balanced
 * 2. Split the operands to different cells in commandOperands array
 *
 * Return -1 if the commas and the operands are not balanced
 * Else will return the number of operands;
 */
int lineForCommand(char *wordArray[MAX_LINE_SIZE], int labelFlag, int wordAmount,
                   char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE], int lineCounter) {
    char line[MAX_LINE_SIZE];
    int index;
    char betterLine[MAX_LINE_SIZE];
    char *token;
    int numOfOperands = ZERO;
    int numOfCommas = ZERO;

    /*
     * Make the operands one line seperated by " " or ",";
     */
    strcpy(line, wordArray[labelFlag + ONE]);
    for (index = labelFlag + TWO; index < wordAmount; index++) {
        strcat(line, wordArray[index]);
        strcat(line, " ");
    }
    /*
     * Count how many operands the line contain
     */
    numOfOperands = numOfOperandsInLine(line);
    /*
     * Count how many commas the line contain
     */
    numOfCommas = countCommas(line);

    /*
     * Checks whether the number of commas and the number of operands are balanced
     */
    if (numOfCommas + ONE != numOfOperands) {
        /* Error */
        printf("Line %d - the number of commas and number of operands is not matching accordingly\n", lineCounter);
        return MINUS_ONE;
    }

    /*
     * Removes all white spaces in the line
     */
    strcpy(betterLine, remove_white_spaces(line));

    index = ZERO;

    /*
     * Separate the line by ","
     */
    token = strtok(betterLine, ",");
    /* Checking that we did not reach the end of the string */
    while (token != NULL) {
        strcpy(commandOperands[index], token);
        token = strtok(NULL, ",");
        index++;
    }

    return index;
}


/* This method prints an error when the number of operands does not match */
void numOfOperandsNotMatch(int lineCounter) {
    printf("Line %d -The number of operands does not match ", lineCounter);
    return;
}

char *remove_white_spaces(char *str) {
    int i = ZERO, j = ZERO;
    while (str[i]) {
        if (str[i] != ' ')
            str[j++] = str[i];
        i++;
    }
    str[j] = '\0';
    return str;
}

/*
 * This method count how many operands are in the line
 * Every operands in the line separated by " " or ","
 */
int numOfOperandsInLine(char *line) {
    int index = ZERO;
    int operandCount = ZERO;
    int operandFlag = ZERO;

    while (line[index] != '\0') {
        if (operandFlag == ZERO && line[index] != ' ' &&
            line[index] != ',') /* If we reached a ' ' or ',' will increase the count*/
        {
            operandCount++;
            operandFlag = TRUE;     /* Turns on the operand flag */
        }

        if (line[index] == ' ' || line[index] == ',')/* If we reached another ' ' or ',' */
        {
            operandFlag = ZERO;/* Turns off the operand flag */
        }
        index++;
    }
    return operandCount;/* Returns operand count*/
}

/*
 * findAddressMethod - Determines the addressing method of a given operand.
 *
 * Parameters:
 *   operand       - The operand string to evaluate
 *   registerTable - Table of valid register names
 *   lineCounter   - Current line number (used for error messages)
 *
 * Returns:
 *   0 - Immediate addressing  (e.g. #5)
 *   1 - Direct addressing     (label)
 *   2 - Relative addressing   (e.g. %label)
 *   3 - Register addressing   (e.g. r1)
 *  -1 - Invalid operand
 */

int findAddressMethod(char *operand, char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], int lineCounter) {
    int numberIndex = ONE;
    int registerIndex;

    removesNewLine(operand);

    if (operand[ZERO] == '#') {
        while (operand[numberIndex] != '\0' && operand[numberIndex] != '\n') {
            if (isdigit(operand[numberIndex]) == ZERO && operand[numberIndex] != '-') {
                printf("Line %d- operand is not correct\n", lineCounter);
                return MINUS_ONE;
            }
            numberIndex++;
        }
        return ZERO;
    }

    for (registerIndex = ZERO; registerIndex < NUM_OF_REGISTER; registerIndex++) {
        if (strcmp(operand, registerTable[registerIndex]) == ZERO) {
            return THREE;
        }
    }

    if (operand[ZERO] == '%') {
        if (isalpha(operand[ONE]) == ZERO) {
            printf("Line %d- invalid relative label\n", lineCounter);
            return MINUS_ONE;
        }

        return TWO;
    }

    return ONE;
}

/*
 * changeMemoryFirstCross - Encodes a command and its operands into RAM during the first pass.
 *
 * Builds the instruction word and operand words in RAM starting at IC:
 *   Word 1 (RAM[IC])     - Instruction word: opcode (bits 8-11), funct (bits 4-7),
 *                          source addressing method (bits 2-3), dest addressing method (bits 0-1)
 *   Word 2 (RAM[IC+1])   - Source operand (if two operands)
 *   Word 3 (RAM[IC+2])   - Destination operand (if two operands)
 *
 * Addressing methods per operand:
 *   0 (Immediate)  - Stores the numeric value after '#'
 *   3 (Register)   - Stores a bitmask with the register's bit set
 *   1,2 (Label/Relative) - Set to 0, completed in second pass
 *
 * Parameters:
 *   firstOperandAddressMethod  - Addressing method of the source operand
 *   secondOperandAddressMethod - Addressing method of the destination operand
 *   RAM                        - Memory array to write into
 *   IC                         - Instruction Counter (index into RAM)
 *   commandIndex               - Index of the command in the commands array
 *   commands                   - Array of command structs
 *   numOfOperands              - Number of operands (0, 1, or 2)
 *   registerTable              - Table of valid register names
 *   commandOperands            - Parsed operand strings
 *
 * Returns: void
 */

void changeMemoryFirstCross(int firstOperandAddressMethod, int secondOperandAddressMethod, int *RAM, int IC,
                             int commandIndex, command commands[], int numOfOperands,
                             char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE],
                             char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE]) {
    int registerIndex;

    /* מילה ראשונה — 12 ביט */
    RAM[IC] = ZERO;
    RAM[IC] |= (commands[commandIndex].opcode << 8);
    RAM[IC] |= (commands[commandIndex].funct  << 4);
    RAM[IC] |= (firstOperandAddressMethod     << 2);
    RAM[IC] |= (secondOperandAddressMethod    << 0);

    if (numOfOperands == ZERO) return;

    /* ── אופרנד יחיד ── */
    if (numOfOperands == ONE) {
        IC++;
        switch (secondOperandAddressMethod) {
            case THREE: /* register: 1 << reg_num */
                RAM[IC] = ZERO;
                for (registerIndex = ZERO; registerIndex < NUM_OF_REGISTER; registerIndex++)
                    if (strcmp(commandOperands[ZERO], registerTable[registerIndex]) == ZERO)
                        RAM[IC] = (ONE << registerIndex);
                break;

            case ZERO: /* immediate: הערך אחרי '#' */
                RAM[IC] = atoi(commandOperands[ZERO] + ONE);
                if (RAM[IC] < ZERO) RAM[IC] &= 0xFFF;
                break;

            case ONE:  /* direct label  — יושלם בפאס 2 */
            case TWO:  /* relative (%L) — יושלם בפאס 2 */
                RAM[IC] = ZERO;
                break;
        }
        return;
    }

    /* ── שני אופרנדים ── */
    if (numOfOperands == TWO) {

        /* מילה שנייה — אופרנד מקור */
        IC++;
        switch (firstOperandAddressMethod) {
            case THREE: /* register */
                RAM[IC] = ZERO;
                for (registerIndex = ZERO; registerIndex < NUM_OF_REGISTER; registerIndex++)
                    if (strcmp(commandOperands[ZERO], registerTable[registerIndex]) == ZERO)
                        RAM[IC] = (ONE << registerIndex);
                break;

            case ZERO: /* immediate */
                RAM[IC] = atoi(commandOperands[ZERO] + ONE);
                if (RAM[IC] < ZERO) RAM[IC] &= 0xFFF;
                break;

            case ONE:  /* direct label  — יושלם בפאס 2 */
            case TWO:  /* relative (%L) — יושלם בפאס 2 */
                RAM[IC] = ZERO;
                break;
        }

        /* מילה שלישית — אופרנד יעד */
        IC++;
        switch (secondOperandAddressMethod) {
            case THREE: /* register */
                RAM[IC] = ZERO;
                for (registerIndex = ZERO; registerIndex < NUM_OF_REGISTER; registerIndex++)
                    if (strcmp(commandOperands[ONE], registerTable[registerIndex]) == ZERO)
                        RAM[IC] = (ONE << registerIndex);
                break;

            case ZERO: /* immediate */
                RAM[IC] = atoi(commandOperands[ONE] + ONE);
                if (RAM[IC] < ZERO) RAM[IC] &= 0xFFF;
                break;

            case ONE:  /* direct label  — יושלם בפאס 2 */
            case TWO:  /* relative (%L) — יושלם בפאס 2 */
                RAM[IC] = ZERO;
                break;
        }
    }
}
/*
 *  This method finds the label number in the table
 */
int labelNum(char *labelName, label *labels, int labelsCount) {

    int labelIndex = ZERO;
    char token[MAX_LINE_SIZE];
    if ((checkAddressMethod3Possibility(labelName)) == TRUE) {
        strcpy(token, strtok(labelName, "["));
        strcpy(labelName, token);
    }

    removesNewLine(labelName);

    for (labelIndex = ZERO; labelIndex < labelsCount; labelIndex++) {
        if ((strcmp(labelName, labels[labelIndex].name) == ZERO)) {
            return labelIndex;
        }
    }

    return MINUS_ONE;   /* Will return -1 if the word is not equal to any of the previously defined labels*/
}
/*
 *This method goes through all the chars in a string and check if they are equal to ]
 */
int checkAddressMethod3Possibility(char *operand) {
    int index = ZERO;
    while (operand[index] != '\0') {
        if (operand[index] == ']') {
            return TRUE;
        }
        index++;
    }
    return FALSE;
}

int removesNewLine(char *str) {
    int index = ZERO;
    while (str[index] != '\n' && str[index] != '\0') {
        index++;
    }
    str[index] = '\0';
    return ZERO;
}
