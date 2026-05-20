#include <stdio.h>
#include <stdlib.h>
#include "helper-functions.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int main() {
    printWelcome();    // Print Welcome Text and Script Information

    // Disk Vars as global vars
    char diskSelection[64];
    char rootPart[128];
    char rootDisk[64];

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
        scanf("%s", diskSelection);
        bool diskExists = checkDiskSelect(diskSelection);            // Stores if the disk the user specified exists
        white();
        if (diskExists) diskAutoFormat(diskSelection, uefi);     // If disk specified exists, auto format
        else {
            clearPrevLine();
            printf("\nSelect an installation disk by typing the disk e.g. sda: "); 
            fflush(stdout);
            goto diskPrompt;
        }
        green();
    }
    else {  // If user wants to use FDISK
        green();
        printf("Use FDISK to create the following partitions: \n");
        printf("1. A boot partition: 1GB (UEFI Systems Only) \n");
        printf("2. A SWAP Partition: Size Variable \n");
        printf("3. A Root Partition \n");
        printf("4. A Separate Home Partition (Not Required)\n");
        printf("Make Sure to Write Down the Address of Each Partition e.g. /dev/sda1 \n");
        printf("Type y and press enter to continue to FDISK: ");
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
        system(fdiskCMD);                                       // Open fdisk for the selected disk
        green();
        printf("Do you want to format another disk? (y/n:) ");  // prompt to format more disks
        char formatMore[32];
        scanf("%s", formatMore);
        if(strcmp(formatMore, "y") == 0 || strcmp(formatMore, "Y") == 0) {  
            clearPrevLine();
            fflush(stdout);
            printf("\nSelect an installation disk by typing the disk e.g. sda: ");
            goto diskPrompt2;
        }
        char bootPart[128];
        if(uefi) {
            printf("\nEnter The Boot Partition Location e.g. sda1: ");
enterBootPartUefi:
            fflush(stdout);
            if (fgets(bootPart, sizeof(bootPart), stdin) == NULL) goto enterBootPartUefi; 
            bootPart[strcspn(bootPart, "\n")] = 0;
            if (strlen(bootPart) == 0) goto enterBootPartUefi;
            if(checkDiskSelect(bootPart)) {    // format boot partition if UEFI
                white();
                char makeBootPart[128] = "mkfs.fat -F32 -I /dev/";
                strcat(makeBootPart, bootPart);
                system(makeBootPart);
                fflush(stdout); 
                green();
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
        if(checkDiskSelect(swapPart)) {  // make swap partition and start SWAP
            white();
            char makeSwapPart[128] = "mkswap -f /dev/";
            strcat(makeSwapPart, swapPart);
            system(makeSwapPart);
            char mountSwapPart[128] = "swapon /dev/";
            strcat(mountSwapPart, swapPart);
            system(mountSwapPart);
            green();
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
        if (fgets(rootPart, sizeof(rootPart), stdin) == NULL) goto enterRootPartUefi;
        rootPart[strcspn(rootPart, "\n")] = 0;
        if (strlen(rootPart) == 0) goto enterRootPartUefi;
        if(checkDiskSelect(rootPart)) {  // format and mount root partition
            white();
            char makeRootPart[128] = "mkfs.ext4 -F /dev/";
            strcat(makeRootPart, rootPart);        
            system(makeRootPart);
            char mountRootPart[128] = "mount /dev/";
            strcat(mountRootPart, rootPart);
            strcat(mountRootPart, " /mnt");
            system(mountRootPart);
            strcpy(rootDisk, rootPart); // Set the root disk variable for grub install on BIOS systems
            rootDisk[strlen(rootDisk) - 1] = '\0';
            if(isdigit(rootDisk[strlen(rootDisk) - 1])) {
                rootDisk[strlen(rootDisk) - 1] = '\0';
            }
            green();
        }
        else {
            clearPrevLine();
            fflush(stdout);
            printf("Partition not found: Enter the Root location e.g. sda1: ");
            goto enterRootPartUefi;
        }
        if(uefi) {    // mount boot partition if UEFI
            char mountBootPart[128] = "mount --mkdir /dev/";
            strcat(mountBootPart, bootPart);
            strcat(mountBootPart, " /mnt/boot");
            system(mountBootPart);
        }        
        printf("\nEnter the Home Partition Location e.g. sda2 (Enter n if you don't want a separate home partition): ");
enterHomePartUefi:
        fflush(stdout);
        char homePart[128];
        if (fgets(homePart, sizeof(homePart), stdin) == NULL) goto enterHomePartUefi;
        homePart[strcspn(homePart, "\n")] = 0;
        if (strlen(homePart) == 0) goto enterHomePartUefi;
        if (strcmp(homePart, "n") == 0 || strcmp(homePart, "N") == 0) {
            goto baseInstall;
        }
        else {
            if(checkDiskSelect(homePart)) {  // format and mount home partition if requested by the user
                white();
                char makeHomePart[128] = "mkfs.ext4 -F /dev/";
                strcat(makeHomePart, homePart);        
                system(makeHomePart);
                char mountHomePart[128] = "mount --mkdir /dev/";
                strcat(mountHomePart, homePart);
                strcat(mountHomePart, " /mnt/home");
                system(mountHomePart);
                green();
            }
            else {
                clearPrevLine();
                fflush(stdout);
                printf("Partition not found: Enter the Home location e.g. sda1: ");
                goto enterHomePartUefi;
            }      
        }  
    }
baseInstall:
    printf("\nPartitioning Has Been Completed!\n");
continueBaseInstallPrompt:
    green();
    printf("Continue to Base Install (y/n): ");
    char doBaseInstall[32];
    scanf("%s", doBaseInstall);
    if(!(strcmp(doBaseInstall, "y") == 0 || strcmp(doBaseInstall, "Y") == 0)) {
        printf("Script Terminated By User After Partitioning\n");
        exit(0);
    } 
    system("touch /mnt/etc/vsconsole.conf");
    printf("Select the kernel you want to install: \n");
    printf("1. Up to date Linux Kernel (linux)\n");
    printf("2. Long Term Support Linux Kernel (linux-lts)\n");
    printf("3. Install Both Kernels\n");
    printf("Type the number with the corresponding option and press enter: ");
    char kernelOption[16];
    scanf("%s", kernelOption);
    white();
    if(strcmp(kernelOption, "1") == 0) {  // The user wants to install linux kernel
        system("pacstrap -K /mnt base linux linux-firmware");
    }
    else if(strcmp(kernelOption, "2") == 0) {  // The user wants to install linux-lts kernel
        system("pacstrap -K /mnt base linux-lts linux-firmware");
    }
    else if(strcmp(kernelOption, "3") == 0) {  // The user wants to install linux, linux-lts kernel
        system("pacstrap -K /mnt base linux linux-lts linux-firmware");
    }
    else {   // The user didn't select a valid option, ask again
        green();
        clearPrevLine();
        fflush(stdout);
        goto continueBaseInstallPrompt;
    }
    green();
    printf("The base system has been installed! Now we can continue to configuring the new system.\n");
    printf("Generating fstab\n");
    white();
    system("genfstab -U /mnt >> /mnt/etc/fstab");
    green();
    printf("Entering Arch Chroot\n");
    printf("Lets set the timezone of the system! Enter the timezone as Area/Location e.g. America/Denver: ");
    char timeZone[128];
    scanf("%s", timeZone);
    char timeZoneCmd[256] = "arch-chroot /mnt ln -sf /usr/share/zoneinfo/";
    strcat(timeZoneCmd, timeZone);
    system(timeZoneCmd);
    system("arch-chroot /mnt hwclock --systohc");
    printf("Timezone has been set!\n");
setLocales:
    printf("Enter the locale you would like to use e.g. en_US: ");
    char locale[32];
    scanf("%s", locale);

    FILE *localePtr;
    localePtr = fopen("/mnt/etc/locale.gen", "a");  // Edit the file to add locales
    char localeString[32] = "\n#";
    strcat(localeString, locale);
    fprintf(localePtr, "%s", localeString);
    fclose(localePtr);

    FILE *langPtr;
    langPtr = fopen("/mnt/etc/locale.conf", "a"); // Edit the file to add the LANG var
    char langString[32] = "LANG=";
    strcat(langString, locale);
    strcat(langString, ".UTF-8");
    fprintf(langPtr, "%s", langString);
    fclose(langPtr);

    printf("The locale has been added. Do you want to add another (y/n): ");
    char addLocale[16];
    scanf("%s", addLocale);
    if(strcmp(addLocale, "y") == 0 || strcmp(addLocale, "Y") == 0) {  // Prompt if the user wants to add more locales
        clearPrevLine();
        fflush(stdout);
        goto setLocales;
    }
    printf("Type the hostname that you want to configure (Your computer's name on the network): "); // Add the hostname file
    char hostname[128];
    scanf("%s", hostname);
    FILE *hostnamePtr;
    hostnamePtr = fopen("/mnt/etc/hostname", "a");
    fprintf(hostnamePtr, "%s", hostname);
    fclose(hostnamePtr);
    
    printf("Generate INTRAMFS\n");  // generate inframfs
    white();
    system("arch-chroot /mnt mkinitcpio -P");
    green();
    printf("Enter your root password:\n");
    white();
    system("arch-chroot /mnt passwd");
    green();
    printf("Installing GRUB bootloader package\n");
    white();
    system("arch-chroot /mnt pacman -S grub --noconfirm");
    green();
    printf("Would you like to install os-prober to detect other operating systems for dual boot (y/n): ");
    char dualBoot[16];
    scanf("%s", dualBoot);
    if(strcmp(dualBoot, "y") == 0 || strcmp(dualBoot, "Y") == 0) {
        white();
        system("arch-chroot /mnt pacman -S os-prober --noconfirm");
    }
    green();
    printf("Installing GRUB bootloader to the system\n");
    if(uefi) {
        system("arch-chroot /mnt pacman -S efibootmgr --noconfirm");
        system("arch-chroot /mnt grub-install --target=x86_64-efi --efi-directory=/boot --bootloader-id=ARCH");
    }
    else {
        if(strcmp(partitioning, "A") == 0 || strcmp(partitioning, "a") == 0) { // if the user chose auto partitioning, we know that partition disk2 is the root partition
            char grubCmd[128] = "arch-chroot /mnt grub-install --target=i386-pc /dev/";
            strcat(grubCmd, diskSelection);
            system(grubCmd);
        }
        else {
            char grubCmd[128] = "arch-chroot /mnt grub-install --target=i386-pc /dev/"; // otherwise, the user gave us the root partition when manual partioning
            strcat(grubCmd, rootDisk);
            system(grubCmd);
        }
    }
    system("arch-chroot /mnt grub-mkconfig -o /boot/grub/grub.cfg");
    green();
    getchar();
    printf("Grub has been installed into the system!\n");
    white();
    system("arch-chroot /mnt pacman -S networkmanager --noconfirm");
    printf("\n%s", rootDisk);
    return 0;
}
