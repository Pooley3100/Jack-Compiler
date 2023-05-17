/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Matthew Poole
Student ID: 201298590
Email: sc19map@leeds.ac.uk
Date Work Commenced: 20/02/2021
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
int lineNumber;
FILE *fp;
char *fileName;
#define resMax 21
char *resWord[resMax];
Token *allTokens;
int filePoint;
//19 reserverd symbols
char resSymbol[] = {'+', '-', '*', '/', '&', '|', '~', '<', '>', '.', '=', ';', ',', '{', '}', '[', ']', '(', ')'};
Token GetAll()
{
  Token t;

  /*Int c is required to read each character
  First it has to check if it's whitespace or comment and consume
  Use /r for carriage return to count line Number
  Types, 
  -1 is eof
  If it is a char/Underscore then it is an identifier or keword
  if digit then it is a integernumber
  else symbol.
  Whenever it encounters a tokentype return that token
  will need an array of reserved words
  */

  /*
  Questions:
  Why /r not /n ? 
  Pretty sure malloc is needed.
  */

  int c = fgetc(fp);
  //printf("%c ", (char)c);

  //Consume Whitespace and comments
  while (c != -1 && (isspace((char)c) || c == '/'))
  {
    //Check if it is a commnet
    if (c == '/')
    {
      int temp = c;
      c = fgetc(fp);
      if (c != '/' && c != '*')
      {
        ungetc(c, fp);
        c = temp;
        break;
      }
      else
      {
        ungetc(c, fp);
        c = temp;
      }
    }

    if (c == '\n')
    {
      lineNumber++;
    }

    //Need to add comment EOF errors.   <<<<
    //Consume Comment inside of whitespace
    if (c == '/')
    {
      c = fgetc(fp);
      //If it is a single line comment
      if (c == '/')
      {
        while (c != '\n')
        {
          c = fgetc(fp);
        }

        lineNumber++;
      }
      //* Closing comment
      else
      {
        while (1)
        {
          //EOF error
          if (c == -1)
          {
            strcpy(t.lx, "Error: unexpected eof in comment");
            t.lx[33] = 0;
            t.tp = ERR;
            t.ec = EofInCom;
            t.ln = lineNumber;
            strcpy(t.fl, fileName);
            return t;
          }
          //Check for end
          if (c == '*')
          {
            c = fgetc(fp);
            if (c == '/')
            {
              //Comments ended
              break;
            }
          }

          if (c == '\n')
          {
            lineNumber++;
          }
          c = fgetc(fp);
        }
      }
    }
    c = fgetc(fp);
  }

  //Is it EOF
  if (c == -1)
  {
    t.tp = EOFile;
    strcpy(t.fl, fileName);
    t.ln = lineNumber;
    return t;
  }

  int lxCount = 0;
  //Is it reserved word, if not thn identifier.
  if (isalpha((char)c) || (c == '_'))
  {
    while ((c != -1) && (isalpha((char)c) || isdigit((char)c) || (c == '_')))
    {
      t.lx[lxCount] = (char)c;
      lxCount++;
      c = fgetc(fp);
    }
    t.lx[lxCount] = 0;
    ungetc(c, fp);
    for (int i = 0; i < resMax; i++)
    {
      if (strcmp(resWord[i], t.lx) == 0)
      {
        //This is a reservedword
        t.tp = RESWORD;
        t.ln = lineNumber;
        strcpy(t.fl, fileName);
        return t;
      }
    }
    //Not reserved just id
    t.tp = ID;
    t.ln = lineNumber;
    strcpy(t.fl, fileName);
    return t;
  }

  //Will need to add EOF error here too
  if (c == '"')
  {
    int sCount = 0;
    c = fgetc(fp);

    while (c != '"')
    {
      //EOF error
      if (c == -1)
      {
        strcpy(t.lx, "Error: unexpected eof in string constant");
        t.lx[41] = 0;
        t.tp = ERR;
        t.ec = EofInStr;
        t.ln = lineNumber;
        strcpy(t.fl, fileName);
        return t;
      }
      //New Line error
      if (c == '\n')
      {
        strcpy(t.lx, "Error: new line in string constant");
        t.lx[35] = 0;
        t.tp = ERR;
        t.ec = NewLnInStr;
        t.ln = lineNumber;
        strcpy(t.fl, fileName);
        return t;
      }

      t.lx[sCount] = (char)c;
      sCount++;
      c = fgetc(fp);
    }
    t.lx[sCount] = 0;
    t.tp = STRING;
    t.ln = lineNumber;
    strcpy(t.fl, fileName);
    return t;
  }

  if (isdigit((char)c))
  {
    int dCount = 0;

    while (isdigit((char)c))
    {
      t.lx[dCount] = (char)c;
      c = fgetc(fp);
      dCount++;
    }
    t.lx[dCount] = 0;
    t.tp = INT;
    t.ln = lineNumber;
    strcpy(t.fl, fileName);
    ungetc(c, fp);
    return t;
  }

  //Must be a symbol?
  for (int i = 0; i < 19; i++)
  {
    if (c == resSymbol[i])
    {
      t.lx[0] = (char)c;
      t.lx[1] = 0;
      t.tp = SYMBOL;
      t.ln = lineNumber;
      strcpy(t.fl, fileName);
      return t;
    }
  }

  strcpy(t.lx, "Error: illegal symbol in source file");
  t.lx[37] = 0;
  t.tp = ERR;
  t.ec = IllSym;
  t.ln = lineNumber;
  strcpy(t.fl, fileName);
  return t;
}

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer(char *file_name)
{
  lineNumber = 1;
  allTokens = malloc(1 * sizeof(Token));
  fileName = file_name;

  //Reserved Word Array.
  resWord[0] = "class";
  resWord[1] = "constructor";
  resWord[2] = "method";
  resWord[3] = "function";
  resWord[4] = "int";
  resWord[5] = "boolean";
  resWord[6] = "char";
  resWord[7] = "void";
  resWord[8] = "var";
  resWord[9] = "static";
  resWord[10] = "field";
  resWord[11] = "let";
  resWord[12] = "do";
  resWord[13] = "if";
  resWord[14] = "else";
  resWord[15] = "while";
  resWord[16] = "return";
  resWord[17] = "true";
  resWord[18] = "false";
  resWord[19] = "null";
  resWord[20] = "this";
  //There has to be a better way to do that, maybe load from a file.

  fp = fopen(file_name, "r");
  if (fp != NULL)
  {
    //Now whole file has to be read.
    int count = 0;
    Token t = GetAll();
    while (t.tp != EOFile)
    {
      allTokens = realloc(allTokens, (count + 1) * sizeof(Token));
      strcpy(allTokens[count].lx, t.lx);
      allTokens[count].tp = t.tp;
      allTokens[count].ec = t.ec;
      strcpy(allTokens[count].fl, t.fl);
      allTokens[count].ln = t.ln;

      t = GetAll();
      count++;
    }
    //Add final token
    allTokens = realloc(allTokens, (count + 1) * sizeof(Token));
    strcpy(allTokens[count].lx, t.lx);
    allTokens[count].tp = t.tp;
    allTokens[count].ec = t.ec;
    strcpy(allTokens[count].fl, t.fl);
    allTokens[count].ln = t.ln;

    filePoint = 0;
    return 1;
  }
  else
  {
    printf("%s Error in file access.\n", file_name);
  }
  return 0;
}

// Get the next token from the source file
Token GetNextToken()
{
  filePoint+=1;
  return allTokens[filePoint-1];
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken()
{
  return allTokens[filePoint];
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer()
{
  free(allTokens);
  fclose(fp);
  return 0;
}

// do not remove the next line
// #ifndef TEST
// int main(int argc, char *argv[])
// {
//   // implement your main function here
//   // NOTE: the autograder will not use your main function

//   int check = InitLexer(argv[1]);
//   Token test = PeekNextToken();
//   printf("%s\n", test.lx);
//   Token t = GetNextToken();
//   while (t.tp != EOFile)
//   {
//     t = PeekNextToken();
//     printf("%s\n", t.lx);

//     printf("%s\n", t.lx);
//     t = GetNextToken();
//   }
//   StopLexer();
//   return 0;
// }
// // do not remove the next line
// #endif