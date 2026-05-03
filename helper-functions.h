#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void green() {
    printf("\033[0;32m");
}

void white() {
    printf("\033[0m");
}

void clear() {
    system("clear");
}

void clearPrevLine() {
    printf("\033[F");
}

bool uefiCheck() {
    char output[2];
    char finalOutput[3] = "";
    FILE *fp = popen("cat /sys/firmware/efi/fw_platform_size", "r");

    if(fp == NULL) return false;

    while(fgets(output, sizeof(output), fp) != NULL) { // Take the output of each character and add it the finalOutput
        strcat(finalOutput, output);
    }
    pclose(fp);
    if(strcmp(finalOutput, "64\n") == 0 || strcmp(finalOutput, "32\n") == 0) {  // Check if system is UEFI
        return true;
    }    
    return false;

}
bool checkDiskSelect(char diskOption[10]) {
    char output[512];
    char finalOutput[1024] = "";
    char commandString[50] = "lsblk -d | grep ";
    strcat(commandString, diskOption);
    FILE *fp = popen(commandString, "r");
    if (fp == NULL) return false;

    while(fgets(output, sizeof(output), fp) != NULL) {       // Take the output of each character and add it the finalOutput
        strcat(finalOutput, output);
    }
    pclose(fp);
    if(strstr(finalOutput, diskOption) != NULL) {          // Check if the output contains the diskOption, if it does, the disk exists
        return true;
    }    
    return false;
    
}
void printWelcome() {
    green();
    printf("|----------------------------------------------------| \n");
    printf("|            Arch Linux Install Assistant            | \n");
    printf("|          --------------------------------          | \n");
    printf("|      This script is designed to assist in the      | \n");
    printf("|    installation of Arch Linux. This script will    | \n");
    printf("|           assist with the following:               | \n");
    printf("| 1) Formatting Disks                                | \n");
    printf("| 2) Installing base system                          | \n");
    printf("| 3) Configuring base system                         | \n");
    printf("| 4) Installing a bootloader (GRUB)                  | \n");
    printf("| 5) Adding Sudo and Users                           | \n");
    printf("| 6) Installing a Display Server                     | \n");
    printf("| 7) Installing a Display Manager                    | \n");
    printf("| 8) Installing a Desktop Environment                | \n");
    printf("|----------------------------------------------------| \n");
    white();
}