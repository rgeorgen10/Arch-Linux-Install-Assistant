#include <stdio.h>
#include <stdlib.h>
#include "helper-functions.h"
#include <string.h>

int main() {
    printWelcome();    // Print Welcome Text and Script Information
    printf("Would you like to continue with the installation of Arch Linux (y/n): ");
continuePrompt:
    char continueVar[2];
    scanf("%s", &continueVar);
    if(strcmp(continueVar, "Y") == 0 || strcmp(continueVar,"y") == 0) {
        goto continueScript;
    }
    else if(strcmp(continueVar,"N") == 0 || strcmp(continueVar,"n") == 0) {
        printf("\nScript terminated by user");
        exit(EXIT_FAILURE);
    }
    else {
        printf("\rWould you like to continue with the installation of Arch Linux (y/n): ");
        fflush(stdout);
        goto continuePrompt;
    }

continueScript:
    printf("\n");
    printf("Script is being continued \n");
    return 0;
}
