/*****************************************************************
* Copyright (C) 2017 60Plus Technology Co.,Ltd.*
******************************************************************
* lora.c
*
* DESCRIPTION:
*     Lora task
* AUTHOR:
*     Ziming
* CREATED DATE:
*     2018/12/20
* REVISION:
*     v0.1
*
* MODIFICATION HISTORY
* --------------------
* $Log:$
*
*****************************************************************/
/*************************************************************************************************************************
 *                                                       INCLUDES                                                        *
 *************************************************************************************************************************/
#include "loraConfig.h"
#include <stdlib.h>
#include <string.h>
#include "iwdg.h"
#include "lora_driver.h"
#include "radio.h"
#include "transmit.h"
#include "attribute.h"
#include "OS_timers.h"
#include "Network.h"
#include "gpio.h"
#include "lora.h"
/*************************************************************************************************************************
 *                                                        MACROS                                                         *
 *************************************************************************************************************************/
/* Panid build time(ms) */
#define PAN_ID_BUILD_TIME                        5000
/* Join request packet send interval time(ms) */
#define JOIN_REQUEST_INTERVAL_TIME               500
/* Beacon packet send interval time(ms) */
#define BEACON_INTERCAL_TIME                     500
/* wait ack max time(ms) */
#define TRANSMIT_ACK_WAIT_TIME                   200
/*************************************************************************************************************************
 *                                                      CONSTANTS                                                        *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                       TYPEDEFS                                                        *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                   GLOBAL VARIABLES                                                    *
 *************************************************************************************************************************/
static bool             transFlag = false;
static TaskHandle_t     notifyTask = NULL;
/*************************************************************************************************************************
 *                                                  EXTERNAL VARIABLES                                                   *
 *************************************************************************************************************************/

/*
 * Lora porcess task handle.
 */
TaskHandle_t                loraTaskHandle;

/*************************************************************************************************************************
 *                                                    LOCAL VARIABLES                                                    *
 *************************************************************************************************************************/
 
/*************************************************************************************************************************
 *                                                 FUNCTION DECLARATIONS                                                 *
 *************************************************************************************************************************/
static void     loraReceiveDone( uint8_t *a_data, uint16_t a_size );
static void     loraReceiveError( void );
static void     loraReceiveTimeout( void );
static void     loraSendDone( void );
static void     loraSendTimeout( void );
static void     loraCadDone( uint8_t a_detected );
static void     loraCadTimeout( void );
/*
 * Full callback function.
 */
static t_radioCallBack loraCallBack =
{
    .RxDone     = loraReceiveDone,
    .RxError    = loraReceiveError,
    .RxTimeout  = loraReceiveTimeout,
    .TxDone     = loraSendDone,
    .TxTimeout  = loraSendTimeout,
    .CadDone    = loraCadDone,
    .CadTimeout = loraCadTimeout,
};
static void     networkBuildSuccess( void );
static void     getChannelStarus( void );
static void     transmitNoAck( void );
/*************************************************************************************************************************
 *                                                   PUBLIC FUNCTIONS                                                    *
 *************************************************************************************************************************/

/*************************************************************************************************************************
 *                                                    LOCAL FUNCTIONS                                                    *
 *************************************************************************************************************************/

/*****************************************************************
* DESCRIPTION: loraInit
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     Initialize lora
*****************************************************************/
void loraInit( void )
{
    /* Open lora mcu */
    SET_GPIO_PIN_HIGH( Lora_Reset_GPIO_Port, Lora_Reset_Pin );
    /* Initialize lora config */
    loraDriverInit();
    /* Register call back */
    loraRegisterCallback( &loraCallBack );
    
}

