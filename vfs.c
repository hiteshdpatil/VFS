#define _CRT_SECURE_NO_WARNINGS  // to disable deprecation warnings

#include<stdio.h>    // header file for standard input output
#include<stdlib.h>  // header file for using functions like malloc() and free()
#include<string.h> // header file for using string related predefined functions
#include<time.h>    // for clock() function
#include<io.h>      // for _write() function
#include<windows.h>

#define MAXINODE 50  // for creating 50 Inodes in DILB

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024  // maximum size(in bytes) of any file created in project
#define REGULAR 1     // type of file
#define SPECIAL 2     // type of file

#define START   0     //  offset at beginning
#define END     2     //  offset at end
#define CURRENT 1     //  offset at current position


// Structure for superblock
typedef struct superblock
{
	int TotalInodes;
	int FreeInodes;
}SUPERBLOCK, *PSUPERBLOCK;

// Structure for Inode
typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char* Buffer;
	int LinkCount;
	int ReferenceCount;
	int Permission;
	struct inode* next;
}INODE, * PINODE, ** PPINODE;

// Structure for FileTable
typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE ptrinode;
}FILETABLE,* PFILETABLE;

// Structure for UFDT
typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;

// global variables
UFDT UFDTArr[MAXINODE];      // Array of structutre which contain a pointer to file table
SUPERBLOCK SUPERBLOCKobj;   //  An object of SuperBlock
PINODE head = NULL;         //  head for Inode linked list

// Function To Initialise the SuperBlock
void InitialiseSuperBlock()
{
	int i=0;
	while(i< MAXINODE)
	{
		UFDTArr[i].ptrfiletable = NULL;   // Initializing every pointer in UFDT Array to NULL
		i++;
	}

	SUPERBLOCKobj.TotalInodes = MAXINODE;    // Total Inodes present will be always equal to MAXINODE
	SUPERBLOCKobj.FreeInodes  = MAXINODE;     //  Initially all Inodes will be Free

}

// Function to create Disk Inode List Block
void CreateDILB()
{
	int i = 0;
	PINODE newn = NULL;
	PINODE temp = head;

	while(i<= MAXINODE)
	{
		newn = (PINODE)malloc(sizeof(INODE));

		newn->ReferenceCount = 0;
	    newn->LinkCount = 0;
	    newn->FileSize = 0;
	    newn->FileType = 0;
	    newn->Buffer = NULL;
	    newn->InodeNumber = i;
	    newn->next = NULL;

	    if(temp == NULL)
	    {
	    	head = newn;
	    	temp = head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}

		i++;
	}
}

// Function to display all files in current directory
void ls_file()
{
	int i=0;
	PINODE temp = head;

	if(SUPERBLOCKobj.FreeInodes == MAXINODE)
	{
		printf("Error: There are no files\n");
		return;
	}

	printf("\nFileName\tInode number\tFile size\tLink count\n");
	printf("----------------------------------\n");

	while(temp != NULL)
	{
		if(temp->FileType != 0)
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName, temp->InodeNumber, temp->FileSize, temp->LinkCount);
		temp = temp->next;
	}

	printf("----------------------------------\n");
}

// Function to close all the currently opened files
void CloseAllFile()
{
	int i=0;

	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
			UFDTArr[i].ptrfiletable->writeoffset = 0;
			(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
			break;
		}
		i++;
	}
}

// Function which displays short info to help the user
void DisplayHelp()
{
	printf("ls : To list all files\n");
	printf("clear : To clear comsole\n");
	printf("open : To open file\n");
	printf("closeall : To close all opened files\n");
	printf("read : To read the contents from file\n");
	printf("write : To write contents into file\n");
	printf("exit : To terminate the file system\n");
	printf("stat : To display file using name\n");
	printf("fstat : To display file using file descripter\n");
	printf("truncate : To remove all data from file\n");
	printf("rm : To delete the file");
}

// FUnction used to delay certain point
void delay(int x)
{
	clock_t temp = clock();
	while(clock() < (temp + x));
}

