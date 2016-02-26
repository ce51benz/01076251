/*TXRX CODE
MEMBERS
1.Kittiwat Lucksanasomboon 55010095 2D/1
2.Yanisa Yimsuwan          55010302 2D/1
3.Phanita Phinyophap       55010906 2D/2
4.Suratchanan Kraidech     55011362 2D/2
*/

//CONSTANT DECLARATION
#define TXDATA 0x3F8
#define LSR (TXDATA + 5)
#define LCR (TXDATA + 3)

//HEADER FILE DECLARATION
#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<dos.h>
#include<time.h>
#include<stdlib.h>
#include<ctype.h>

//Global Variable Declaration::
//Reduce memory allocation and program mistake.
short frameNumber = 0; //Stand for frameNumber of Transmitter.
short desiredFrameNumber = 0; //Stand for frameNumber of desire frameNumber to be received from receiver.
static int data[8] = {0,0,0,0,0,0,0,0};  //Array size 8 use as Transmit/receive buffer.
unsigned long timeout = 0; //Timeout for auto send mode.
short i; //Iterator object.
static char *name = "",*opname = ""; //Name of sender
static char strin[200];
int HParity = 0,VParity = 0,EXTParity = 0; //2D-Parity value buffer.

//Function Prototype Declaration::
//1.Sending function
void send_string(char *str,int mode);
void sendfile();
void send_frame(int* data,int *datasize,int *mode);
void send_fileframe(int* data,int datasize);
void send_filereqACK();
void send_ACK(int mode);
void send_EOF();
//2.Receiving function
int receive_filereq();
int receive_frame(char *opname);
int receive_fileframe(FILE *writefile);
int receive_ACK();
int wait_fileACK();
//3.Parity Function
void get_VParity(int* data,int *datasize);
void get_HParity(int* data,int *datasize);
void get_EXTParity(int *EXTParity,int HParity);
int check_VParity(int* data,int *datasize);
int check_HParity(int* data,int *datasize);
int check_EXTParity(int *EXTParity,int HParity);
//4.ETC.function
void setup_serial();
void send_name(char *name);
char* receive_oname();

//Main Program
int main(){
	char ch;int token;
	clrscr();
	//Initialized Serial
	setup_serial();

	//Set display color of cprintf
	textcolor(11);

	//Input Sender or receiver
	while((ch!= 'S' && ch!= 's') && (ch!= 'R' && ch!= 'r')){
		cprintf("Instant MSG Program (type CTRL + Q to EXIT chat)\r\n");
		cprintf("Send Or Receive:");
		ch = getche();
		printf("\n");
	}

	//Input name
	printf("Enter your name:");
	gets(name);

	//Send name to opponent
	send_name(name);
	//Receive name from opppnent
	opname = receive_oname();

	//Get timeout value
	do{
			cprintf("Timeout (Second):");
			gets(strin);
			if(strlen(strin)>10)continue;
			else{
				for(token = 0;token < strlen(strin);token++){
					if(!isdigit(strin[token]))goto checkpoint;
				}
				//use atol (string to long) to convert string input to long.
				timeout = atol(strin);
			}
checkpoint:
		}while(!timeout);
	//Sequence of program will work by check who is sender
	//and who is receiver.
	//if input as first receiver.
	if(ch == 'R' || ch == 'r'){

		do{
			//Waiting for opponent.
			while(!(token = receive_frame(opname)));
			if(token == 2)_exit(0);
			//Input string
			printf("%s:",name);
			gets(strin);

			//If string is CTRL + Q (ASCII value 17) and no other character follow
			//It will send this string and after receive ACK it will terminate.
			if(!strlen(strin)){
				strcpy(strin," ");
				send_string(strin,0);
				}
			else if(strin[0] == 17){
				send_string(strin,0);
				_exit(0);
			}
			//Else if whether Input string is SEND it will be work in sendfile(); function.
			else if(strcmp(strin,"SEND")==0){
				sendfile();
			}
			//If input is not same as above ::send as automode.
			else{
				send_string(strin,0);
			}
		}while(1);
	}
	//if input as first sender. work same as above but swap sequence.
	else{
		do{
			printf("%s:",name);
			gets(strin);
			if(!strlen(strin)){
				strcpy(strin," ");
				send_string(strin,0);
				}
			else if(strin[0] == 17){
				send_string(strin,0);
				_exit(0);
			}
			else if(strcmp(strin,"SEND")==0){
				sendfile();
			}
			else{
				send_string(strin,0);
			}
			while(!(token = receive_frame(opname)));
			if(token == 2)_exit(0);
		}while(1);
	}
	return 0;
}

