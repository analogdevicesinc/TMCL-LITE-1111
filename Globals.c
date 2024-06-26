/*******************************************************************************
* Copyright © 2016 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/**
  This file contains all globally used variables.
*/

#include "stepRocker.h"


//! Global parameters (here only used for the RS232 interface)
TModuleConfig ModuleConfig=
{
  0,      //!< RS485 bitrate (0=9600)
  1,      //!< RS485 address
  2,      //!< RS485 reply address
  1,      //!< CAN receive ID
  2,      //!< CAN send ID
  8,      //!< CAN bit rate (8=1000kBit/s)
};

//! Motor configuration data
TMotorConfig MotorConfig[N_O_MOTORS]=
{{
  128,   //!< IRun
  32,    //!< IStandby
  8,     //!< MicrostepResolution
  0,     //!< SwitchMode
  0,     //!< FreewheelingDelay
  0,     //!< StallVMin
  0,     //!< SettingDelay
  128,   //!< BoostCurrent
  0,     //!< RampType
  1,     //!< UnitMode
  0,     //!< StepDirMode
  4096,  //!< EncoderResolution
  0,     //!< MaxPositionDeviation
  0,     //!< MaxVelocityDeviation
  51200 << 2, //!< AMax
  51200 << 2, //!< DMax
  25600, //!< AStart
  25600, //!< DFinal
  10000, //!< DStop
  200,   //!< MotorResolution
 }
};

//! Closed loop configuration
TClosedLoopConfig ClosedLoopConfig[N_O_MOTORS]=
{{
  0,           //ClosedLoopMode
  300000,      //GammaVMin
  600000,      //GammaVAdd
  255,         //Gamma
  255,         //Beta
  0,           //Offset
  100,         //CurrentScalerMinimum
  240,         //CurrentScalerMaximum
  0,           //CurrentScalerStartUp
  1000,        //UpscaleDelay
  10000,       //DownscaleDelay
  10000,       //CorrectionVelocityP
  20,          //CorrectionVelocityI
  10,          //CorrectionVelocityIClip
  0,           //CorrectionVelocityDClk
  1073741823,  //CorrectionVelocityDClip
  65536,       //PositionCorrectionP
  100,         //PositionCorrectionTolerance
  100,         //PositionWindow
  0,           //EncVMeanWait
  7,           //EncVMeanFilter
  500,         //EncVMeanInt
  0,           //EncoderCorrectionYOffset
 }
};

UCHAR SmartEnergy[N_O_MOTORS];      //!< actual smartEnergy values
UCHAR StallFlag[N_O_MOTORS];        //!< actual stallGuard flag states
UINT StallLevel[N_O_MOTORS];        //!< actual stallGuard load values
UCHAR DriverFlags[N_O_MOTORS];      //!< actual driver error flags
UCHAR MotorDisable[N_O_MOTORS];     //!< actual motor disable flags
UCHAR StandbyFlag[N_O_MOTORS];      //!< standby current flags
UCHAR FreewheelingActive[N_O_MOTORS];        //!< freewheeling flags
UCHAR SlowRunCurrent[N_O_MOTORS];            //!< slow run current
TCoolStepConfig CoolStepConfig[N_O_MOTORS];  //!< coolStep configuration
UCHAR VMaxModified[N_O_MOTORS];              //!< TRUE when motor max. speed has been changed by ROL/ROR command
int ClosedLoopPositionOffset[N_O_MOTORS];
int EncoderOffset[N_O_MOTORS];
UINT GearRatio[N_O_MOTORS];
UCHAR DeviationFlag[N_O_MOTORS];

UCHAR ExitTMCLFlag;   //!< This will be set to TRUE for exiting TMCL and branching to the boot loader
