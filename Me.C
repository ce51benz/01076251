/*ME CODE
MEMBERS
1.Kittiwat Lucksanasomboon 55010095 2D/1
2.Yanisa Yimsuwan          55010302 2D/1
3.Phanita Phinyophap       55010906 2D/2
4.Suratchanan Kraidech     55011362 2D/2
*/

//CONSTANT DECLARATION
//COM3
#define COM3TX 0x3E8
#define COM3LSR (COM3TX + 5)
#define COM3LCR (COM3TX + 3)

//COM4
#define COM4TX 0x2E8
#define COM4LSR (COM4TX + 5)
#define COM4LCR (COM4TX + 3)

//HEADER FILE DECLARATION
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<dos.h>

//Function Prototype Declaration::
//1.Utility functions
void setup_COM3();
void setup_COM4();
void receive_name(char *com1_name,char *com2_name);
void send_COM1name(char *com1_name);
void send_COM2name(char *com2_name);
//2.Error functions
void make_error();
void make_error_ACK();
void make_error_EOF();
//3.Receive and Send function
void do_autosendMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
int do_autoACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_manualNAKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_fileREQMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_fileREQ_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_sendfileMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_sendfile_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);
void do_EOFMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName);

//Global Variable Declaration::
//Reduce memory allocation and program mistake.
int receiveval = 0;
int startbyte = 0,stopbyte = 0;
int contfdatasize = 0;
int data[8] = {0,0,0,0,0,0,0,0};
int HParity = 0,VParity = 0,EXTParity = 0,datasize = 0,i = 0,key = 0,key1 = 0;
int control = 0,frameNumber = 0,flagdata = 0;
char *com2_name = "";
//Main program
int main(){
	char* com1_name;
	int com3txrx = COM3TX,com3stat = COM3LSR,com4txrx = COM4TX,com4stat = COM4LSR;
	//Set up serial port COM3,COM4
	clrscr();
	setup_COM3();
	setup_COM4();
	//Set color display of cprintf to green
	textcolor(10);
	cprintf("==========Instant MSG Monitor==========\r\n\r\n");
	//Waiting for name of COM1 and COM2
	receive_name(com1_name,com2_name);
	printf("COM1 is %s\n",com1_name);
	printf("COM2 is %s\n",com2_name);
	//Send both received name to each other
	send_COM1name(com1_name);
	send_COM2name(com2_name);
	//Set color display of cprintf to white
	textcolor(15);
	while(1){
		//Check where (source) frame receive is come from?
		if((inportb(COM3LSR) & 0x01)==0x01){
			//Receive start field
			startbyte = inport(COM3TX);
			//Receive control field
			while((inportb(COM3LSR) & 0x01)!=0x01);
			//Extract control field to get control/framenumber
			//in which that frame will continue to receive and work...
			contfdatasize = inport(COM3TX);
			control = (contfdatasize & 0xF0)>>4;
			frameNumber = (contfdatasize & 0x08)>>3;

			//If control value = 0 then work in auto send mode
			if(control == 0x00) {
				do_autosendMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 1 then work in auto ACK mode
			else if(control == 0x01){
				if(do_autoACKMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name))break;
			}

			//If control value = 2 then work in namual NAK mode
			else if(control == 0x02){
				do_manualNAKMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 3 then work in auto send file request mode
			else if(control == 0x03){
				do_fileREQMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 4 then work in auto file request ACK mode
			else if(control == 0x04){
				do_fileREQ_ACKMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 5 then work in manual send file mode
			else if(control == 0x05){
				do_sendfileMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 6 then work in manual file ACK mode
			else if(control == 0x06){
				do_sendfile_ACKMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}

			//If control value = 7 then work in manual EOF mode
			else if(control == 0x07){
				do_EOFMode(&com3txrx,&com3stat,&com4txrx,&com4stat,com1_name);
			}
		}

		//Another receive source COM4 work procedure is similar as above
		else if((inportb(COM4LSR) & 0x01)==0x01){
			startbyte = inport(COM4TX);
			while((inportb(COM4LSR) & 0x01)!=0x01);
			contfdatasize = inport(COM4TX);
			control = (contfdatasize & 0xF0)>>4;
			frameNumber = (contfdatasize & 0x08)>>3;
			//Auto send mode
			if(control == 0x00){
				do_autosendMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}

			//ACK MODE
			else if(control == 0x01){
				if(do_autoACKMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name))break;
			}

			//NAK Mode
			else if(control == 0x02){
				do_manualNAKMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}
			//SENDING FILE REQUEST.
			else if(control == 0x03){
				do_fileREQMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}

			//File REQ ACK
			else if(control == 0x04){ 
				do_fileREQ_ACKMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}

			//SENDING FILE MODE
			else if(control == 0x05){
				do_sendfileMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}

			//FILE_ACK MODE
			else if(control == 0x06){
				do_sendfile_ACKMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}

			//EOF MODE
			else if(control == 0x07){
				do_EOFMode(&com4txrx,&com4stat,&com3txrx,&com3stat,com2_name);
			}
		}
	}
	printf("\nEnd of communication\nPress anykey to Exit.");
	getch();
	return 0;
}


//void do_autosendMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and send it to receiver.
//It will display as auto ACK frame mode.
void do_autosendMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Extract control field to get datasize
	datasize = contfdatasize & 0x07;

	//Receive data
	for(i = 0;i <= datasize;i++){
		while((inportb(*senderstat) & 0x01)!=0x01);
		data[i] = inport(*sender);
	}

	//Receive Parity
	if(datasize>=1){
		while((inportb(*senderstat) & 0x01)!=0x01);
		HParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		EXTParity = inport(*sender);
	}
	else{
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
	}
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);

	//Display detail of frame in HEX format and ASCII converted format
	cprintf("%s SEND:%x %x %x %x ",senderName,startbyte,control,frameNumber,datasize);
	for(i = 0;i <= datasize;i++)
		cprintf("%x",data[i]);
	printf(" ");
	if(datasize>=1){
		cprintf("%x %x %x %x\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n",VParity,stopbyte);


	//========================================
	cprintf("HEADER:%x %x %x %x\r\n",startbyte,control,frameNumber,datasize);
	cprintf("DATA:");
	for(i = 0;i <= datasize;i++)
		cprintf("%c",data[i]);

	cprintf("\r\nTRAILER:");

	if(datasize>=1){
		cprintf("%x %x %x %x\r\n\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================


	//Send frame continue to receiver and exit function to receive next.
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);

	for(i = 0;i <= datasize;i++){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,data[i]);
	}

	if(datasize>=1){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,HParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,EXTParity);
	}
	else{
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);
	}

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
}

