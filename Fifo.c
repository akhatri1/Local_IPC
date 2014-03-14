#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

#define FIFO1 "./fifo.1"
#define FIFO2 "./fifo.2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define MAXBUFF 4096

/*Message Structure*/
struct Message
{
	int msg_len;
	int msg_type;
	char msg_buff[MAXBUFF];
}msg;


void executeCommand(char*);

int main(int argc, char **argv)
{
	/*Initializing the Variables*/
	int n=0, len, readfd_client,writefd_client,readfd_server,writefd_server, flag = 0;
	pid_t childPid;
	char buff[512],command[512],command1[512],*Com = NULL,*checkOpcode = NULL, *reply;
	setbuf(stdout,NULL);
	if((mkfifo(FIFO1,0666) < 0) && (errno != EEXIST))
	{
		printf("Problem creating FIFO 1\n");
	}
	if((mkfifo(FIFO2,0666) < 0) && (errno != EEXIST))
	{
		printf("Problem creating FIFO 2\n");
		unlink(FIFO1);
	}
	
	
						childPid = fork();
						if(childPid == 0)
						{
							/*Server Process*/
	
							/*opening FIFO1 and FIFO2*/							
							if((writefd_server = open(FIFO2,O_WRONLY,0))<0)
							{
								printf("Error opening FIFO in write mode\n");
							}
							if((readfd_server = open(FIFO1,O_RDONLY,0)) <0)
							{
								printf("Error opening FIFO in read mode\n");
							}	
							
							//printf("%d %d\n",readfd_server,writefd_server);

							while(1){
							
							/*Reading the message from the Client through FIFO1*/
							read(readfd_server,&msg,4096);
							
							executeCommand(msg.msg_buff);
							
							msg.msg_len = strlen(msg.msg_buff);
							msg.msg_type = 1;
							/*Writing the message to the Client through FIFO2*/
							write(writefd_server,&msg,4096);
							
							}
						
						
							
						}
						else
						{
								/*Opening FIFO1 and FIFO2*/
								if((readfd_client = open(FIFO2,O_RDONLY,0))<0)
								{
									printf("Error opening the FIFO in read mode\n");
								}
								if((writefd_client = open(FIFO1,O_WRONLY,0)) < 0)
								{
									printf("Error opening the FIFO in the write mode\n");
								}
								//printf("%d %d\n",readfd_client,writefd_client);
								while(1)
								{
							
		 							printf("Enter the command:\n");
									/*Take the input from the console*/
		
									gets(command);
									strcpy(command1,command);
			
									checkOpcode = strtok(command1," ");
									if(checkOpcode == NULL)
									{
										printf("Please enter a proper command\n");
									}
									else
									{
										/*Validating Opcode*/
										if(strcmp(checkOpcode,"Read") == 0 || strcmp(checkOpcode,"Delete") == 0 || strcmp(checkOpcode,"Exit") == 0)
										{
											if(strcmp(checkOpcode,"Exit") == 0)	
											{	
											///*Garbage Collection*/
											kill(childPid,SIGKILL);
											unlink(FIFO1);
											unlink(FIFO2);
											exit(0);
											}
											else
											{
												strcpy(msg.msg_buff,command);
												msg.msg_len = strlen(command);
												msg.msg_type = 1;
												/*Writing the message to the Server through FIFO1*/
												write(writefd_client,&msg,4096);
												/*Reading the message from the Client through FIFO2*/
												while((n=read(readfd_client,&msg,4096))<0);
												/*Displaying the Output on the console*/
												printf("%s\n",msg.msg_buff);
											}
										}
										else
										{
											printf("Please specify the Opcode Correctly\n");
											printf("\tRead\n");
											printf("\tDelete\n");
											printf("\tExit\n");
										}
									}
								}
	
							}
}

/*Function for Reading the file, Deleting the file*/
void executeCommand(char *com)
{
	char response[512], *filename, *opcode, ch,com1[512];
	int i=0, Status;
	FILE *fp1 = NULL;
	strcpy(com1,com);
	opcode = strtok(com1," ");
	filename = strtok(NULL," ");
	
	if((fp1 = fopen(filename,"r")) == NULL)
	{
		strcpy(com,"File not found :: Unsuccessful Operation");
		return;
	}
	fclose(fp1);
	
	fp1 = NULL;
	
	/*Read operation*/
	if(strcmp(opcode,"Read") == 0)
	{
		
		fp1 = fopen(filename,"r");
		while(1)
    	        {
      			ch = fgetc(fp1);

      			if(ch==EOF)
         		break;
      			else
          		{ response[i] = ch;
				i++;
			}
    		}
		response[i] = '\0';
		strcpy(com,response);
		fclose(fp1);
	}

	/*Delete operation*/	
	if(strcmp(opcode,"Delete") == 0)
	{
			
		
		/*Removing the requested file*/
		Status = remove(filename);
		if(Status == 0)
		{
			strcpy(com,"File is removed");
		}
		/*What if the file is present but the directory doesn't have write permission*/
		else
		{
			strcpy(com,"Error File cannot be deleted: Seems like the parent directory doesn't have write permissions");
		}
		
	}
	return; 
	/*Finishing the process*/
	
}