/*****************************************************************
* DESCRIPTION: loraProcess
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void loraProcess( void *parm )
{
    uint32_t eventId;
        
    while(1)
    {
        eventId = 0;
        /* wait task notify */
        xTaskNotifyWait( (uint32_t)0, ULONG_MAX, &eventId, portMAX_DELAY );
        /* Transmit data */
        if( (eventId & LORA_NOTIFY_TRANSMIT_START) == LORA_NOTIFY_TRANSMIT_START )
        {
            if( !transFlag )
            {
                transFlag = true;
                macPIB.BE = MAC_MIN_BE;
                macPIB.NB = MAC_MIN_NB;
                macPIB.CW = MAC_VALUE_CW;
            }
            startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );

            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_START;
        }
        /* Transmit done */
        else if( (eventId & LORA_NOTIFY_TRANSMIT_DONE) == LORA_NOTIFY_TRANSMIT_DONE )
        {
            /* Stop timeout timer */
            stopTimer( LORA_TIMEOUT_EVENT, SINGLE_TIMER );
            loraDoneHandler();
            
            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_DONE;
        }
        /* Transmit timeout */
        else if( (eventId & LORA_NOTIFY_TRANSMIT_TIMEOUT) == LORA_NOTIFY_TRANSMIT_TIMEOUT )
        {
            loraTimeoutHandler();
            
            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_TIMEOUT;
        }
        /* transmit command message */
        else if( (eventId & LORA_NOTIFY_TRANSMIT_COMMAND) == LORA_NOTIFY_TRANSMIT_COMMAND )
        {
            startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
            
            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_COMMAND;
        }
        /* Join request */
        else if( (eventId & LORA_NOTIFY_TRANSMIT_JOIN_REQUEST) == LORA_NOTIFY_TRANSMIT_JOIN_REQUEST )
        { 
            /* Set the network status to join request */
            setNetworkStatus( NETWORK_JOIN_REQUEST );
            startReloadTimer( NETWORK_JOIN_NWK_EVENT, JOIN_REQUEST_INTERVAL_TIME, transmitJoinRequest );
            /* Freed dog,It takes a lot of time here */
            systemFeedDog();
            
            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_JOIN_REQUEST;
        }
        /* Transmit beacon */
        else if( (eventId & LORA_NOTIFY_TRANSMIT_BEACON) == LORA_NOTIFY_TRANSMIT_BEACON )
        {
            startReloadTimer( NETWORK_BEACON_EVENT, BEACON_INTERCAL_TIME, transmitBeacon );
            
            eventId ^= (uint32_t)LORA_NOTIFY_TRANSMIT_BEACON;
        }
#ifdef SELF_ORGANIZING_NETWORK
        /* Set pan network */
        else if( (eventId & LORA_NOTIFY_SET_PANID) == LORA_NOTIFY_SET_PANID )
        {
            setNetworkStatus( NETWORK_BUILD );
            srand(HAL_GetTick());
            nwkAttribute.m_panId = (uint16_t)rand();
            startSingleTimer( NETWORK_BUILD_EVENT, PAN_ID_BUILD_TIME, networkBuildSuccess );
            eventId ^= (uint32_t)LORA_NOTIFY_SET_PANID;
        }
#endif
        /* Returns the untreated event */
        if( eventId )
        {
            xTaskNotify( loraTaskHandle, eventId, eSetBits );
        }
        taskYIELD();
    }
}

/*****************************************************************
* DESCRIPTION: loraAllowJoinNetwork
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void loraAllowJoinNetwork( uint32_t a_time )
{
    if( getNetworkStatus() != NETWORK_COOR )
    {
        return;
    }
    /* Notify task send beacon packet start */
    xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_BEACON, eSetBits );
    /* Duration time */
    startSingleTimer( LORA_ALLOW_JOIN_TIME_EVENT, a_time, NULL );
}

