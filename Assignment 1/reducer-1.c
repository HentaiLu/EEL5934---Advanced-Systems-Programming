#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char const *argv[])
{
	char word[100];
	char key_str[100][50];
	long int freq[100];
	int i=0,listed=0,j,k;
	int index=0;
	int p_index=0;
	
	while(scanf("%s",word) != EOF)
		{
			listed=0;
			char * key,*key_value,*temp;
			key = strtok(word," (,)");
                        key_value = strtok(NULL," (),");
			long int integer_key_value = strtol(key_value, &temp,10);
			
			if (index==0)

			{
				strcpy(key_str[0],key);
				freq[0]=integer_key_value;
			}

			else
			{
				if(key[0] > key_str[index-1][0])
				{		
					
					for(k=p_index;k < index;k++)
					{
					printf("(%s,%ld)\n", key_str[k] , freq[k]);
					}
					p_index=index;
					strcpy(key_str[index],key);
					freq[index]+=integer_key_value;
					index=index+1;
				}
				else
				{
					for(j=0;j<index;j++)
					{
						if(strcmp(key,key_str[j])==0)
						{
							freq[j]+=integer_key_value;
							listed=1;
						}
					}
					if(!listed)
					{
						strcpy(key_str[index],key);
						freq[index]+=integer_key_value;
						index=index+1;
					}
				}
			}   
			
			if(index == 0)
			index=index+1;
		}
					
	for(k=p_index;k < index;k++)
	{
		printf("(%s,%ld)\n", key_str[k] , freq[k]);
	}		
	return 0;

}