//void do_autoACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and send it to receiver
//It will display as auto ACK frame mode.
//REMARK:if last frame sender sent: CTRL + Q program will be terminate.
int do_autoACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Receive Parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	VParity = inport(*sender);
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);
	//Display detail of frame in auto ACK frame mode
	printf("%s SEND:%x %x %x ",senderName,startbyte,control,frameNumber);
	printf("%x %x\n",VParity,stopbyte);
	//========================================
	printf("HEADER:%x %x\n",startbyte,control);
	printf("DATA:ACK%d\n",frameNumber);
	printf("TRAILER:");
	printf("%x %x\n\n",VParity,stopbyte);
	//========================================

	//Send received frame continue to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,VParity);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);

	//REMARK if last frame which sender sent CTRL + Q
	//The program will be terminated
	if(data[0] == 17 && data[1] == '\0')return 1;
	else return 0;
}

//void do_manualNAKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and
//prompt user to do sth with current frame
//It will display as manual NAK frame mode.
void do_manualNAKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Receive parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	VParity = inport(*sender);
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);
	//Display detail of received frame
	cprintf("%s SEND:%x %x %x ",senderName,startbyte,control,frameNumber);
	cprintf("%x %x\r\n",VParity,stopbyte);
	//========================================
	cprintf("HEADER:%x %x\r\n",startbyte,control);
	cprintf("DATA:NAK%d\r\n",frameNumber);
	cprintf("TRAILER:");
	cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================

	//Prompt user to do sth with frame
	while(1){
		textcolor(10);
		cprintf("Press [Space] Key to do transmit current frame.\r\n");
		cprintf("Press E Key to make frame error.\r\n");
		cprintf("Press [Delete] Key to drop current frame.\r\n");
		cprintf("What do you want to do?[Space or E or Delete]:");
		key = getch();
		if(key!=0&&key!=0x45&&key!=0x65&&key!=0x20){textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		//If error mode is use it will show 
		//detail of edited frame too.(In yellow text display).
		if(key == 0x45 || key == 0x65){
			cprintf("E\r\nYou Enter error mode.\r\n\r\n");
			make_error_ACK();

			textcolor(14);
			cprintf("%s SENT EDITED FRAME:%x %x %x ",senderName,startbyte,control,frameNumber);
			cprintf("%x %x\r\n",VParity,stopbyte);
			//========================================
			cprintf("HEADER:%x %x\r\n",startbyte,control);
			cprintf("DATA:NAK%d\r\n",frameNumber);
			cprintf("TRAILER:");
			cprintf("%x %x\r\n\r\n",VParity,stopbyte);
			//========================================

			break;
		}
		if(key == 0x20){cprintf("[Space]\r\n\r\n");break;}
		if((getch())==0x53){cprintf("[Delete]\r\n\r\n");goto exitpoint;}
		else{textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		printf("\n\n");
	}
	//Send NAK frame to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,VParity);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
exitpoint:
	//exitpoint use for skip send current frame
	//imply current frame is deleted.

	//set text color display to white.
	textcolor(15);
}

//void do_fileREQMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and send it to receiver
//It will display as auto send file request frame mode.
void do_fileREQMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Extract datasize from control field
	datasize = contfdatasize & 0x07; 
	//Receive data
	for(i = 0;i <= datasize;i++){
		while((inportb(*senderstat) & 0x01)!=0x01);
		data[i] = inport(*sender);
	}

	//Receive parity
	if(datasize>=1){
		while((inportb(*senderstat) & 0x01)!=0x01);
		HParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		EXTParity = inport(*sender);
	}
	else{
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
	}
	//Receive parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);

	//Display detail of received frame in hex and ASCII format
	cprintf("%s SEND:%x %x %x %x ",senderName,startbyte,control,frameNumber,datasize);
	for(i = 0;i <= datasize;i++)
		cprintf("%x",data[i]);
	printf(" ");
	if(datasize>=1){
		cprintf("%x %x %x %x\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n",VParity,stopbyte);

	//========================================
	cprintf("HEADER:%x %x %x %x\r\n",startbyte,control,frameNumber,datasize);
	cprintf("DATA:");
	for(i = 0;i <= datasize;i++)
		cprintf("%c",data[i]);

	cprintf("\r\nTRAILER:");

	if(datasize>=1){
		cprintf("%x %x %x %x\r\n\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================

	//Send received frame continue to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);

	for(i = 0;i <= datasize;i++){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,data[i]);
	}

	if(datasize>=1){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,HParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,EXTParity);
	}
	else{
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);
	}

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
}

