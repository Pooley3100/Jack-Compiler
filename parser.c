#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"

// you can declare prototypes of parser functions below
ParserInfo exitP(SyntaxErrors err, ParserInfo pi);
ParserInfo GetNextToken_Parse(ParserInfo pi);
ParserInfo PeekNextToken_Parse(ParserInfo pi);
ParserInfo operand(ParserInfo pi);
ParserInfo factor(ParserInfo pi);
ParserInfo term(ParserInfo pi);
ParserInfo ArithmeticExpression(ParserInfo pi);
ParserInfo relationalExpression(ParserInfo pi);
ParserInfo expression(ParserInfo pi);
ParserInfo returnStatemnt(ParserInfo pi);
ParserInfo expressionList(ParserInfo pi);
ParserInfo subroutineCall(ParserInfo pi);
ParserInfo doStatement(ParserInfo pi);
ParserInfo whileStatement(ParserInfo pi);
ParserInfo ifStatement(ParserInfo pi);
ParserInfo letStatemnt(ParserInfo pi);
ParserInfo varDeclarStatement(ParserInfo pi);
ParserInfo statement(ParserInfo pi);
ParserInfo subroutineBody(ParserInfo pi);
int paramList(ParserInfo pi);
ParserInfo subroutineDeclar(ParserInfo pi);
char *type(ParserInfo pi);
ParserInfo classVarDeclar(ParserInfo pi);
ParserInfo memberDeclar(ParserInfo pi);
ParserInfo classDeclar(ParserInfo pi);
ParserInfo pi_Master;
char className[128];
FILE *vm_FP;   //Global file pointer to vm file.
int loopCount; //Used to add to label names.

//Function to add to vm file
void WriteVM(char *entry)
{
    //First check one pass of files has occured
    //Also check if library file to not write, not really necessary??
    if (secondrun == 2)
    {
        fprintf(vm_FP, "%s\n", entry);
    }
}

/** Parser will use attribute grammar to add to symbol table **/

//Should have used match function...

// On each exitP must check if error already set.
ParserInfo exitP(SyntaxErrors err, ParserInfo pi)
{
    if (pi_Master.er == none)
    {
        pi_Master.er = err;
        pi_Master.tk = pi.tk;
    }
    pi.er = err;
    return pi;
}

//Checks no lexer errors and to not change if error set
ParserInfo GetNextToken_Parse(ParserInfo pi)
{
    if (pi.tk.tp != EOFile)
    {
        pi.tk = GetNextToken();
        if (pi.tk.tp == ERR)
        {
            pi = exitP(lexerErr, pi);
        }
    }
    if (pi.tk.tp == EOFile && pi_Master.er == none)
    {
        //Not sure if I should have this, just to stop an infini loop.
        pi_Master.er = syntaxError; //not really right error
        pi_Master.tk = pi.tk;
    }
    return pi;
}

//not to change if error set
ParserInfo PeekNextToken_Parse(ParserInfo pi)
{

    pi.tk = PeekNextToken();

    return pi;
}

