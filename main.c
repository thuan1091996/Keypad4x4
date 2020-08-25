/*--------Project information--------
 * Project name: Control 4x4 keypad matrix
 * Author: Tran Minh Thuan
 * Company: Viet Mold Machine
 * Supported by: Nguyen Duc Quyen
 * Created: 15th August, 2018
//------------------------------------*/

/*--------Requirement Documents--------
 * Baud rate: 9600
 * Frame Data: 1 Start bit, 8 bits data, 1 stop bit
 * Keypad[num_Rows][num_Cols]=  |1 |2 |3 |A|
                                |4 |5 |6 |B|
                                |7 |8 |9 |C|
                                |* |0 |# |D|
//------------------------------------*/

/*--------Pre-processor Directives Section--------
//--------Libraries including------------*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
//--------------Defined section--------------//
//--------------UART defining----------------//
#define RX0_PIN     GPIO_PIN_0
#define TX0_PIN     GPIO_PIN_1
//--------------Keypad defining--------------//
#define num_Rows    4
#define num_Cols    4
#define Row_0       GPIO_PIN_0
#define Row_1       GPIO_PIN_1
#define Row_2       GPIO_PIN_2
#define Row_3       GPIO_PIN_3
#define Col_0       GPIO_PIN_4
#define Col_1       GPIO_PIN_5
#define Col_2       GPIO_PIN_6
#define Col_3       GPIO_PIN_7
#define Rows        Row_0|Row_1|Row_2|Row_3
#define Cols        Col_0|Col_1|Col_2|Col_3
#define Rows_port   GPIO_PORTB_BASE
#define Cols_port   GPIO_PORTB_BASE
const uint8_t Keypad[num_Rows][num_Cols]=  {  {1,2,3,'A'},
                                              {4,5,6,'B'},
                                              {7,8,9,'C'},
                                              {'*',0,'#','D'} };
uint8_t RowPins[num_Rows]={Row_0,Row_1,Row_2,Row_3}; //Depend on pin connections
uint8_t ColPins[num_Cols]={Col_0,Col_1,Col_2,Col_3}; //Depend on pin connections

/*--------Global Declarations section--------*/
//---------------Function declarations------------//
uint8_t Check_KPad();       //Check if any sw are pressed
uint8_t Get_Key();          //Return the key that is pressed
void UART0_Init();          //UART 0 initialization communicate with PC
void GPIO_Keypad_Init();    //GPIO initialization for keypad
//void Interrupt_Init();      //Interrupts initialization
//---------------Global variables------------//
uint8_t cur_Col,cur_Row,K_pressed=0;
uint32_t Input_value;
uint8_t Test;
uint8_t count=0;

//--------------------------------------------------*/
/*--------------Subroutine Section------------*/
//void Receive0_ISR(){
//    if(UARTCharsAvail(UART0_BASE)==1)
//    {
//
//    }
//}
//-------Mandatory routine---------//
int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); //50MHz
    GPIO_Keypad_Init();             //GPIO initialization for keypad
    UART0_Init();                   //UART 0 initialization communicate with PC
    while(1)
    {
        if(strcmp(Test,'A')==1) {
            count++;
            SysCtlDelay(SysCtlClockGet()/30);
        }
        if(Check_KPad()==1)
        {
            SysCtlDelay(SysCtlClockGet()/30);
            if(Check_KPad()==1)
            {
            Test=Get_Key();
            if (Test<10) UARTCharPut(UART0_BASE, Test+'0');
            else         UARTCharPut(UART0_BASE, Test);
            }
        }
    }
}

/*-----GPIO Initialization for keypad-----//
 * Rows         -Inputs (pulls-up) - GPIO portB pins 0-3
 * Columns      -Outputs           - GPIO portB pins 4-7
 */
void GPIO_Keypad_Init(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(Cols_port, Cols);
    GPIOPinTypeGPIOInput(Rows_port, Rows);
    GPIOPadConfigSet(Rows_port, Rows, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
}

/*-----Check Keypad if any key is pressed----//
 * Use input (rows) to check
 * If any row don't have value "1" --> at least one key pressed
 * retun 1
*/
uint8_t Check_KPad(){
    GPIOPinWrite(Cols_port, Cols, 0);
    Input_value=GPIOPinRead(GPIO_PORTB_BASE,Rows);
    if((Input_value&0x0F)!=0x0F)    return 1;
    else                            return 0;
}

/*Get the key are pressed in the  keyboard if detect at least one key is pressed//
 *because all inputs pulls-up so they have value "0" if button is pressed
 * -First set all outputs pin to 1, then clear col 0 (other have "1") to 0 to check
 *   if pressed button in col 0 by collects all inputs from rows ([1,2,3,4][1])
 *   if any inputs have "0" -> button pressed may have  1 4 7 * (in col 0)
 * -If no inputs have "0" so the button is another cols so check the next column and so on
*/
uint8_t Get_Key(){
    K_pressed=0;
    for(cur_Col=0;cur_Col<num_Cols;cur_Col++)
    {
        GPIOPinWrite(GPIO_PORTB_BASE,Cols,0xFF);              //Set all outputs to 1
        GPIOPinWrite(GPIO_PORTB_BASE,ColPins[cur_Col],0);     //Repeatedly clear one column
        for(cur_Row=0;cur_Row<num_Rows;cur_Row++)             //Check to see if the rows have "0"
        {                                                     //Specify the button in that column
            if(GPIOPinRead(GPIO_PORTB_BASE, RowPins[cur_Row])==0) //if true return [row][col]
            K_pressed=Keypad[cur_Row][cur_Col];                   //else check another cols
        }
    }
    return K_pressed;
}

/*------------UART0_Init-------------//
 * Baud rate: 115200
 * Frame: 1 Start - 8 bits data - 1 Stop and 0 Parity check
 * Parity: None
*/
void UART0_Init(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);                //PINMUX for RX0
    GPIOPinConfigure(GPIO_PA1_U0TX);                //PINMUX for TX0
    GPIOPinTypeUART(GPIO_PORTA_BASE, RX0_PIN |TX0_PIN);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

/* Interrupt Initialization-----//
 * UART0 receive interrupt enable
 * UART0 receive handler will be Reiceive0_ISR function
*/
//void Interrupt_Init(){
//    IntMasterEnable();
//    IntEnable(INT_UART0);
//    UARTIntEnable(UART0_BASE, UART_INT_RX);     //Receive interrupt
//    UARTIntRegister(UART0_BASE,Receive0_ISR);  //Specify UART0 Receive ISR
//}