//void do_fileREQ_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and send it to receiver
//It will display as auto send file request ACK frame mode.
void do_fileREQ_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Receive Parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	VParity = inport(*sender);
	//Receive stopbyte
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);
	//Extract flagdata from control field
	flagdata = contfdatasize & 0x0F;

	//Display detail of received frame
	printf("%s SEND:%x %x ",senderName,startbyte,control);
	printf("%x %x %x\n",flagdata,VParity,stopbyte);
	//========================================
	printf("HEADER:%x %x\n",startbyte,control);
	printf("DATA:%x [File_REQ_ACK]\n",flagdata);
	printf("TRAILER:");
	printf("%x %x\n\n",VParity,stopbyte);
	//========================================
	//Send received frame continue to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,VParity);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
}

//void do_sendfileMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and
//prompt user to do sth with current frame
//It will display as manual send file frame mode.
void do_sendfileMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Extract datasize
	datasize = contfdatasize & 0x07; 

	//Receive data
	for(i = 0;i <= datasize;i++){
		while((inportb(*senderstat) & 0x01)!=0x01);
		data[i] = inport(*sender);
	}

	//Receive parity
	if(datasize>=1){
		while((inportb(*senderstat) & 0x01)!=0x01);
		HParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
		while((inportb(*senderstat) & 0x01)!=0x01);
		EXTParity = inport(*sender);
	}
	else{
		while((inportb(*senderstat) & 0x01)!=0x01);
		VParity = inport(*sender);
	}
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);

	//Display detail of received frame
	cprintf("%s SEND:%x %x %x %x ",senderName,startbyte,control,frameNumber,datasize);
	for(i = 0;i <= datasize;i++)
		cprintf("%x",data[i]);
	printf(" ");
	if(datasize>=1){
		cprintf("%x %x %x %x\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n",VParity,stopbyte);
	//========================================
	cprintf("HEADER:%x %x %x %x\r\n",startbyte,control,frameNumber,datasize);
	cprintf("DATA:");
	for(i = 0;i <= datasize;i++)
		cprintf("%x ",data[i]);

	cprintf("\r\nTRAILER:");

	if(datasize>=1){
		cprintf("%x %x %x %x\r\n\r\n",HParity,VParity,EXTParity,stopbyte);
	}
	else
		cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================
	//Prompt user to do sth with frame
	//Delete Frame
	//Make Error
	//Transmit
	while(1){
		textcolor(10);
		cprintf("Press [Space] Key to do transmit current frame.\r\n");
		cprintf("Press E Key to make frame error.\r\n");
		cprintf("Press [Delete] Key to drop current frame.\r\n");
		cprintf("What do you want to do?[Space or E or Delete]:");
		key = getch();
		if(key!=0&&key!=0x45&&key!=0x65&&key!=0x20){textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		if(key == 0x45 || key == 0x65){
			//If error mode is used
			//It will display detail of edited frame in yellow text.
			cprintf("E\r\nYou Enter error mode.\r\n");
			make_error();
			textcolor(14);
			cprintf("%s SENT EDITED FRAME:%x %x %x %x ",senderName,startbyte,control,frameNumber,datasize);
			for(i = 0;i <= datasize;i++)
				cprintf("%x",data[i]);
			printf(" ");
			if(datasize>=1){
				cprintf("%x %x %x %x\r\n",HParity,VParity,EXTParity,stopbyte);
			}
			else
				cprintf("%x %x\r\n",VParity,stopbyte);
			//========================================
			cprintf("HEADER:%x %x %x %x\r\n",startbyte,control,frameNumber,datasize);
			cprintf("DATA:");
			for(i = 0;i <= datasize;i++)
				cprintf("%x ",data[i]);

			cprintf("\r\nTRAILER:");

			if(datasize>=1){
				cprintf("%x %x %x %x\r\n\r\n",HParity,VParity,EXTParity,stopbyte);
			}
			else
				cprintf("%x %x\r\n\r\n",VParity,stopbyte);
			//========================================
			break;
		}
		if(key == 0x20){cprintf("[Space]\r\n\r\n");break;}
		if((getch())==0x53){cprintf("[Delete]\r\n\r\n");goto exitpoint;}
		else{textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		printf("\n\n");
	}


	//Send received frame to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);

	for(i = 0;i <= datasize;i++){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,data[i]);
	}

	if(datasize>=1){
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,HParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);

		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,EXTParity);
	}
	else{
		while((inportb(*receiverstat) & 0x40)!=0x40);
		outport(*receiver,VParity);
	}

	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
