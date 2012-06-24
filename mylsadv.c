#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include <sys/stat.h>
#include<string.h>
void main()
{

//	const char *name="/home/dell/project";
	char *path,*currentdir;
	path = (char*)malloc(100 * sizeof (char));
	currentdir = (char*)malloc(100 * sizeof (char));

	getcwd(currentdir,100);
	const char *name=currentdir;

	DIR *ptr;
	
	ptr=opendir(name);

	struct dirent *content;
	struct stat *status;

	content=readdir(ptr);

	while(content!=NULL)
	{
		strcpy(path,name);
	
	      	int len = strlen(path);

		path[len]='/';

		path[len+1]='\0';

	//	printf("%s\t\n",content->d_name);

		strcat(path,content->d_name);

		stat(path,status);

	//	printf("kkk\n");

		printf("%-100s%jd bytes\n",content->d_name,status->st_size);

	//	printf("ko\n");
		content=readdir(ptr);
	}
}
