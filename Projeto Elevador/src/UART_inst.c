//FELIPE MOMMA PROJETO ELEVADOR

#include <stdbool.h>
#include <stdio.h>
#include "cmsis_os2.h" // CMSIS-RTOS

// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h"


osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id, thread5_id, thread6_id, thread7_id;
osMutexId_t uart_id;
int andarElevador[3] = {0,0,0};
int direcaoElevador[3] = {0,0,0}; // 0- parado 1-sobe 2-desce 3- parado mas deve continuar a subir 4- parado mas deve continuar descer
int flagElevador[3][2] = {{0,0},{0,0},{0,0}}; //flag[elevador][sobe/desce] sobe/desce 0-nao 1-sim
int paradasElevador[3][2][15] = {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
                                 {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
                                 {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}};
//[elevador][subida,descida][andares]

char readWriteMode = 'X';
char cr = 0x0D;
//----------
// UART definitions
extern void UARTStdioIntHandler(void);

void UARTInit(void){
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 115200, SystemCoreClock);

  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
  if(readWriteMode == 'X') {
    readWriteMode = 'R';
  } else if(readWriteMode == 'W'){
    readWriteMode = 'X';
  }
  
  
} // UART0_Handler
//----------


// osRtxIdleThread
__NO_RETURN void osRtxIdleThread(void *argument){
  (void)argument;
  
  while(1){
    asm("wfi");
  } // while
} // osRtxIdleThread

// osRtxTimerThread

//funcoes comuns
void iniciaElevadores() {
  
  osMutexAcquire(uart_id, osWaitForever);
  readWriteMode = 'W';
  UARTprintf("er%c",cr);
  readWriteMode = 'W';
  UARTprintf("cr%c",cr);
  readWriteMode = 'W';
  UARTprintf("dr%c",cr);
  osMutexRelease(uart_id);
}

void subirElevador(char elevador) {
  osMutexAcquire(uart_id, osWaitForever);
  readWriteMode = 'W';
  UARTprintf("%cf%c",elevador,cr); //fechar as portas
  readWriteMode = 'W';
  UARTprintf("%cs%c",elevador,cr); //manda subir
  osMutexRelease(uart_id);
}

void descerElevador(char elevador) {
  osMutexAcquire(uart_id, osWaitForever);
  readWriteMode = 'W';
  UARTprintf("%cf%c",elevador,cr); //fechar as portas
  readWriteMode = 'W';
  UARTprintf("%cd%c",elevador,cr); //manda descer
  osMutexRelease(uart_id);
}

void paraElevador(char elevador) {
  int elevadorInt = 0;

  osMutexAcquire(uart_id, osWaitForever);
  readWriteMode = 'W';
  UARTprintf("%cp%c",elevador,cr); //parar elevador
  readWriteMode = 'W';
  UARTprintf("%ca%c",elevador,cr); //abre as portas
  osMutexRelease(uart_id);
  
  if(elevador == 'e'){
    elevadorInt = 0;
  } else if(elevador == 'c') {
    elevadorInt = 1;
  } else {
    elevadorInt = 2;
  }

  if(direcaoElevador[elevadorInt] == 1 && flagElevador[elevador][0] == 1) {
    direcaoElevador[elevadorInt] = 3;
  } else if(direcaoElevador[elevadorInt] == 2 && flagElevador[elevador][1] == 1) {
    direcaoElevador[elevadorInt] = 4;
  }
}

//threads
__NO_RETURN void thread1(void *arg){
  
  while(1){
    if(readWriteMode == 'R') {
      char response;
      int elevador = 0;
      
      osMutexAcquire(uart_id, osWaitForever);
      UARTgets(&response, NULL);
      osMutexRelease(uart_id);
      readWriteMode = 'X';
      
      //processa leitura
      //elevador
      if((&response)[1] == 'e'){
        elevador = 0;
      } else if((&response)[1] == 'c') {
        elevador = 1;
      } else {
        elevador = 2;
      }
      //comandos  
      if((&response)[1] == 'I') {
        //botao interno
      } else if((&response)[1] == 'E') {
        //botao andar
        int andar = (10*((&response)[2]) + (&response)[3]) - 1;
        int direcao = 0;
        if((&response)[4] == 'd'){
          direcao = 1;
        }
        
        paradasElevador[elevador][direcao][andar] = 1;
        if (direcao == 0) {
          flagElevador[elevador][direcao] = 1;
        }
        
      } else if((&response)[1] == 'A') {
        //porta aberta
      } else if((&response)[1] == 'F') {
        //porta fechada
      } else {
        //seta andar elevador
        andarElevador[elevador] = (&response)[2];
      }
    }
  } // while
} // thread1 leitura de evento

__NO_RETURN void thread2(void *arg){
  char elevador = (char)arg;
  int elevadorInt = 0;
  int direcao = 0;
  
  if(elevador == 'e'){
    elevadorInt = 0;
  } else if(elevador == 'c') {
    elevadorInt = 1;
  } else {
    elevadorInt = 2;
  }

  while(1){
    // se elevador chegou andar verifica o andar pra parar
    // se nao tiver mais paradas na mesma direcao muda direcao
    direcao = direcaoElevador[elevadorInt];
    if(paradasElevador[elevadorInt][direcao][andarElevador[elevadorInt]] == 1){
      paraElevador(elevador);
    }
    //se o elevador esta parado ou deve continuar a subir E a esta sinalizada pra subir
    //se o elevador esta parado ou deve continuar a descer E a esta sinalizada pra descer
    if((direcaoElevador[elevadorInt] == 0 || direcaoElevador[elevadorInt] == 3) && flagElevador[elevador][0] == 1) {
      direcaoElevador[elevadorInt] = 1;
      subirElevador(elevador);
    } else if ((direcaoElevador[elevadorInt] == 0 || direcaoElevador[elevadorInt] == 4) && flagElevador[elevador][1] == 1){
      direcaoElevador[elevadorInt] = 2;
      descerElevador(elevador);
    }
    
  } // while
} // thread2 controle elevador

__NO_RETURN void thread3(void *arg){
  char elevador = (char)arg;
  int elevadorInt = 0;
  
  if(elevador == 'e'){
    elevadorInt = 0;
  } else if(elevador == 'c') {
    elevadorInt = 1;
  } else {
    elevadorInt = 2;
  }

  while(1){
    for(int i =0; i<15; i++){
      if(paradasElevador[elevadorInt][0][i] == 1){
        flagElevador[elevador][0] = 1;
      } else {
        flagElevador[elevador][0] = 0;
      }
      if(paradasElevador[elevadorInt][1][i] == 1){
        flagElevador[elevador][1] = 1;
      } else {
        flagElevador[elevador][1] = 0;
      }
    }
    
  } // while
} // thread3 controle elevador andares e flags

void main(void){
  UARTInit();
  iniciaElevadores();
  
  if(osKernelGetState() == osKernelInactive)
     osKernelInitialize();
  
  readWriteMode = 'X';
  thread1_id = osThreadNew(thread1, NULL, NULL);
  thread2_id = osThreadNew(thread2, (void *)'e', NULL);
  thread3_id = osThreadNew(thread2, (void *)'c', NULL);
  thread4_id = osThreadNew(thread2, (void *)'d', NULL);
  thread5_id = osThreadNew(thread3, (void *)'e', NULL);
  thread6_id = osThreadNew(thread3, (void *)'c', NULL);
  thread7_id = osThreadNew(thread3, (void *)'d', NULL);
  uart_id = osMutexNew(NULL);
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