exitpoint://exitpoint use for deleted frame(to skip send frame cmd)
	//Set display text to white to avoid display mistake
	textcolor(15);
}

//void do_sendfile_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and
//prompt user to do sth with current frame
//It will display as manual send file ACK frame mode.
void do_sendfile_ACKMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Receive parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	VParity = inport(*sender);
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);
	//Display detail of received frame
	cprintf("%s SEND:%x %x %x ",senderName,startbyte,control,frameNumber);
	cprintf("%x %x\r\n",VParity,stopbyte);
	//========================================
	cprintf("HEADER:%x %x\r\n",startbyte,control);
	cprintf("DATA:FILE_ACK%d\r\n",frameNumber);
	cprintf("TRAILER:");
	cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================

	//Prompt user to do sth with frame
	//Delete Frame
	//Make Error
	//Transmit

	while(1){
		textcolor(10);
		cprintf("Press [Space] Key to do transmit current frame.\r\n");
		cprintf("Press E Key to make frame error.\r\n");
		cprintf("Press [Delete] Key to drop current frame.\r\n");
		cprintf("What do you want to do?[Space or E or Delete]:");
		key = getch();
		if(key!=0&&key!=0x45&&key!=0x65&&key!=0x20){textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		if(key == 0x45 || key == 0x65){
			//If error mode is used
			//It will display detail of edited frame in yellow text.
		cprintf("E\r\nYou Enter error mode.\r\n\r\n");
		make_error_ACK();
		textcolor(14);
		cprintf("%s SENT EDITED FRAME:%x %x %x ",senderName,startbyte,control,frameNumber);
		cprintf("%x %x\r\n",VParity,stopbyte);
		//========================================
		cprintf("HEADER:%x %x\r\n",startbyte,control);
		cprintf("DATA:FILE_ACK%d\r\n",frameNumber);
		cprintf("TRAILER:");
		cprintf("%x %x\r\n\r\n",VParity,stopbyte);
		//========================================
		break;
		}
		if(key == 0x20){cprintf("[Space]\r\n\r\n");break;}
		if((getch())==0x53){cprintf("[Delete]\r\n\r\n");goto exitpoint;}
		else{textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		printf("\n\n");
	}
	//Send received frame to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,VParity);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
exitpoint://exitpoint use to skip send frame cmd
		  //and imply current frame is deleted.
	//Set text color display to white to avoid display mistake
	textcolor(15);
}

//void do_EOFMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName)
//Parameter:sender,senderstat,receiver,receiverstat as pointer of integer
//senderName as array of character
//Return value:None
//Description:This function will receive frame which sender has sent
//then display detail of that frame and
//prompt user to do sth with current frame
//It will display as manual send EOF frame mode.
void do_EOFMode(int *sender,int *senderstat,int *receiver,int *receiverstat,char *senderName){
	//Receive flag data
	while((inportb(*senderstat) & 0x01)!=0x01);
	flagdata = inport(*sender);
	//Receive parity
	while((inportb(*senderstat) & 0x01)!=0x01);
	VParity = inport(*sender);
	//Receive stop field
	while((inportb(*senderstat) & 0x01)!=0x01);
	stopbyte = inport(*sender);
	//Display detail of received frame
	cprintf("%s SEND:%x %x %x %x ",senderName,startbyte,control,frameNumber,flagdata);
	cprintf("%x %x\r\n",VParity,stopbyte);
	//========================================
	cprintf("HEADER:%x %x %x\r\n",startbyte,control,frameNumber);
	cprintf("DATA:EOF %x\r\n",flagdata);
	cprintf("TRAILER:");
	cprintf("%x %x\r\n\r\n",VParity,stopbyte);
	//========================================

	//Prompt user to do sth with frame
	//Delete Frame
	//Make Error
	//Transmit

	while(1){
		textcolor(10);
		cprintf("Press [Space] Key to do transmit current frame.\r\n");
		cprintf("Press E Key to make frame error.\r\n");
		cprintf("Press [Delete] Key to drop current frame.\r\n");
		cprintf("What do you want to do?[Space or E or Delete]:");
		key = getch();
		if(key!=0&&key!=0x45&&key!=0x65&&key!=0x20){textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		if(key == 0x45 || key == 0x65){
			//If error mode is used
			//It will display detail of edited frame in yellow text.
			cprintf("E\r\nYou Enter error mode.\r\n\r\n");
			make_error_EOF();
			textcolor(14);
			cprintf("%s SENT EDITED FRAME:%x %x %x %x ",senderName,startbyte,control,frameNumber,flagdata);
			cprintf("%x %x\r\n",VParity,stopbyte);
			//========================================
			cprintf("HEADER:%x %x %x\r\n",startbyte,control,frameNumber);
			cprintf("DATA:EOF %x\r\n",flagdata);
			cprintf("TRAILER:");
			cprintf("%x %x\r\n\r\n",VParity,stopbyte);
			break;
		}
		if(key == 0x20){cprintf("[Space]\r\n\r\n");break;}
		if((getch())==0x53){cprintf("[Delete]\r\n\r\n");goto exitpoint;}
		else{textcolor(12);cprintf("Invalid Input.\r\n\r\n");textcolor(10);continue;}
		printf("\n\n");
	}

	//Send received frame to receiver
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,startbyte);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,contfdatasize);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,flagdata);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,VParity);
	while((inportb(*receiverstat) & 0x40)!=0x40);
	outport(*receiver,stopbyte);