//void sendfile() function
//Parameter:None
//Return Value:None
//Description:This function use for manual send file mode
//The step start by
//1.Prompt user for Input filename
//2.Send file request frame by send filename(without send full path)
//3.Wait for file request ACK.
//4.Read stream for input file and save in buffer
//5.If buffer is full then send 1 fileframe as manual.
//6.If stream reach EOF then send EOF frame to tell receiver as END OF FILE.
void sendfile(){
	FILE *fr=NULL;
	char *filename;
	int *b; //b represent byte that read from input file.
	int datasize = 0;
	//Waiting for input filename
	while(!fr){
		printf("Enter path and file name to sent:");
		gets(filename);
		fr = fopen(filename,"rb");
	}
	//SEND FILEREQ as mode 3 and wait for ACK.
	send_string(filename,3);
	//Read stream and send until EOF
	while(1){
		if(fread(b,1,1,fr)){

			if(datasize==8)
			{
				send_fileframe(data,datasize);
				datasize = 0;
			}
			data[datasize++] = *b;
		}
		else{
			if(datasize!=0)
			send_fileframe(data,datasize);
			send_EOF();
			break;
		}
	}
	//In send_fileframe function there is use colored display
	//To avoid mistake display in normal printf
	//set color display to white
	textcolor(15);cprintf("\r\n");
	fclose(fr);
}


//Setup serial port :COM1
void setup_serial(){
	outportb(LCR,0x80);
	outportb(TXDATA,0x0C);
	outportb(TXDATA+1,0x00);
	outportb(LCR,0x0B);
}


//void send_string(char* str,int mode) function
//Parameter:str is Array of character,mode is integer
//Return value:None
//Description:This function will send string 8 character per time
//Specially of this function is mode
//If mode is 0 then send as auto mode
//Else if mode is 3 then send as file request mode
//in which sender will wait for file request ACK
void send_string(char* str,int mode){
	int datasize = 0;
	short shiftpos = 0;//shiftpos use for mode 3 send filename without full path.
	if(mode == 3){
		for(i = 0;i<strlen(str);i++){
			if(str[i] == '\\')shiftpos = i + 1;
		}
		*(str+=shiftpos);
	}
	while(1){
		if(datasize == 8 || *(str+1) == '\0'){//send 8 byte data and reset datasize
			if(datasize < 8){
				data[datasize++] = *(str++);
				break;
			}
			else if(datasize == 8 && *(str+1) == '\0'){
				send_frame(data,&datasize,&mode);
				datasize = 0;
				data[datasize++] = *(str++);
				break;
			}
			else{
				send_frame(data,&datasize,&mode);
				datasize = 0;
			}

		}
		data[datasize++] = *(str++);
	}
	//Send the rest and end character.
	if(datasize == 8){
		send_frame(data,&datasize,&mode);
		datasize = 0;
	}
	data[datasize++] = *(str);
	send_frame(data,&datasize,&mode);
	if(mode == 3){while(!wait_fileACK());}
}


//void send_frame(int* data,int *datasize,int *mode)
//Parameter:data as array of integer
//datasize and mode as pointer of integer
//Return value:None
//Description:This function will send regular frame
//It work in auto mode(mode 0) and fileREQ mode(mode 3)
//Sending frame in this function will differ and control field.
void send_frame(int* data,int *datasize,int *mode){
	unsigned long start; //start use to capture running time.
	int datasizeval = (*datasize)-1;
resend:
	//SEND START FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
	//SEND CONTROL MODE WITH FRAMNUMBER FIELD and Datasize
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,((*mode)<<4)|(frameNumber<<3)|datasizeval);

	//SEND DATA in buffer
	for(i = 0;i < *datasize;i++){
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,data[i]);
	}
	//Calculate 2D-Parity or 1D-parity and send
	if(*datasize>1){
		get_HParity(data,datasize);
		get_VParity(data,datasize);
		get_EXTParity(&EXTParity,HParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,HParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,VParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,EXTParity);
	}
	else{
		get_EXTParity(&VParity,data[0]);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,VParity);
	}

	//SEND STOP FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
	//Update FrameNumber
	if(++frameNumber>1)frameNumber = 0;
	//CHECK TIMEOUT
	//CLOCK() RETURN VALUE IN MILLISECONDS

	start = clock();
	while(!receive_ACK()){
		if(((clock()-start)/1000) > timeout){goto resend;}
	}
}

