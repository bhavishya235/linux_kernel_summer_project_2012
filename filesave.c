#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdio.h>
const char* filename = "journal.log";
void write_entry(char *);
void main()
{
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
