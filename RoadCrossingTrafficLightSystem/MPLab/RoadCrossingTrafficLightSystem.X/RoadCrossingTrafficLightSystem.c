#include<xc.h>
#include<htc.h>
#include<pic.h>
#include<stdio.h> 

#define _XTAL_FREQ 8000000

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

char UART_Data_Ready(){
    return RCIF;
}

char UART_Read(){
    while(!RCIF);
    return RCREG;
}

void UART_Read_Text(char *Output, unsigned int length)
{
  int i;
  for(int i=0;i<length;i++)
  Output[i] = UART_Read();
}


int count = 0;
char en = 0;
int at = 20;
int wt = 10;
char c1[3];
char c2[3];

//int charToInt(char cd[3]){
 //   if(cd[0]=='0'){
        
 //   }
//}

unsigned char decTo7Seg(int a){
    switch(a){
        case 0: return 0x3F;
        case 1: return 0x06;
        case 2: return 0x5B;
        case 3: return 0x4F;
        case 4: return 0x66;
        case 5: return 0x6D;
        case 6: return 0x7D;
        case 7: return 0x07;
        case 8: return 0x7F;
        case 9: return 0x6F;
    }
}

void interrupt ISR(){
    
    if((INTF == 1)){
        
        if(en!=0){
            __delay_ms(5000);
        }else if(en==0){
            int dl = wt/2;
            __delay_ms(dl*1000);
            __delay_ms(dl*1000);
        }
        en = 0;
        RB2 = 1;
        for(int i = 5;i>-1;i--){
            
            
            PORTD = decTo7Seg(i);
            PORTC = 0x3F;
            __delay_ms(1000);
            
            if(i==3){
                RB7 = 0;
                RB6 = 1;
            }
            if(i==1){
                RB6 = 0;
                RB5 = 1;
            }
        }
        PORTC = 0x00;
        PORTD = 0x00;
        
        __delay_ms(1000);
        RB2 =0;
        int x = at;
        
        do{
            int x2 = x/100;
            int x1 = (x%100)/10;
            int x0 = (x%100)%10;
            
            if(x2>0){
                PORTC = 0x6F;
                PORTD = 0x6F;
            }else{
                PORTC = decTo7Seg(x1);
                PORTD = decTo7Seg(x0);
            }
            
            __delay_ms(1000);
            
            x--;
            
        }while(x>=0);
        RB2 = 1;
        PORTC = 0x00;
        PORTD = 0x00;
        
        for(int j = 0;j<5;j++){
            __delay_ms(1000);
            if(j==2){
                RB6 = 1;
            }
            if(j==4){
                RB5 = 0;
                RB6 = 0;
                RB7 = 1;
            }
        }
        
        en = 0;
        
        
        INTF=0;
        
    }
    
}

void main(){
    
    UART_Init(9600);
    
    TRISC = 0x00;
    TRISD = 0x00;
    
    PORTC = 0x00;
    PORTD = 0x00;
    
    TRISB0= 1;
    
    TRISB2= 0;
    RB2 =1;
    
    TRISB5= 0;
    TRISB6= 0;
    TRISB7= 0;
    
    RB5 = 0;
    RB6 = 0;
    RB7 = 0;
    
    int k =0;
    
    
    
    while(k<5){
        
        RB6 = 1;
        __delay_ms(500);
        RB6 = 0;
        __delay_ms(500);
        k++;
    }
    RB7 = 1;
    
    __delay_ms(1000);
    
    GIE=1;          //Enable Global Interrupt
    PEIE=1;         //Enable the Peripheral Interrupt
    
    INTE=1;
    
    char atc = 0;
    char wtc = 0;
    
    while(1){
        
        RB5 = 0;
        RB6 = 0;
        RB7 = 1;
        __delay_ms(100);
        if(UART_Data_Ready()){
            if(UART_Read()=='a'){
                atc = 1;
                __delay_ms(100);
            }else if(UART_Read()=='w'){
                wtc = 1;
                __delay_ms(100);
            }
        }
        
        __delay_ms(1000);
        
        if(UART_Data_Ready()){
            if(atc == 1){
                UART_Read_Text(c1,3);
                at = c1-'0';
                atc = 0;
                __delay_ms(100);
            }else if(wtc==1){
                UART_Read_Text(c2,3);
                wt = c2-'0';
                wtc = 0;
                __delay_ms(100);
            }
        }
        
        if(en==0){
            __delay_ms(5000);
            en = 1;
        }
  
    }
      
}
