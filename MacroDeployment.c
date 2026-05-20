#include "ProjectFunctions.h"

int macrosDeploy(FILE *fp, FILE *newfp) {
    char line[MAX_LINE_SIZE];              /* Will contain the current line */
    int flagMacro = ZERO;                  /* Represents a number indicating whether we have encountered a macro in the code so far */
    macro *macros = NULL;                  /* A pointer to an array of macros */
    int macrosCount = ZERO;                /* Number of macros encountered */
    macro *tempMacros;                     /* Will be used to make sure the realloc didn't return a NULL */
    int wordAmount = ZERO;                 /* Amount of words in the line */
    char *wordArray[MAX_LINE_SIZE];        /* Will contain an array of the line divided into words */
    char copyLine[MAX_LINE_SIZE];          /* Will be used to save the line before making changes to it */
    int macrosRunIndex = ZERO;             /* Will be used to index macros */
    int endmFlag = ZERO;                   /* Indicates whether we encountered an end of macro in the line */
    int currentIndex = ZERO;               /* Indicates an index until we reach end of line */
    char newWord[MAX_LINE_SIZE];

    /*
     * Opens the file
     * Gets his Macros
     * Copies the content to a new file and changes the macro contents
     */

    while (fgets(line, MAX_LINE_SIZE, fp)) { /* Reads a line from the file */

        strcpy(copyLine, line);  /* Save the line into a var */

        /*
         * Separate the line into words separated by spaces and insert them into the array
         */
        wordAmount = splitLineIntoWords(line, wordArray);

        if (wordAmount == ZERO) {
            fputs(copyLine, newfp);
            continue;
        }

        /*
         * If the first word is mcro then it's a start of mcro
         */
        if (strcmp(wordArray[ZERO], "mcro") == ZERO) {

            endmFlag = ZERO;
            macrosCount++;

            /* If this is the first macro observed in the texts */
            if (flagMacro == ZERO) {
                flagMacro = START;
                macros = (macro *) malloc(sizeof(macro)); /* Dynamically allocates an array of macros */
                if (macros == NULL) {
                    puts("wasn't able to dynamically allocate an array, will stop the program from running");
                    return FALSE;
                }
            } else {
                tempMacros = (macro *) realloc(macros, macrosCount * sizeof(macro));
                if (tempMacros != NULL) {
                    macros = tempMacros;
                } else {
                    puts("wasn't able to dynamically allocate an array, will stop the program from running");
                    free(macros);
                    return FALSE;
                }
            }

            /*
             * Inserting the macro's name into struct
             */
            strcpy(macros[macrosCount - STEP].name, wordArray[START]);
            macros[macrosCount - STEP].content[ZERO] = '\0';

            /*
             * We will add lines until we reached the end of the macro
             */
            while (endmFlag != START && fgets(line, MAX_LINE_SIZE, fp)) {

                strcpy(copyLine, line);
                wordAmount = splitLineIntoWords(line, wordArray);

                if (wordAmount == ZERO) {
                    strcat(macros[macrosCount - STEP].content, copyLine);
                    continue;
                }

                currentIndex = ZERO;
                removeNewLine(wordArray[currentIndex], newWord);

                if (strcmp(newWord, "mcroend") == ZERO) {
                    endmFlag = START;
                } else {
                    strcat(macros[macrosCount - STEP].content, copyLine);
                }
            }
        }

        else {
            for (macrosRunIndex = ZERO; macrosRunIndex < macrosCount; macrosRunIndex++) {
                if (strcmp(macros[macrosRunIndex].name, wordArray[ZERO]) == ZERO) {
                    fputs(macros[macrosRunIndex].content, newfp);  /* Copy the macro contents to the new file */
                    break;
                }
            }

            /*
             * If it's not a macro name then we will print the line to the file
             */
            if (macrosRunIndex == macrosCount) {
                fputs(copyLine, newfp); /* Will copy the line to the new file */
            }
        }
    }

    if (macros != NULL) {
        free(macros);
    }

    return TRUE;
}
