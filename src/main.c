#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define CLOCK24 24000000
#define CLOCK120 120000000

void main(void)
{
    int i = 0;
    bool led = 0;
    uint32_t clock = SysCtlClockFreqSet(SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480 | SYSCTL_XTAL_25MHZ, CLOCK24);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

    while (1)
    {
        for (i = 0; i < 2400000; i++);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, led);
        led = !led;
    }
}