ParserInfo operand(ParserInfo pi)
{
    pi = PeekNextToken_Parse(pi);
    if (pi.tk.tp == INT)
    {
        pi = GetNextToken_Parse(pi);
        //
        char buffer[400];
        sprintf(buffer, "%s%s", "push constant ", pi.tk.lx);
        WriteVM(buffer);
    }
    else if (pi.tk.tp == ID)
    {
        //variables to store id,function, or Object, then object function
        char idName[100];
        char subIdName[100];
        subIdName[0] = -1;
        char Buffered[400];
        Buffered[0] = -1;
        int arrayCheck = 0;
        int functionCheck = 0;

        pi = GetNextToken_Parse(pi);
        //

        strcpy(idName, pi.tk.lx);

        //Check ID declared.
        int check = lookup_ID_Expression(pi.tk.lx);

        int checkClass = lookup_Class(pi.tk.lx);
        int scope = -1;
        // can either be in class or another class.
        if (check == 0 && checkClass == 0)
        {
            pi = exitP(undecIdentifier, pi);
        }
        else //both a class and exists in a scope
        {

            // if class find scope
            scope = lookup_Class_Scope(pi.tk.lx);
        }

        //Now can be 0/1 .id , 0/1 [expression] or (expressionlist)
        pi = PeekNextToken_Parse(pi);
        if (strcmp(pi.tk.lx, ".") == 0)
        {
            pi = GetNextToken_Parse(pi);
            //
            pi = GetNextToken_Parse(pi);
            strcpy(subIdName, pi.tk.lx);

            //If this is object variable this has to be push now as argument 0
            char typeObject1[100];
            strcpy(typeObject1, getType(idName));
            if (strcmp(typeObject1, idName) != 0)
            {

                sprintf(Buffered, "push %s %d", getStack(idName), getOffset(idName));
                //WriteVM(Buffered);
            }
            //END OF FIRST ARGUMENT ADDITION

            if (pi.tk.tp == ID)
            {
                //
                // can only be class scope ??
                int check = lookup_In_Scope(pi.tk.lx, scope);
                if (check <= 0)
                {
                    pi = exitP(undecIdentifier, pi);
                }
                else
                {
                }
            }
            else
            {
                pi = exitP(idExpected, pi);
            }
            pi = PeekNextToken_Parse(pi);
        }

        if (strcmp(pi.tk.lx, "[") == 0)
        {
            pi = GetNextToken_Parse(pi);
            //
            pi = expression(pi);
            pi = GetNextToken_Parse(pi);
            if (strcmp(pi.tk.lx, "]") == 0)
            {
                //
            }
            else
            {
                pi = exitP(closeBracketExpected, pi);
            }

            arrayCheck = 1;
            char arrayBuffer[100];
            sprintf(arrayBuffer, "push %s %d", getStack(idName), getOffset(idName));
            WriteVM(arrayBuffer);
            WriteVM("add");
            WriteVM("pop pointer 1");
            WriteVM("push that 0");
        }
        else if (strcmp(pi.tk.lx, "(") == 0)
        {
            functionCheck = 1;
            pi = GetNextToken_Parse(pi);
            //
            //Object Argument
            if (Buffered[0] != -1)
            {
                WriteVM(Buffered);
            }
            pi = expressionList(pi);
            pi = GetNextToken_Parse(pi);
            if (strcmp(pi.tk.lx, ")") == 0)
            {
                //
            }
            else
            {
                pi = exitP(closeParenExpected, pi);
            }
        }

        //Now need to push the id.
        //first find if function or if sub is function.
        //then find which stack, ie local, arg, this
        //stack check
        if (subIdName[0] != -1 && functionCheck)
        {
            char buffer[300];

            //int checkF = getFunctionNumberArgs_Specific(subIdName,lookup_In_Scope(subIdName,lookup_Class_Scope(idName)));
            int checkF = getFunctionNumberArgs_Specific(subIdName, getType(idName));
            //check if function
            if (checkF >= 0)
            {
                char typeObject[100];
                strcpy(typeObject, getType(idName));
                sprintf(buffer, "call %s.%s %d", typeObject, subIdName, checkF);
                //Check if method or function
                int checkM = checkMethod(subIdName);
                if (checkM)
                {
                    //THIS IS NO LONGER NECESSARY, FIRST ARGUMENT HAS BEEN ADDED
                    if (strcmp(typeObject, idName) == 0)
                    {
                        WriteVM("push pointer 0");
                    }
                    else
                    {
                        char bufferExtra[300];
                        sprintf(bufferExtra, "push %s %d", getStack(idName), getOffset(idName));
                        //WriteVM(bufferExtra);
                    }
                }
            }
            else
            {
                //Don't think this can happen as you can't access directly non method
            }
            WriteVM(buffer);
        }
        else if (!arrayCheck)
        {
            char buffer[300];

            //check if function
            int checkF = getArgs(idName);
            if (checkF > 0 && functionCheck)
            {
                char LocalClassName[100];
                strcpy(LocalClassName, getClass(idName));
                sprintf(buffer, "%s %s.%s %d", "call", LocalClassName, idName, checkF);
            }
            else
            {

                char stackType[50];
                strcpy(stackType, getStack(idName));
                int offset = getOffset(idName);

                sprintf(buffer, "%s %s %d", "push", stackType, offset);
            }
            WriteVM(buffer);
        }
    }
    else if (strcmp(pi.tk.lx, "(") == 0)
    {
        pi = GetNextToken_Parse(pi);
        //
        pi = expression(pi);
        pi = GetNextToken_Parse(pi);
        if (strcmp(pi.tk.lx, ")") == 0)
        {
            //
        }
        else
        {
            pi = exitP(closeParenExpected, pi);
        }
    }
    else if (pi.tk.tp == STRING)
    {
        //
        pi = GetNextToken_Parse(pi);
        // String requires string library, finding length then new, then push char then appendchar

        int length = strlen(pi.tk.lx);
        char buffer[400];
        sprintf(buffer, "%s%d", "push constant ", length);
        WriteVM(buffer);
        WriteVM("call String.new 1");
        for (int i = 0; i < length; i++)
        {
            sprintf(buffer, "%s%d", "push constant ", pi.tk.lx[i]);
            WriteVM(buffer);
            WriteVM("call String.appendChar 2");
        }
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "true") == 0)
    {
        //
        pi = GetNextToken_Parse(pi);
        WriteVM("push constant 0");
        WriteVM("not");
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "false") == 0)
    {
        //
        pi = GetNextToken_Parse(pi);
        WriteVM("push constant 0");
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "null") == 0)
    {
        //
        pi = GetNextToken_Parse(pi);
        //Not sure what this is supposed to be.
        WriteVM("push constant 0");
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "this") == 0)
    {
        //
        pi = GetNextToken_Parse(pi);
        WriteVM("push pointer 0");
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }
    return pi;
}