//void send_fileframe(int* data,int datasize)
//Parameter:data as array of integer,datasize is integer(pass by value)
//Return value:None
//Description:This function will send file frame
//In each use it work as manual.
void send_fileframe(int* data,int datasize){
	int key,modeACK;//Key stand for value which key is pressed.
	int datasizeval = (datasize-1);
resendpoint://Checkpoint for resend 
	//SEND START FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
	//SEND CONTROL MODE5 WITH FRAMNUMBER FIELD and Datasize
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x50|(frameNumber<<3)|datasizeval);
	
	//SEND DATA
	for(i = 0;i < datasize;i++){
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,data[i]);
	}

	//Calculate 2D-Parity or 1D-Parity
	if(datasize>1){
		get_HParity(data,&datasize);
		get_VParity(data,&datasize);
		get_EXTParity(&EXTParity,HParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,HParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,VParity);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,EXTParity);
	}
	else{
		get_EXTParity(&VParity,data[0]);
		while((inportb(LSR) & 0x40) != 0x40);
		outport(TXDATA,VParity);
	}

	//SEND STOP FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);

	//Set display of colored text as LIGHTCYAN
	textcolor(11);


	//MANUAL MODE....
	//Display current frame that send
	cprintf("Current frame is....\r\n");
	cprintf("%s SEND:%x %x %x %x ",name,0x7E,0x5,frameNumber,datasizeval);
	for(i = 0;i < datasize;i++)
		cprintf("%x",data[i]);
	cprintf(" ");
	if(datasize>1){
		cprintf("%x %x %x %x\r\n",HParity,VParity,EXTParity,0x7E);
	}
	else
		cprintf("%x %x\r\n",VParity,0x7E);
	//========================================
	cprintf("HEADER:%x %x %x %x\r\n",0x7E,0x5,frameNumber,datasizeval);
	cprintf("DATA:");
	for(i = 0;i < datasize;i++)
		cprintf("%x ",data[i]);

	cprintf("\r\nTRAILER:");

	if(datasize>1){
		cprintf("%x %x %x %x\r\n\r\n",HParity,VParity,EXTParity,0x7E);
	}
	else
		cprintf("%x %x\r\n\r\n",VParity,0x7E);
	//========================================
	//Prompt user to press <- or -> button 
	//whether transmit next or timeout.
	while(1){
		cprintf("Press ==> Key to do next step.\r\n");
		cprintf("Press <== Key to imply it timeout.\r\n");
		cprintf("What do you want to do? [<== or ==>]:");
		if(getch()!=0){printf("\n\n");continue;}
	if((key = getch())==0x4b){cprintf("You press <==\r\n");goto resendpoint;}
	else if(key == 0x4d){cprintf("You press ==>\r\n");break;}
		cprintf("\r\n");
	}
	if(++frameNumber>1)frameNumber = 0;
	//Check for ACK/NAK return value.
	//If modeACK == 1 or 6 then sender can send next frame.
	//Else modeACK == 2 it mean that sender will resend latest frame.
	while(1){
		modeACK = receive_ACK();
		if(modeACK == 1||modeACK == 6)break;
		else if(modeACK == 2){
			goto resendpoint;
		}
	}
	printf("\n\n");
	
}		

//void get_HParity(int* data,int *datasize)
//void get_VParity(int* data,int *datasize)
//void get_EXTParity(int *EXTParity,int &HParity)
//Parameter:data as array of integer
//EXTParity,HParity,datasize as pointer of integer
//Return value:None
//Description:This function will calculate 2D-parity which include
//HParity :Horizontal parity
//VParity :Vertical parity
//EXTParity:Parity of Horizontal parity.
void get_HParity(int* data,int *datasize){
	HParity = data[0] ^ data[1];
	for(i = 2;i<*datasize;i++)
		HParity ^= data[i];
}

