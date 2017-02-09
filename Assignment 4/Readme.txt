ASSIGNMENT 4

Vaibhav Mittal

Following steps have to be followed-

Step 1: Extract 4 files which are listed below -
	1) driver.c
	2) userapp.c
	3) MakeFile
	4) ReadMe.txt

Step 2: Enter the extracted directory
Step 3: Enter "make clean"
Step 4: Enter "make". This will create files - char_driver.ko and userapp
Step 5:	Login as super user - sudo su. 
Step 6: Enter "insmod driver.ko NUM_DEVICES=x", where x can be any number. Default is 3.
Step 7: "ls /dev" checks for device created.
Step 8: Enter "./userapp x", where x is the device number you want to use. Suggestion is to do write first and then read because then it will have something to read.

ioctl is modified in the userapp to test the functionality.

NOTE : During the entire process, we can always use dmesg command to understand the flow.
       You can also remove the kernels made using the command - rmmod char_driver.ko

Known Bug : "insmod: ERROR: could not insert module char_driver.ko: File exists".
This happens when you run the command - insmod char_driver.ko NUM_DEVICES=x and the devices are already created. Care should be taken that you do not duplicate this command.
