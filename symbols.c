
/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Matthew Poole
Student ID: 201298590
Email: sc19map@leeds.ac.uk
Date Work Commenced: 12/04/2021
*************************************************************************/

#include "symbols.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Symbols *symbol_Class;
Symbols *symbol_Sub;
int size_Class, size_Sub;
int classScope;

void initSymbolTable()
{
    classScope = 0;
    size_Class = 0;
    size_Sub = 0;
    symbol_Class = (Symbols *)malloc(1 * sizeof(Symbols));
    symbol_Sub = (Symbols *)malloc(1 * sizeof(Symbols));
}

void setSymbolClassScope(int count)
{
    classScope = count;
}

void Free_SymbolTable()
{
    free(symbol_Class);
    free(symbol_Sub);
}

void resetLocal(char *className, char *functionName)
{
    //First set functionName insideVars (this is just max varCount).
    int total = 0;
    for (int i = 0; i < size_Sub; i++)
    {
        if (symbol_Sub[i].kind != Arg)
        {
            total++;
        }
    }
    //Now set total variables for that function.
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, functionName) == 0))
        {
            symbol_Class[i].NumberVars = total;
        }
    }

    free(symbol_Sub);
    //classScope = 0;
    size_Sub = 0;
    symbol_Sub = (Symbols *)malloc(1 * sizeof(Symbols));
}

void addThisArg()
{
    //Add this
    size_Sub += 1;
    symbol_Sub = (Symbols *)realloc(symbol_Sub, size_Sub * sizeof(Symbols));
    strcpy(symbol_Sub[size_Sub - 1].id, "this");

    for (int i = 0; i < size_Class; i++)
        {
            if (symbol_Class[i].class_Scope == classScope && symbol_Class[i].kind == Class)
            {
                strcpy(symbol_Sub[size_Sub - 1].type, symbol_Class[i].type);
            }
        }

    symbol_Sub[size_Sub - 1].kind = Arg;
    symbol_Sub[size_Sub - 1].varCount = 0;
    symbol_Sub[size_Sub - 1].class_Scope = classScope;

}

void append(char *id, char *type, Kind kind)
{
    if (kind == Static || kind == Field || kind == Class || kind == None)
    {
        // Class Scope
        // As a double pass is required, can't enter class variable twice.
        int check = lookup_Repeats(id);
        if (check == 0)
        {
            size_Class += 1;
            // Copy variables and initalzie all symbol attributes
            symbol_Class = (Symbols *)realloc(symbol_Class, size_Class * sizeof(Symbols));
            strcpy(symbol_Class[size_Class - 1].id, id);
            strcpy(symbol_Class[size_Class - 1].type, type);
            symbol_Class[size_Class - 1].kind = kind;
            symbol_Class[size_Class - 1].isConst = 0;
            symbol_Class[size_Class - 1].class_Scope = classScope;
            symbol_Class[size_Class - 1].repeat_Count = 0;
            symbol_Class[size_Class - 1].varCount = countKind(kind) - 1;
        }
    }
    else
    {
        //Sub Scope
        size_Sub += 1;
        // Copy variables and initalzie all symbol attributes
        symbol_Sub = (Symbols *)realloc(symbol_Sub, size_Sub * sizeof(Symbols));

        strcpy(symbol_Sub[size_Sub - 1].id, id);
        strcpy(symbol_Sub[size_Sub - 1].type, type);
        symbol_Sub[size_Sub - 1].kind = kind;
        symbol_Sub[size_Sub - 1].isConst = 0;
        symbol_Sub[size_Sub - 1].class_Scope = classScope;
        symbol_Sub[size_Sub - 1].repeat_Count = 0;
        symbol_Sub[size_Sub - 1].varCount = countKind(kind) - 1;
    }
}

int countKind(Kind kind)
{
    int count = 0;
    if (kind == Static || kind == Field || kind == None || kind == Class)
    {
        for (int i = 0; i < size_Class; i++)
        {
            if (symbol_Class[i].kind == kind && symbol_Class[i].class_Scope == classScope)
            {
                count++;
            }
        }
    }
    else
    {
        for (int i = 0; i < size_Sub; i++)
        {
            if (symbol_Sub[i].kind == kind)
            {
                count++;
            }
        }
    }
    return count;
}

int lookup_ID(char *id)
{
    int count = 0;
    //search all
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0) && classScope == symbol_Sub[i].class_Scope)
        {
            count += 1;
        }
    }
    //This loop needs to check reCount to, only add if > 2.
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && classScope == symbol_Class[i].class_Scope)
        {
            // Only return found if already added twice in theory (tho it hasn't just attempted to)
            if (symbol_Class[i].repeat_Count > 1)
            {
                count += 1;
            }
        }
    }
    return count;
}

int lookup_ID_KindCheck(char *id,char *type)
{
    int count = 0;
    //search all
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0) && classScope == symbol_Sub[i].class_Scope)
        {
            count += 1;
        }
    }
    //This loop needs to check reCount to, only add if > 2.
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && classScope == symbol_Class[i].class_Scope && strcmp(symbol_Class[i].type,type)==0)
        {
            // Only return found if already added twice in theory (tho it hasn't just attempted to)
            if (symbol_Class[i].repeat_Count > 1)
            {
                count += 1;
            }
        }
    }
    return count;
}

int lookup_ID_Expression(char *id)
{
    int count = 0;
    //search all
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0) && classScope == symbol_Sub[i].class_Scope)
        {
            count += 1;
        }
    }
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && classScope == symbol_Class[i].class_Scope)
        {
            count += 1;
        }
    }
    return count;
}

