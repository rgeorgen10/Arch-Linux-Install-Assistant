#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void green() {
    printf("\033[0;32m");
}

void white() {
    printf("\033[0m");
}

void clear() {
    system("clear");
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