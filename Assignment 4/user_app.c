#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/mycdrv"

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CHGACCDIR _IO(CDRV_IOC_MAGIC,1)

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Device number not specified\n");
		return 1;
	}
	char sentence[]= "TESTSTR0";  
	char sentence1[]="1RTSTSET";
	int ret, orignal_dir; 
	int dev_no = atoi(argv[1]);
	char dev_path[20];
	int i,fd;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;
	int dir;
	sprintf(dev_path, "%s%d", DEVICE, dev_no);
	fd = open(dev_path, O_RDWR);
	if(fd == -1) {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}

	printf(" r = read from device after seeking to desired offset\n"
			" w = write to device \n");
	printf(" c = reverse direction of data access");
	printf("\n\n enter command :");

	scanf("%c", &ch);
	switch(ch) {
	case 'w':
		printf("Enter Data to write: ");
		scanf(" %[^\n]", write_buf);
		write(fd, write_buf, sizeof(write_buf));
		break;

	case 'c':
		/******************************/
		
		//printf("Enter the string (Max 10 chars)");
		//scanf("%hhu", sentence);
		//printf("1");
		//*sentence1=*sentence;
		//printf("2");
		printf(" 0 = regular \n 1 = reverse \n");
		strcpy(write_buf, sentence);
		
		printf("Written in buffer:%s\n",write_buf);
    		ret = write(fd, write_buf, sizeof(sentence)-1);
		orignal_dir=ioctl(fd, ASP_CHGACCDIR, 1);   
		printf("Changing direction from %d to 1.\n", orignal_dir);
		ret = read(fd, read_buf, sizeof(sentence)-1); 
    		printf("Read from device:%s\n", read_buf);
		lseek(fd,8,0); 
		strcpy(write_buf, sentence1);
		printf("Written in buffer :%s\n",write_buf);  
		ret = write(fd, write_buf, sizeof(sentence)-1);
		orignal_dir=ioctl(fd, ASP_CHGACCDIR, 0); 
		printf("Changing direction from %d t0 0.\n", orignal_dir); 
		ret = read(fd, read_buf, sizeof(sentence)-1); 
		printf("Read from device:%s\n", read_buf);
		break;


	case 'r':
		printf("Origin \n 0 = beginning \n 1 = current \n 2 = end \n\n");
		printf(" enter origin :");
		scanf("%d", &origin);
		printf(" \n enter offset :");
		scanf("%d", &offset);
		lseek(fd, offset, origin);
		if (read(fd, read_buf, sizeof(read_buf)) > 0) {
			printf("\ndevice: %s\n", read_buf);
		} else {
			fprintf(stderr, "Reading failed\n");
		}
		break;

	default:
		printf("Command not recognized\n");
		break;

	}
	close(fd);
	return 0;
}