// Function to display the Statistical Information about file with the help of file name
int stat_file(char* name)
{
	PINODE temp = head;
	int i=0;

	if(name == NULL)
	{
		return -1;
	}

	while(temp != NULL)
	{
		if(strcmp(name , temp->FileName) == 0)
			break;

		temp = temp->next;
	}

	if(temp == NULL)
		return -2;

	printf("\n----------Statistical Information about file----------\n");
	printf("File Name: %s\n",temp->FileName);
	printf("Inode Number: %d\n",temp->InodeNumber);
	printf("File Size: %d\n",temp->FileSize);
	printf("Actual File Size: %d\n",temp->FileActualSize);
	printf("Link Count: %d\n",temp->LinkCount);
	printf("Reference Count: %d\n",temp->ReferenceCount);

	if(temp->Permission == 1)
				printf("File Permission : Read Only\n");
	else if(temp->Permission == 2)
				printf("File Permission : Write Only\n");
	else if(temp->Permission == 3)
				printf("File Permission : Read & Write\n");

	return 0;
}

// Function to display the Statistical Information about file with the help of file descriptor
int fstat_file(int fd)
{
	int i=0;
	PINODE temp = head;

	if(fd == 0)
		return -1;

	if(UFDTArr[fd].ptrfiletable == NULL)
		return -2;

	temp = UFDTArr[fd].ptrfiletable->ptrinode;

	printf("\n-----------Statistical Information about file-----------\n");
	printf("File Name: %s\n",temp->FileName);
	printf("Inode Number: %d\n",temp->InodeNumber);
	printf("File Size: %d\n",temp->FileSize);
	printf("Actual File Size: %d\n",temp->FileActualSize);
	printf("Link Count: %d\n",temp->LinkCount);
	printf("Reference Count: %d\n",temp->ReferenceCount);

	if(temp->Permission == 1)
				printf("File Permission : Read Only\n");
	else if(temp->Permission == 2)
				printf("File Permission : Write Only\n");
	else if(temp->Permission == 3)
				printf("File Permission : Read & Write\n");

	return 0;
}

// Function to get file descriptor of particular file
int GetFDFromName(char* name)
{
	int i=0;
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(_stricmp(UFDTArr[i].ptrfiletable->ptrinode->FileName , name) == 0)
				break;
		}

		i++;
	}

	if(i==50)
		return -1;
	return i;
}

// Function to close a single file
int CloseFileByName(char* name)
{
	int i=0;
	i = GetFDFromName(name);

	if(i == -1)
		return -1;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

	return 0;
}

// Function to deelte the created file
int rm_File(char* name)
{
	int fd = 0;

	fd = GetFDFromName(name);

	if(fd == -1)
		return fd;

	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
		free(UFDTArr[fd].ptrfiletable);
	}

	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInodes)++;

	return 0;
}

// Function which describes particular command
void man(char* name)
{
	if(name == NULL)
		return;

	if(_stricmp(name,"create") == 0)
	{
		printf("Description : Used to create new regular file\n");
		printf("Usage : create File_Name Permission\n");
	}
	else if(_stricmp(name,"read") == 0)
	{
		printf("Description : Used to read data from regular file\n");
		printf("Usage : read File_Name No_Of_Bytes_To_Read\n");
	}
	else if(_stricmp(name,"write") == 0)
	{
		printf("Description : Used to write data from regular file\n");
		printf("Usage : write FIle_Name\n After this enter the data you want to write");
	}
	else if(_stricmp(name,"ls") == 0)
	{
		printf("Description : Used to list all information of files\n");
		printf("Usage : ls\n");
	}
	else if(_stricmp(name,"stat") == 0)
	{
		printf("Description : Used to display information of file\n");
		printf("Usage : stat File_Name\n");
	}
	else if(_stricmp(name,"fstat") == 0)
	{
		printf("Description : Used to display information of file\n");
		printf("Usage : fstat File_Descriptor\n");
	}

	else if(_stricmp(name,"truncate") == 0)
	{
		printf("Description : Used to remove data from file\n");
		printf("Usage : truncate File_Name\n");
	}
	else if(_stricmp(name,"open") == 0)
	{
		printf("Description : Used to open existing file\n");
		printf("Usage : open File_Name mode\n");
	}
	else if(_stricmp(name,"close") == 0)
	{
		printf("Description : Used to close opened file\n");
		printf("Usage : close File_Name\n");
	}
	else if(_stricmp(name,"closeall") == 0)
	{
		printf("Description : Used to close all opened files\n");
		printf("Usage : closeall\n");
	}
	else if(_stricmp(name,"lseek") == 0)
	{
		printf("Description : Used to change file offset\n");
		printf("Usage : lseek File_Name ChangeInOffset StartPoint\n");
	}
	else if(_stricmp(name,"rm") == 0)
	{
		printf("Description : Used to delete the file\n");
		printf("Usage : rm File_Name\n");
	}
	else
	{
		printf("Error : No manual entry available\n");
	}
}

