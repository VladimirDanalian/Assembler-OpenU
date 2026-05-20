#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_SIZE 81
#define ZERO 0
#define START 1
#define FALSE 0
#define STEP 1
#define MINUS_TWO -2
#define TRUE 1
#define SAVED_WORDS_AMOUNT 28
#define TEN 10
#define MINUS_ONE -1
#define MINUS_TWO -2
#define MAX_LABEL_SIZE 32
#define COMMAND_NAME_LENGTH 5
#define MAX_ATTRIBUTES 4
#define MAX_ATTRIBUTE_LINE_SIZE 10
#define RAM_SIZE 4097
#define ONE 1
#define EXIT 1
#define THREE 3
#define TWO 2
#define FOUR 4
#define FIVE 5
#define SIX 6
#define SEVEN 7
#define EIGHT 8
#define NINE 9
#define ELEVEN 11
#define THIRTEEN  13
#define FOURTEEN 14
#define  FIFTEEN 15

#define TURN_OFF_BITS_FOR_NEGATIVE_NUMBERS 4095
#define RAM_START 100
#define TWELVE 12
#define AMOUNT_OF_COMMANDS 16
#define NUM_OF_REGISTER 8
#define ARE_A 0  /* ביט 0 */
#define ARE_R 0  /* ביט 1 */
#define ARE_E 3  /* ביט 2 */






/* Represents the location of parts in the binary*/
#define INT_A 12

/*
 * This struct to save a macro and its contents
 */
typedef struct macro {
    char name[MAX_LINE_SIZE];
    char content[MAX_LINE_SIZE * MAX_LINE_SIZE];
} macro;
/*
 * This struct for keeping a label and their details
 */
typedef struct label {
    char name[MAX_LABEL_SIZE];
    int length;
    int address;
    char attributes[MAX_ATTRIBUTES][MAX_ATTRIBUTE_LINE_SIZE];
    int attributesCount;
} label;

/*
 * This struct for keeping a command and its numeric values
 */
typedef struct command {
    char name[COMMAND_NAME_LENGTH];
    int opcode;
    int funct;
} command;

void createExternFile(FILE *externFile, label *externs, int externsCount);

void createEntryFile(FILE *entryFile, label **labels, int labelsCount);

void createObjectFile(FILE *objectFile, int DCF, int ICF, int RAM[RAM_SIZE],label *labels, int labelCount,label *externs, int externsCount,FILE *amFile, command commands[],char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE]) ;

int firstqPASS(FILE *newfile,int RAM[RAM_SIZE], label **labels,int *labelCount,int *entryFlag,command commands[],int *extFlag,int *DCF,char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE]) ;

int secondPass(FILE *newfp, int RAM[RAM_SIZE], label **labels, int labelCount, command commands[],char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], label **externs, int *externsCount);

int splitLineIntoWords(char inputLine[MAX_LINE_SIZE], char **wordsArray);

void removeNewLine(char line[MAX_LINE_SIZE], char word[MAX_LINE_SIZE]);

int macrosDeploy(FILE *fp, FILE *newfp);

int checkLabelName(char word[], label labels[], int labelCount);

void labelWithoutColon(char labelBefore[MAX_LINE_SIZE], char labelWithoutColon[MAX_LINE_SIZE]);

int removesNewLine(char *str);

void makeIntLine(char *wordArray[MAX_LINE_SIZE], int wordAmount, char tempLine[MAX_LINE_SIZE], int labelFlag);

int countCommas(char line[MAX_LINE_SIZE]);

int makeItNumArray(int IntArray[MAX_LINE_SIZE], char intLine[MAX_LINE_SIZE]);

int searchCommand(command commands[], char *name);

void numOfOperandsNotMatch(int lineCounter);

char *remove_white_spaces(char *str);

int numOfOperandsInLine(char *line);

int lineForCommand(char *wordArray[MAX_LINE_SIZE], int labelFlag, int wordAmount,char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE], int lineCounter);

int findAddressMethod(char *operand, char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], int lineCounter);

void changeMemoryFirstCross(int firstOperandAddressMethod, int secondOperandAddressMethod, int *RAM, int IC, int commandIndex, command commands[], int numOfOperands,char registerLabel[MAX_LINE_SIZE][MAX_LINE_SIZE],char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE]);

int checkAddressMethod3Possibility(char *operand);

int labelNum(char *labelName, label *labels, int labelsCount);

