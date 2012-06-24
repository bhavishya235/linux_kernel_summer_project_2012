/*
  This program uses fsync() system call and saves the data directly to the disk
  skipping the buffer stage. This is useful as if we are typing some information
  and suddenly our system crashes/sht downs. Don't worry in these conditions as 
  your data would be saved*/

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdio.h>
const char* filename;
void write_entry(char *);
void main(int argc,char *argv[])
{
	if(argc!=2)
	{
		printf("USAGE>>>$./persave [file name]\n");
		exit(0);
	}

	filename=argv[1];
	char c;
	printf("ENTER DATA\n");
	while(c!=EOF)
	{
		scanf("%c",&c);
		write_entry(&c);
	}
	
}
void write_entry (char* entry)
{
int fd = open (filename, O_WRONLY | O_CREAT | O_APPEND, 0660);
write (fd, entry, strlen (entry));
//write (fd, "\n", 1);
fsync (fd);
close (fd);
}
