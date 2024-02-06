#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_STATES 100
#define MAX_ALPHABET_SIZE 100
#define MAX_STATE_LENGTH 10
#define MAX_INPUT_LENGTH 10
/// Function epsilonClosure computes all the possible currentstates we could be in.
/// Meaning that if we are now in q1 and q1 has an epsilon transition to q2, we could be either in q1 or q2 at this moment.




typedef struct {
    char currentState[MAX_STATE_LENGTH];
    char inputSymbol[MAX_INPUT_LENGTH];
    char nextState[MAX_STATE_LENGTH];
} NFATransition;

typedef struct {
    int numStates;
    int numTransitions;
    char startState[MAX_STATE_LENGTH];
    char acceptState[MAX_STATE_LENGTH];
    NFATransition transitions[MAX_STATES];
} NFA;


void printStates(NFA nfa, char** currentStates, int numCurrentStates, char nameOfStates[MAX_STATES][MAX_STATE_LENGTH]) {
    /// nameOfStates was added in case the states provided are not like "q[number]"
    for (int i = 0; i < nfa.numStates; i++) {
        int isActive = 0;
        for (int j = 0; j < numCurrentStates; j++) {

            if (currentStates[j] != NULL && strcmp(currentStates[j], nameOfStates[i]) == 0) { /// if the state i is in the
                    /// current states vector, then it is active
                isActive = 1;
                break;
            }
        }
        printf("%d ", isActive);
    }
    printf("\n");
}

void epsilonClosure(NFA nfa, char** states, int* numStates) {
    int changed = 1;  // Flag to check if any changes were made in this iteration

    while (changed) { /// we need to compute in depth, meaning we can have q1->e->q2, q2->e->q3, q3-e->q4
            /// and our next states has to be [q1,q2,q3,q4], so every iteration that still had changes, we
            /// travel on the new possible e-transitions
        changed = 0;

        for (int i = 0; i < *numStates; i++) {
            char* currentState = states[i]; /// for every state is the states vector, we will consume epsilon-transitions

            for (int j = 0; j < nfa.numTransitions; j++) {
                NFATransition transition = nfa.transitions[j];

                if (!strcmp(transition.currentState, currentState) &&
                    !strcmp(transition.inputSymbol, "e")) { /// we travel on all possible epsilon-transitions from the current state

                    char* nextState = transition.nextState;
                    int alreadyAdded = 0;

                    // Check if nextState is already in states
                    for (int k = 0; k < *numStates; k++) {
                        if (!strcmp(states[k], nextState)) {
                            alreadyAdded = 1;
                            break;
                        }
                    } /// same as in the inAccepted function, we add the states reacheable by epsilon to nextStates

                    if (!alreadyAdded) {

                        states[*numStates] = (char*)malloc(sizeof(char) * MAX_INPUT_LENGTH);
                        strcpy(states[*numStates], nextState);
                        (*numStates)++;
                        changed = 1;
                    }
                }
            }
        }
    }
}