ParserInfo factor(ParserInfo pi)
{
    int check = 0;
    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "-") == 0)
    {
        //
        check = 2; 
        pi = GetNextToken_Parse(pi);
    }
    else if (strcmp(pi.tk.lx, "~") == 0)
    {
        //
        pi = GetNextToken_Parse(pi);
        check = 1;
    }

    pi = operand(pi);

    if (check == 1)
    {
        WriteVM("not");
    }
    else if(check == 2)
    {
        //not sure what this is for
        WriteVM("neg");
    }
    return pi;
}

ParserInfo term(ParserInfo pi)
{
    pi = factor(pi);
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "*") == 0 || (strcmp(pi.tk.lx, "/") == 0))
    {
        int operation = 0;
        if (strcmp(pi.tk.lx, "*") == 0)
        {
            operation = 1;
        }
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //absord
        pi = GetNextToken_Parse(pi);
        //
        pi = factor(pi);

        //WRITE VM operation
        if (operation == 0)
        {
            WriteVM("call Math.divide 2");
        }
        else
        {
            WriteVM("call Math.multiply 2");
        }

        pi = PeekNextToken_Parse(pi);
    }
    return pi;
}

ParserInfo ArithmeticExpression(ParserInfo pi)
{
    pi = term(pi);
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "+") == 0 || (strcmp(pi.tk.lx, "-") == 0))
    {
        int operation = 0;
        if (strcmp(pi.tk.lx, "+") == 0)
        {
            operation = 1;
        }
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //absord
        pi = GetNextToken_Parse(pi);
        //
        pi = term(pi);

        //WRITE VM operation
        if (operation == 0)
        {
            WriteVM("sub");
        }
        else
        {
            WriteVM("add");
        }

        pi = PeekNextToken_Parse(pi);
    }
    return pi;
}

ParserInfo relationalExpression(ParserInfo pi)
{
    pi = ArithmeticExpression(pi);
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "=") == 0 || (strcmp(pi.tk.lx, ">") == 0) || (strcmp(pi.tk.lx, "<") == 0))
    {
        int operation = 0;
        if (strcmp(pi.tk.lx, "=") == 0)
        {
            operation = 1;
        }
        else if (strcmp(pi.tk.lx, ">") == 0)
        {
            operation = 2;
        }
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //absord
        pi = GetNextToken_Parse(pi);
        //
        pi = ArithmeticExpression(pi);

        //WRITE VM operation
        if (operation == 0)
        {
            WriteVM("lt");
        }
        else if (operation == 1)
        {
            WriteVM("eq");
        }
        else
        {
            WriteVM("gt");
        }

        pi = PeekNextToken_Parse(pi);
    }
    return pi;
}

ParserInfo expression(ParserInfo pi)
{
    pi = relationalExpression(pi);
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "&") == 0 || (strcmp(pi.tk.lx, "|") == 0))
    {
        int operation = 0;
        if (strcmp(pi.tk.lx, "|") == 0)
        {
            operation = 1;
        }

        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //absord
        pi = GetNextToken_Parse(pi);
        //
        pi = relationalExpression(pi);

        if (operation == 0)
        {
            WriteVM("and");
        }
        else if (operation == 1)
        {
            WriteVM("or");
        }

        pi = PeekNextToken_Parse(pi);
    }
    return pi;
}

