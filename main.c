#include "ProjectFunctions.h"

/*
 * main - Entry point of the assembler.
 *
 * Runs the full assembly pipeline:
 *   1. Macro deployment     (input.as -> output.am)
 *   2. First pass           (build labels table, encode instructions into RAM)
 *   3. Second pass          (resolve labels and external references)
 *   4. Output files         (output.ob, output.ent, output.ext)
 *
 * Returns TRUE on success, FALSE on any error.
 */
int main() {
    FILE *fp       = NULL;  /* Input source file (.as) */
    FILE *newfp    = NULL;  /* Macro-expanded file (.am) */
    FILE *objectFile = NULL; /* Output object file (.ob) */
    FILE *entryFile  = NULL; /* Output entry file (.ent) */
    FILE *externFile = NULL; /* Output extern file (.ext) */

    int RAM[RAM_SIZE];       /* Main memory array for encoded instructions and data */
    label *labels  = NULL;   /* Dynamic array of all defined labels */
    int labelCount = ZERO;   /* Number of labels in the labels table */
    int entryFlag  = FALSE;  /* Set to TRUE if any .entry directive is found */
    int extFlag    = FALSE;  /* Set to TRUE if any .extern directive is found */
    int DCF        = ZERO;   /* Final Data Counter — number of data words */
    int ICF        = ZERO;   /* Final Instruction Counter — address after last instruction */
    int index      = ZERO;   /* General-purpose loop index */

    label *externs    = NULL; /* Dynamic array of external label references */
    int externsCount  = ZERO; /* Number of external label references */
    int secondPassResult = ZERO; /* Return value of the second pass */

    /* Command table: name, opcode, funct */
    command commands[AMOUNT_OF_COMMANDS] = {
        {"mov",  0,  0},
        {"cmp",  1,  0},
        {"add",  2,  10},
        {"sub",  2,  11},
        {"lea",  4,  0},
        {"clr",  5,  10},
        {"not",  5,  11},
        {"inc",  5,  12},
        {"dec",  5,  13},
        {"jmp",  9,  10},
        {"bne",  9,  11},
        {"jsr",  9,  12},
        {"red",  12, 0},
        {"prn",  13, 0},
        {"rts",  14, 0},
        {"stop", 15, 0}
    };

    /* Register name table: r0-r7 */
    char registerTable[NUM_OF_REGISTER][MAX_LINE_SIZE] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    };

    /* Initialize all RAM cells to zero */
    for (index = ZERO; index < RAM_SIZE; index++) RAM[index] = ZERO;
    index = ZERO;

    /* --- Stage 1: Macro Deployment --- */
    fp = fopen("input.as", "r");
    if (fp == NULL) {
        printf("Error - could not open input.as\n");
        return FALSE;
    }

    newfp = fopen("output.am", "w");  /* Write macro-expanded output */
    if (newfp == NULL) {
        printf("Error - could not open output.am\n");
        fclose(fp);
        return FALSE;
    }

    if (macrosDeploy(fp, newfp) == FALSE) {
        printf("Error - macro deployment failed\n");
        fclose(fp);
        fclose(newfp);
        return FALSE;
    }

    fclose(fp);
    fclose(newfp);

    /* --- Stage 2: First Pass --- */
    newfp = fopen("output.am", "r");  /* Read macro-expanded file */
    if (newfp == NULL) {
        printf("Error - could not open output.am for first pass\n");
        return FALSE;
    }

    ICF = firstqPASS(newfp, RAM, &labels, &labelCount, &entryFlag, commands, &extFlag, &DCF, registerTable);
    if (ICF == FALSE) {
        printf("Error - first pass failed\n");
        fclose(newfp);
        if (labels != NULL) free(labels);
        return FALSE;
    }


    /* --- Stage 3: Second Pass --- */
    secondPassResult = secondPass(newfp, RAM, &labels, labelCount, commands, registerTable, &externs, &externsCount);
    fclose(newfp);

    if (secondPassResult == MINUS_ONE) {
        printf("Error - second pass failed\n");
        if (labels  != NULL) free(labels);
        if (externs != NULL) free(externs);
        return FALSE;
    }


    /* --- Stage 4: Create Object File (.ob) --- */
    newfp = fopen("output.am", "r");  /* פתח מחדש */
    if (newfp == NULL) {
        printf("Error - could not open output.am for object file\n");
        return FALSE;
    }

    objectFile = fopen("output.ob", "w");
    if (objectFile == NULL) {
        printf("Error - could not open output.ob\n");
    } else {
        createObjectFile(objectFile, DCF, ICF, RAM, labels, labelCount,
                 externs, externsCount,
                 newfp, commands, registerTable);
        fclose(objectFile);
    }

    fclose(newfp);
    /* --- Stage 5: Create Entry File (.ent) --- */
    if (entryFlag == TRUE) {
        entryFile = fopen("output.ent", "w");
        if (entryFile == NULL) {
            printf("Error - could not open output.ent\n");
        } else {
            createEntryFile(entryFile, &labels, labelCount);
            fclose(entryFile);
        }
    }

    /* --- Stage 6: Create Extern File (.ext) --- */
    if (extFlag == TRUE && externsCount > ZERO) {
        externFile = fopen("output.ext", "w");
        if (externFile == NULL) {
            printf("Error - could not open output.ext\n");
        } else {
            createExternFile(externFile, externs, externsCount);
            fclose(externFile);
        }
    }

    /* Free dynamically allocated memory */
    if (labels  != NULL) free(labels);
    if (externs != NULL) free(externs);

    return TRUE;
}
