#include <stdio.h>
#include <stdlib.h>
#include "helper-functions.h"
#include <string.h>
#include <stdbool.h>

int main() {
    printWelcome();    // Print Welcome Text and Script Information
    printf("Would you like to continue with the installation of Arch Linux (y/n): ");
continuePrompt:       // Ask user to continue
    char continueVar[2];
    scanf("%s", &continueVar);
    if(strcmp(continueVar, "Y") == 0 || strcmp(continueVar,"y") == 0) {
        goto continueScript;
    }
    else if(strcmp(continueVar,"N") == 0 || strcmp(continueVar,"n") == 0) {
        printf("\nScript terminated by user\n");
        exit(EXIT_FAILURE);
    }
    else {   // Prompt the user to answer again if they enter an invalid character
        clearPrevLine();
        printf("Would you like to continue with the installation of Arch Linux (y/n): ");
        fflush(stdout);
        goto continuePrompt;
    }

continueScript:   // Continue the script
    printf("\n");
    green();
    bool uefi = uefiCheck();  // True if system has UEFI
    printf("Would you like to manually partition using FDISK or automatically partition (m/a): ");   // Prompt if the user wants automatic or manual partitioning.
partitionPrompt:
    char partitioning[2];
    scanf("%s", &partitioning);
    if(strcmp(partitioning, "A") == 0 || strcmp(partitioning, "a") == 0) {          // If the user wants automatic partitioning
        green();
        system("fdisk -l");
        white();
        printf("\nSelect an installation disk by typing the disk e.g. sda: ");      // Display the disk options and have the user select one.
        char diskSelection[10];
        scanf("%s", &diskSelection);
        bool diskExists = checkDiskSelect(diskSelection);            // Stores if the disk the user specified exists
        if(diskExists) printf("The disk exists");
    }
    return 0;
}
