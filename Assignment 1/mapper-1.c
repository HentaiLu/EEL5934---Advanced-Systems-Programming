#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char word_read[1000],c,key='1';
	char filename[100];
	//printf("Enter Filename\n");
	scanf("%s", filename);
	FILE *fptr;
	if ((fptr=fopen(filename,"r"))==NULL)
	{
		printf("Error! No such filen\n");
		exit(1);
	}

   	else
	{
		c = fscanf(fptr,"%s",word_read);
		while(c!=EOF) 
		{
			printf("(%s,%c)\n",word_read , key);
			c = fscanf(fptr,"%s",word_read);
   		}
	}
	fclose(fptr);
	return 0;
}