exitpoint:
			//exitpoint use to skip send frame cmd
			//and imply current frame is deleted.
	//Set text color display to white to avoid display mistake
	textcolor(15);
}

//void setup_COM3() and void setup_COM4()
//use for set up serial port
void setup_COM3(){
	outportb(COM3LCR,0x80);
	outportb(COM3TX,0x0C);
	outportb(COM3TX+1,0x00);
	outportb(COM3LCR,0x0B);
}

void setup_COM4(){
	outportb(COM4LCR,0x80);
	outportb(COM4TX,0x0C);
	outportb(COM4TX+1,0x00);
	outportb(COM4LCR,0x0B);
}


//void make_error()
//Parameter:None
//Return value:None
//Description:This function will make error to current frame
//before send it to receiver.
//Error can make any byte expept start/stop/control
//You can also select sequence of edit byte.
void make_error(){
	int editseq[11],editsize;
	int edittime = 0,n,input;
	while(1){
		//Prompt user to input edit size
		cprintf("Enter total of byte do you want to edit:");
		scanf("%d",&editsize);
		if(datasize == 0){
			if((!editsize>0&&editsize<=2)) //data 1 byte + parity 1 byte
			{
				textcolor(12);
				cprintf("Invalid size\r\n");
				cprintf("Please enter editsize range 1 - 2\r\n\r\n");
				textcolor(10);
			}
			else break;
		}
		//+3 from parity and + 1 from real datasize value
		else if(editsize>0&&editsize<=datasize+1+3)break;
		else{
			textcolor(12);
			cprintf("Invalid size\r\n");
			cprintf("Please enter size range 1 - %d\r\n\r\n",datasize+1+3);
			textcolor(10);
		}
	}

	//Prompt user to input editseq to edit byte.
	//Case datasize is 0 (or 1 in real)
	//There is possible 2 bytes that can edit.
	if(datasize ==0){
		cprintf("Enter edit sequence to edit byte\r\n");
		cprintf("Remark:0 is Data,1 is EXTParity.(Have one byte parity)\r\n");
		while(edittime<editsize){
checkpoint01:
			cprintf("Enter seq.%d:",edittime+1);
			scanf("%d",&input);

			//Check is it correct input?
			if(!(input >= 0 && input <= 1)){
				textcolor(12);
				cprintf("Invalid input\r\n");
				cprintf("Please enter sequence range 0 to 1\r\n\r\n");
				textcolor(10);
				continue;
			}

			//Check is sequence duplicate?
			if(edittime>0){
				for(n = 0;n<edittime;n++){
					if(editseq[n] == input){
						textcolor(12);
						cprintf("Duplicate input!!!!\r\n");
						cprintf("Please enter other sequence.\r\n\r\n");
						textcolor(10);
						goto checkpoint01;
					}
				}
			}

			editseq[edittime++] = input;
		}

		//Input new byte from input sequence
		for(n = 0;n <editsize;n++){
checkpoint02:
			cprintf("Enter new Byte in HEX format for Byte %d:",editseq[n]);
			scanf("%x",&input);
			if(!(input>=0&&input<=0xFF))
			{
				textcolor(12);
				cprintf("Invalid input!!!!\r\n");
				cprintf("Please enter HEX range 0x00 to 0xFF.\r\n\r\n");
				textcolor(10);
				goto checkpoint02;
			}
			//Since it has possible 2 byte that can edit
			//check sequence to change value for them.
			if(editseq[n] == 0)
				data[0] = input;
			else
				VParity = input;

		}
	}

	//CASE datasize is more than one
	else{
		//Prompt user to input edit sequence
		cprintf("Enter edit sequence to edit byte\r\n");
		cprintf("Remark:0 to %d is Data\r\n",datasize);
		cprintf("%d is Horizontal parity\r\n",datasize+1);
		cprintf("%d is Vertical parity\r\n",datasize+2);
		cprintf("%d is EXT parity\r\n",datasize+3);

		while(edittime<editsize){
checkpoint03:
			cprintf("Enter seq.%d:",edittime+1);
			scanf("%d",&input);

			//Check is it correct input?
			if(!(input >= 0 && input < 11)){
				textcolor(12);
				cprintf("Invalid input\r\n");
				cprintf("Please enter sequence range 0 to 10\r\n\r\n");
				textcolor(10);
				continue;
			}

			//Check is sequence duplicate?
			for(n = 0;n<edittime;n++){
				if(editseq[n] == input){
					textcolor(12);
					cprintf("Duplicate input!!!!\r\n");
					cprintf("Please enter other sequence.\r\n\r\n");
					textcolor(10);
					goto checkpoint03;
				}
			}

			editseq[edittime++] = input;
		}

		//Input new byte!!
		for(n = 0;n <editsize;n++){
checkpoint04:
			cprintf("Enter new Byte in HEX format for Byte %d:",editseq[n]);
			scanf("%x",&input);
			if(!(input>=0&&input<=0xFF))
			{
				textcolor(12);
				cprintf("Invalid input!!!!\r\n");
				cprintf("Please enter HEX range 0x00 to 0xFF.\r\n\r\n");
				textcolor(10);
				goto checkpoint04;
			}

			if(editseq[n] <= 7)
				data[editseq[n]] = input;
			else if(editseq[n] == 8)
				HParity = input;
			else if(editseq[n] == 9)
				VParity = input;
			else
				EXTParity = input;
		}
		
	}
	//END OF EDIT BYTE
}