ParserInfo returnStatemnt(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "return") == 0)
    {
        //
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ";") == 0)
    {
        //absorb ;
        pi = GetNextToken_Parse(pi);
        //
        //This means return is void so write push 0
        WriteVM("push constant 0");
        return pi;
    }
    else if (strcmp(pi.tk.lx, "}") == 0)
    {
        pi = exitP(semicolonExpected, pi);
    }

    pi = expression(pi);

    pi = GetNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ";") == 0)
    {
        //
    }
    else
    {
        pi = exitP(semicolonExpected, pi);
    }
    return pi;
}

ParserInfo expressionList(ParserInfo pi)
{
    //check if nothing.
    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ")") == 0)
    {
        return pi;
    }

    pi = expression(pi);

    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, ",") == 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //Absorb ,
        pi = GetNextToken_Parse(pi);
        //
        pi = expression(pi);
        pi = PeekNextToken_Parse(pi);
    }
    return pi;
}

ParserInfo subroutineCall(ParserInfo pi)
{
    // if a.Method then find Object.Method
    char *classObject;
    char functionName[60];
    functionName[0] = -1;
    // if function then find CurrentObject.Function
    char localClassName[200];

    pi = GetNextToken_Parse(pi);
    int scope = -1;
    if (pi.tk.tp == ID)
    {
        strcpy(localClassName, pi.tk.lx);
        //

        // check this class or subroutine exists in scope

        int checkClass = lookup_Class(pi.tk.lx);
        int checkScope = lookup_ID_Expression(pi.tk.lx);
        if (checkScope == 0 && checkClass == 0)
        {
            pi = exitP(undecIdentifier, pi);
        }
        else
        {
            // if class find scope
            scope = lookup_Class_Scope(pi.tk.lx);
        }
    }
    else
    {
        pi = exitP(idExpected, pi);
    }

    int externalFunctionCheck = 0;

    //zero or one .id
    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ".") == 0)
    {
        externalFunctionCheck = 1;
        // absorb .
        pi = GetNextToken_Parse(pi);
        //
        pi = GetNextToken_Parse(pi);

        //Get type of variable (if class) so that they can do Object.function
        classObject = getType(localClassName);

        if (pi.tk.tp == ID)
        {
            //
            strcpy(functionName, pi.tk.lx);
            // can only be class scope ??
            int check = lookup_In_Scope(pi.tk.lx, scope);
            if (check <= 0)
            {
                pi = exitP(undecIdentifier, pi);
            }
        }
        else
        {
            pi = exitP(idExpected, pi);
        }
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "(") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openParenExpected, pi);
    }


    classObject = getType(localClassName);

    //Need to either push pointer 0 or push object onto stack if method or other object
    int checkM = checkMethod(localClassName);
    char buffered[400];
    if (strcmp(getClass(localClassName), localClassName) != 0 && externalFunctionCheck)
    {
        //other object being accessed
        sprintf(buffered, "push %s %d", getStack(localClassName), getOffset(localClassName));
        WriteVM(buffered);
    }
    else if (checkM)
    {
        WriteVM("push pointer 0");
    }
    else if (externalFunctionCheck)
    {
        checkM = checkMethod(functionName);
        if (checkM)
        {
            WriteVM("push pointer 0");
        }
    }

    pi = expressionList(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ")") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeParenExpected, pi);
    }

    //Now write to vm function call
    char buffer[500];
    if (functionName[0] == -1)
    {
        
        char currentFunctionName[200];
        strcpy(currentFunctionName, localClassName);
        strcpy(localClassName, getClass(currentFunctionName));

        sprintf(buffer, "call %s.%s %d", className, currentFunctionName, getArgs(currentFunctionName));
    }
    else
    {
        sprintf(buffer, "%s%s%s%s%s%d", "call ", classObject, ".", functionName, " ", getArgs(functionName));
    }

    WriteVM(buffer);
    //Then pop of result to temp as it is do statment and return not required? i think
    WriteVM("pop temp 0");

    return pi;
}

ParserInfo doStatement(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "do") == 0)
    {
        //
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    pi = subroutineCall(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ";") == 0)
    {
        //
    }
    else
    {
        pi = exitP(semicolonExpected, pi);
    }

    return pi;
}