// Function to write data into the file
int WriteFileX(int fd,char* arr, int isize)
{
	if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ+WRITE))
		return -1;

	if(UFDTArr[fd].ptrfiletable->writeoffset == MAXINODE)
		return -2;

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
		return -3;

	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);
	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;
	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

	return isize;
}

// Function to clear all data from a file
int truncate_File(char* name)
{
	int fd = GetFDFromName(name);
	if(fd == -1)
		return -1;

	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
	return 0;
}

// Function to get the inode number of a particular file
PINODE Get_Inode(char* name)
{
	int i=0;
	PINODE temp = head;

	if(name == NULL)
		return NULL;

	while(temp != NULL)
	{
		if(strcmp(name,temp->FileName) == 0)
			break;

		temp = temp->next;
	}
	return temp;
}

// Function to create a new file with certain permission
int CreateFileX(char* name, int permission)
{
	int i=0;
	PINODE temp = head;

	if(name == NULL || permission == 0 || permission >3)
		return -1;

	if(SUPERBLOCKobj.FreeInodes == 0)
		return -2;

	if(Get_Inode(name) != NULL)
		return -3;

	while(temp != NULL)
	{
		if(temp->FileType == 0)
			break;

		temp = temp->next;
	}

	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
			break;
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDTArr[i].ptrfiletable == NULL)
		return -4;

	UFDTArr[i].ptrfiletable->count =1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset= 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize =0;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount =1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;
	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
	UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFILESIZE);
	memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer,0,1024);

	(SUPERBLOCKobj.FreeInodes)-- ;

	return i;
}

// Function to open a new file with certain mode
int OpenFileX(char* name , int mode)
{
	int i=0;
	PINODE temp = head;

	if(name == NULL || mode <=0)
		return -1;

	temp = Get_Inode(name);

	if(temp == NULL)
		return -2;

	if(temp->Permission < mode)
		return -3;

	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
			break;
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
	}

	UFDTArr[i].ptrfiletable->count =1;
	UFDTArr[i].ptrfiletable->mode = mode;

	if(mode == READ+WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset= 0;
	}
	else if(mode == READ)
	{
			UFDTArr[i].ptrfiletable->readoffset =0;
	}
	else if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount ++;

	return i;
}