bool isAccepted(NFA nfa, char input[100][10], int inputLen, char nameOfStates[MAX_STATES][MAX_STATE_LENGTH]) {
    char* currentStates[MAX_STATES];
    int numCurrentStates = 1;
    currentStates[0] = (char*)malloc(sizeof(char) * MAX_STATE_LENGTH); // Allocate memory for the start state
    strcpy(currentStates[0], nfa.startState); /// We start travelling the NFA from the start state
    epsilonClosure(nfa, currentStates, &numCurrentStates); /// If the start states has epsilon transitions
    /// the currentstates vector will be composed of [startState + states reacheable by epsilon trans]

    for (int i = 0; i < inputLen; i++) { /// we iterate through every input symbol

        char* nextStates[MAX_STATES]; /// we need to know which states we will be in after consuming the current symbol
        int numNextStates = 0;

        printf("%s ", input[i]);

        for (int j = 0; j < numCurrentStates; j++) {
            char* currentState = currentStates[j]; /// we iterate through every current state, because, at the moment
            /// we can be in any of the current states

            for (int k = 0; k < nfa.numTransitions; k++) {
                NFATransition transition = nfa.transitions[k];

                if (!strcmp(transition.currentState, currentState) &&
                    (!strcmp(transition.inputSymbol, input[i]))) { /// for every non-epsilon transition, we consume the symbol
                        /// and add the transition next state to our next states set(unique values)

                    char* nextState = transition.nextState;
                    //printf("%s %s %s", nextState, transition.inputSymbol, currentState);
                    int alreadyAdded = 0;

                    for (int l = 0; l < numNextStates; l++) {
                        if (!strcmp(nextStates[l], nextState)) { ///  make sure we don't add the same state twice
                            alreadyAdded = 1;
                            break;
                        }
                    }

                    if (!alreadyAdded) {
                        nextStates[numNextStates] = (char*)malloc(sizeof(char) * MAX_STATE_LENGTH); // Allocate memory for nextState
                        strcpy(nextStates[numNextStates], nextState);
                        numNextStates++;
                    } /// we add the transition's next state to our next states
                }
            }

        }
        epsilonClosure(nfa, nextStates, &numNextStates); /// for all the states reached with the current symbol, we
        /// calculate epsilon closure, meaning we can be in any of the [nextStates + states reacheable with epsilon-trans
        /// from any of the next states].


        // Free memory for the previous currentStates
        for (int j = 0; j < numCurrentStates; j++) {
            free(currentStates[j]);
        }

        // Update currentStates to nextStates
        numCurrentStates = numNextStates;
        for (int j = 0; j < numCurrentStates; j++) { /// our next states are now the current states for the next iteration
            currentStates[j] = nextStates[j];


        }

        printStates(nfa, currentStates, numCurrentStates, nameOfStates); /// print the active states after consuming the symbol input[i]

    }

    bool isAccepted = false;
    for (int i = 0; i < numCurrentStates; i++) {
        if (!strcmp(currentStates[i], nfa.acceptState)) { /// deciding if we accept the input when we have reached the end of it
            isAccepted = true; /// if we are currently(end of input) in the accept state, then we consider that the word is accepted
            break;
        }
    }
    /// otherwise, it is rejected

    // Free memory for the final currentStates
    for (int i = 0; i < numCurrentStates; i++) {
        free(currentStates[i]);
    }

    return isAccepted;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <NFA_input_file>\n", argv[0]);
        return 1;
    }
    FILE* inputFile = fopen(argv[1], "r");

    if (inputFile == NULL) {

        perror("Error opening input file");
        return 1;
    }

    int alphabetSize;
    fscanf(inputFile, "%d", &alphabetSize);
    char alphabet[MAX_ALPHABET_SIZE][MAX_INPUT_LENGTH];
    for (int i = 0; i < alphabetSize; i++) {
        fscanf(inputFile, " %s", alphabet[i]);

    }

    // Read the number of states and list
    int numStates;
    fscanf(inputFile, "%d", &numStates);
    char states[MAX_STATES][MAX_STATE_LENGTH];

    for (int i = 0; i < numStates; i++) {
        fscanf(inputFile, "%s", states[i]);
    }

    // Read start state and accept state
    char startState[MAX_STATE_LENGTH]; // State names are strings
    char acceptState[MAX_STATE_LENGTH];
    fscanf(inputFile, "%s", startState);
    fscanf(inputFile, "%s", acceptState);

    // Read input string
    int inputStringLength;
    char inputString[100][MAX_INPUT_LENGTH];
    fscanf(inputFile, "%d", &inputStringLength);


    for(int i = 0; i<inputStringLength;i++)
    {
        fscanf(inputFile, " %s", inputString[i]);
    }


    // Read number of transitions
    int numTransitions;
    fscanf(inputFile, "%d", &numTransitions);
    NFATransition transitions[MAX_STATES];
    for (int i = 0; i < numTransitions; i++) {
        fscanf(inputFile, "%s %s %s",
               transitions[i].currentState,
               transitions[i].inputSymbol,
               transitions[i].nextState);

        //printf("%s %s %s \n", transitions[i].currentState, transitions[i].inputSymbol, transitions[i].nextState);
    }

    fclose(inputFile);



    // Create the NFA structure
    NFA nfa;
    nfa.numStates = numStates;
    nfa.numTransitions = numTransitions;
    strcpy(nfa.startState, startState); // Copy state names
    strcpy(nfa.acceptState, acceptState);

    // test printf("%s %s %d %d \n", nfa.acceptState, nfa.startState, nfa.numStates, nfa.numTransitions);


    memcpy(nfa.transitions, transitions, sizeof(transitions));

    // Check if the input word is accepted by the NFA
    bool accepted = isAccepted(nfa, inputString, inputStringLength, states);

    if (accepted) {
        printf("accept\n");
    } else {
        printf("reject\n");
    }

    return 0;
}