ParserInfo whileStatement(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "while") == 0)
    {
        //
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    //Start of while loop label
    char buffer[300];
    sprintf(buffer, "label WHILE_EXP%d", loopCount);
    WriteVM(buffer);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "(") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openParenExpected, pi);
    }

    pi = expression(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ")") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeParenExpected, pi);
    }

    //Now for if goto
    //first not
    WriteVM("not");
    sprintf(buffer, "if-goto WHILE_END%d", loopCount);
    WriteVM(buffer);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "{") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openBraceExpected, pi);
    }

    //zero or more statements
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "}") != 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = statement(pi);
        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "}") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeBraceExpected, pi);
    }

    //Go back to top of while loop
    sprintf(buffer, "goto WHILE_EXP%d", loopCount);
    WriteVM(buffer);

    //End of loop to skip when statement no longer true
    sprintf(buffer, "label WHILE_END%d", loopCount);
    WriteVM(buffer);

    loopCount++;

    return pi;
}

ParserInfo ifStatement(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "if") == 0)
    {
        //
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "(") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openParenExpected, pi);
    }

    pi = expression(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ")") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeParenExpected, pi);
    }

    //ig-goto label true
    //WriteVM("not");
    char buffer[300];
    sprintf(buffer, "if-goto IF_TRUE%d", loopCount);
    WriteVM(buffer);
    sprintf(buffer, "goto IF_FALSE%d", loopCount);
    WriteVM(buffer);
    sprintf(buffer, "label IF_TRUE%d", loopCount);
    WriteVM(buffer);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "{") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openBraceExpected, pi);
    }

    //zero or more statements
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "}") != 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = statement(pi);
        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "}") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeBraceExpected, pi);
    }

    //zero or one else
    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "else") == 0)
    {
        //goto if-end
        sprintf(buffer, "goto IF_END%d", loopCount);
        WriteVM(buffer);
        // absorb else
        pi = GetNextToken_Parse(pi);
        //
    }
    else
    {
        //label if false
    sprintf(buffer, "label IF_FALSE%d", loopCount);
    WriteVM(buffer);
        return pi;
    }

    //label if false
    sprintf(buffer, "label IF_FALSE%d", loopCount);
    WriteVM(buffer);
    

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "{") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openBraceExpected, pi);
    }

    //zero or more statements
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "}") != 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = statement(pi);
        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "}") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeBraceExpected, pi);
    }

    //if end
    sprintf(buffer, "label IF_END%d", loopCount);
    WriteVM(buffer);
    loopCount++;
    return pi;
}

ParserInfo letStatemnt(ParserInfo pi)
{
    char idName[50];
    int arrayCheck = 0;
    //let
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "let") == 0)
    {
        //
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    //identifier
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == ID)
    {
        //
        int check = lookup_ID_Expression(pi.tk.lx);
        if (!(check > 0))
        {
            pi = exitP(undecIdentifier, pi);
        }
        strcpy(idName, pi.tk.lx);
    }
    else
    {
        pi = exitP(idExpected, pi);
    }

    //Zero or one []
    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "[") == 0)
    {
        //consume [
        pi = GetNextToken_Parse(pi);
        pi = expression(pi);

        pi = GetNextToken_Parse(pi);
        if (strcmp(pi.tk.lx, "]") == 0)
        {
            //
        }
        else
        {
            pi = exitP(closeBracketExpected, pi);
        }

        //Add pointer
        char bufferArray[200];
        sprintf(bufferArray, "push %s %d", getStack(idName), getOffset(idName));
        WriteVM(bufferArray);
        WriteVM("add");
        arrayCheck = 1;
    }

    pi = GetNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "=") == 0 && pi.tk.tp == SYMBOL)
    {
        //
    }
    else
    {
        pi = exitP(equalExpected, pi);
    }

    pi = expression(pi);

    pi = GetNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ";") == 0 && pi.tk.tp == SYMBOL)
    {
        //
    }
    else
    {
        pi = exitP(semicolonExpected, pi);
    }
    //If array pop temp answer
    if (arrayCheck)
    {
        WriteVM("pop temp 0");
        WriteVM("pop pointer 1");
        WriteVM("push temp 0");
        WriteVM("pop that 0");
    }
    else
    {

        //pop to id
        char buffer[300];
        sprintf(buffer, "pop %s %d", getStack(idName), getOffset(idName));
        WriteVM(buffer);
    }

    return pi;
}