void get_VParity(int* data,int *datasize){
	int tempParity;
	int token;
	VParity = 0;
	for(i = 0;i < *datasize ;i++){
		token = data[i];
		tempParity = ((token & 0x80)>>7) ^ ((token & 0x40)>>6);
		tempParity ^= ((token & 0x20)>>5);
		tempParity ^= ((token & 0x10)>>4);
		tempParity ^= ((token & 0x08)>>3);
		tempParity ^= ((token & 0x04)>>2);
		tempParity ^= ((token & 0x02)>>1);
		tempParity ^= token & 0x01;
		tempParity = tempParity << i;
		VParity |= tempParity;
	}
}

void get_EXTParity(int *EXTParity,int HParity){
	*EXTParity = ((HParity & 0x80)>>7) ^ ((HParity & 0x40)>>6);
	*EXTParity ^= ((HParity & 0x20)>>5);
	*EXTParity ^= ((HParity & 0x10)>>4);
	*EXTParity ^= ((HParity & 0x08)>>3);
	*EXTParity ^= ((HParity & 0x04)>>2);
	*EXTParity ^= ((HParity & 0x02)>>1);
	*EXTParity ^= HParity & 0x01;
}

//int receive_frame(char *opname)
//Parameter:opname as array of character
//Return value:1 if frame receiving is complete.
//			   0 if frame receiving is not finished yet.
//Description:This function is use for receive frame
//In which it can work in auto mode or pre-work in manual mode by
//Prompt user for save file and wait for stream fileframe from sender
int receive_frame(char *opname){
	static int isReceived = 0; //static variable use for memorized status of receiving
	int contfdatasize;
	int datasize;
	int control,framenum;
	FILE *fw=NULL; //File pointer

	//Receive start field and check whether is receive right or not.
	if((inportb(LSR) & 0x01)!=0x01)return 0;
	if(inport(TXDATA) != 0x7E){printf("START Synchronized ERROR!!!!!!\n");while(1);}

	//Receive control field
	while((inportb(LSR) & 0x01)!=0x01);
	contfdatasize = inport(TXDATA);

	//Extract control field to get control,datasize and framenumber
	control = (contfdatasize & 0xF0)>>4;
	datasize = contfdatasize & 0x07; 
	framenum = (contfdatasize & 0x08)>>3;

	//If first frame received control is 3 (file receive)
	//Then prepare for receive file frame.
	if(control == 0x03){
		//receive first file request frame and prompt for save file
		for(i = 0;i <= datasize;i++){
			while((inportb(LSR) & 0x01)!=0x01);
			data[i] = inport(TXDATA);
		}
		if(datasize>=1){
			while((inportb(LSR) & 0x01)!=0x01);
			HParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			EXTParity = inport(TXDATA);
		}
		else{
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);
		}

		while((inportb(LSR) & 0x01)!=0x01);
		if(inport(TXDATA) != 0x7E){printf("STOP Synchronized ERROR!!!!!!\n");return 0;}

		if(framenum != desiredFrameNumber)
			printf("AUTOMODE::>FRAME NUMBER ERROR!!!\n");

		printf("%s Will send ",opname);
		for(i = 0;i <= datasize;i++){
			if(data[i]!='\0')
				printf("%c",data[i]);
		}


		if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
		send_ACK(1);
		if(!data[datasize] == '\0')
			while(!receive_filereq());

		//Prompt user to enter save target.
		while(!fw){
			printf("\nEnter Path to save file:");
			gets(strin);
			fw = fopen(strin,"wb");
		}
		//send file request ACK to tell sender to be send file frame.
		send_filereqACK();
		//Receive file frame and write binary data to target via fw pointer.
		while(!receive_fileframe(fw));
		fclose(fw);
		return 1;
	}

	//Auto MODE
	else{
		//Receive normal DATA
		for(i = 0;i <= datasize;i++){
			while((inportb(LSR) & 0x01)!=0x01);
			data[i] = inport(TXDATA);
		}


		//Receive 2D-Parity (or 1D-parity)field
		if(datasize>=1){
			while((inportb(LSR) & 0x01)!=0x01);
			HParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			EXTParity = inport(TXDATA);

		}
		else{
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);

		}

		//Receive stop field and check whether is receive right or not.
		while((inportb(LSR) & 0x01)!=0x01);
		if(inport(TXDATA) != 0x7E){printf("STOP Synchronized ERROR!!!!!!\n");return 0;}

		//CHECK FOR ERROR
		if(datasize>=1){
			if(!check_VParity(data,&datasize)){printf("VParity Error.\n");while(1);}
			if(!check_HParity(data,&datasize)){printf("HParity Error.\n");while(1);}
			if(!check_EXTParity(&EXTParity,HParity)){printf("EXTParity Error.\n");while(1);}
		}
		else{if(!check_EXTParity(&VParity,data[0])){printf("1D-Parity Error.\n");while(1);}}
		//Check framenumber with desiredFrameNumber
		//In auto mode is don't care when it error.
		if(framenum != desiredFrameNumber)
			printf("AUTOMODE::>FRAME NUMBER ERROR!!!\n");

		//Display received message
		//If isReceived = 0 display name of sender and follow by message
		//Else display only received message.
		if(isReceived==0){
			printf("%s:",opname);
			if(data[0] == 17 && datasize == 1){
					if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
					send_ACK(1);return 2;
}
			isReceived=1;
}


			for(i = 0;i <= datasize;i++){
				if(data[i]!='\0')
					printf("%c",data[i]);

			}

		//Update desiredFrameNumber
		if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
		//send Normal ACK
		send_ACK(1);

		//Check buffer in last index to tell whether receiving is finished or not?
		if((data[datasize])=='\0'){printf("\n");isReceived=0;return 1;}
		else return 0;
	}

}


