/* Parse tree using ASCII graphics
        -original NCurses code from "Game Programming in C with the Ncurses Library"
         https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
         and from "NCURSES Programming HOWTO"
         http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#ifndef NOGRAPHICS

#include <ncurses.h>
#endif

typedef struct
{
   int lineNumber;
   char command[20];
   char arg1[20];
   char arg2[20];
   char arg3[20];
} ProgramLine;

typedef struct
{
   int value;
   char variableName[11];
} intVariable;

int findVariableByName(intVariable searchedVar, intVariable allVariables[], int variablesCounter)
{
   for (int i = 0; i < variablesCounter; i++)
   {
      intVariable varToBeCompared = allVariables[i];
      if (!strcmp(searchedVar.variableName, varToBeCompared.variableName))
         return i;
   }
   return -1; // variable not found
}

int findVariableByNameString(char searchedString[], intVariable allVariables[], int variablesCounter)
{
   for (int i = 0; i < variablesCounter; i++)
   {
      intVariable varToBeCompared = allVariables[i];
      if (!strcmp(searchedString, varToBeCompared.variableName))
         return i;
   }
   return -1; // variable not found
}

bool evaluateExpression(intVariable firstVariable, intVariable secondVariable, char operator[])
{
   if (!strcmp(operator, "eq") && firstVariable.value == secondVariable.value)
      return true;
   else if (!strcmp(operator, "ne") && firstVariable.value != secondVariable.value)
      return true;
   else if (!strcmp(operator, "gt") && firstVariable.value > secondVariable.value)
      return true;
   else if (!strcmp(operator, "gte") && firstVariable.value >= secondVariable.value)
      return true;
   else if (!strcmp(operator, "lt") && firstVariable.value < secondVariable.value)
      return true;
   else if (!strcmp(operator, "lte") && firstVariable.value <= secondVariable.value)
      return true;
   return false; // any other case is evaluated as false
}

ProgramLine parse_line(char *line)
{
   ProgramLine programLine;
   int res = sscanf(line, "%d %s %s %s %s", &programLine.lineNumber, programLine.command,
                    programLine.arg1, programLine.arg2, programLine.arg3);
   if (res < 5)
   {
      programLine.arg3[0] = '\0';
      if (res < 4)
      {
         programLine.arg2[0] = '\0';
         if (res < 3)
            programLine.arg1[0] = '\0';
      }
   }
   return programLine;
}

#define SCREENSIZE 200
void print(int row, int col, char *str);
#ifndef NOGRAPHICS

// curses output
// row indicates in which row the output will start - larger numbers
//      move down
// col indicates in which column the output will start - larger numbers
//      move to the right
// when row,col == 0,0 it is the upper left hand corner of the window
void print(int row, int col, char *str)
{
   mvprintw(row, col, str);
}
#endif

void execute_program(ProgramLine *programLines, int numLines)
{
   intVariable variables[1000] = {}; // storage for variables
   int currentVariableCounter = 0;
   bool isProgramRunning = false;
   int currentState = 0; // initial state

   for (int i = 0; i < numLines; i++)
   {
      ProgramLine line = programLines[i];
      currentState = line.lineNumber;

      // Logic for each command
      if (strcmp(line.command, "int") == 0)
      {
         if (findVariableByNameString(line.arg1, variables, currentVariableCounter) != -1)
         { /// we found a variable with this name, throw an error or something
            // printf("Variable %s already declared (line %d)\n", line.arg1, line.lineNumber);
            perror("Variable already declared"); // this case shouldnt be reached if tests are ok
         }
         else
         {
            strcpy(variables[currentVariableCounter].variableName, line.arg1);
            variables[currentVariableCounter].value = atoi(line.arg2);
            currentVariableCounter++;
         }
      }
      else if (strcmp(line.command, "set") == 0)
      {

         int varLocation = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int value = atoi(line.arg2);
         variables[varLocation].value = value;
         // printf("Line %d: Set %s to %d\n", line.lineNumber, variables[varLocation].variableName, variables[varLocation].value);
      }
      else if (strcmp(line.command, "add") == 0)
      {
         // Add operation
         int varLocation = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int value = atoi(line.arg2);
         variables[varLocation].value += value;
         // printf("Line %d: Add %s by %d\n", line.lineNumber, variables[varLocation].variableName, value);
      }
      else if (strcmp(line.command, "sub") == 0)
      {
         // Sub operation
         int varLocation = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int value = atoi(line.arg2);
         variables[varLocation].value -= value;
         // printf("Line %d: Subtract %s by %d\n", line.lineNumber, variables[varLocation].variableName, value);
      }
      else if (strcmp(line.command, "mult") == 0)
      {
         // Mult operation
         int varLocation = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int value = atoi(line.arg2);
         variables[varLocation].value *= value;
         // printf("Line %d: Multiply %s by %d\n", line.lineNumber, variables[varLocation].variableName, value);
      }
      else if (strcmp(line.command, "div") == 0)
      {
         // Mult operation
         int varLocation = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int value = atoi(line.arg2);
         variables[varLocation].value /= value;
         // printf("Line %d: Divide %s by %d\n", line.lineNumber, variables[varLocation].variableName, value);
      }
      else if (strcmp(line.command, "begin") == 0)
      {
         isProgramRunning = true; // setting that so we know if we are actually running the program(may be useful?)
      }
      else if (strcmp(line.command, "end") == 0)
      {
         isProgramRunning = false;
         // printf("Program finished \n"); testing
         return;
      }
      else if (strcmp(line.command, "if") == 0)
      {
         int firstVar = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int secondVar = findVariableByNameString(line.arg3, variables, currentVariableCounter);
         bool evaluatedValue = evaluateExpression(variables[firstVar], variables[secondVar], line.arg2);

         if (!evaluatedValue) // jump over next line
            i++;
      }
      else if (strcmp(line.command, "goto") == 0)
      {
         int gotoLineNumber = atoi(line.arg1);
         int foundIndex = -1;
         for (int j = 0; j < numLines; j++)
            if (programLines[j].lineNumber == gotoLineNumber)
            {
               foundIndex = j;
            }
         if (foundIndex > 0)
         {
            currentState = programLines[foundIndex].lineNumber;
            i = foundIndex - 1;
         }
         // printf("Jump to line %d \n", foundIndex);
      }
      else if (strcmp(line.command, "print") == 0)
      {

         int firstVar = findVariableByNameString(line.arg1, variables, currentVariableCounter);
         int secondVar = findVariableByNameString(line.arg2, variables, currentVariableCounter);

#ifdef NOGRAPHICS

         printf("%d %d %s\n", variables[firstVar].value, variables[secondVar].value, line.arg3);
#else
         print(variables[firstVar].value, variables[secondVar].value, line.arg3);
#endif
      }
   }
}

int main(int argc, char *argv[])
{
   int c;

#ifndef NOGRAPHICS
   // initialize ncurses
   initscr();
   noecho();
   cbreak();
   timeout(0);
   curs_set(FALSE);
#endif

   /* read and interpret the file starting here */
   if (argc != 2)
   {
      fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
      return EXIT_FAILURE;
   }

   FILE *file = fopen(argv[1], "r");
   if (file == NULL)
   {
      perror("Error opening file");
      return EXIT_FAILURE;
   }

   char line[100];
   ProgramLine programLines[1000];
   int numLines = 0;

   while (fgets(line, sizeof(line), file) != NULL)
   {
      programLines[numLines] = parse_line(line);
      numLines++;
   }

   fclose(file);

   execute_program(programLines, numLines);

   // // Print the parsed program lines (for testing purposes)
   // for (int i = 0; i < numLines; i++)
   // {
   //    printf("%d %s %s %s %s\n", programLines[i].lineNumber, programLines[i].command,
   //           programLines[i].arg1, programLines[i].arg2, programLines[i].arg3);
   // }

#ifndef NOGRAPHICS
   /* loop until the 'q' key is pressed */
   while (1)
   {
      c = getch();
      if (c == 'q')
         break;
   }

   // shut down ncurses
   endwin();
#endif
}