//void make_error_ACK()
//Parameter:None
//Return value:None
//Description:This function will make error to current ACK frame
//before send it to receiver.
//Error can make any byte expept start/stop/control
//You cannot select sequence since there is one possible edit byte 
//which is parity itself.
void make_error_ACK(){
	while(1){
		printf("Enter new Byte for 1D-Parity value:");
		scanf("%x",&VParity);
		if(VParity>=0&&VParity<=0xFF)break;
		else{
			textcolor(12);
			cprintf("Invalid input!!!!\r\n");
			cprintf("Please enter HEX range 0x00 to 0xFF.\r\n\r\n");
			textcolor(10);cprintf("");
		}
	}
}

//void make_error_EOF()
//Parameter:None
//Return value:None
//Description:This function will make error to current EOF frame
//before send it to receiver.
//Error can make any byte expept start/stop/control
//You cannot select sequence since there is two possible edit byte
//Enter same value of current value of that field to leave unchanged.
void make_error_EOF(){
	while(1){
		printf("Enter new Byte for Flagdata\r\n(Enter the same value(0xFF) to leave unchanged):");
		scanf("%x",&flagdata);
		if(flagdata>=0&&flagdata<=0xFF)break;
		else{
			textcolor(12);
			cprintf("Invalid input!!!!\r\n");
			cprintf("Please enter HEX range 0x00 to 0xFF.\r\n\r\n");
			textcolor(10);cprintf("");
		}
	}

	while(1){
		printf("Enter new Byte for 1D-Parity value\r\n(Enter the same value to leave unchanged):");
		scanf("%x",&VParity);
		if(VParity>=0&&VParity<=0xFF)break;
		else{
			textcolor(12);
			cprintf("Invalid input!!!!\r\n");
			cprintf("Please enter HEX range 0x00 to 0xFF.\r\n\r\n");
			textcolor(10);cprintf("");
		}
	}
}

