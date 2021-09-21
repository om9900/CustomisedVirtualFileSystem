//Header files 
#include<stdio.h>
#include<stdlib.h>	//lib and wrapper fns
#include<unistd.h>
#include<string.h>	//memset memcpy hai sagle hycha madhe ahe
//#include<iostream>

//********************************************************************************
//********************************************************************************
//Macros
//********************************************************************************
//********************************************************************************
//#define _CRT_SECURE_NO_WARNINGS		Used in windows sdk
#define MAXINODE 50				//Here we declared max node as 50 if in future you want 
								//to increase nodes you just have to change here
#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024		//In future you want to increase the file size increase here

//File Types
#define REGULAR 1
#define SPECIAL 2

//ELSEEK madhe lagtat
#define START 0
#define CURRENT 1
#define END 2

//********************************************************************************
//********************************************************************************
//Structure declaration for superblock 
//which contains TotalNodes and freeInodes//baki lihale nahi karan te adhi cha memory madhe ahe
//********************************************************************************
//********************************************************************************
typedef struct superblock
{
	int TotalInodes;
	int FreeINodes;
}SUPERBLOCK,*PSUPERBLOCK;

//********************************************************************************
//********************************************************************************
//structure declaration for inode
//********************************************************************************
//********************************************************************************
typedef struct inode
{
	//Name of File
	char FileName[50];

	//inode number of file  
	int iNodeNumber;

	//it is implicetly given by  1024 bytes
	int FileSize;

	//it is Actual file size 
	int FileActualSize;

	//type of file regular or special
	int FileType;

	//use to fetch data from file
	//It is like data block
	char *Buffer;				//char *Buffer[3];  buffer is one dimentional array which 
								//contains 3 elements each element is of type char*

	//
	int LinkCount;

	//at current situation how many number of times the file is open
	int ReferenceCount;
	
	//permission for read write for owner group and other
	int permission;

	//pointer pointing to next inode;
	struct inode *next;
}INODE,*PINODE,**PPINODE;

//********************************************************************************
//********************************************************************************
//Structure declaration for file table
//********************************************************************************
//********************************************************************************
typedef struct filetable
{
	//last time where you left while file opened in read mode
	int readoffset;

	//last time where you left while file opened in write mode
	int writeoffset;

	// always 1 if we implement fork syatem call then it changes
	int count;

	//mode in while file is opened write read 
	int mode;

	PINODE ptrinode;		//Ponter Pointing to Inode Table
}FILETABLE,*PFILETABLE;

//********************************************************************************
//********************************************************************************
//Structure declaration for uarea
//it can be array but for future use we declared as structure
//********************************************************************************
//********************************************************************************

typedef struct ufdt
{
	PFILETABLE ptrfiletable;	//pointer to filetable
	//We can add new member in future
}UFDT;
UFDT UFDTArr[MAXINODE];
/////////////////////////////
//
SUPERBLOCK SUPERBLOCKobj;
PINODE head=NULL;				//Inode chy LL cha head Pointer ahe
////////////////////////

//********************************************************************************
//********************************************************************************
//man function

/*man command in Linux is used to display the user manual of any command that
  we can run on the terminal. It provides a detailed view of the command which
  includes NAME, SYNOPSIS, DESCRIPTION, OPTIONS, EXIT STATUS, RETURN VALUES,
  ERRORS, FILES, VERSIONS, EXAMPLES, AUTHORS and SEE ALSO	
*/
//********************************************************************************
//********************************************************************************

