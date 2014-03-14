#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include<signal.h>
#include<sys/wait.h>
#include <fcntl.h>

#define MAX_SIZE 512
#define QUEUE_NAME  "/test_queue23"
#define QUEUE_NAME1  "/test_queue123"

void executeCommand(char*);

int main(int argc, char **argv)
{
	/*Initialising the Variables*/
	int n=0, flag = 0;
	pid_t childPid;
	char buffer[512],command[512],command1[512],*Com = NULL,*checkOpcode = NULL, *reply;
	ssize_t bytes_read;
	mqd_t mq, mq1;
	struct mq_attr attr;
	setbuf(stdout,NULL);	
						/*Forking: Client and Server process*/
						childPid = fork();
						if(childPid == 0)
						{
							
   							    /* Server process */
							    

   							 /* initialize the queue attributes */
   							attr.mq_flags = 0;
   							attr.mq_maxmsg = 10;
    							attr.mq_msgsize = MAX_SIZE;
							attr.mq_curmsgs = 0;	
							mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
							while((mq1 = mq_open(QUEUE_NAME1, O_WRONLY)) <0);
							//printf("%d %d\n",mq1,mq);
							while(1){
							/*Recieving from the Client through Message Queue mq*/
							if((bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL)) < 0){ continue;}
							buffer[bytes_read] = '\0';
							executeCommand(buffer);
							/*Sending the buffer to the client through Message Queue mq1*/
							mq_send(mq1, buffer, MAX_SIZE, 0);
							}
						}
						else
						{
								/*Client Process*/

							/* initialize the queue attributes */
							attr.mq_flags = 0;
   							attr.mq_maxmsg = 10;
    							attr.mq_msgsize = MAX_SIZE;
							attr.mq_curmsgs = 0;	
							mq1 = mq_open(QUEUE_NAME1, O_CREAT | O_RDONLY, 0644, &attr);
							while((mq = mq_open(QUEUE_NAME, O_WRONLY)) <0);
							//printf("%d %d\n",mq1,mq);
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
											mq_close(mq);
											mq_close(mq1);
											mq_unlink(QUEUE_NAME1);
											mq_unlink(QUEUE_NAME);
												exit(0);
											}
											else
											{
												strcpy(buffer,command);
												/*Sending the buffer to the Server through Message Queue mq*/
												mq_send(mq, buffer, MAX_SIZE, 0);
												/*Receiving from the Client through the Message Queue mq1*/
												while((bytes_read = mq_receive(mq1, buffer, MAX_SIZE, NULL)) <0);
												buffer[bytes_read] = '\0';
												/*Printing the output on the Console*/
												printf("%s\n",buffer);
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
			strcpy(com,"File not found:: Unsuccessful Operation");
			
			return;
	}
	fclose(fp1);
	fp1 = NULL;
	
	/*Read operation*/
	if(strcmp(opcode,"Read") == 0)
	{

		/*Coping the requested file in MessageTransfer.txt*/
		
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