ParserInfo varDeclarStatement(ParserInfo pi)
{
    char *typeC = (char *)malloc(64 * sizeof(char));
    char *id = (char *)malloc(64 * sizeof(char));
    Kind kind;

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "var") == 0)
    {
        //
        kind = Var;
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }

    free(typeC);
    typeC = type(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == ID)
    {
        //
        strcpy(id, pi.tk.lx);
    }
    else
    {
        pi = exitP(idExpected, pi);
    }

    //Add to symbol table
    int check = lookup_ID(pi.tk.lx);
    if (check == 0)
    {
        append(id, typeC, kind);
    }
    else
    {
        pi = exitP(redecIdentifier, pi);
    }

    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, ",") == 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = GetNextToken_Parse(pi);
        //
        //absorb ,
        pi = GetNextToken_Parse(pi);
        if (pi.tk.tp == ID)
        {
            //
            strcpy(id, pi.tk.lx);
        }
        else
        {
            pi = exitP(idExpected, pi);
        }

        //Add to symbol table
        int check = lookup_ID(pi.tk.lx);
        if (check == 0)
        {
            append(id, typeC, kind);
        }
        else
        {
            pi = exitP(redecIdentifier, pi);
        }

        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ";") == 0)
    {
        //
    }
    else
    {
        pi = exitP(semicolonExpected, pi);
    }

    free(typeC);
    free(id);
    return pi;
}

ParserInfo statement(ParserInfo pi)
{
    pi = PeekNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD)
    {
        if (strcmp(pi.tk.lx, "var") == 0)
        {
            pi = varDeclarStatement(pi);
        }
        else if (strcmp(pi.tk.lx, "let") == 0)
        {
            pi = letStatemnt(pi);
        }
        else if (strcmp(pi.tk.lx, "if") == 0)
        {
            pi = ifStatement(pi);
        }
        else if (strcmp(pi.tk.lx, "while") == 0)
        {
            pi = whileStatement(pi);
        }
        else if (strcmp(pi.tk.lx, "do") == 0)
        {
            pi = doStatement(pi);
        }
        else if (strcmp(pi.tk.lx, "return") == 0)
        {
            pi = returnStatemnt(pi);
            //Write return
            WriteVM("return");
        }
        else
        {
            pi = exitP(syntaxError, pi);
        }
    }
    else
    {
        pi = exitP(syntaxError, pi);
    }
    return pi;
}

ParserInfo subroutineBody(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "{") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openBraceExpected, pi);
    }

    //Zero or more statements.
    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, "}") != 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = statement(pi);
        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "}") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeBraceExpected, pi);
    }
    return pi;
}

int paramList(ParserInfo pi)
{
    char *typeC = (char *)malloc(64 * sizeof(char));
    char *id = (char *)malloc(64 * sizeof(char));
    Kind kind;
    int nArgs = 0;

    //Check if none first
    pi = PeekNextToken_Parse(pi);
    if (pi.tk.tp != RESWORD && pi.tk.tp != ID)
    {
        // Might not be the best way to do type check
        free(typeC);
        free(id);
        return 0;
    }

    kind = Arg;
    free(typeC);
    typeC = type(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp != ID)
    {
        pi = exitP(idExpected, pi);
    }
    else
    {
        nArgs++;
        strcpy(id, pi.tk.lx);
    }

    //Add to symbol table
    int check = lookup_ID(pi.tk.lx);
    if (check == 0)
    {
        append(id, typeC, kind);
    }
    else
    {
        pi = exitP(redecIdentifier, pi);
    }

    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, ",") == 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        //Eat the comma.
        pi = GetNextToken_Parse(pi);
        //type
        free(typeC);
        typeC = type(pi);

        pi = GetNextToken_Parse(pi);
        if (pi.tk.tp != ID)
        {
            pi = exitP(idExpected, pi);
        }
        else
        {
            nArgs++;
            strcpy(id, pi.tk.lx);
        }

        //Add to symbol table
        int check = lookup_ID(pi.tk.lx);
        if (check == 0)
        {
            append(id, typeC, kind);
        }
        else
        {
            pi = exitP(redecIdentifier, pi);
        }

        pi = PeekNextToken_Parse(pi);
    }

    free(typeC);
    free(id);
    return nArgs;
}