//int receive_fileframe(FILE *writefile)
//Parameter:writefile as FILE pointer
//Return Value:1 if file receiving is finished.
//             0 if file receiving is not finished.
//Description:This function is use for receive file frame and
//Check error-framenumber then write binary data to "writefile"
//If receive frame is EOF then this function will return 1
//Each frame receiving will send file ACK everytime.
int receive_fileframe(FILE *writefile){
	int contfdatasize;
	int *ptr,flagdata; /* *ptr use for reference to each 
	//data in receiver buffer*/
	int datasize;
	int control,framenum;

	//Receive start field and check whether is receive right or not.
	if((inportb(LSR) & 0x01)!=0x01)return 0;
	if(inport(TXDATA) != 0x7E){printf("START Synchronized ERROR!!!!!!\n");while(1);}

	//Receive control field
	while((inportb(LSR) & 0x01)!=0x01);
	contfdatasize = inport(TXDATA);
	//Extract control field to get control,datasize and framenumber
	control = (contfdatasize & 0xF0)>>4;
	datasize = contfdatasize & 0x07; 
	framenum = (contfdatasize & 0x08)>>3;
	//if control = 7 it mean EOF.
	if(control == 0x07){
		//Receive parity field and check for error
		while((inportb(LSR) & 0x01)!=0x01);
		flagdata = inport(TXDATA);
		while((inportb(LSR) & 0x01)!=0x01);
		VParity = inport(TXDATA);

		while((inportb(LSR) & 0x01)!=0x01);
		if(inport(TXDATA) != 0x7E){printf("STOP Synchronized ERROR!!!!!!\n");return 0;}

		if(!check_EXTParity(&VParity,flagdata))
			{printf("EXTParity Error.\n");send_ACK(2);return 0;}

		if(framenum != desiredFrameNumber){
		printf("FRAME NUMBER ERROR!!!\nDuplicated EOF.");
		printf("END OF FILE REACHED.\n");
		send_ACK(6);
		return 1;
	    }

		//ACK of last frame cannot be lost or error.
		printf("END OF FILE REACHED.\n");
		if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
		//send file ACK(mode 6) and return 1
		send_ACK(6);
		return 1;

	}

	//receive binary data
	for(i = 0;i <= datasize;i++){
		while((inportb(LSR) & 0x01)!=0x01);
		data[i] = inport(TXDATA);
	}

	//receive 2D-Parity (or 1D-parity)field
	if(datasize>=1){
		while((inportb(LSR) & 0x01)!=0x01);
		HParity = inport(TXDATA);
		while((inportb(LSR) & 0x01)!=0x01);
		VParity = inport(TXDATA);
		while((inportb(LSR) & 0x01)!=0x01);
		EXTParity = inport(TXDATA);
	}
	else{
		while((inportb(LSR) & 0x01)!=0x01);
		VParity = inport(TXDATA);
	}
	//Receive stop field and check whether is receive right or not.
	while((inportb(LSR) & 0x01)!=0x01);
	if(inport(TXDATA) != 0x7E){printf("STOP Synchronized ERROR!!!!!!\n");return 0;}

	//CHECK FOR ERROR if error -> send NCK and wait for new frame....
		if(datasize>=1){
			if(!check_VParity(data,&datasize)){printf("VParity Error.\n");send_ACK(2);return 0;}
			if(!check_HParity(data,&datasize)){printf("HParity Error.\n");send_ACK(2);return 0;}
			if(!check_EXTParity(&EXTParity,HParity)){printf("EXTParity Error.\n");send_ACK(2);return 0;}
		}
		else{if(!check_EXTParity(&VParity,data[0])){printf("1D-Parity Error.\n");send_ACK(2);return 0;}}

	//Check number frame whether it right or wrong...
		if(framenum != desiredFrameNumber){
		printf("FRAME NUMBER SEQ ERROR!!!\nSend latest ACK to correct it.\n");
		send_ACK(6);
		return(0);
	}
		//Write binary data to writefile pointer.
		for(i = 0;i <= datasize;i++){
		*ptr = data[i];
		fwrite(ptr,1,1,writefile);
	}
	//Update desiredFrameNumber and send ACK in mode 6
	if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
	send_ACK(6);
	return 0;
}


