#define _XTAL_FREQ 8000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#include<xc.h>
#include<htc.h>
#include<pic.h>
#include<stdio.h> 
#include<string.h>

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


char UART_Init(const long int baudrate){
    
    unsigned int x;
    x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);  
    
    if(x>255){
        x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); 
        BRGH = 1;                                     
    }
    if(x<256){
        SPBRG = x;                                    
        SYNC = 0;                                     
        SPEN = 1;                                     
        TRISC7 = 1;                                   
        TRISC6 = 1;                                   
        CREN = 1;                                     
        TXEN = 1;                                     
        return 1;                                    
    }
    return 0;                                       
}

void UART_Write(char data){
    while(!TRMT);
    TXREG = data;
}

char UART_TX_Empty(){
    return TRMT;
}

void UART_Write_Text(char *text){
    int i;
    for(i=0;text[i]!='\0';i++){
        UART_Write(text[i]);
    }
}


//LCD Functions Developed by Circuit Digest.
void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;

    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;

    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;

    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a); //Incoming Hex value
    EN  = 1;         
        __delay_ms(4);
        EN  = 0;         
}

Lcd_Clear()
{
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the curser to first position
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x08); //Select Row 1
  Lcd_Cmd(0x00); //Clear Row 1 Display
  Lcd_Cmd(0x0C); //Select Row 2
  Lcd_Cmd(0x00); //Clear Row 2 Display
  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)  //Send 8-bits through 4-bit mode
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;             // => RS = 1
   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP(); 
   EN = 0;
   Lcd_SetBit(Lower_Nibble); //Send Lower half
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 
}


int actTm = 20;
int watTm = 10;

void main()
{
    
    UART_Init(9600);
    
    unsigned int a;
    
    TRISD = 0x00;
    TRISB0 = 1;
    TRISB1 = 1;
    TRISB2 = 1;
    TRISB3 = 1;
    TRISB4 = 1;
    TRISB5 = 1;
    TRISB6 = 1;
    
    Lcd_Start();
    while(1)
    {
        Lcd_Clear();
        Lcd_Set_Cursor(1,4);
        Lcd_Print_String("RCTL System");
        Lcd_Set_Cursor(2,2);
        Lcd_Print_String("Menu for setup");
        __delay_ms(200);
        
        while(1){
            __delay_ms(50);
            if(RB0==1){
                break;
            }
        }
        while(1){
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            Lcd_Print_String("Active time - 1");
            Lcd_Set_Cursor(2,1);
            Lcd_Print_String("Wait Time   - 2");

            char B1 = 0;
            char B2 = 0;
            char clr = 0;
            char back = 0;

            while(1){
                __delay_ms(50);
                if(RB1==1){
                    B1 = 1;
                    break;
                }else if(RB2==1){
                    B2 = 1;
                    break;
                }else if(RB6==1){
                    clr = 1;
                    break;
                }
            }
            
            if(B1==1){
                
                int at = actTm;
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("Active time(Sec)");
                Lcd_Set_Cursor(2,13);
                char c1[3];
                
                while(1){
                
                    __delay_ms(50);
                    
                    c1[0] = (at/100)+'0';
                    c1[1] = (at/10)+'0';
                    c1[2] = (at%10)+'0';
                    Lcd_Set_Cursor(2,13);
                    Lcd_Print_String(c1);
                    
                    while(1){
                        __delay_ms(300);
                        if(RB3==1){
                            if(at<50){
                                at = at+1;
                            }
                            break;
                        }else if(RB4==1){
                            if(at>20){
                                at = at-1;
                            }
                            break;
                        }else if(RB5==1){
                            
                            actTm = at;
                            back = 1;
                            Lcd_Clear();
                            Lcd_Set_Cursor(1,5);
                            Lcd_Print_String("Updated !");
                            __delay_ms(1500);
                            
                            //do{
                                
                                UART_Write('a');
                                __delay_ms(100);
                                
                                UART_Write_Text(c1);
                                __delay_ms(100);
                            //}while(1);
                            
                            break;
                        }else if(RB6==1){
                            back = 1;
                            break;
                        }
                    }//selection end up down while
                    
                    if((back==1)){
                        break;
                    }
                    
                } //Up/Down/Back/Clear while end
                B1=0;
                
            }else if(B2==1){//B==1 end
                
                int wt = watTm;
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("Wait time(Sec)");
                Lcd_Set_Cursor(2,13);
                char c2[3];
                
                while(1){
                
                    __delay_ms(50);
                    
                    c2[0] = (wt/100)+'0';
                    c2[1] = (wt/10)+'0';
                    c2[2] = (wt%10)+'0';
                    Lcd_Set_Cursor(2,13);
                    Lcd_Print_String(c2);
                    
                    while(1){
                        __delay_ms(300);
                        if(RB3==1){
                            if(wt<15){
                                wt = wt+1;
                            }
                            break;
                        }else if(RB4==1){
                            if(wt>10){
                                wt = wt-1;
                            }
                            break;
                        }else if(RB5==1){
                            
                            watTm = wt;
                            back = 1;
                            Lcd_Clear();
                            Lcd_Set_Cursor(1,5);
                            Lcd_Print_String("Updated !");
                            __delay_ms(1500);
                            
                                UART_Write('w');
                                __delay_ms(100);
                                
                                UART_Write_Text(c2);
                                __delay_ms(100);
                            
                            break;
                        }else if(RB6==1){
                            back = 1;
                            break;
                        }
                    }//selection end up down while
                    
                    if((back==1)){
                        break;
                    }
                    
                } //Up/Down/Back/Clear while end
                B2=0;
                
            }//B2==1 end
            
            
            if(back==1){
                back=0;
                continue;
            }else if(clr==1){
                clr=0;
                break;
            }
            
        }//selection while end
        
    }

}