ParserInfo subroutineDeclar(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    char typeC[64];
    int constructorCheck = 0;
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "constructor") == 0)
    {
        //
        strcpy(typeC, "constructor");
        constructorCheck = 1;
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "function") == 0)
    {
        //
        strcpy(typeC, "function");
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "method") == 0)
    {
        //
        strcpy(typeC, "method");
        addThisArg();
        constructorCheck = 2;
    }
    else
    {
        pi = exitP(subroutineDeclarErr, pi);
    }

    //Currently don't do anything with this
    char returnType[16];

    pi = PeekNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "void") == 0)
    {
        pi = GetNextToken_Parse(pi);
        //
        strcpy(returnType, "void");
    }
    else
    {
        char *typeL = type(pi);
        strcpy(returnType, typeL);
        free(typeL);
    }

    char functionCall[32];
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == ID)
    {
        //
        //Add sub to symbol table
        int check = lookup_ID_KindCheck(pi.tk.lx, typeC);
        if (check == 0)
        {
            strcpy(functionCall, pi.tk.lx);
            append(pi.tk.lx, typeC, None);
            //Write FunctionCall
            char buffer[400];
            sprintf(buffer, "%s%s%s%s%s%d", "function ", className, ".", functionCall, " ", getVars(functionCall));
            WriteVM(buffer);
        }
        else
        {
            pi = exitP(redecIdentifier, pi);
        }
    }
    else
    {
        pi = exitP(idExpected, pi);
    }

    //If constructor malloc vars
    if (constructorCheck == 1)
    {
        int totalGlobal = getGlobalVars();
        char buffer[400];
        sprintf(buffer, "push constant %d", totalGlobal);
        WriteVM(buffer);
        WriteVM("call Memory.alloc 1");
        WriteVM("pop pointer 0");
    }
    else if (constructorCheck == 2)
    {
        WriteVM("push argument 0");
        WriteVM("pop pointer 0");
    }
    //if method pop this



    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, "(") == 0)
    {
        //
    }
    else
    {
        pi = exitP(openParenExpected, pi);
    }
    int nArgs = paramList(pi);
    if (constructorCheck == 2)
    {
        nArgs++;
    }
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == SYMBOL && strcmp(pi.tk.lx, ")") == 0)
    {
        //
    }
    else
    {
        pi = exitP(closeParenExpected, pi);
    }

    //Set the number of parameters
    setArgs(nArgs, functionCall);

    pi = subroutineBody(pi);
    //Clear subroutine symbol table this is also asigns total variables
    resetLocal(className, functionCall);
    return pi;
}

char *type(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    char *typeC = (char *)malloc(64 * sizeof(char));
    if (pi.tk.tp == RESWORD || pi.tk.tp == ID)
    {
        if (strcmp(pi.tk.lx, "int") == 0)
        {
            //
            strcpy(typeC, "int");
        }
        else if (strcmp(pi.tk.lx, "char") == 0)
        {
            //
            strcpy(typeC, "char");
        }
        else if (strcmp(pi.tk.lx, "boolean") == 0)
        {
            //
            strcpy(typeC, "boolean");
        }
        else if (pi.tk.tp == ID)
        {
            //
            //This needs more checks.
            int check = lookup_Class(pi.tk.lx);
            if (check > 0)
            {
                strcpy(typeC, pi.tk.lx);
            }
            else
            {
                //Still copies in case of first pass
                strcpy(typeC, pi.tk.lx);
                pi = exitP(undecIdentifier, pi);
            }
        }
        else
        {
            pi = exitP(illegalType, pi);
        }
    }
    else
    {
        pi = exitP(illegalType, pi);
    }
    return typeC;
}

ParserInfo classVarDeclar(ParserInfo pi)
{
    Kind kind;
    char *typeC = (char *)malloc(64 * sizeof(char));
    char *id = (char *)malloc(64 * sizeof(char));
    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "static") == 0)
    {
        //
        kind = Static;
    }
    else if (pi.tk.tp == RESWORD && strcmp(pi.tk.lx, "field") == 0)
    {
        //
        kind = Field;
    }
    else
    {
        pi = exitP(classVarErr, pi);
    }

    //Type will return char*
    free(typeC);
    typeC = type(pi);

    pi = GetNextToken_Parse(pi);
    if (pi.tk.tp == ID)
    {
        //
        strcpy(id, pi.tk.lx);
    }
    else
    {
        pi = exitP(idExpected, pi);
    }

    //Add to symbol table
    int check = lookup_ID(pi.tk.lx);
    if (check == 0)
    {
        append(id, typeC, kind);
    }
    else
    {
        pi = exitP(redecIdentifier, pi);
    }

    pi = PeekNextToken_Parse(pi);
    while (strcmp(pi.tk.lx, ";") != 0)
    {
        //Error check for no infinite loops in all whiles
        if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
        {
            break;
        }
        pi = GetNextToken_Parse(pi);
        if (strcmp(pi.tk.lx, ",") == 0)
        {
            //
            pi = GetNextToken_Parse(pi);
            if (pi.tk.tp == ID)
            {
                //
                strcpy(id, pi.tk.lx);
            }
            else
            {
                pi = exitP(idExpected, pi);
            }
        }
        else
        {
            pi = exitP(syntaxError, pi);
        }

        //Add to symbol table
        int check = lookup_ID(pi.tk.lx);
        if (check == 0)
        {
            append(id, typeC, kind);
        }
        else
        {
            pi = exitP(redecIdentifier, pi);
        }

        pi = PeekNextToken_Parse(pi);
    }

    pi = GetNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, ";") == 0)
    {
        //
    }
    else
    {
        pi = exitP(semicolonExpected, pi);
    }

    free(typeC);
    free(id);
    return pi;
}