//void receive_name(char *com1_name,char *com2_name)
//Parameter:com1_name,com2_name as array of character
//Return value:None
//Description:This function will receive name of each other
//to identify who is?
void receive_name(char *com1_name,char *com2_name){
	int com1_nameset = 0; //flag status variable
	int com2_nameset = 0; //flag status variable
	int ch;
	//Iterate check all time until all flag is set to 1
	while(!com1_nameset||!com2_nameset){
		if(!com1_nameset){
			if((inportb(COM3LSR) & 0x01) == 0x01){
				while(1){
					while((inportb(COM3LSR) & 0x01) != 0x01);
					ch = inport(COM3TX);
					*(com1_name++) = ch;
					if(ch == '\0')break;
				}
				com1_nameset = 1;
			}
		}
		if(!com2_nameset){
			if((inportb(COM4LSR) & 0x01) == 0x01){
				while(1){
					while((inportb(COM4LSR) & 0x01) != 0x01);
					ch = inport(COM4TX);
					*(com2_name++) = ch; 
					if(ch == '\0')break;
				}
				com2_nameset = 1;
			}
		}
	}
}

//void send_COM1name() and void send_COM2name()
//Parameter:com1_name,com2_name as array of character
//Return value:None
//Description:This function will send both name to each other
void send_COM1name(char *com1_name){
	int ch;
	while(1){
		while((inportb(COM4LSR) & 0x40) != 0x40);
		ch = *(com1_name++);
		outport(COM4TX,ch);
		if(ch == '\0')break;
	}
}

void send_COM2name(char *com2_name){
	int ch;
	while(1){
		while((inportb(COM3LSR) & 0x40) != 0x40);
		ch = *(com2_name++);
		outport(COM3TX,ch);
		if(ch == '\0')break;
	}
}