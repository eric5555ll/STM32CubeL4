/**
  ******************************************************************************
  * @file    usbd_storage.c
  * @author  MCD Application Team   
  * @brief   Memory management layer
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage.h"
#include "k_storage.h"
#include "main.h"

/** @addtogroup USB_DEVICE_MODULE
  * @{
  */

/** @defgroup USB_STORAGE
  * @brief usb storage routines 
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR                  1  
#define STORAGE_BLK_NBR                  0x10000  
#define STORAGE_BLK_SIZ                  0x200


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* USB Mass storage Standard Inquiry Data */
const int8_t STORAGE_Inquirydata[] = { /* 36 */
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer: 8 bytes  */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product     : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0','1',                      /* Version     : 4 Bytes  */
}; 

/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);


USBD_StorageTypeDef USBD_DISK_fops = {
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata, 
};

static uint32_t prev_state = 1;
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the storage unit (medium)       
  * @param  lun: Logical unit number
  * @retval Status (0 : Ok / -1 : Error)
  */
int8_t STORAGE_Init(uint8_t lun)
{
  if(k_StorageGetStatus (MSD_DISK_UNIT) == 1)
  {     
    prev_state = 1;
  }
  else
  {
    prev_state = 0;
  }
  return 0;
}

/**
  * @brief  Returns the medium capacity.      
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  HAL_SD_CardInfoTypeDef info;
  int8_t ret = -1;  
  
  if(k_StorageGetStatus (MSD_DISK_UNIT) == 1)
  {    

    BSP_SD_GetCardInfo(&info);
   
    *block_num = info.BlockNbr;    
    *block_size = STORAGE_BLK_SIZ;
    ret = 0;
  }
  return ret;
}

/**
  * @brief  Checks whether the medium is ready.  
  * @param  lun: Logical unit number
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
  int8_t ret = -1;
  
  if(k_StorageGetStatus (MSD_DISK_UNIT) == 1)
  {
    if(prev_state == 0)
    {     
        BSP_SD_Init();
    }
    prev_state = 1;
    ret = 0;
  }
  else if(prev_state == 1)
  {
    prev_state = 0; 
  }
  return ret;
}

/**
  * @brief  Checks whether the medium is write protected.
  * @param  lun: Logical unit number
  * @retval Status (0: write enabled / -1: otherwise)
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  return 0;
}

/**
  * @brief  Reads data from the medium.
  * @param  lun: Logical unit number
  * @param  buf: pointer to data buffer  
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0: Ok / -1: Error)
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = -1;  
  uint32_t timeout = 100000;
   
    if(k_StorageGetStatus (MSD_DISK_UNIT) == 1)
    {  
      BSP_SD_ReadBlocks((uint32_t *)buf, blk_addr, blk_len, SDMMC_DATATIMEOUT);
      while(BSP_SD_GetCardState() != SD_TRANSFER_OK)
      {  
        if (timeout-- == 0)
        {
          return (-1);
        }
      }
      ret = 0;
  }
  return ret;
}

/**
  * @brief  Writes data into the medium.
  * @param  lun: Logical unit number
  * @param  buf: pointer to data buffer    
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0 : Ok / -1 : Error)
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = -1;  
  uint32_t timeout = 100000;
  
    if(k_StorageGetStatus (MSD_DISK_UNIT) == 1)
    { 
      BSP_SD_WriteBlocks((uint32_t *)buf, blk_addr, blk_len, SDMMC_DATATIMEOUT);
      while(BSP_SD_GetCardState() != SD_TRANSFER_OK)
      {  
        if (timeout-- == 0)
        {       
          return (-1);
        }
      }    
      ret = 0;
    }
  return ret;
}

/**
  * @brief  Returns the Max Supported LUNs.   
  * @param  None
  * @retval Lun(s) number
  */
int8_t STORAGE_GetMaxLun(void)
{
  return(STORAGE_LUN_NBR - 1);
}

/**
  * @}
  */

/**
  * @}
  */

