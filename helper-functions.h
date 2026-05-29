#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void green() {
    printf("\033[0;32m");
}

void white() {
    printf("\033[0;37m");
}

void clear() {
    system("clear");
}

void clearPrevLine() {
    printf("\033[A\033[2K");
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

int cpuBrand() {             // return 1 for AMD, 2 for Intel, 3 for Neither/Unknown
    char output[16];
    char amdCheck[512] = "";
    char intelCheck[512] = "";

    FILE *cpuCmd = popen("lscpu | grep AMD", "r");
    if(cpuCmd == NULL) return false;
    while(fgets(output,sizeof(output), cpuCmd) != NULL) {
        strcat(amdCheck, output);
    }
    pclose(cpuCmd);
    if (!strcmp(amdCheck, "") == 0) {     // if AMD, return 1
        return 1;
    }


    FILE *cpuCmd2 = popen("lscpu | grep Intel", "r");
    if(cpuCmd2 == NULL) return false;
    while(fgets(output,sizeof(output), cpuCmd2) != NULL) {
        strcat(intelCheck, output);
    }
    pclose(cpuCmd2);
    if (!strcmp(intelCheck, "") == 0) {    // if Intel, return 2
        return 2;
    }

    return 3;             // if neither, return 3

}

bool checkDiskSelect(char diskOption[64]) {    
    char output[512];
    char finalOutput[1024] = "";
    char commandString[50] = "lsblk -f | grep -w ";   // -w check for whole disk option phrase
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
    char diskAddr[32] = "/dev/";
    strcat(diskAddr, diskSelection);     // store the disk to be used
    if(isdigit(diskSelection[strlen(diskSelection) - 1])) { // checks if partitions are labeled as p1 instead of just 1 ex: /dev/nvme0n1p3
        strcat(diskAddr, "p"); //
    }
    char unmountDisk[128] = "umount ";
    strcat(unmountDisk, diskAddr);
    strcat(unmountDisk, "1");
    // printf(unmountDisk);
    system(unmountDisk);                  // unmount partition 1 on the disk
    char clearDisk[128] = "parted -s ";
    strcat(clearDisk, diskAddr);
    strcat(clearDisk, " mklabel gpt");
    // printf(clearDisk);
    system(clearDisk);
    if(uefi) { // replace true with uefi
        char makeBoot[128] = "parted -s /dev/";
        strcat(makeBoot, diskSelection);
        strcat(makeBoot," mkpart primary FAT32 0% 1G");
        // printf(makeBoot);
        system(makeBoot);

        char makeSwap[128] = "parted -s /dev/";
        strcat(makeSwap, diskSelection);
        strcat(makeSwap, " mkpart primary linux-swap 1001M 8G");
        // printf(makeSwap);
        system(makeSwap);

        char makeRoot[128] = "parted -s /dev/";
        strcat(makeRoot, diskSelection);
        strcat(makeRoot, " mkpart primary ext4 8001M 100%");
        // printf(makeRoot);
        system(makeRoot);

        char formatBoot[128] = "mkfs.fat -F32 -I ";   // char array, lowercase 'fat'
        strcat(formatBoot, diskAddr);
        strcat(formatBoot, "1");
        // printf(formatBoot);
        system(formatBoot);

        char formatSwap[128] = "mkswap -f ";           // don't forget swap!
        strcat(formatSwap, diskAddr);
        strcat(formatSwap, "2");
        // printf(formatSwap);
        system(formatSwap);

        char formatRoot[128] = "mkfs.ext4 -F ";        // char array
        strcat(formatRoot, diskAddr);
        strcat(formatRoot, "3");
        // printf(formatRoot);
        system(formatRoot);

        char mountRoot[128] = "mount ";
        strcat(mountRoot, diskAddr);
        strcat(mountRoot, "3 /mnt"); 
        // printf(mountRoot);
        system(mountRoot);

        char mountSwap[128] = "swapon ";
        strcat(mountSwap, diskAddr);
        strcat(mountSwap, "2");
        // printf(mountSwap);
        system(mountSwap);

        char mountBoot[128] = "mount --mkdir ";
        strcat(mountBoot, diskAddr);
        strcat(mountBoot, "1 /mnt/boot");
        // printf(mountBoot);
        system(mountBoot);
    }
    else {  // Case for Legacy BIOS systems
        char mbr[128] = "parted -s /dev/";
        strcat(mbr, diskSelection);
        strcat(mbr, " mklabel msdos");
        system(mbr);
        
        char makeSwap[128] = "parted -s /dev/";
        strcat(makeSwap, diskSelection);
        strcat(makeSwap, " mkpart primary linux-swap 0% 8G");
        // printf(makeSwap);
        system(makeSwap);

        char makeRoot[128] = "parted -s /dev/";
        strcat(makeRoot, diskSelection);
        strcat(makeRoot, " mkpart primary ext4 8001M 100%");
        // printf(makeRoot);
        system(makeRoot);

        char formatSwap[128] = "mkswap -f ";           // don't forget swap!
        strcat(formatSwap, diskAddr);
        strcat(formatSwap, "1");
        // printf(formatSwap);
        system(formatSwap);

        char formatRoot[128] = "mkfs.ext4 -F ";        // char array
        strcat(formatRoot, diskAddr);
        strcat(formatRoot, "2");
        // printf(formatRoot);
        system(formatRoot);

        char mountRoot[128] = "mount ";
        strcat(mountRoot, diskAddr);
        strcat(mountRoot, "2 /mnt"); 
        // printf(mountRoot);
        system(mountRoot);

        char mountSwap[128] = "swapon ";
        strcat(mountSwap, diskAddr);
        strcat(mountSwap, "2");
        // printf(mountSwap);
        system(mountSwap);
    }
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

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}