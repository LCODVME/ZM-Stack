/*****************************************************************
* Copyright (C) 2017 60Plus Technology Co.,Ltd.*
******************************************************************
* flash.h
*
* DESCRIPTION:
*     Flash read write
* AUTHOR:
*     Ziming
* CREATED DATE:
*     2018/12/13
* REVISION:
*     v0.1
*
* MODIFICATION HISTORY
* --------------------
* $Log:$
*
*****************************************************************/
#ifndef OS_FLASH_H
#define OS_FLASH_H
#ifdef __cplusplus
extern "C"
{
#endif
/*************************************************************************************************************************
 *                                                        MACROS                                                         *
 *************************************************************************************************************************/
#define EEPROM_ADDR_STEP            4
/*************************************************************************************************************************
 *                                                      CONSTANTS                                                        *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                       TYPEDEFS                                                        *
 *************************************************************************************************************************/

/*************************************************************************************************************************
 *                                                  EXTERNAL VARIABLES                                                   *
 *************************************************************************************************************************/
    
/*************************************************************************************************************************
 *                                                   PUBLIC FUNCTIONS                                                    *
 *************************************************************************************************************************/
     
/*****************************************************************
* DESCRIPTION: flashReadData
*     
* INPUTS:
*     
* OUTPUTS:
*     E_typeErr
* NOTE:
*     null
*****************************************************************/
E_typeErr flashReadData( uint16_t a_addr, uint8_t *a_data, uint8_t a_size );
/*****************************************************************
* DESCRIPTION: flashWriteData
*     
* INPUTS:
*     
* OUTPUTS:
*     E_typeErr
* NOTE:
*     null
*****************************************************************/
E_typeErr flashWriteData( uint16_t a_addr, uint8_t *a_data, uint8_t a_size );
/*****************************************************************
* DESCRIPTION: flashEraseData
*     
* INPUTS:
*     
* OUTPUTS:
*     E_typeErr
* NOTE:
*     擦除的地址必须为4的倍数，一次擦除4个字节
*****************************************************************/
E_typeErr flashEraseData( uint16_t a_addr, uint8_t a_size );
 
#ifdef __cplusplus
}
#endif
#endif /* OS_flash.h */
