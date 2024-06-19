/*******************************************************************************
* Copyright © 2016 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/**
  This file contains the main() function.
*/

#include <stdlib.h>
#if defined(MK20DX128)
  #include "derivative.h"
#elif defined(GD32F425)
  #include "gd32f4xx.h"
#endif
#include "bits.h"
#include "stepRocker.h"
#include "SysTick.h"
#include "RS485.h"
#include "Commands.h"
#include "Globals.h"
#include "SPI.h"
#include "TMC4361.h"
#include "TMC262.h"
#include "IO.h"
#include "SysControl.h"
#include "Can.h"
#include "Usb.h"

#if defined(MK20DX128)
//These are the flash configuration bytes at address 0x400. Do not remove
//them.
const unsigned char Protection[] __attribute__ ((section(".cfmconfig")))=
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //Backdoor key
  0xff, 0xff, 0xff, 0xff,                          //Flash protection (FPPROT)
  0x7e,                                            //Flash security   (FSEC)
  0xf9,                                            //Flash option     (FOPT) (NMI off, EzPort off, Normal power)
  0xff,                                            //reserved
  0xff                                             //reserved
};
#endif

const UCHAR VersionString[]="1111V101";

#ifdef BOOTLOADER
__attribute__ ((section(".bldata"))) UINT BLMagic;
#endif


#if defined(MK20DX128)
void SetBASEPRI(uint32_t Level)
{
  __asm ("msr basepri, %[input]"::[input] "r" (Level):);
}


/***************************************************************//**
   \fn InitClocks(void)
   \brief Initialize the clock PLL

   For VLx7 CPUs the following frequencies are used:
     Core:    72MHz
     Bus:     36MHz
     FlexBus: 36MHz
     Flash:   18MHz

   This routine has been generated by ProcessorExpert and cleaned
   up manually.
********************************************************************/
void InitClocks(void)
{
  //Turn on clocking for all ports to enable pin routing
  SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
              | SIM_SCGC5_PORTB_MASK
              | SIM_SCGC5_PORTC_MASK
              | SIM_SCGC5_PORTD_MASK
              | SIM_SCGC5_PORTE_MASK );

  //PLL already selected by bootloader? => exit
  if((MCG_S & 0x0C)==0x0C) return;

  SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0x00) |
                SIM_CLKDIV1_OUTDIV2(0x01) |
                SIM_CLKDIV1_OUTDIV3(0x03) |
                SIM_CLKDIV1_OUTDIV4(0x03); /* Set the system prescalers to safe value */

  if ((PMC_REGSC & PMC_REGSC_ACKISO_MASK) != 0x0U)
  {
    /* PMC_REGSC: ACKISO=1 */
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK; /* Release IO pads after wakeup from VLLS mode. */
  }
  /* SIM_CLKDIV1: OUTDIV1=0,OUTDIV2=1,OUTDIV3=1,OUTDIV4=3,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0 */
  SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0x00) |
                SIM_CLKDIV1_OUTDIV2(0x01) |
                SIM_CLKDIV1_OUTDIV3(0x01) |
                SIM_CLKDIV1_OUTDIV4(0x03); /* Update system prescalers */
  /* SIM_SOPT2: PLLFLLSEL=1 */
  SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK|SIM_SOPT2_CLKOUTSEL(6); /* Select PLL as a clock source for various peripherals */
  /* SIM_SOPT1: OSC32KSEL=3 */
  SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL(0x03); /* LPO 1kHz oscillator drives 32 kHz clock for various peripherals */
  /* Switch to FBE Mode */
  /* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=0 */
  MCG_C2 = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);
  /* OSC_CR: ERCLKEN=1,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
  OSC_CR = OSC_CR_ERCLKEN_MASK;
  /* MCG_C7: OSCSEL=0 */
//  MCG_C7 &= (uint8_t)~(uint8_t)(MCG_C7_OSCSEL_MASK);      //***TEST_OK: Kann zum Absturz führen und ist eigentlich auch nicht nötig.
  /* MCG_C1: CLKS=2,FRDIV=4,IREFS=0,IRCLKEN=1,IREFSTEN=0 */
  MCG_C1 = (MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x04) | MCG_C1_IRCLKEN_MASK);
  /* MCG_C4: DMX32=0,DRST_DRS=0 */
  MCG_C4 &= (uint8_t)~(uint8_t)((MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x03)));
  /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=7 */
  MCG_C5 = MCG_C5_PRDIV0(0x07);
  /* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=12 */
  MCG_C6 = MCG_C6_VDIV0(0x0C);
  while((MCG_S & MCG_S_OSCINIT0_MASK) == 0x00U); /* Check that the oscillator is running */
  while((MCG_S & MCG_S_IREFST_MASK) != 0x00U);   /* Check that the source of the FLL reference clock is the external reference clock. */
  while((MCG_S & 0x0CU) != 0x08U);               /* Wait until external reference clock is selected as MCG output */
  /* Switch to PBE Mode */
  /* MCG_C6: LOLIE0=0,PLLS=1,CME0=0,VDIV0=0 */
  MCG_C6 = (MCG_C6_PLLS_MASK | MCG_C6_VDIV0(0x0C));
  while((MCG_S & 0x0CU) != 0x08U);              /* Wait until external reference clock is selected as MCG output */
  while((MCG_S & MCG_S_LOCK0_MASK) == 0x00U);   /* Wait until locked */
  /* Switch to PEE Mode */
  /* MCG_C1: CLKS=0,FRDIV=4,IREFS=0,IRCLKEN=1,IREFSTEN=0 */
  MCG_C1 = (MCG_C1_CLKS(0x00) | MCG_C1_FRDIV(0x04) | MCG_C1_IRCLKEN_MASK);
  while((MCG_S & 0x0CU) != 0x0CU);    /* Wait until output of the PLL is selected */
}


