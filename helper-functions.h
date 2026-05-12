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
bool checkDiskSelect(char diskOption[10]) {           // This function could have the bug if the user enters part of the disk name such as sd instead of sda. It will return that sd exists
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
void diskAutoFormat(char diskSelection[10], bool uefi) {
    char diskAddr[14] = "/dev/";
    strcat(diskAddr, diskSelection);     // store the disk to be used
    char unmountDisk[128] = "umount ";
    strcat(unmountDisk, diskAddr);
    strcat(unmountDisk, "1");
    system(unmountDisk);                  // unmount partition 1 on the disk
    char clearDisk[128] = "parted –s ";
    strcat(clearDisk, diskAddr);
    strcat(clearDisk, " mklabel gpt");
    printf(clearDisk);
    system(clearDisk);
    if(uefi) { // replace true with uefi
        char makeBoot[128] = "parted -s ";
        strcat(makeBoot, diskAddr);
        strcat(makeBoot,"1 mkpart primary FAT32 0%% 1G");
        printf(makeBoot);
        system(makeBoot);
    }
    char makeSwap[128] = "parted -s ";
    strcat(makeSwap, diskAddr);
    strcat(makeSwap, "2 mkpart primary mkswap 1001M 8G");
    printf(makeSwap);
    system(makeSwap);
    char makeRoot[128] = "parted -s ";
    strcat(makeRoot, diskAddr);
    strcat(makeRoot, "3 mkpart primary ext4 8001M 100%");
    printf(makeRoot);
    system(makeRoot);
    char mountBoot[128] = "mount /dev/";
    strcat(mountBoot, diskSelection);
    strcat(mountBoot, "1 /mnt"); 
    printf(mountBoot);
    system(mountBoot);
    char mountSwap[128] = "mount /dev/";
    strcat(mountSwap, diskSelection);
    strcat(mountSwap, "2 /mnt");
    printf(mountSwap);
    system(mountSwap);
    char mountRoot[128] = "mount /dev/";
    strcat(mountRoot, diskSelection);
    strcat(mountRoot, "3 /mnt");
    printf(mountRoot);
    system(mountRoot);
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