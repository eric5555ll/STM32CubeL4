/**
  ******************************************************************************
  * @file    k_bsp.h
  * @author  MCD Application Team
  * @brief   Header for k_bsp.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __K_BSP_H
#define __K_BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void k_BspInit(void);
void k_TouchUpdate(void);
void k_ResourcesCopy(WM_HWIN hItem, FIL * pResFile, uint32_t Address);
void k_JumpToSubDemo(__IO uint32_t BackUpReg, uint32_t SubDemoAddress);
void k_FlashProgram(WM_HWIN hItem, FIL * pResFile, uint32_t Address);

#ifdef __cplusplus
}
#endif

#endif /*__K_BSP_H */

