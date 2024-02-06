#include <stdio.h>
#include <string.h>

int matchhere(char *regexp, char *text, int* matchlength);
int matchstar(int c, char *regexp, char *text, int* matchlength);
int matchplus(int c, char *regexp, char *text, int* matchlength);
int matchquestion(int c, char *regexp, char *text, int* matchlength);
int match(char *regexp, char *text, int* matchlength);


void findAndPrintMatches(char *regexp, char *text, int matches[1000], int *noMatches) {
    int startingPos = 0;

    while (text[startingPos] != '\0') {
        int matchedLength = 0;
        int matchFound = matchhere(regexp, text + startingPos, &matchedLength);

        if (matchFound) {
            matches[++*noMatches] = startingPos;

            startingPos += matchedLength; // Move to the character after the matched substring
        } else {
            startingPos += 1; // Move to the next character
        }
    }
}


int matchcharsets(char *regexp, char *text, int* matchlength)
{
    // Handle character sets within brackets
    int negate = 0;
    if (regexp[1] == '^') {
        negate = 1;
        regexp++;
    }
    regexp++; // Move past '['

    while (regexp[0] != ']' && regexp[0] != '\0') {
        if (regexp[1] == '-' && regexp[2] != '\0') {
            // Handle character range
            if ((*text >= regexp[0] && *text <= regexp[2]) ||
                (negate && (*text < regexp[0] || *text > regexp[2]))) {
                (*matchlength)++;
                return 1;  // Match found
            }
            regexp += 2;
        } else {
            // Match individual characters
            if ((*text == regexp[0]) ||
                (negate && (*text != regexp[0]))) {
                (*matchlength)++;
                return 1;  // Match found
            }
        }
        regexp++;
    }

    return 0;
}

/* match: search for regexp anywhere in text */
int match(char *regexp, char *text, int* matchlength)
{
    if (regexp[0] == '^')
        return matchhere(regexp+1, text, matchlength);
    do {    /* must look even if string is empty */
        if (matchhere(regexp, text, matchlength))
            return 1;
    } while (*text++ != '\0');
    return 0;
}


/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text, int* matchlength)
{
    int temp = *matchlength;
    if (regexp[0] == '\0')
        return 1;
    if (regexp[0] == '\\') {
        // Handle the escape character
        if (regexp[1] == *text) {
            (*matchlength)++;
            return matchhere(regexp + 2, text + 1, matchlength);
        }
        return 0;
    }
    if (regexp[1] == '*')
        return matchstar(regexp[0], regexp+2, text, matchlength);
    if (regexp[1] == '+')
        return matchplus(regexp[0], regexp+2, text, matchlength);
    if (regexp[1] == '?')
        return matchquestion(regexp[0], regexp + 2, text, matchlength);

    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\0';
    if (regexp[0] == '[')
        return matchcharsets(regexp, text, matchlength);
    if (*text!='\0' && (regexp[0]=='.' || regexp[0]==*text)) {
        (*matchlength)++;
        return matchhere(regexp + 1, text + 1, matchlength);
    }
    *matchlength = temp;
    return 0;
}

int matchplus(int c, char *regexp, char *text, int* matchlength)
{
    int foundA = 0;

    while (*text != '\0' && (*text == c || c == '.')) {
        if (*text == c) {
            foundA = 1; // Set the flag when 'c' is found
        }

        text++;
        (*matchlength)++;
    }

    return foundA ? matchhere(regexp, text, matchlength) : 0; // Require at least one 'c' before continuing
}


int matchquestion(int c, char *regexp, char *text, int* matchlength)
{
    if (matchhere(regexp, text, matchlength)) {
        if(regexp[0] == '\0')
            (*matchlength)++;
        return 1;
    }

    if (*text != '\0' && (c == '.' || c == *(text))) {
        if (matchhere(regexp, text + 1, matchlength)) {
            (*matchlength)++;
            return 1;
        }
    }

    return 0;
}

/* matchstar: search for c*regexp at beginning of text */
int matchstar(int c, char *regexp, char *text, int* matchlength)
{
    char *t;

    for (t = text; *t != '\0' && (*t == c || c == '.'); t++, (*matchlength)++)
        ;
    do {	/* * matches zero or more */
        if (matchhere(regexp, t, matchlength)) {
            (*matchlength)++;
            return 1;
        }
    } while (t-- > text);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening the file");
        return 1;
    }

    char regexp[100];
    char text[1000];

    // Use fscanf to read regular expression and text from the file
    if (fscanf(file, "%99[^\n]\n%999[^\n]", regexp, text) != 2) {
        perror("Error reading regular expression and text");
        fclose(file);
        return 1;
    }
    //printf("%s %s", regexp, text);
    fclose(file);
//    int matchedLengthTest = 0;
//    int defaultResult = match(regexp, text, &matchedLengthTest); // this is obtained using original function from Rob Pike code, testing purpose
//    //printf("%d %d\n", defaultResult, matchedLengthTest);


    int matchesFound[1000], noOfMatches = 0;
    findAndPrintMatches(regexp, text, matchesFound, &noOfMatches);
    if(noOfMatches > 0) {
        printf("match");
        for (int i = 1; i <= noOfMatches; i++)
            printf(" %d", matchesFound[i]);
    }
    else printf("no match");

    return 0;
}
