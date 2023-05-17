#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"


// define your own types and function prototypes for the symbol table(s) module below

/**Uses:
Two tables for subroutine and class scope
Each symbol table can link to attributes of symbol table
Symbol table contains name,type, kind, number of kind.
enum of kinds (field, var, arg, static) **/

typedef enum
{
    Static,
    Field,
    Arg,
    Var,
    None, //None means function (constructor, function or method). will be in type
    Class
} Kind;

typedef struct
{
    char id[128];
    char type[128];
    Kind kind;
    int varCount;  //Number of kind in scope
    int NumberVars; //Total variables inside
    int isConst;
    int nArgs;  // function only
    int repeat_Count; // stop double entry due to double pass of files
    int class_Scope; // so it only searches it's own class. 
} Symbols;

// Initalizes two arrays of symbols, class scope and local
void initSymbolTable();
// Reset Subroutine scope, always requires this as first argument
void resetLocal(char* className,char *functionName);
//append id function. (this will have name, kind and type)(function then looks at kind for scope)
void append(char* id, char* type, Kind kind);
//lookup, returns 0 if no match, looks across current class and sub scope, Note, only returns found if it has been added more than once for class scope. Use for appends.
int lookup_ID(char *id);
// count number of each kind
int countKind(Kind kind);
// free's arrays
void Free_SymbolTable(); 
// add filecount so symbols can be linked to a class
void setSymbolClassScope(int count);
// Checks all class declerations in type check, also looks if array class. 
int lookup_Class(char* id);
// Required to stop double entry of class variable (yes this is a terrible way of doing this....)
int lookup_Repeats(char *id);
// Used for expression, doesn't care if it redeclared, same class scope (i hate this layout).
int lookup_ID_Expression(char *id);
// looks for a class's index, returns -1 if class not found
int lookup_Class_Scope(char *id);
// looks across a class index, 0 if no matches
int lookup_In_Scope(char *id, int scope);
//Get offset function
int getOffset(char *id);
//get scope ie local, argument etc
//so if arg then argument, if sub then local,
char* getStack(char *id);
// if function return nArgs
int getFunctionNumberArgs_Specific(char *id,char* class);
// if function find number of variables inside it.
int getVars(char* id);
// find functions argument size
int getArgs(char* id);
// set argument size for a function
int setArgs(int nArgs, char* id); 
// gets type
char* getType(char* id);

int getGlobalVars();

void addThisArg();

char* getClass(char* id);

int checkMethod(char* id);

int lookup_ID_KindCheck(char *id,char *type);

/**Parser will do semantic checking such as will check for any undeclared or redeclared variables. Make attribute grammer for parser **/

/** Still Required: (lots of things required for semantic checks)
 *  should add attribute table to link to.
 *  method id checks
 * check return type okay
 * check types okay
 * check correct number of arguments
 * **/

#endif