void man(char *name)
{
	if(name==NULL)
	{
		return ;
	}
	//Create command 
	if(strcmp(name,"create")==0)
	{
		printf("Description: Used to create new regular file\n");
		printf("Usage: create File_name Permission\n");
	}
	//read command
	else if(strcmp(name,"read")==0)
	{
		printf("Description: Used to read data from regular file\n");
		printf("Usgae: read file_name No_Of_Bytes_To_Read\n");
	}
	//write command
	else if(strcmp(name,"write")==0)
	{
		printf("Description: Used to write into Regular File\n");
		printf("Usgae: write File_name\n After this enter the data that you want to write\n");

	}
	//ls
	else if(strcmp(name,"ls")==0)
	{
		printf("Description: Used to List all information of Files\n");
		printf("Usgae: ls\n");
	}
	//stat
	else if(strcmp(name,"stat")==0)
	{
		printf("Description: Used to Diaplay information of file\n");
		printf("Usage: stat File_name\n");
	}
	//fstat
	else if(strcmp(name,"fstat")==0)
	{
		printf("Description: Used to Display information of file \n");
		printf("Usage: stat File_Descriptor\n");
	}
	//truncate
	else if(strcmp(name,"truncate")==0)
	{
		printf("Description : Used to remove data from file\n");
		printf("Usage : truncate File_name\n");
	}
	else if(strcmp(name,"open")==0)
	{
		printf("Description : Used to open existing file\n");
		printf("Usage : open File_name mode\n");
	}
	else if(strcmp(name,"close")==0)
	{
		printf("Description : Used to close opened file\n");
		printf("Usage : close File_name\n");
	}
	else if(strcmp(name,"closeall") == 0)
	{
		printf("Description : Used to close all opened file\n");
		printf("Usage : closeall\n");
	}
	else if(strcmp(name,"lseek") == 0)
	{
		printf("Description : Used to change file offset\n");
		printf("Usage : lseek File_Name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(name,"rm") == 0)
	{
		printf("Description : Used to delete the file\n");
		printf("Usage : rm File_Name\n");
	}
	else
	{
		printf("ERROR:No Manual entry available\n");
	}
}

//********************************************************************************
//********************************************************************************
//Diaplay all commands and its usage using help Function
//********************************************************************************
//********************************************************************************
void DisplayHelp()
{
	printf("ls : To List out all Files\n");
	printf("clear : To clear console\n");
	printf("open : To open the file\n");
	printf("close : To close the file\n");
	printf("closeall : To close all opened file\n");
	printf("read : To Read the contents  of file\n");
	printf("write : To write contents into file\n");
	printf("exit : To Terminate the file syatem\n");
	printf("stat : To Display information of file using name\n");
	printf("fstat : To Display information of file using file descriptor\n");
	printf("truncate : To remove all data from file\n");
	printf("rm : To delete the file \n");
}

//********************************************************************************
//********************************************************************************
//Function for fetting file descriptor
//********************************************************************************
//********************************************************************************

int GetFDFromName(char *name)
{
	int i=0;
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable!=NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
			{
				break;
			}
		}
		i++;
	}
	if(i==50)
	{
		return -1;
	}
	else
	{
		return i;
	}
}

//********************************************************************************
//********************************************************************************
//Function Foe getting Inode of file using file name
//********************************************************************************
//********************************************************************************

PINODE Get_Inode(char *name)
{
	PINODE temp=head;
	int i=0;

	if(name==NULL)
	{
		return NULL;
	}
	while(temp!=NULL)
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
		}
		temp=temp->next;
	}
	return temp;	//LL chy shevat parent gela tar null yail ani ty nava che file nasnar
					//ani jar asel tar te null send karnar nahinb mahnje file adhi cha ahe
}

//********************************************************************************
//********************************************************************************
//Creating our own dilb block i.e LL of Inode
//********************************************************************************
//********************************************************************************

void CreateDILB()
{
	int i=1;			//inode no
	PINODE newn=NULL;	//
	PINODE temp=head;	

	while(i<=MAXINODE)		//(50 vela firto)
	{
		newn=(PINODE)malloc(sizeof(INODE));		//navin inode 

		newn->LinkCount=newn->ReferenceCount=0;		//Initialize kela
		newn->FileType=newn->FileSize=0;			//Initialize kela

		newn->Buffer=NULL;				//buffer pointer
		newn->next=NULL;				//Initialize kela
		newn->iNodeNumber=i;			////Initialize kela inode number la 1 la 

		if(temp==NULL)			//if first node
		{
			head=newn;
			temp=head;
		}
		else                       //It is like inser last logic
		{
			temp->next=newn;
			temp=temp->next;
		}
		i++;
	}
	printf("DILB Created Successfully\n");
}

