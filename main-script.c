#include <stdio.h>
#include <stdlib.h>
#include "helper-functions.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int main() {
    printWelcome();    // Print Welcome Text and Script Information
    green();
    printf("Would you like to continue with the installation of Arch Linux (y/n): ");
continuePrompt:       // Ask user to continue
    char continueVar[2];
    scanf("%s", continueVar);
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
    scanf("%s", partitioning);
    if(strcmp(partitioning, "A") == 0 || strcmp(partitioning, "a") == 0) {          // If the user wants automatic partitioning
        system("fdisk -l");
        green();
        printf("\nSelect an installation disk by typing the disk e.g. sda: ");      // Display the disk options and have the user select one.
diskPrompt:
        char diskSelection[64];
        scanf("%s", diskSelection);
        bool diskExists = checkDiskSelect(diskSelection);            // Stores if the disk the user specified exists
        if (diskExists) diskAutoFormat(diskSelection, uefi);     // If disk specified exists, auto format
        else {
            clearPrevLine();
            printf("\nSelect an installation disk by typing the disk e.g. sda: "); 
            fflush(stdout);
            goto diskPrompt;
        }
        white();
        printf("Disks have been successfully partitioned");
    }
    else {  // If user wants to use FDISK
        green();
        printf("Use FDISK to create the following partitions: \n");
        printf("1. A boot partition: 1GB (UEFI Systems Only) \n");
        printf("2. A SWAP Partition: Size Variable \n");
        printf("3. A Root Partition \n");
        printf("4. A Separate Home Partition (Not Required)\n");
        printf("Make Sure to Write Down the Address of Each Partition e.g. /dev/sda1 \n");
        printf("Press Enter to continue to FDISK: ");
        char fdiskCont[128];
        flushInput();
        scanf("%s", fdiskCont);
        system("fdisk -l");
        green();
        printf("\nSelect an installation disk by typing the disk e.g. sda: ");      // Display the disk options and have the user select one.
diskPrompt2:
        char diskSelection[64];
        flushInput();
        scanf("%s", diskSelection);
        bool diskExists = checkDiskSelect(diskSelection);            // Stores if the disk the user specified exists
        if (!diskExists) {
            clearPrevLine();
            printf("\nSelect an installation disk by typing the disk e.g. sda: "); 
            fflush(stdout);
            goto diskPrompt2;
        }
        char fdiskCMD[128] = "fdisk /dev/";
        strcat(fdiskCMD, diskSelection);
        system(fdiskCMD);
        green();
        char bootPart[128];
        if(uefi) {
            printf("\nEnter The Boot Partition Location e.g. sda1: ");
enterBootPartUefi:
            fflush(stdout);
            if (fgets(bootPart, sizeof(bootPart), stdin) == NULL) goto enterBootPartUefi;
            bootPart[strcspn(bootPart, "\n")] = 0;
            if (strlen(bootPart) == 0) goto enterBootPartUefi;
            if(checkDiskSelect(bootPart)) {
                char makeBootPart[128] = "mkfs.fat -F32 -I /dev/";
                strcat(makeBootPart, bootPart);
                system(makeBootPart);
                fflush(stdout); 
            }
            else {
                clearPrevLine();
                fflush(stdout);
                printf("Partition not found: Enter the boot location e.g. sda1: ");
                goto enterBootPartUefi;
            }
        }
        printf("Enter the Swap Partition Location e.g. sda2: ");
enterSwapPartUefi:
        fflush(stdout);
        char swapPart[128];
        if (fgets(swapPart, sizeof(swapPart), stdin) == NULL) goto enterSwapPartUefi;
        swapPart[strcspn(swapPart, "\n")] = 0;
        if (strlen(swapPart) == 0) goto enterSwapPartUefi;
        if(checkDiskSelect(swapPart)) {
            char makeSwapPart[128] = "mkswap -f /dev/";
            strcat(makeSwapPart, swapPart);
            system(makeSwapPart);
            printf("DEBUG");
            char mountSwapPart[128] = "swapon /dev/";
            strcat(mountSwapPart, swapPart);
            system(mountSwapPart);
            goto enterRootPartUefi;
        }
        else {
            clearPrevLine();
            fflush(stdout);
            printf("Partition not found: Enter the Swap location e.g. sda1: ");
            goto enterSwapPartUefi;
        }
        printf("\nEnter the Root Partition Location e.g. sda2: ");
enterRootPartUefi:
        fflush(stdout);
        char rootPart[128];
        if (fgets(rootPart, sizeof(rootPart), stdin) == NULL) goto enterRootPartUefi;
        rootPart[strcspn(rootPart, "\n")] = 0;
        if (strlen(rootPart) == 0) goto enterRootPartUefi;
        if(checkDiskSelect(rootPart)) {
            char makeRootPart[128] = "mkfs.ext4 -F /dev/";
            strcat(makeRootPart, rootPart);
            system(makeRootPart);
            char mountRootPart[128] = "mount /dev/";
            strcat(mountRootPart, rootPart);
            strcat(mountRootPart, " /mnt");
            system(mountRootPart);
            goto baseInstall;
        }
        else {
            clearPrevLine();
            fflush(stdout);
            printf("Partition not found: Enter the Root location e.g. sda1: ");
            goto enterRootPartUefi;
        }
        if(uefi) {
            char mountBootPart[128] = "mount --mkdir /dev/";
            strcat(mountBootPart, bootPart);
            strcat(mountBootPart, " /mnt/boot");
            system(mountBootPart);
            goto enterSwapPartUefi;
        }        
baseInstall:
    }
    return 0;
}