int lookup_Repeats(char *id)
{
    int count = 0;
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && classScope == symbol_Class[i].class_Scope)
        {
            symbol_Class[i].repeat_Count++;
            count += 1;
        }
    }
    return count;
}

int lookup_Class(char *id)
{
    int count = 0;
    for (int i = 0; i < size_Class; i++)
    {
        if (((strcmp(symbol_Class[i].id, id) == 0) && symbol_Class[i].kind == Class))
        {
            count += 1;
        }
    }
    return count;
}

int lookup_Class_Scope(char *id)
{
    int scope = -1;
    char type_Local[64];
    //First find type of id, then find index of that type.
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0))
        {
            strcpy(type_Local, symbol_Class[i].type);
        }
    }
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0))
        {
            strcpy(type_Local, symbol_Sub[i].type);
        }
    }

    //Now find type decleration for the scope index.
    for (int i = 0; i < size_Class; i++)
    {
        if (((strcmp(symbol_Class[i].id, type_Local) == 0) && symbol_Class[i].kind == Class))
        {
            scope = symbol_Class[i].class_Scope;
        }
    }

    return scope;
}

int lookup_In_Scope(char *id, int scope)
{
    int count = 0;
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && symbol_Class[i].class_Scope == scope)
        {
            count += 1;
        }
    }
    return count;
}

//Functions added for code generation

//Get offset function
int getOffset(char *id)
{
    int result = -1;
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0))
        {
            result = symbol_Sub[i].varCount;
            return result;
        }
    }
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0)&&symbol_Class[i].class_Scope==classScope)
        {
            result = symbol_Class[i].varCount;
            return result;
        }
    }
    return result;
}
//get scope ie local, argument etc
//so if arg then argument, if sub then local,
//return NULL if not found
char *getStack(char *id)
{
    char *stack;
    stack = (char *)malloc(16 * sizeof(char));
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0))
        {
            if (symbol_Sub[i].kind == Arg)
            {
                strcpy(stack, "argument");
                return (stack);
            }
            else
            {
                strcpy(stack, "local");
                return (stack);
            }
        }
    }
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0))
        {
            if (symbol_Class[i].kind == Static)
            {
                strcpy(stack, "static");
                return (stack);
            }
            else
            {
                //This is when it requires some sort of memory alloc.....
                strcpy(stack, "this");
                return (stack);
            }
        }
    }
    //Not found
    strcpy(stack, "NULL");
    return (stack);
}

// if function return nArgs
int getFunctionNumberArgs_Specific(char *id,char* class)
{
    if(strcmp(id,"new")==0 && strcmp(class,"SquareGame")==0){
        int test = 0; 
    }

    int scope = -1; 
    //First find scope of class
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, class) == 0)){
            scope = symbol_Class[i].class_Scope; 
        }
    }

    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0) && symbol_Class[i].kind == None && symbol_Class[i].class_Scope == scope)
        {
            return symbol_Class[i].nArgs;
        }
    }
    
    return -1;
}

// if function find number of variables inside it.
int getVars(char *id)
{
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0))
        {
            return symbol_Class[i].NumberVars;
        }
    }
}

// find functions argument size
int getArgs(char *id)
{
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0)&& symbol_Class[i].kind == None )
        {
            return symbol_Class[i].nArgs;
        }
    }
    return -1;
}

// set argument size for a function
int setArgs(int nArgs, char *id)
{
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0)&&symbol_Class[i].class_Scope==classScope)
        {
            symbol_Class[i].nArgs = nArgs;
        }
    }
}

//return null if none found
char *getType(char *id)
{
    char *typeName;
    typeName = (char *)malloc(16 * sizeof(char));
    typeName[0] = -1;
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0)&&symbol_Class[i].class_Scope==classScope)
        {
            strcpy(typeName, symbol_Class[i].type);
            return typeName;
        }
    }
    for (int i = 0; i < size_Class; i++)
    {
        if ((strcmp(symbol_Class[i].id, id) == 0))
        {
            strcpy(typeName, symbol_Class[i].type);
            return typeName;
        }
    }
    for (int i = 0; i < size_Sub; i++)
    {
        if ((strcmp(symbol_Sub[i].id, id) == 0))
        {
            strcpy(typeName, symbol_Sub[i].type);
            return typeName;
        }
    }
    strcpy(typeName, "NULL");
    return typeName;
}

int getGlobalVars()
{
    int total = 0;
    for (int i = 0; i < size_Class; i++)
    {
        if (symbol_Class[i].class_Scope == classScope && symbol_Class[i].kind == Field)
        {
            total++;
        }
    }
    return total;
}

char* getClass(char* id){
    int localScope = -1;
    char* className = (char*)malloc(30*sizeof(char));
    for (int i = 0; i < size_Class; i++)
        {
            if (strcmp(symbol_Class[i].id,id)==0)
            {
                localScope = symbol_Class[i].class_Scope; 
            }
        }
    for (int i = 0; i < size_Class; i++)
        {
            if (symbol_Class[i].class_Scope == localScope && symbol_Class[i].kind == Class)
            {
                strcpy(className, symbol_Class[i].type);
            }
        }
    return className; 
}

int checkMethod(char* id){
    for (int i = 0; i < size_Class; i++)
        {
            if (strcmp(symbol_Class[i].id,id)==0 && strcmp(symbol_Class[i].type,"method")==0)
            {
                return 1;
            }
        }
    return 0; 
}

//checks current scope against the found field scope if diff