//********************************************************************************
//********************************************************************************
//Initialising our Super Block
//********************************************************************************
//********************************************************************************

void InitialiseSuperBlock()
{
	int i=0;
	while(i<50)		//(i<MAXINODE)
	{
		UFDTArr[i].ptrfiletable=NULL;	//ufdt cha pratik pointer madhe null taknare
		i++;
	}
	//Auxillary data
	SUPERBLOCKobj.TotalInodes=MAXINODE;
	SUPERBLOCKobj.FreeINodes=MAXINODE;
}

//********************************************************************************
//********************************************************************************
//Function for Creating File
//********************************************************************************
//********************************************************************************

int CreateFile(char *name,int permission)
{
	int i=0;
	PINODE temp=head;

	if((name==NULL)||(permission==0)||(permission>3))
	{
		return -1;
	}
	if(SUPERBLOCKobj.FreeINodes==0)		//50 file create karun zalya
	{
		return -2;
	}
	

	if(Get_Inode(name)!=NULL)	//file cha nav pathavla repeat file create karat asel tar
	{
		return -3;
	}
	//
	(SUPERBLOCKobj.FreeINodes)--;	//Inode cha count eka na kami kela inode 1 book kela adhich
	//

	while(temp!=NULL)
	{
		if(temp->FileType == 0)		//rikama inode shodhtoy
		{
			break;
		}
		temp=temp->next;		//file type zero aslela cha addres yail (in diagram 600)
	}

	//i=3;			//jar first 3 reserve thevaiche astil tar
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDTArr[i].ptrfiletable == NULL)
	{
		return -4;
	}

	UFDTArr[i].ptrfiletable->count=1;				
	UFDTArr[i].ptrfiletable->mode=permission;
	UFDTArr[i].ptrfiletable->readoffset=0;
	UFDTArr[i].ptrfiletable->writeoffset=0;

	UFDTArr[i].ptrfiletable->ptrinode=temp;					//ufdt madher point karnarya la ufdt la connect kartoy
	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);//data madhe demo.txt		String madhe convert kela
										//inode cha structure madhe  filename assign kela
	UFDTArr[i].ptrfiletable->ptrinode->FileType=REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize=MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize=0;
	UFDTArr[i].ptrfiletable->ptrinode->permission=permission;

	UFDTArr[i].ptrfiletable->ptrinode->Buffer=(char *)malloc(MAXFILESIZE);	//1024 byte cha buffer allocate kela

	//
	memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer,0,MAXFILESIZE);	//safety purpose dynammicalya allocate memory la zero kelya
	//(address,kasha na set karicha ,kiti set karicha)
	return i;			//ufdt madhli index or fd
}
//********************************************************************************
//********************************************************************************
//Function for removing file 
//********************************************************************************
//********************************************************************************

int rm_File(char *name)
{
	int fd=0;

	fd=GetFDFromName(name);
	if(fd==-1)
	{
		return -1;
	}
	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if((UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)==0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType=0;
		free(UFDTArr[fd].ptrfiletable);
	}
	UFDTArr[fd].ptrfiletable=NULL;
	(SUPERBLOCKobj.FreeINodes)++;
}

//********************************************************************************
//********************************************************************************
//Function for Reading the file 
//********************************************************************************
//********************************************************************************

int ReadFile(int fd,char *arr,int iSize)
{
	int read_size=0;

	if(UFDTArr[fd].ptrfiletable==NULL)
	{
		return -1;
	}

	if(UFDTArr[fd].ptrfiletable->mode!=READ && UFDTArr[fd].ptrfiletable->mode!=READ+WRITE)
	{
		return -2;
	}
	if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ+WRITE)
	{
		return -2;
	}
	if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -3;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType!=REGULAR)
	{
		return -4;
	}
	//jar adhi file open keli asel read karila ani oarat tithun pahije Aasel tar 
	//tevde bytes minus karav lagtil
	read_size=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)-
		(UFDTArr[fd].ptrfiletable->readoffset);