ParserInfo memberDeclar(ParserInfo pi)
{
    pi = PeekNextToken_Parse(pi);
    if (pi.tk.tp == RESWORD)
    {
        if (strcmp(pi.tk.lx, "static") == 0)
        {
            pi = classVarDeclar(pi);
        }
        else if (strcmp(pi.tk.lx, "field") == 0)
        {
            pi = classVarDeclar(pi);
        }
        else if (strcmp(pi.tk.lx, "constructor") == 0)
        {
            pi = subroutineDeclar(pi);
        }
        else if (strcmp(pi.tk.lx, "function") == 0)
        {
            pi = subroutineDeclar(pi);
        }
        else if (strcmp(pi.tk.lx, "method") == 0)
        {
            pi = subroutineDeclar(pi);
        }
        else
        {
            pi = exitP(memberDeclarErr, pi);
        }
    }
    else if (pi.tk.tp == ERR)
    {
        pi = exitP(lexerErr, pi);
    }
    else
    {
        pi = exitP(memberDeclarErr, pi);
    }
    return pi;
}

ParserInfo classDeclar(ParserInfo pi)
{
    pi = GetNextToken_Parse(pi);
    if (strcmp(pi.tk.lx, "class") == 0 && pi.tk.tp == RESWORD)
    {
        //
    }
    else
    {
        pi = exitP(classExpected, pi);
    }
    pi = GetNextToken_Parse(pi);

    if (pi.tk.tp == ID)
    {
        //
        strcpy(className, pi.tk.lx);
        //Add class to symbol table
        int check = lookup_ID(pi.tk.lx);
        if (check == 0)
        {
            append(pi.tk.lx, pi.tk.lx, Class);
        }
        else
        {
            pi = exitP(redecIdentifier, pi);
        }

        pi = GetNextToken_Parse(pi);
        if (strcmp(pi.tk.lx, "{") == 0)
        {
            //

            //Zero or more occurences of memberDeclar.
            pi = PeekNextToken_Parse(pi);
            while (strcmp(pi.tk.lx, "}") != 0 && (pi.er == none || pi.er == undecIdentifier || pi.er == redecIdentifier))
            {
                //Error check for no infinite loops in all whiles
                if ((pi_Master.er != none) && (pi_Master.er != undecIdentifier) && (pi_Master.er != redecIdentifier))
                {
                    break;
                }
                pi = memberDeclar(pi);
                pi = PeekNextToken_Parse(pi);
            }
            if (strcmp(pi.tk.lx, "}") == 0)
            {
                //
            }
            else
            {
                pi = exitP(closeBraceExpected, pi);
            }
        }
        else
        {
            pi = exitP(openBraceExpected, pi);
        }
    }
    else
    {
        pi = exitP(idExpected, pi);
    }
    return pi;
}

int InitParser(char *file_name)
{
    //Create vm file
    //replace extension
    char editFile_Name[128];
    strcpy(editFile_Name, file_name);

    int replaceCheck = 0;
    for (int i = 0; i < strlen(editFile_Name); i++)
    {
        if (editFile_Name[i] == '.')
        {
            replaceCheck = i;
        }
    }
    editFile_Name[replaceCheck + 1] = 'v';
    editFile_Name[replaceCheck + 2] = 'm';
    editFile_Name[replaceCheck + 3] = 0;

    vm_FP = fopen(editFile_Name, "w");
    return 1;
}

ParserInfo Parse()
{
    ParserInfo pi;
    pi.er = none;
    pi_Master.er = none;

    // implement the function
    pi = classDeclar(pi);

    fclose(vm_FP);

    return pi_Master;
}

int StopParser()
{
    return 1;
}

// #ifndef TEST_PARSER
// int main(int argc, char *argv[])
// {
// 	int check = InitLexer(argv[1]);
// 	ParserInfo pi = Parse();
// 	printf("%s %d", ErrorString(pi.er), pi.tk.ln);
// 	return 1;
// }
// #endif