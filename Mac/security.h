/*****************************************************************
* Copyright (C) 2017 60Plus Technology Co.,Ltd.*
******************************************************************
* security.h
*
* DESCRIPTION:
*     Data safe
* AUTHOR:
*     Ziming
* CREATED DATE:
*     2018/12/27
* REVISION:
*     v0.1
*
* MODIFICATION HISTORY
* --------------------
* $Log:$
*
*****************************************************************/
#ifndef SECURITY_H
#define SECURITY_H
#ifdef __cplusplus
extern "C"
{
#endif
/*************************************************************************************************************************
 *                                                        MACROS                                                         *
 *************************************************************************************************************************/

/*************************************************************************************************************************
 *                                                      CONSTANTS                                                        *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                       TYPEDEFS                                                        *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                  EXTERNAL VARIABLES                                                   *
 *************************************************************************************************************************/
extern const uint8_t securityKey[SECURITY_KEY_LEN];
/*************************************************************************************************************************
 *                                                   PUBLIC FUNCTIONS                                                    *
 *************************************************************************************************************************/

/*****************************************************************
* DESCRIPTION: dataEncrypt
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
uint8_t dataEncrypt( uint8_t *a_data, uint8_t a_size );
/*****************************************************************
* DESCRIPTION: dataDecode
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
E_typeErr dataDecode( uint8_t a_keyNum, uint8_t *a_data, uint8_t a_size );

#ifdef __cplusplus
}
#endif
#endif /* security.h */