//***************************
	if(read_size<iSize)
	{
	strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + 
		(UFDTArr[fd].ptrfiletable->readoffset),read_size);

	UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->
		readoffset + read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) +
			(UFDTArr[fd].ptrfiletable->readoffset),iSize);
		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->
			readoffset) + iSize;
	}
	return iSize;
//*********************
}

//********************************************************************************
//********************************************************************************
//Function for writing in file
//********************************************************************************
//********************************************************************************
				//(    ,data ,ani kiti size)
int WriteFile(int fd,char *arr,int iSize)
{
	if(((UFDTArr[fd].ptrfiletable->mode)!=WRITE)&& (UFDTArr[fd].ptrfiletable->mode)!=READ+WRITE)
	{
		return -1;
	}
	if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) !=WRITE) &&
		((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ+WRITE))
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable->writeoffset==MAXFILESIZE)
	{
		return -2;
	}
	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType!=REGULAR)
	{
		return -3;
	}
//*********************
	strncpy(((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+
		UFDTArr[fd].ptrfiletable->writeoffset),arr,iSize);	//Over write karat nahi hy line mula

	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset )+iSize;//jevda add kela tecda add kela

	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable
		->ptrinode->FileActualSize)+iSize;			//actual size update zali

//********************
	return iSize;

}

//********************************************************************************
//********************************************************************************
//Open file by name
//********************************************************************************
//********************************************************************************

int OpenFile(char *name,int mode)
{
	int i=0;
	PINODE temp=NULL;

	if(name==NULL||mode<=0)
	{
		return -1;
	}
	temp=Get_Inode(name);

	if(temp==NULL)
	{
		return -2;
	}

	if(temp->permission<mode)
	{
		return -3;
	}

	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable=NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDTArr[i].ptrfiletable==NULL)
	{
		return -1;
	}

	UFDTArr[i].ptrfiletable->count=1;
	UFDTArr[i].ptrfiletable->mode=mode;

	if(mode==READ+WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset=0;
		UFDTArr[i].ptrfiletable->writeoffset=0;
	}
	else if(mode==READ)
	{
		UFDTArr[i].ptrfiletable->readoffset=0;
	}
	else if(mode==WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset=0;
	}

	UFDTArr[i].ptrfiletable->ptrinode=temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

	return i;
}

//********************************************************************************
//********************************************************************************
//Function for closing file by file descriptor
//********************************************************************************
//********************************************************************************

//void CloseFileByName(int fd)
//{
//	UFDTArr[fd].ptrfiletable->readoffset = 0;
//	UFDTArr[fd].ptrfiletable->writeoffset = 0;
//	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
//}

//********************************************************************************
//********************************************************************************
//Function for closing file by name
//********************************************************************************
//********************************************************************************

int CloseFileByName(char *name)
{
	int i=0;
	i=GetFDFromName(name);
	if(i==-1)
	{
		return -1;
	}

	UFDTArr[i].ptrfiletable->readoffset=0;
	UFDTArr[i].ptrfiletable->writeoffset=0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
	//strcpy(FileName,"");

	return 0;
}

//********************************************************************************
//********************************************************************************
//Function for closing all opened files
//********************************************************************************
//********************************************************************************

void CloseAllFile()
{
	int i=0;
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable!=NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset=0;
			UFDTArr[i].ptrfiletable->writeoffset=0;
			(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
			break;
		}
		i++;
	}
}

//********************************************************************************
//********************************************************************************
//Function for list of all files
//********************************************************************************
//********************************************************************************

void ls_file()
{
	int i=0;
	PINODE temp=head;

	if(SUPERBLOCKobj.FreeINodes==MAXINODE)
	{
		printf("There are no files \n");
		return;
	}

	printf("\nFile Name\tInode number\tFile size\tLink count\n");
	printf("---------------------------------------------------------------\n");

	while(temp!=NULL)
	{
		if(temp->FileType!=0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->
				iNodeNumber,temp->FileActualSize,temp->LinkCount);
		}
		temp=temp->next;
	}
	printf("---------------------------------------------------------------\n");
}

