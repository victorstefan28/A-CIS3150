/* Parse tree using ASCII graphics
        -original NCurses code from "Game Programming in C with the Ncurses Library"
         https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
         and from "NCURSES Programming HOWTO"
         http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#define SCREENSIZE 150

// current position in string
int position;
// indentation
int width;

int regexp(char *, int);
int concat(char *, int);
int term(char *, int);
int star(char *, int);
int element(char *, int);
int group(char *, int);
int character(char *, int);
int symbol(char *, int);
int alphanum(char *, int);
int metachar(char *, int);
int white(char *, int);
int tab(char *, int);
int vtab(char *, int);
int nline(char *, int);
int endofline(char *, int);

// draws text on screen at location (depth, width) using ncurses
void print(int depth, char *str)
{
    // offset in depth, used to break line and move it down the screen
    // when the max line length is reached
    static int offset = 0;

    // if the output reaches the max width of the window then
    // move down and back to the left edge (carriage return and newline)
    if (width > SCREENSIZE)
    {
        width = 0;
        offset += 15;
    }
    // ncurses command to draw textstr
    mvprintw(depth + offset, width, str);
}

int drawTree(char *regex)
{
    char c;
    int depth = 0;

    // start in leftmost position
    width = 0;

    // ncurses clear screen
    clear();
    position = 0;
    regexp(regex, depth);

    refresh();
    // parse tree
    // parsing functions calls print to draw output
    // -regex is the string containing the regular expression to be parsed
    // -depth contains the current depth in the parse tree, it is
    //    incremented with each recursive call

    // read keyboard and exit if 'q' pressed
    while (1)
    {
        c = getch();
        if (c == 'q')
            return (1);
    }
}

int star(char *regex, int depth)
{
    print(depth, "star");
    if (element(regex, depth + 1))
    {
        if (regex[position] == '*')
        {
            position++;
            return 1;
        }
    }
    return 0;
}

int element(char *regex, int depth)
{
    print(depth, "element");
    const int lastPos = position;
    if (group(regex, depth + 1))
    {
        return 1;
    }
    else if ((width += 10) && character(regex, depth + 1))
    {
        return 1;
    }
    else
        return 0;
}

int group(char *regex, int depth)
{
    print(depth, "group");
    if (regex[position] == '(')
    {
        position++;
        if (regexp(regex, depth + 1))
        {
            if (regex[position] == ')')
            {
                position++;
                print(depth + 1, "match");
                return 1;
            }
        }
        else
        {
            if (regex[position] == ')')
            {
                position++;

                return 0;
            }
        }

        return 0;
    }
    print(depth + 1, "fail");
    return 0;
}

int term(char *regex, int depth)
{
    print(depth, "term");
    const int lastPos = position;

    if (star(regex, depth + 1))
    {
        return 1;
    }
    else
    {
        position = lastPos;
        if (element(regex, depth + 1))
            return 1;
        else
        {
            return 0;
        }
    }
}

int concat(char *regex, int depth)
{
    print(depth, "concat");

        const int lastPos = position;
    if (term(regex, depth + 1))
    {
        if (concat(regex, depth + 1))
            return 1;
    }
    else
    {
        position = lastPos;
        if (term(regex, depth + 1))
        {
            return 1;
        }
        else if (endofline(regex, depth + 1))
        {

            return 1;
        }
        else
        {

            return 0;
        }
    }
}
int character(char *regex, int depth)
{
    print(depth, "char");
    if (alphanum(regex, depth + 1))
    {
        print(depth + 1, "match");
        width += 10;
        return 1;
    }
    else if (symbol(regex, depth + 1))
    {
        print(depth + 1, "match");
        width += 10;
        return 1;
    }
    else if (white(regex, depth + 1))
    {

        print(depth + 1, "match");
        width += 10;
        return 1;
    }
    else
    {
        print(depth + 1, "fail");
        width += 10;
        return 0;
    }
}

int alphanum(char *regex, int depth)
{
    char current = regex[position];
    if ((current >= 'A' && current <= 'Z') || (current >= 'a' && current <= 'z') || (current >= '0' && current <= '9'))
    {
        position++;
        return 1;
    }
    return 0;
}

int symbol(char *regex, int depth)
{
    char current = regex[position];
    if (current == 0)
    {
        print(depth, "fail");
        return 0;
    }

    // Define the symbols that are part of the regular expression grammar
    const char *symbols = "!\"#$%&'+,-./:;<=>?@[\\]^_`{|} ~\t\v\n";
    if (strchr(symbols, current) != NULL)
    {

        position++;
        return 1;
    }
    print(depth, "fail");
    return 0;
}

int white(char *regex, int depth)
{
    if (tab(regex, depth + 1) || vtab(regex, depth + 1) || nline(regex, depth + 1))
        return 1;
    else
        return 0;
}

int tab(char *regex, int depth)
{
    if (regex[position] == '\t')
    {
        position++;
        return 1;
    }
    return 0;
}

int vtab(char *regex, int depth)
{
    if (regex[position] == '\v')
    {

        position++;
        return 1;
    }
    return 0;
}

int nline(char *regex, int depth)
{
    if (regex[position] == '\n')
    {
        position++;
        return 1;
    }
    return 0;
}

int endofline(char *regex, int depth)
{
    print(depth, "eoln");

    if (regex[position] == '\0')
    {
        print(depth + 1, "match");
        width += 10;
        return 1;
    }
    else if (regex[position] == ')')
    {
        print(depth + 1, "fail");
        width += 10;
        return 1;
    }
    return 0;
}

int regexp(char *regex, int depth)
{
    print(depth, "regexp");
    if (concat(regex, depth + 1))
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input_filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        perror("Error opening the file");
        return 1;
    }
    char *ptr = malloc(1000);
    // Use fscanf to read regular expression and text from the file
    if (fscanf(file, "%s", &ptr[0]) != 1)
    {
        perror("Error reading regular expression");
        fclose(file);
        return 1;
    }

    // initialize ncurses
    initscr();
    noecho();
    cbreak();
    timeout(0);
    curs_set(FALSE);

    // replace the static regular expression with a string read
    // from the input file - do this before calling drawTree()

    // traverse and draw the parse tree
    position = 0;
    drawTree(ptr);

    // shut down ncurses
    endwin();
}