/*****************************************************************
* DESCRIPTION: loraCloseBeacon
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void loraCloseBeacon( void )
{
    clearTimer( NETWORK_BEACON_EVENT, ALL_TYPE_TIMER );
    clearTimer( LORA_ALLOW_JOIN_TIME_EVENT, ALL_TYPE_TIMER );
}

/*****************************************************************
* DESCRIPTION: detectionChannel
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void detectionChannel( TaskHandle_t a_notifyTask )
{
    notifyTask = a_notifyTask;
    getChannelStarus();
}

/*****************************************************************
* DESCRIPTION: setPanId
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void networkBuildSuccess( void )
{
    setNetworkStatus( NETWORK_COOR );
    nwkAttribute.m_nwkStatus = true;
    loraAllowJoinNetwork( 120000 );
}
/*****************************************************************
* DESCRIPTION: getChannelStarus
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void getChannelStarus( void )
{
    if( getLoraStatus() == RFLR_STATE_RX_RUNNING )
    {
        loraEnterStandby();
    }
    if( loraEnterCAD() != E_SUCCESS )
    {
        startSingleTimer( TRANSMIT_NB_TIME_EVENT, 20, getChannelStarus );
    }
}

/*****************************************************************
* DESCRIPTION: transmitNoAck
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void transmitNoAck( void )
{
    if(++macPIB.retransmit < MAC_RETRANSMIT_NUM)
    {
        transFlag = false;
        xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_START, eSetBits );
    }
    else
    {
        /* transmit faild */
        //macPIB.retransmit = 0;
        transmitFreeHeadData();
    }
}
/*****************************************************************
* DESCRIPTION: loraReceiveDone
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraReceiveDone( uint8_t *a_data, uint16_t a_size )
{
#ifdef SELF_ORGANIZING_NETWORK
    if( getNetworkStatus() == NETWORK_BUILD && 
        nwkAttribute.m_panId == ((t_transmitPacket *)a_data)->m_panId )
    {
        xTaskNotify( loraTaskHandle, LORA_NOTIFY_SET_PANID, eSetBits );
    }
#endif
    //TOGGLE_GPIO_PIN(LED_GPIO_Port, LED_Pin);
    static uint8_t notifyBuf[20];
    uint32_t notifyPoint = (uint32_t)notifyBuf;
    switch( transmitRx( (t_transmitPacket *)a_data ) )
    {
    case DATA_ORDER:
        if( getNetworkStatus() == NETWORK_COOR )
        {
           // memset( notifyBuf, 0, 20 );
            memcpy( notifyBuf, ((t_transmitPacket *)a_data)->m_data, ((t_transmitPacket *)a_data)->m_size );
            notifyBuf[((t_transmitPacket *)a_data)->m_size + 1] = (uint8_t)(((t_transmitPacket *)a_data)->m_srcAddr >> 8);
            notifyBuf[((t_transmitPacket *)a_data)->m_size] = (uint8_t)(((t_transmitPacket *)a_data)->m_srcAddr);
            xTaskNotify( networkTaskHandle, notifyPoint, eSetValueWithOverwrite );
        }
        break;
    default:
        break;
    }
    
}
/*****************************************************************
* DESCRIPTION: loraReceiveError
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraReceiveError( void )
{
    //startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
}
/*****************************************************************
* DESCRIPTION: loraReceiveTimeout
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraReceiveTimeout( void )
{
    //startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
}
/*****************************************************************
* DESCRIPTION: loraSendDone
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraSendDone( void )
{
    static uint8_t broadcastCount = 0;
    if( transFlag && macPIB.CW == 0 )
    {
        transFlag = false;
        if( getTransmitPacket()->m_transmitPacket.m_dstAddr.addrMode != broadcastAddr )
        {
            startSingleTimer( TRANSMIT_WAIT_ACK_EVENT, TRANSMIT_ACK_WAIT_TIME, transmitNoAck );
        }
        else
        {
            /* Num of broadcast times is BROADCAST_MAX_NUM */
            if( ++broadcastCount < BROADCAST_MAX_NUM )
            {
                xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_START, eSetBits );
            }
            else
            {
                broadcastCount = 0;
                transmitFreeHeadData();
            }
        }
    }
    loraReceiveData();
}
/*****************************************************************
* DESCRIPTION: loraSendTimeout
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraSendTimeout( void )
{
    if( transFlag && macPIB.CW == 0 )
    {
        transFlag = false;
        xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_START, eSetBits );
    }
    else
    {
        xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_COMMAND, eSetBits );
    }
}
/*****************************************************************
* DESCRIPTION: loraCadDone
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraCadDone( uint8_t a_detected )
{
    switch( a_detected )
    {
    case RF_CHANNEL_EMPTY:
        
        if( transmitSendCommand() )
        {
            startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
        }
        else if( transFlag )
        {
            if( --macPIB.CW )
            {
                startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
            }
            else
            {
                //transFlag = false;
                transmitSendData();
            }
        }
        
        break;
    case RF_CHANNEL_ACTIVITY_DETECTED:
        if( transFlag )
        {
            if( ++macPIB.NB < MAC_MAX_NB )
            {
                macPIB.CW = MAC_VALUE_CW;
                macPIB.BE = minValue(++macPIB.BE, MAC_MAX_BE);
                //startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
            }
            else
            {
                transFlag = false;
                xTaskNotify( loraTaskHandle, LORA_NOTIFY_TRANSMIT_START, eSetBits );
            }
        }
        loraReceiveData();
        startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime()+300, getChannelStarus );
        
        break;
    default:
        startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
        return;
    }
    /* Send notify to Task which need */
    if( notifyTask )
    {
        xTaskNotify( notifyTask, a_detected, eSetValueWithOverwrite );
        notifyTask = NULL;
    }
}
/*****************************************************************
* DESCRIPTION: loraCadTimeout
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
static void loraCadTimeout( void )
{
    startSingleTimer( TRANSMIT_NB_TIME_EVENT, getAvoidtime(), getChannelStarus );
}




/****************************************************** END OF FILE ******************************************************/