//********************************************************************************
//********************************************************************************
//Function for Displaying stat of file using file descriptor as parameter it as fast as compare to stat function
//********************************************************************************
//********************************************************************************

int fstat_file(int fd)
{
	PINODE temp=head;

	int i=0;

	if(fd<0)
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable==NULL)
	{
		return -2;
	}

	temp=UFDTArr[fd].ptrfiletable->ptrinode;

	printf("\n---------Statistical Information about file----------\n");
	printf("File name : %s\n",temp->FileName);
	printf("Inode Number %d\n",temp->iNodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);

	if(temp->permission==1)
	{
		printf("File Permission : Read only\n");
	}
	else if(temp->permission==2)
	{
		printf("File Permission : write\n");
	}

	else if(temp->permission==3)
	{
		printf("File Permission : Read & write\n");
	}

	printf("---------------------------------------------------------------\n");
	return 0;

}

//********************************************************************************
//********************************************************************************
//Function for Displaying stat of file using file anme as parameter
//********************************************************************************
//********************************************************************************

int stat_file(char *name)
{
	PINODE temp=head;		//global ghetya tymula temp madhe kadhla
	int i=0;

	if(name==NULL)
	{
		return -1;
	}

	while(temp!=NULL)				
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
		}
		temp=temp->next;
	}
	if(temp==NULL)
	{
		return -2;
	}
	printf("\n---------Statistical Information about file----------\n");
	printf("File name : %s\n",temp->FileName);
	printf("Inode Number %d\n",temp->iNodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);

	if(temp->permission==1)
	{
		printf("File Permission : Read only\n");
	}
	else if(temp->permission==2)
	{
		printf("File Permission : write\n");
	}

	else if(temp->permission==3)
	{
		printf("File Permission : Read & write\n");
	}

	printf("---------------------------------------------------------------\n\n");
	return 0;
}

//********************************************************************************
//********************************************************************************
//Delete the contents of file 
//********************************************************************************
//********************************************************************************

int truncate_File(char *name)
{
	int fd=GetFDFromName(name);

	if(fd==-1)
	{
		return -1;
	}

	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);

	UFDTArr[fd].ptrfiletable->readoffset=0;
	UFDTArr[fd].ptrfiletable->writeoffset=0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize=0;
}