//void send_ACK(int mode)
//Parameter:mode as integer
//Return value:Node
//Description:Similar to send normal frame but differ in mode
//mode will determine which type of ACK to be send.
void send_ACK(int mode){
	//SEND START FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);

	//SEND CONTROL WITH FRAMNUMBER FIELD and ACKmode
	//CASE1:NORMAL ACK mode
	if(mode == 1){
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x10|(desiredFrameNumber<<3));
	}
	
	//CASE2:NAK
	else if(mode == 2){
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x20|(desiredFrameNumber<<3));
	}
	
	//CASE3:FILE_ACK
	else if(mode == 6){
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x60|(desiredFrameNumber<<3));
	}

	//calculate 1D-Parity and send
	get_EXTParity(&VParity,desiredFrameNumber);
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,VParity);


	//SEND STOP FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
}


//int receive_ACK()
//Parameter:None
//Return value:1 if ACK is normal then it work in auto mode
//             2 if NAK has received then sender will send latest frame
//             6 if FILE ACK has received then sender can send next file frame
//Description:This function will receive ACK frame
//and determine which work will sender to do? (From 3 possible return value.) 
int receive_ACK(){
	int control,acknum;
	//Receive start field and check whether is receive right or not.
	if((inportb(LSR) & 0x01) != 0x01) return 0;
	if(inport(TXDATA) == 0x7e){
		//receive control field and extract data from control field
		while((inportb(LSR) & 0x01) != 0x01);
		control = inport(TXDATA);
		acknum = (control & 0x08)>>3;
		//receive 1D-parity.
		while((inportb(LSR) & 0x01) != 0x01);
		VParity = inport(TXDATA);
		//Receive stop field and check whether is receive right or not.
		while((inportb(LSR) & 0x01) != 0x01);
		if(inport(TXDATA) != 0x7e){printf("STOP ACK synchronized ERROR!!!\n");return 0;}
		control = control >> 4;
		//Check mode whether is NORMAL ACK(mode 1,6) or NAK(mode 2)
		if(control == 1||control ==6){
			//Check whether frame error???
			if(!check_EXTParity(&VParity,acknum)){
				textcolor(12);
				cprintf("ACK FRAME ERROR!!!!\r\n");
				cprintf("Resend latest frame automatically.....\r\n");
				cprintf("");
				textcolor(11);
				if(--frameNumber < 0)frameNumber = 1;
				return 2;
			}
			//check frame number sequence.
			if(acknum == frameNumber)
				return 1;
		}
		else if(control ==2){
			//Check whether frame error???
			if(!check_EXTParity(&VParity,acknum)){
				textcolor(12);
				cprintf("NAK FRAME ERROR!!!!\r\n");
				cprintf("Resend latest frame automatically.....\r\n");
				cprintf("");
				textcolor(11);
			}
			frameNumber = acknum;
			return 2;
		}
	}
	else
		printf("START ACK synchronized ERROR!!!");
	return 0; //return 0 if send-receive is ERROR.
}


