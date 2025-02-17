/*********************************************************************
*          Portions COPYRIGHT 2017 STMicroelectronics                *
*          Portions SEGGER Microcontroller GmbH & Co. KG             *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The  software has  been licensed  to STMicroelectronics International
N.V. a Dutch company with a Swiss branch and its headquarters in Plan-
les-Ouates, Geneva, 39 Chemin du Champ des Filles, Switzerland for the
purposes of creating libraries for ARM Cortex-M-based 32-bit microcon_
troller products commercialized by Licensee only, sublicensed and dis_
tributed under the terms and conditions of the End User License Agree_
ment supplied by STMicroelectronics International N.V.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : lcdconf.c
Purpose     : Display controller initialization
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @file    lcdconf.c
  * @author  MCD Application Team
  * @brief   This file implements the configuration for the GUI library
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "GUI.h"
#include "GUIDRV_FlexColor.h"
#include "main.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_dma2d.h"

/** @addtogroup LCD CONFIGURATION
* @{
*/

/** @defgroup LCD CONFIGURATION
* @brief This file contains the LCD Configuration
* @{
*/ 

/** @defgroup LCD CONFIGURATION_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 

/** @defgroup LCD CONFIGURATION_Private_Defines
* @{
*/

/* Physical display size */
#define XSIZE_PHYS  240
#define YSIZE_PHYS  240

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   GUICC_M565
  #error Color conversion not defined!
#endif
#ifndef   GUIDRV_FLEXCOLOR
  #error No display driver defined!
#endif

/*********************************************************************
*
*       Defines, sfrs
*
**********************************************************************
*/

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/********************************************************************
*
*       LcdWriteReg
*
* Function description:
*   Sets display register
*/
static void LcdWriteReg(U16 Data) 
{
  LCD_IO_WriteReg(Data);
}

/********************************************************************
*
*       LcdWriteData
*
* Function description:
*   Writes a value to a display register
*/
static void LcdWriteData(U16 Data) 
{
  LCD_IO_WriteData(Data);
}

/********************************************************************
*
*       LcdWriteDataMultiple
*
* Function description:
*   Writes multiple values to a display register.
*/
static void LcdWriteDataMultiple(U16 * pData, int NumItems) 
{
#if STM32L496G_DISCOVERY_REV_A_01   
  while (NumItems--) 
  {
    LCD_IO_WriteData(*pData++);
  }
#else
  /* Set input memory address */
  LL_DMA2D_FGND_SetMemAddr(DMA2D, (uint32_t)pData);
  /* Set number of lines */
  LL_DMA2D_SetNbrOfLines(DMA2D, NumItems); 

  /* Start DMA2D transfer */
  LL_DMA2D_Start(DMA2D);
  
/* Wait for transfer end */
  while (LL_DMA2D_IsTransferOngoing(DMA2D));
#endif 
}

/********************************************************************
*
*       LcdReadDataMultiple
*
* Function description:
*   Reads multiple values from a display register.
*/
static void LcdReadDataMultiple(U16 * pData, int NumItems) 
{
  __IO uint16_t tmp = 0;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(tmp);

  LCD_IO_WriteReg(ST7789H2_READ_RAM);
  /* Dummy read */
  tmp =  LCD_IO_ReadData();
  while (NumItems--) 
  {
    *pData++ = LCD_IO_ReadData();
  }
}

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/

/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval LCD state
  */
void LCD_LL_Init(void)
{     
  /* LCD Init */
  if (BSP_LCD_Init() == LCD_ERROR)
  {
    Error_Handler();
  }
  

#if !defined( STM32L496G_DISCOVERY_REV_A_01) 
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2D);
    
  /* Configure the DMA2D Color Mode */  
  LL_DMA2D_SetOutputColorMode(DMA2D, LL_DMA2D_OUTPUT_MODE_RGB565);
  
  /* Foreground Configuration:     */
  /* Set Alpha value to full opaque */
  LL_DMA2D_FGND_SetAlpha(DMA2D, 0xFF);
  /* Foreground layer format is RGB565 (16 bpp) */
  LL_DMA2D_FGND_SetColorMode(DMA2D, LL_DMA2D_INPUT_MODE_RGB565);

  /* Set output address (remains constant throughout the application) */
  LL_DMA2D_SetOutputMemAddr(DMA2D, (uint32_t)(&(LCD_ADDR->REG)));  
  /* Set number of pixels per line (remains constant throughout the application) */
  LL_DMA2D_SetNbrOfPixelsPerLines(DMA2D, 0x01);

#endif   /* !defined( STM32L496G_DISCOVERY_REV_A_01)   */
  
}

/*********************************************************************
*
*       LCD_X_Config
*
* Function description:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/
void LCD_X_Config(void) 
{
  GUI_DEVICE * pDevice;
  CONFIG_FLEXCOLOR Config = {0};
  GUI_PORT_API PortAPI = {0};

  /* Set display driver and color conversion */
  pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_FLEXCOLOR, GUICC_M565, 0, 0);

  /* Display driver configuration, required for Lin-driver */
  LCD_SetSizeEx (0, XSIZE_PHYS , YSIZE_PHYS);
  LCD_SetVSizeEx(0, VXSIZE_PHYS, VYSIZE_PHYS);
    
  Config.Orientation = GUI_SWAP_XY | GUI_MIRROR_X;
  GUIDRV_FlexColor_Config(pDevice, &Config);

  /* Set controller and operation mode */
  PortAPI.pfWrite16_A0  = LcdWriteReg;
  PortAPI.pfWrite16_A1  = LcdWriteData;
  PortAPI.pfWriteM16_A1 = LcdWriteDataMultiple;
  PortAPI.pfReadM16_A1  = LcdReadDataMultiple;
  GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66709, GUIDRV_FLEXCOLOR_M16C0B16);
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Function description:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  (void) LayerIndex;
  (void) pData;
  
  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
    
    LCD_LL_Init();
    
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}