// Function to read a new file's data
int ReadFileX(int fd ,char* arr,int isize)
{
	int read_size =0;

	if(UFDTArr[fd].ptrfiletable == NULL)
		return -1;

	if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
		return -2;

	if(UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE)
		return -2;

	if(UFDTArr[fd].ptrfiletable->readoffset  == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
		return -3;
	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
		return -4;

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - UFDTArr[fd].ptrfiletable->readoffset;
	if(read_size < isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + isize;
	}

	return isize;
}

// Function to  change the read or write offset of a  file
int LseekFile(int fd, int size, int from)
{
	if(fd < 0 || from > 2)
		return -1;

	if(UFDTArr[fd].ptrfiletable == NULL)
		return -2;

	if(UFDTArr[fd].ptrfiletable->mode == READ  || UFDTArr[fd].ptrfiletable->mode == READ+WRITE)
	{
		if(from == CURRENT)
		{
			if((UFDTArr[fd].ptrfiletable->readoffset + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				return -1;
			if(UFDTArr[fd].ptrfiletable->readoffset + size < 0)
				return -1;

			UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + size;
		}

		else if(from == START)
		{
			if(size > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
				return -1;
			if(size < 0)
				return -1;

			UFDTArr[fd].ptrfiletable->readoffset = size;
		}

		else if(from == END)
		{
			if(size + (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) > MAXFILESIZE)
				return -1;
			if(UFDTArr[fd].ptrfiletable->readoffset + size < 0)
				return -1;

			UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size;
		}
	}

	else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if(from == CURRENT)
		{
			if((UFDTArr[fd].ptrfiletable->writeoffset + size) > MAXFILESIZE);
				return -1;
			if(UFDTArr[fd].ptrfiletable->writeoffset + size < 0)
				return -1;

			if((UFDTArr[fd].ptrfiletable->writeoffset + size)>( UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize));
			UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize
				= UFDTArr[fd].ptrfiletable->writeoffset + size;

			UFDTArr[fd].ptrfiletable->writeoffset = UFDTArr[fd].ptrfiletable->writeoffset + size ;

		}
		else if(from == START)
		{
			if(size >MAXFILESIZE)
				return -1;
			if(size < 0)
				return -1;
			if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = size;
			UFDTArr[fd].ptrfiletable->writeoffset = size;
		}
		else if(from == END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
				return -1;
			if(UFDTArr[fd].ptrfiletable->writeoffset + size < 0)
				return -1;

			UFDTArr[fd].ptrfiletable->writeoffset  = UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size;
		}
	}
	return fd;
}

//// Function for user authentication
void Authenticate()
{
	while(1)
	{
		char password[25], test;
		int i=0;
		printf("Enter password\t");
		for(;i<25;i++)
		{
			test = getch();

			if(test == 13)
				break;

			if(test == 8)
			{
				if(i<2)
				  continue;
				i = i-2;
				printf("%c",8);
				printf(" ");
				printf("%c",8);
				continue;
			}
			password[i]  = test;
			printf("*");
		}
		password[i] = '\0';

		if(strcmp(password,"Hitesh") != 0)
		{
			char res;
			printf("\nIncorrect password\n E : Exit \t T : try again\n\t");
			scanf("%c",&res);
			printf("\n");

			 if(res == 'T' || res =='t')
				continue;
			else
			{
				exit(0);
			}
		}
		else
			break;

	}

}

//Entry point function
int main()
{
    Authenticate();

	char* ptr = NULL;
	int ret = 0, fd = 0, count = 0;

	char command[4][80];    //  character array to accept the command
	char str[80];          //  character array to
	char arr[80];          //  character array to

	InitialiseSuperBlock();
	CreateDILB();


	while(1)
	{
		fflush(stdin);
		strcpy_s(str,80,"");
		printf("\n\n VIRTUAL FILE SYSTEM: > ");
		fgets(str,80,stdin);

		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

		if(count == 1)
		{
			if(_stricmp(command[0],"ls") == 0)
			{
				ls_file();
				continue;
			}
			else if(_stricmp(command[0] , "closeall") == 0)
			{
				CloseAllFile();
				printf("All files closed successfully\n");
				continue;
			}
			else if(_stricmp(command[0] , "clear") == 0)
			{
				system("cls");
				continue;
			}
			else if(_stricmp(command[0] , "help") == 0)
			{
				DisplayHelp();
				continue;
			}
			else if(_stricmp(command[0] , "exit") == 0)
			{
				char ch;
				printf("Do you want to exit: Y- Yes   Any button - No\n");
				scanf("%c",&ch);

				if(ch == 'Y' || ch =='y')
				{

					printf("Terminating VIRTUAL FILE SYSTEM");
					printf(".");
					delay(1000);
					printf(".");
					delay(1000);
					printf(".");
					delay(1000);

					break;
				}
				else
				{
					printf("\n");
					continue;
				}

			}
			else
			{
				printf("\nERROR : Command not found!!!\n");
				continue;
			}
		}

		else if(count ==2)
		{
			if(_stricmp(command[0],"stat") == 0)
			{
				ret = stat_file(command[1]);
				if(ret == -1)
					printf("ERROR : Incorrect parameters\n");
				if(ret == -2)
					printf("ERROR : There is no such file\n");

				continue;
			}
			else if(_stricmp(command[0],"fstat") == 0)
			{
					ret = fstat_file(atoi(command[1]));
					if(ret == -1)
						printf("ERROR : Incorrect parameters\n");
					if(ret == -2)
						printf("ERROR : There is no such file\n");

				continue;
			}
			else if(_stricmp(command[0],"close") == 0)
			{
				ret = CloseFileByName(command[1]);

				if(ret == -1)
					printf("ERROR: There is no such file\n");

				continue;
			}
			else if(_stricmp(command[0],"rm") == 0)
			{
				ret = rm_File(command[1]);

				if(ret == -1)
					printf("ERROR: There is no such file\n");

				continue;
			}
			else if(_stricmp(command[0],"man") == 0)
			{
				man(command[1]);
			}
			else if(_stricmp(command[0],"write") == 0)
			{
				fd = GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR: Incorrect Parameter\n");
					continue;
				}

				printf("Enter data: \n");
				scanf("%[^\n]",arr);

				ret = strlen(arr);
				if(ret == 0)
				{
					printf("Error : Incorrect parameter\n");
					continue;
				}

				ret = WriteFileX(fd,arr,ret);

				if(ret == -1)
					printf("ERROR: Permission Denied\n");
				else if(ret == -2)
					printf("ERROR: There is no sufficient memory to write\n");
				else if(ret == -3)
					printf("ERROR: Its not a regular file\n");
			}

			else if(_stricmp(command[0],"truncate") == 0)
			{
				ret = truncate_File(command[1]);
				if(ret == -1)
					printf("ERROR:Incorrect parameter\n");
		    }
			else
			{
				printf("ERROR: Command not found\n");
				continue;
			}
		}

		else if(count == 3)
		{
			if(_stricmp(command[0],"create") == 0)
			{
				ret = CreateFileX(command[1],atoi(command[2]));
				if(ret >= 0)
					printf("File is successfully created with file descriptor : %d\n",ret);
				if(ret == -1)
					printf("ERROR : Incorrect parameters\n");
				if(ret == -2)
					printf("ERROR : There are no inodes\n");
				if(ret == -3)
					printf("ERROR : File already exists\n");
				if(ret == -4)
					printf("ERROR : Memory allocation failure\n");

				continue;
			}
			else if(_stricmp(command[0],"open") == 0)
			{
					ret = OpenFileX(command[1],atoi(command[2]));
					if(ret > 0)
					printf("File is successfully opened with file descriptor : %d\n",ret);
				if(ret == -1)
					printf("ERROR : Incorrect parameters\n");
				if(ret == -2)
					printf("ERROR : File not present\n");
				if(ret == -3)
					printf("ERROR : Permission denied\n");

				continue;
			}
			else if(_stricmp(command[0],"read") == 0)
			{
				fd = GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR: Incorrect parameter\n");
					continue;
				}

				ptr = (char*)malloc(sizeof(atoi(command[2]) + 1));
				if(ptr == NULL)
				{
					printf("ERROR: Memory allocation failure\n");
					continue;
				}

				ret = ReadFileX(fd,ptr,atoi(command[2]));
				if(ret == -1)
					printf("ERROR : File not existing\n");
				if(ret == -2)
					printf("ERROR : Permission denied\n");
				if(ret == -3)
					printf("ERROR : Reached at the end of file\n");
				if(ret == -4)
					printf("ERROR : It is not regular file\n");
				if(ret == 0)
					printf("ERROR : File Empty\n");

				if(ret > 0)
				{
					_write(2,ptr,ret);
				continue;
			}
			}
			else
			{
				printf("\nERROR : Command not found !!!\n");
				continue;
			}
		}
		else if(count == 4)
		{
			if(_stricmp(command[0],"lseek") == 0)
			{
				fd = GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR: Incoorect parameter\n");
					continue;
				}

				ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
				if(ret == -1)
				{
					printf("ERROR: Unable to perform lseek\n");
					continue;
				}
			}
			else
			{
				printf("\nERROR: Command not found\n");
				continue;
			}
		}
		else
		{
			printf("\nERROR: Command not found\n");
			continue;
		}
	}

	return 0;
}