//void send_EOF()
//Parameter:None
//Return value:None
//Description:This function will send special frame which called "EOF frame"
//this frame work in mode 7 it will tell receiver as END OF FILE.
void send_EOF(){
	int key,modeACK;
	
resend_EOF:
	//SEND START FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
	//SEND EOF CONTROL WITH FRAMENUMBER
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x70|(frameNumber<<3));

	//send data flag
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0xFF);

	//Calculate 1D-Parity
	get_EXTParity(&VParity,0xFF);
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,VParity);

	//SEND STOP FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);

	//MANUAL MODE....
	//Display current frame that send
	textcolor(11);
	cprintf("");
	cprintf("Current frame is....");
	printf("\n");
	cprintf("%s SEND:%x %x %x ",name,0x7E,0x07,frameNumber);
	cprintf("%x ",0xFF);
	cprintf("%x %x\r\n",VParity,0x7E);
	//========================================
	cprintf("HEADER:%x %x %x\r\n",0x7E,0x7,frameNumber);
	cprintf("DATA:");
	cprintf("EOF %x\r\n",0xFF);
	cprintf("");
	cprintf("TRAILER:");
	cprintf("%x %x\r\n\r\n",VParity,0x7E);
	//========================================
	//Prompt <- and -> button to transmit next or timeout
	while(1){
		cprintf("Press ==> Key to do next step.\r\n");
		cprintf("Press <== Key to imply it timeout.\r\n");
		cprintf("What do you want to do? [<== or ==>]:");
	if(getch()!=0)continue;
	if((key = getch())==0x4b){cprintf("You press <==\r\n");goto resend_EOF;}
	else if(key == 0x4d){cprintf("You press ==>\r\n");break;}
		cprintf("\r\n");
	}
	//update frame number
	if(++frameNumber>1)frameNumber = 0;

	//Check for ACK/NAK
	while(1){
		modeACK = receive_ACK();
		if(modeACK == 1||modeACK == 6)break;
		else if(modeACK == 2){
			goto resend_EOF;
		}
	}
	textcolor(14);
	cprintf("SEND FILE SUCCESS!\r\n");
}

//int receive_filereq()
//Parameter:None
//Return value:1 if last file request frame is received.
//             0 if is not.
//Description:This function will receive the rest of file request in which receiver
//will received sender's filename.
int receive_filereq(){
	int contfdatasize;
	int datasize;
	//Receive start field and check whether is receive right or not.
	if((inportb(LSR) & 0x01)!=0x01)return 0;
	if(inport(TXDATA) == 0x7E){
		//Receive control field
		while((inportb(LSR) & 0x01)!=0x01);
		contfdatasize = inport(TXDATA);
		//Extract control field to get datasize
			datasize = contfdatasize & 0x07; 
		//Receive data
		for(i = 0;i <= datasize;i++){
			while((inportb(LSR) & 0x01)!=0x01);
			data[i] = inport(TXDATA);
		}
		//Receive 2D-parity(or 1D-parity)
		if(datasize>=1){
			while((inportb(LSR) & 0x01)!=0x01);
			HParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);
			while((inportb(LSR) & 0x01)!=0x01);
			EXTParity = inport(TXDATA);
		}
		else{
			while((inportb(LSR) & 0x01)!=0x01);
			VParity = inport(TXDATA);
		}
		//Receive stop field and check whether is receive right or not.
		while((inportb(LSR) & 0x01)!=0x01);
		if(inport(TXDATA)!=0x7E)
			printf("File REQ:STOP FLAG ERROR!!!\n");

		//Display the rest of sender's filename.
		for(i = 0;i <= datasize;i++){
			if(data[i]!='\0')
				printf("%c",data[i]);
		}
		//Update frame number and send normal sub ACK.
		if(++desiredFrameNumber > 1)desiredFrameNumber = 0;
		send_ACK(1);
		if(data[datasize] == '\0')return 1;
		else return 0;
	}
	else{
		printf("File REQ:START FLAG ERROR!!!\n"); return 0;
	}
}