//********************************************************************************
//********************************************************************************
//Entry Point Function
//********************************************************************************
//********************************************************************************
int main()
{
	char *ptr=NULL;	//pointer for taking input from user
	char str[80];
	char command[4][80],arr[1024];
	int count=0;
	int fd=0,ret=0;

	InitialiseSuperBlock();			//total inode and freea inode 
	CreateDILB();					//50 inode chr  LL tayerv karnare

	while(1)			//Creating Shell Like Environment
	{
		fflush(stdin);		//keyboard cha input buffer
		strcpy(str,"");		

		printf("\nProject VFS $:");		//
		//scanf("%s[^'\n]s",str);
		fgets(str,80,stdin);		//For Taking Command From user

		//count number of instrction given by user suppose 4 commands are given then
		//it will cut the command after the space into 4 parts 
		//Tokanisation
		count=sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

		if(count==1)		//if count is one then it will go there
		{
			if(strcmp(command[0],"ls")==0)
			{
				ls_file();
			}
			else if(strcmp(command[0],"closeall")==0)	//user wants to close all files that are open
			{
				CloseAllFile();
				printf("All Files Are Successfully closed\n");
				continue;
			}
			else if(strcmp(command[0],"clear")==0)
			{
				system("clear");
				continue;
			}
			else if(strcmp(command[0],"help")==0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(command[0],"exit")==0)
			{
				printf("Thank you for using application\n");
				break;
			}
			else
			{
				printf("\nERROR:Command not Found!!!\n");
				continue;
			}

		}
		else if(count==2)
		{
			if(strcmp(command[0],"stat")==0)
			{
				ret=stat_file(command[1]);
				if(ret==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"fstat")==0)
			{
				ret=fstat_file(atoi(command[1]));
				if(ret==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"close")==0)
			{
				ret=CloseFileByName(command[1]);
				if(ret==-1)
				{
					printf("There is no such file\n");
				}
				continue;
			}

			else if(strcmp(command[0],"rm")==0)
			{
				ret=rm_File(command[1]);
				if(ret==-1)
				{
					printf("ERROR : There is no such file\ns");
				}
				continue;	
			}
			else if(strcmp(command[0],"man")==0)
			{
				man(command[1]);
			}
			else if(strcmp(command[0],"write")==0)
			{
				fd=GetFDFromName(command[1]);

				if(fd==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
					continue;
				}
				printf("Enter the data:\n");
				scanf("%s[^\n]",arr);

				ret=strlen(arr);

				if(ret==0)
				{
					printf("ERROR : Incorrect Parameters\n");
					continue;
				}
				ret=WriteFile(fd,arr,ret);
				if(ret==-1)
				{
					printf("ERROR : Permission Denied\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no sufficient memory to write\n");
				}
				if(ret==-3)
				{
					printf("ERROR : It is not regular file\n");
				}
			}

			else if(strcmp(command[0],"truncate")==0)
			{
				ret=truncate_File(command[1]);
				if(ret==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
			}

			else
			{
				printf("\n ERROR : Command not found!!!\n");
				continue;
			}


		}
		else if(count==3)
		{
			if(strcmp(command[0],"create")==0)
			{
				//					name 	ani permission
				ret=CreateFile(command[1],atoi(command[2]));	//ascii to integer (atoi)


				if(ret>=0)
				{
					printf("File Successfully created with file descriptor : %d\n",ret);
				}
				if(ret==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : there are no inodes\n");
				}
				if(ret==-3)
				{
					printf("ERROR : File already exists\t");
				}
				if(ret==-4)
				{
					printf("ERROR : memory allocation failure\n");
				}
				continue;
			}

			else if(strcmp(command[0],"open")==0)
			{
				ret=OpenFile(command[1],atoi(command[2]));
				if(ret>=0)
				{
					printf("File is Successfully opened with file descriptor : %d\n",ret);
				}
				if(ret==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : File not present\n");
				}
				if(ret==-3)
				{
					printf("ERROR : Permission Denied\t");
				}
				continue;
			}
			else if(strcmp(command[0],"read")==0)
			{
				fd=GetFDFromName(command[1]);

				if(fd==-1)
				{
					printf("ERROR : Incorrect Parameters\n");
					continue;
				}
				ptr=(char *)malloc(sizeof(atoi(command[2]))+1);

				if(ptr==NULL)
				{
					printf("ERROR : memory allocation failure\n");
					continue;
				}

				ret=ReadFile(fd,ptr,atoi(command[2]));

				if(ret==-1)
				{
					printf("ERROR : File not exists\n");
				}
				if(ret==-3)
				{
					printf("ERROR : Reaches at the end of file\n");
				}
				if(ret==-2)
				{
					printf("ERROR : Permission Denied\n");
				}
				if(ret==-4)
				{
					printf("ERROR : It is not regular file\n");
				}
				if(ret>0)
				{
					write(2,ptr,ret);
				}
				continue;
			}
			else
			{
				printf("\n ERROR : Command not found!!!\n");
				continue;
			}
		}
		else if(count==4)
		{
			if(strcmp(command[0],"lseek")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd==-1)
				{
					printf("Incorrect Parameters\n");
				}
				//ret=LseekFile(fd,atoi(command[2]),atoi(command[3]));
				if(ret==-1)
				{
					printf("\n ERROR : Command not found!!!\n");
					continue;
				}
			}
			else
			{
				printf("\n ERROR : Command not found !!! \n");
				continue;
			}
		}
		else
		{
			printf("Command not found\n");
		}
	}
	return 0;
}
