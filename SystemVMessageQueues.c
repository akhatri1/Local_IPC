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
#include<sys/ipc.h>
#include<sys/msg.h>


#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define MAXBUFF 512
#define MSGPERM 0600

struct Message
{
	int msg_type;
	char msg_buff[MAXBUFF];
}msg;


void executeCommand(char*);

int main(int argc, char **argv)
{
	int n=0, msgqid1,rc=0, msgqid2=0;
	pid_t childPid;
	char buff[512],command[512],command1[512],*Com = NULL,*checkOpcode = NULL, *reply;
	setbuf(stdout,NULL);

	/*Opening the Message Queue msgqid1 ans msgqid2*/
	msgqid1 = msgget(IPC_PRIVATE, MSGPERM|IPC_CREAT|IPC_EXCL);
	msgqid2 = msgget(IPC_PRIVATE, MSGPERM|IPC_CREAT|IPC_EXCL);
	if(msgqid1 < 0) {
   	 perror(strerror(errno));
    	 printf("failed to create message queue with msgqid = %d\n", msgqid1);
         return 1;
  	}
  		
	
	if(msgqid2 < 0) {
   	 perror(strerror(errno));
    	 printf("failed to create message queue with msgqid = %d\n", msgqid2);
         return 1;
  	}



						childPid = fork();
						if(childPid == 0)
						{
							
							
							/* Server process */
							while(1){
							/*Receiving message from the client through the Message Queue msgqid1*/
							rc = msgrcv(msgqid1, &msg, sizeof(msg), 0, 0); 
 							 if (rc < 0) {
							    perror( strerror(errno) );
							    printf("msgrcv failed, rc=%d\n", rc);
							    return 1;
							  } 
							
							 executeCommand(msg.msg_buff);

							/*Sending the result to the client through Message Queue msgqid2*/
							rc = msgsnd(msgqid2, &msg, sizeof(msg), 0); 
  							if (rc < 0) {
							perror( strerror(errno) );
							printf("msgsnd failed, rc = %d\n", rc);
							return 1;
							}
							
							}
						
						
							
						}
						else
						{
								/*Client Process*/
								
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
											msgctl(msgqid1, IPC_RMID, NULL);
											msgctl(msgqid2, IPC_RMID, NULL);
												exit(0);
											}
											else
											{
												strcpy(msg.msg_buff,command);
												msg.msg_type = 1;
												/*Sending the Message from Server to Client */							
												rc = msgsnd(msgqid1, &msg, sizeof(msg), 0); 
  												if (rc < 0) {
												    perror( strerror(errno) );
												    printf("msgsnd failed, rc = %d\n", rc);
												    return 1;
												  }
												/*Recieving the Message from Server to Client*/
												rc = msgrcv(msgqid2, &msg, sizeof(msg), 0, 0); 
 							 					if (rc < 0) {
							    					perror( strerror(errno) );
							    					printf("msgrcv failed, rc=%d\n", rc);
							    					return 1;
							  					} 
												/*Printing the output on the console*/
												printf(" %s\n",msg.msg_buff);
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