//int wait_fileACK()
//Parameter:None
//Return value:1 if file request ACK is received successfully.
//             0 if is not.
//Description:This function will receive file request ACK to tell sender
//To send file frame.
int wait_fileACK(){
	int control,flagdata;
	//Receive start field and check whether is receive right or not.
	if((inportb(LSR) & 0x01) != 0x01) return 0;
	if(inport(TXDATA) == 0x7e){
		//Receive control with flagdata field.
		while((inportb(LSR) & 0x01) != 0x01);
		control = inport(TXDATA);
		//Extract flagdata
		flagdata = control & 0x0F;
		//Receive 1D-Parity
		while((inportb(LSR) & 0x01) != 0x01);
		VParity = inport(TXDATA);
		//Receive stop field and check whether is receive right or not.
		while((inportb(LSR) & 0x01) != 0x01);
		if(inport(TXDATA) != 0x7e){printf("STOP ACK ERROR!!!\n");return 0;}
		else{
			if(check_EXTParity(&VParity,flagdata))return 1;
			else
			{printf("flag data error!!!\n");return 0;}
		}
	}
	else{
		printf("START ACK ERROR0!!!\n");return 0;
	}

}

//void send_filereqACK()
//Parameter:None
//Return value:None
//Description:This function will send file request ACK to sender if
//receiver has receive all file request from sender.
void send_filereqACK(){

	//SEND START FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);

	//SEND CONTROL mode 4 filereqACK. and flagdata 0xF (0x40 | 0xF)
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x4F);

	//Calculate 1D-Parity and send
	get_EXTParity(&VParity,0xF);
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,VParity);

	//SEND STOP FIELD
	while((inportb(LSR) & 0x40) != 0x40);
	outport(TXDATA,0x7E);
}


//int check_VParity(int* data,int *datasize)
//int check_HParity(int* data,int *datasize)
//int check_EXTParity(int *EXTParity,int HParity)
//Parameter:data as array of integer,EXTParity,datasize as pointer of integer
//HParity as integer(pass by value)
//Return value:1 if parity is correct (EVEN PARITY)
//             0 if is not.
//Description:This function will check 2D-parity which include
//HParity :Horizontal parity
//VParity :Vertical parity
//EXTParity:Parity of Horizontal parity.
int check_VParity(int* data,int *datasize){
	int tempParity,sumParity = 0x00;
	int token;
	for(i = 0;i <= *datasize ;i++){
		token = data[i];
		tempParity = ((token & 0x80)>>7) ^ ((token & 0x40)>>6);
		tempParity ^= ((token & 0x20)>>5);
		tempParity ^= ((token & 0x10)>>4);
		tempParity ^= ((token & 0x08)>>3);
		tempParity ^= ((token & 0x04)>>2);
		tempParity ^= ((token & 0x02)>>1);
		tempParity ^= token & 0x01;
		tempParity = tempParity << i;
		sumParity |= tempParity;
	}
	sumParity ^= VParity;
	if(!sumParity)return 1;
	else return 0;
}

int check_HParity(int* data,int *datasize){
	int tempParity;
	tempParity = data[0] ^ data[1];
	for(i = 2;i<=*datasize;i++)
		tempParity ^= data[i];
	tempParity ^= HParity;
	if(!tempParity)return 1;
	else return 0;
}

int check_EXTParity(int *EXTParity,int HParity){
	int tempParity;
	tempParity = ((HParity & 0x80)>>7) ^ ((HParity & 0x40)>>6);
	tempParity ^= ((HParity & 0x20)>>5);
	tempParity ^= ((HParity & 0x10)>>4);
	tempParity ^= ((HParity & 0x08)>>3);
	tempParity ^= ((HParity & 0x04)>>2);
	tempParity ^= ((HParity & 0x02)>>1);
	tempParity ^= HParity & 0x01;
	tempParity ^= *EXTParity;
	if(!tempParity)return 1;
	else return 0;
}


//void send_name(char *name)
//Parameter:name as array of character
//Return value:None
//send name to receiver to tell what is sender name.
void send_name(char *name){
	int ch;
	while(1){
		while((inportb(LSR) & 0x40) != 0x40);
		ch = *(name++);
		outport(TXDATA,ch);
		if(ch == '\0')break;
	}
}


//void receive_oname()
//Parameter:None
//Return value:Array of character
//receiver name from receiver to tell what is receiver name.
char * receive_oname(){
	char *rname = "",*start;
	start = rname;
	while((inportb(LSR) & 0x01) != 0x01);
	while(1){
		while((inportb(LSR) & 0x01) != 0x01);
		*(rname++) = inport(TXDATA);
		if(*(rname-1)=='\0')break;

	}
	return start;
}