/***************************************************************//**
   \fn LowLevelInit(void)
   \brief Low level initialization of the MCU

   This function does some low level initialization of the MCU.
   Generated by ProcessorExpert and then cleaned up manually.
********************************************************************/
void LowLevelInit(void)
{
      /* Initialization of the SIM module */
  /* PORTA_PCR4: ISF=0,MUX=7 */
//  PORTA_PCR4 = (uint32_t)((PORTA_PCR4 & (uint32_t)~(uint32_t)(
//                PORT_PCR_ISF_MASK
//               )) | (uint32_t)(
//                PORT_PCR_MUX(0x07)
//               ));
        /* Initialization of the RCM module */    //***TEST_OK: Kann je nach Beschaltung des Reset-Pins zum Absturz führen
  /* RCM_RPFW: RSTFLTSEL=0 */
  RCM_RPFW &= (uint8_t)~(uint8_t)(RCM_RPFW_RSTFLTSEL(0x1F));
  /* RCM_RPFC: RSTFLTSS=0,RSTFLTSRW=0 */
  RCM_RPFC &= (uint8_t)~(uint8_t)(
               RCM_RPFC_RSTFLTSS_MASK |
               RCM_RPFC_RSTFLTSRW(0x03)
              );

      /* Initialization of the PMC module */
  /* PMC_LVDSC1: LVDACK=1,LVDIE=0,LVDRE=1,LVDV=0 */
  PMC_LVDSC1 = (uint8_t)((PMC_LVDSC1 & (uint8_t)~(uint8_t)(
                PMC_LVDSC1_LVDIE_MASK |
                PMC_LVDSC1_LVDV(0x03)
               )) | (uint8_t)(
                PMC_LVDSC1_LVDACK_MASK |
                PMC_LVDSC1_LVDRE_MASK
               ));
  /* PMC_LVDSC2: LVWACK=1,LVWIE=0,LVWV=0 */
  PMC_LVDSC2 = (uint8_t)((PMC_LVDSC2 & (uint8_t)~(uint8_t)(
                PMC_LVDSC2_LVWIE_MASK |
                PMC_LVDSC2_LVWV(0x03)
               )) | (uint8_t)(
                PMC_LVDSC2_LVWACK_MASK
               ));
  /* PMC_REGSC: BGEN=0,ACKISO=0,BGBE=0 */
  PMC_REGSC &= (uint8_t)~(uint8_t)(
                PMC_REGSC_BGEN_MASK |
                PMC_REGSC_ACKISO_MASK |
                PMC_REGSC_BGBE_MASK
               );
  /* SMC_PMPROT: ??=0,??=0,AVLP=0,??=0,ALLS=0,??=0,AVLLS=0,??=0 */
  SMC_PMPROT = 0x00U;                  /* Setup Power mode protection register */

  /* Common initialization of the CPU registers */
  /* NVICIP73: PRI73=0 (USB Interrupt)*/
  NVICIP73 = NVIC_IP_PRI73(0x00);

  //Interrupt priority base setting
  SetBASEPRI(0);
}
#endif


/*********************************//**
  \fn main(void)
  \brief Main program

  This is the main function. It does
  all necessary initializations and
  then enters the main loop.
*************************************/
int main(void)
{
  UINT BlinkDelay;

#if defined(MK20DX128)
  InitClocks();
  LowLevelInit();
  EnableInterrupts();
#elif defined(GD32F425)
  nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
  __enable_irq();
#endif

#if !defined(BOOTLOADER)
  ModuleConfig.SerialBitrate=0;
  ModuleConfig.SerialModuleAddress=1;
  ModuleConfig.SerialHostAddress=2;
#endif

  InitIO();
  InitSysTimer();
  InitRS485(0);
  InitSPI();
  InitTMC43xx();
  InitMotorDrivers();
  InitCan(ModuleConfig.CANBitrate, ModuleConfig.CANReceiveID, 0);
  InitUSB();

  InitTMCL();
  BlinkDelay=GetSysTimer();

  SetUARTTransmitDelay(5);

  for(;;)
  {
    ProcessCommand();
    SystemControl();

    if(abs(GetSysTimer()-BlinkDelay)>1000)
    {
      LED1_TOGGLE();
      BlinkDelay=GetSysTimer();
    }
  }
}
