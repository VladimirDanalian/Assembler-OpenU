#include "ProjectFunctions.h"

/*
 *  createExternFile  Writes all external labels and their addresses to the extern file (.ext).
 *  Parameters:
 *   externFile   - Pointer to the output .ext file
 *   externs      - Array of extern labels
 *   externsCount - Number of extern labels
 */

void createExternFile(FILE *externFile, label *externs, int externsCount) {
    int index = ZERO;

    fseek(externFile, ZERO, SEEK_SET);

    for (index = ZERO; index < externsCount; index++) {
        fprintf(externFile, "%s %04d\n", externs[index].name, externs[index].address);
    }
}
