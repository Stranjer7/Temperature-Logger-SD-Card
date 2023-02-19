char file_name[]="TEMPERTRTXT" ;
unsigned short character;            
unsigned long file_size,i,rec_size;   
unsigned char ch1,ch2,flag,ret_status,choice;
unsigned char temperature[10],txt[12];

void Newline()
{
Uart_Write(0x0D); // Send carriage-return
Uart_Write(0x0A); // Send line-feed
}
/*This function sends a space chaacter to the uart*/
void Space(){
     Soft_UART_Write(0x020);       // Enters/Sends a Space
}

/*This function sends a text to the serial port*/
void Text_To_Usart(unsigned char *m){
     unsigned char i;
     i=0;

     while(m[i] != 0){
                Soft_UART_Write(m[i]);
                i++;
     }
}

/*This function reads the temperature from analog input AN0*/
void Get_Temperature()
{
     unsigned long Vin,Vdec,Vfrac;
     unsigned char op[12];
     unsigned char i,j;
     
     Vin = Adc_Read(0);         // Read from analog channel 0
     Vin = 488*Vin ;            // Scale up the result to get a more accurate result. 5000/1024=4.88
     Vin = Vin / 10;            // Convert temperature to C
     Vin = Vin /100 ;           // Integer division returns the decimal part
     Vdec = Vin % 100;          // Modulus operation returns the fractional part
     LongToStr(Vdec,op);        // COnvert Vdec to string in "op"
     
/*Remove leading strings*/
     j = 0;
     for (i=0;i<=11 ;i++)
     {
      if(op[i] !='')
         {
            temperature [j] = op[i];
            j++;
         }
     }
     
     temperature[j]=".";    //adds a decimal point
     ch1 = Vfrac / 10 ;     // fractional part
     ch2 = Vfrac % 10;
     
     j++;
    temperature[j] = 48+ch1; // Add fractional part
    j++;
    temperature[j] = 48+ch2;
    j++;
    temperature[j] = 0x0D; // Add carriage-return
    j++;
    temperature[j] = 0x0A; // Add line-feed
    j++;
    temperature[j]='\0';
}

/*Start of main program*/
void main() {
     rec_size = 0;
     
/*Configure ADC*/
     TRISA = 0xFF;
     ADCON1 = 0x80;        // USE ANO, Vref = +5V  CHECK THIS TOO!!!!!!!!!!!
     
/*Configure Serial Port*/
     UART1_Init(2400);

/*Initialise SPI bus*/
     SPI1_Init_Advanced(MASTER_OSC_DIV16,DATA_SAMPLE_MIDDLE,CLK_IDLE_LOW,LOW_2_HIGH);  // Check this !!!!!!
     
/*Initialise SD bus*/
     while(Mmc_Init(&PORTC,2));

/*Initialise  FAT file system*/
     while(Mmc_Fat_Init(&PORTC,2));
     
/* Display the MENU and get user choice */

      Newline();
      Text_To_Usart("TEMPERATURE DATA LOGGER");
      Newline();
      Newline();
      Text_To_Usart("1. Send temperature data to the PC");
      Newline();
      Text_To_Usart("2. Save temperature data in a new file");
      Newline();
      Text_To_Usart("3. Append temperature data to an existing file");
      Newline();
      Newline();
      Text_To_Usart("Choice ? ");

/*Read character from keyboard*/
       flag = 0;
       do{
          if (UART_Data_Ready())
          {
           choice = UART1_Read;
           UART1_Write(choice);
           flag = 1;
          }
       }while (!flag);
       Newline();
       Newline();
// Now process user choice
//
      switch(choice)
      {
      case '1':
      ret_status = Mmc_Fat_Assign(&filename,1);
      if(!ret_status)
      {
      Text_To_Usart("File does not exist..No saved data...");
      Newline();
      Text_To_Usart("Restart the program and save data to the file...");
      Newline();
      for(;;);
      }
      else
      {
      //
      // Read the data and send to UART
      //
      Text_To_Usart("Sending saved data to the PC...");
      Newline();
      Mmc_Fat_Reset(&file_size);
      for(i=0; i<file_size; i++)
      {
      Mmc_Fat_Read(&character);
      Uart_Write(character);
      }
      Newline();
      text_To_Usart("End of data...");
      Newline();
      for(;;);
      }
      case '2':
      //
      // Start the A/D converter, get temperature readings every
      // 10 seconds, and then save in a NEW file
      //
      Text_To_Usart("Saving data in a NEW file...");
      Newline();
      Mmc_Fat_Assign(&filename,0x80); // Assign the file
      Mmc_Fat_Rewrite(); // Clear
      Mmc_Fat_Write("TEMPERATURE DATA - SAVED EVERY 10
      SECONDS\r\n",43);
      //
      // Read the temperature from A/D converter, format and save
      //
      for(;;)
      {
      Mmc_Fat_Append();
      Get_Temperature();
      Mmc_Fat_Write(temperature,9);
      rec_size++;
      LongToStr(rec_size,txt);
      Newline();
      Text_To_Usart("Saving record:");
      Text_To_Usart(txt);
      Delay_ms(10000);
      }
      break;
      case '3':
      //
      // Start the A/D converter, get temperature readings every
      // 10 seconds, and then APPEND to the existing file
      //
      Text_To_Usart("Appending data to the existing file...");
      Newline();
      ret_status = Mmc_Fat_Assign(&filename,1); // Assign the file
      if(!ret_status)
      {
      Text_To_Usart("File does not exist - can not append...");
      Newline();
      Text_To_Usart("Restart the program and choose option 2...");
      Newline();
      for(;;);
      }
      else
      {
      //
      // Read the temperature from A/D converter, format and save
      //
      for(;;)
      {
      Mmc_Fat_Append();
      Get_Temperature();
      Mmc_Fat_Write(temperature,9);
      rec_size++;
      LongToStr(rec_size,txt);
      Newline();
      Text_To_Usart("Appending new record:");
      Text_To_Usart(txt);
      Delay_ms(10000);
      }
      }
      default:
      Text_To_Usart("Wrong choice...Restart the program and try again...");
      Newline();
      for(;;);
      }
      }
}
