/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Matthew Poole
Student ID: 201298590
Email: sc19map@leeds.ac.uk
Date Work Commenced: 12/04/2021
*************************************************************************/

#include "compiler.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int InitCompiler()
{
    secondrun = 0;
	return 1;
}

char** appendLib(char** fileList){
	int filecount = 8;
	fileList = (char **)realloc(fileList, (filecount + 1) * sizeof(char *));
	fileList[0] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[0], "Array.jack");
	fileList[1] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[1], "Keyboard.jack");
	fileList[2] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[2], "Math.jack");
	fileList[3] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[3], "Memory.jack");
	fileList[4] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[4], "Output.jack");
	fileList[5] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[5], "Screen.jack");
	fileList[6] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[6], "String.jack");
	fileList[7] = (char *)malloc(64 * sizeof(char));
	strcpy(fileList[7], "Sys.jack");

	return fileList;
}

ParserInfo compile(char *dir_name)
{
    secondrun = 0;
	ParserInfo p;
	p.er = none;
	int filecount = 0;
	char **fileList;
	fileList = (char **)malloc(sizeof(char *));

	// append all libraries to fileList.
	fileList = appendLib(fileList);
	filecount = 8; //8 libraries have been added

	// check for dir
	DIR *dir;
	struct dirent *ent;

	char dest[100];
	memset(dest, '\0', sizeof(dest));
	for (int i = 0; i < strlen(dir_name); i++)
	{
		dest[i] = dir_name[i];
	}

	//strcpy(dest,dir_name);
	strcat(dest, "/");
	if ((dir = opendir(dest)) != NULL)
	{
		/* get array of files in dir */
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_name[0] == '.')
			{
				continue;
			}
			fileList = (char **)realloc(fileList, (filecount + 1) * sizeof(char *));
			fileList[filecount] = (char *)malloc((strlen(ent->d_name) + strlen(dest) + 1) * sizeof(char));
			strcpy(fileList[filecount], dest);
			strcat(fileList[filecount], ent->d_name);
			filecount++;
		}

		closedir(dir);
	}

	//Multiple files to pass, and multiple passes required now.
	initSymbolTable();
	for (int i = 0; i < 2; i++)
	{
        secondrun++;
		for (int j = 0; j < filecount; j++)
		{
            if(strstr(fileList[j],".vm")!= NULL){
                continue;
            }
			p.er = none;
			setSymbolClassScope(j);
			InitLexer(fileList[j]);
			InitParser(fileList[j]);

			p = Parse();
            StopLexer();
			if(i>0 && p.er != none){ //Stops error being reset on second pass.
				break;
			}

		}
	}


	free(fileList);
	Free_SymbolTable();
	return p;
}

int StopCompiler()
{

	return 1;
}

#ifndef TEST_COMPILER
int main()
{
	InitCompiler();
	ParserInfo p = compile("Pong");
	PrintError(p);
	StopCompiler();
	return 1;
}
#endif