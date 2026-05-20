#include "ProjectFunctions.h"

/*
 * createObjectFile - Writes the assembled output to the object file (.ob).
 *
 * This function performs two passes over the data:
 *
 *  Pass 1 - Scan the .am source file to build a word-type map:
 *     For every RAM cell that holds a label reference, records:
 *       - Which label is referenced (index into labels[])
 *       - Whether it is a local reference (code/data/entry) -> R
 *       - Whether it is an external reference (extern)      -> E
 *       - Whether it is a relative reference (%label)       -> A
 *
 *  Pass 2 - Write the object file:
 *     Line 1 : <code size> <data size>
 *     Rest   : <address> <value (hex)> <ARE flag>
 *
 * ARE flags:
 *   A - Absolute    (immediate, register, or relative addressing)
 *   R - Relocatable (direct reference to a local label: code/data/entry)
 *   E - External    (reference to an extern label)
 *
 * Addressing methods:
 *   0 - Immediate  (#n)     -> A
 *   1 - Direct     (label)  -> R if local, E if extern
 *   2 - Relative   (%label) -> A (distance, not address)
 *   3 - Register   (rN)     -> A
 *
 * Parameters:
 *   objectFile   - Pointer to the output .ob file
 *   DCF          - Number of data words
 *   ICF          - Address after last instruction (end of code segment)
 *   RAM          - Encoded memory array
 *   labels       - Labels table (built during first pass)
 *   labelCount   - Number of labels in the labels table
 *   externs      - External label references (built during second pass)
 *   externsCount - Number of external label references
 *   amFile       - Pointer to the macro-expanded source file (.am)
 *   commands     - Array of command structs (name, opcode, funct)
 *   registerTable- Table of valid register names (r0-r7)
 */
void createObjectFile(FILE *objectFile, int DCF, int ICF, int RAM[RAM_SIZE],
                      label *labels, int labelCount,
                      label *externs, int externsCount,
                      FILE *amFile, command commands[],
                      char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE]) {

    int index = ZERO;
    int labelIndex = ZERO;
    int attrIndex = ZERO;
    int value = ZERO;
    char areChar[4];
    const char *word = NULL;

    /* wordLabel[i]   - index into labels[] of the label referenced at RAM[i], or -1 */
    int wordLabel[RAM_SIZE];
    /* wordIsLocal[i] - 1 if RAM[i] holds a direct reference to a local label (-> R) */
    int wordIsLocal[RAM_SIZE];

    char line[MAX_LINE_SIZE];
    char *wordArray[MAX_LINE_SIZE];
    int  wordAmount, labelFlag, commandIndex, opCount;
    char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE];
    char labelToken[MAX_LINE_SIZE];
    int  method, lblIdx, operandIndex;
    int  RAMIndex = RAM_START;  /* Tracks current RAM address while scanning */

    /* Initialize word-type map */
    for (index = ZERO; index < RAM_SIZE; index++) {
        wordLabel[index]   = MINUS_ONE;
        wordIsLocal[index] = ZERO;
    }

    /* ------------------------------------------------------------------ */
    /* Pass 1: Scan .am file and build word-type map                       */
    /* ------------------------------------------------------------------ */
    fseek(amFile, ZERO, SEEK_SET);

    while (fgets(line, MAX_LINE_SIZE, amFile)) {
        wordAmount = splitLineIntoWords(line, wordArray);
        if (wordAmount == ZERO) continue;
        if (wordArray[ZERO][ZERO] == ';') continue;         /* skip comments */
        if (strcmp(wordArray[ZERO], "\n") == ZERO) continue; /* skip blank lines */

        /* Detect label prefix */
        labelFlag = ZERO;
        if (wordArray[ZERO][strlen(wordArray[ZERO]) - ONE] == ':')
            labelFlag = ONE;

        word = wordArray[labelFlag];

        /* Skip directives — they do not produce instruction words */
        if (strcmp(word, ".data")   == ZERO ||
            strcmp(word, ".string") == ZERO ||
            strcmp(word, ".entry")  == ZERO ||
            strcmp(word, ".extern") == ZERO)
            continue;

        /* Skip unknown commands */
        commandIndex = searchCommand(commands, (char *)word);
        if (commandIndex == MINUS_ONE) continue;

        /* 0-operand commands: advance past instruction word only */
        if (strcmp(word, "rts")  == ZERO ||
            strcmp(word, "stop") == ZERO) {
            RAMIndex++;
            continue;
        }

        /* Parse operands */
        opCount = lineForCommand(wordArray, labelFlag,
                                 wordAmount, commandOperands, ZERO);
        if (opCount <= ZERO) { RAMIndex++; continue; }

        RAMIndex++;   /* skip instruction word */

        /* Classify each operand and record in word-type map */
        for (operandIndex = ZERO; operandIndex < opCount; operandIndex++) {
            method = findAddressMethod(commandOperands[operandIndex],
                                       registerTable, ZERO);

            if (method == ONE) {
                /* Direct label reference -> R if local, E if extern */
                strcpy(labelToken, commandOperands[operandIndex]);
                removesNewLine(labelToken);
                lblIdx = labelNum(labelToken, labels, labelCount);
                if (lblIdx != MINUS_ONE) {
                    wordLabel[RAMIndex] = lblIdx;
                    wordIsLocal[RAMIndex] =
                        (strcmp(labels[lblIdx].attributes[ZERO], "extern") != ZERO)
                        ? ONE : ZERO;
                }
                RAMIndex++;

            } else if (method == TWO) {
                /* Relative label reference (%label) -> always A
                 * The value stored is a distance, not an address */
                strcpy(labelToken, commandOperands[operandIndex] + ONE);
                removesNewLine(labelToken);
                lblIdx = labelNum(labelToken, labels, labelCount);
                if (lblIdx != MINUS_ONE) {
                    wordLabel[RAMIndex] = lblIdx;
                    wordIsLocal[RAMIndex] = ZERO;
                }
                RAMIndex++;

            } else {
                /* Immediate (#n) or register (rN) -> always A */
                RAMIndex++;
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /* Pass 2: Write object file                                           */
    /* ------------------------------------------------------------------ */
    fseek(objectFile, ZERO, SEEK_SET);
    fprintf(objectFile, "%d %d\n", ICF - RAM_START, DCF);

    for (index = RAM_START; index < ICF + DCF; index++) {
        value = RAM[index];
        strcpy(areChar, "A");

        /* E — address holds a reference to an extern label */
        for (labelIndex = ZERO; labelIndex < externsCount; labelIndex++) {
            if (externs[labelIndex].address == index) {
                strcpy(areChar, "E");
            }
        }

        /* R — address holds a direct reference to a local label */
        if (wordIsLocal[index] == ONE) {
            lblIdx = wordLabel[index];
            if (lblIdx != MINUS_ONE) {
                for (attrIndex = ZERO;
                     attrIndex < labels[lblIdx].attributesCount;
                     attrIndex++) {
                    if (strcmp(labels[lblIdx].attributes[attrIndex], "code")  == ZERO ||
                        strcmp(labels[lblIdx].attributes[attrIndex], "data")  == ZERO ||
                        strcmp(labels[lblIdx].attributes[attrIndex], "entry") == ZERO) {
                        strcpy(areChar, "R");
                    }
                }
            }
        }

        fprintf(objectFile, "%04d %03X %s\n", index, value, areChar);
    }
}
