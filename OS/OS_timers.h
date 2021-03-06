/*****************************************************************
* Copyright (C) 2017 60Plus Technology Co.,Ltd.*
******************************************************************
* OS_timers.h
*
* DESCRIPTION:
*     OS timers
* AUTHOR:
*     Ziming
* CREATED DATE:
*     2018/12/21
* REVISION:
*     v0.1
*
* MODIFICATION HISTORY
* --------------------
* $Log:$
*
*****************************************************************/
#ifndef OS_TIMERS_H
#define OS_TIMERS_H
#ifdef __cplusplus
extern "C"
{
#endif
/*************************************************************************************************************************
 *                                                        MACROS                                                         *
 *************************************************************************************************************************/

/* Timer task priority. */
#define TIMER_TASK_PRIORITY             5
/* Timer task depth. */
#define TIMER_TASK_DEPTH                256
/* Timer queue length */
#define TIMER_QUEUE_LENGTH              5
/* Timer queue size(byte) */
#define TIMER_QUEUE_SIZE                1
    
/*************************************************************************************************************************
 *                                                      CONSTANTS                                                        *
 *************************************************************************************************************************/
typedef enum
{
    NO_EVENT = 0U,
    
/* Network events */
    NETWORK_BUILD_EVENT,
    NETWORK_DETECTION_CHANNEL_EVENT,
    NETWORK_WAIT_BEACON_EVENT,
    NETWORK_JOIN_NWK_EVENT,
    NETWORK_BEACON_EVENT,
    
/* Lora events */
    LORA_TIMEOUT_EVENT,
    LORA_ALLOW_JOIN_TIME_EVENT,
    
/* Transmit events */
    //CTS_DURATION_EVENT,
    TRANSMIT_NB_TIME_EVENT,
    TRANSMIT_WAIT_ACK_EVENT,
    
/* Other events */
    SYSTEM_FEED_DOG_EVENT,
    
}E_timerEvent;
    
typedef enum
{
    SINGLE_TIMER = 0U,
    RELOAD_TIMER,
    ALL_TYPE_TIMER,
    NO_TIMER,
}E_timerType;
/*************************************************************************************************************************
 *                                                       TYPEDEFS                                                        *
 *************************************************************************************************************************/
typedef void (*timerCallback_t)(void);
     
typedef struct T_timerList
{
    TimerHandle_t               m_timerHandle;
    uint8_t                     m_timerId;
    E_timerEvent                m_timerEvent;
    uint32_t                    m_time;
    bool                        m_reloadTimer;
    timerCallback_t             m_timerCallback;
    struct T_timerList         *m_next;
}t_timerList;
/*************************************************************************************************************************
 *                                                  EXTERNAL VARIABLES                                                   *
 *************************************************************************************************************************/
extern t_timerList                 *timerListHead;
extern TaskHandle_t                 timerTaskHandle;
extern QueueHandle_t                timerQueueHandle;
/*************************************************************************************************************************
 *                                                   PUBLIC FUNCTIONS                                                    *
 *************************************************************************************************************************/

/*****************************************************************
* DESCRIPTION: osTimerInit
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void osTimerInit( void );
/*****************************************************************
* DESCRIPTION: osTimerProcess
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void osTimerProcess( void *parm );
/*****************************************************************
* DESCRIPTION: startSingleTimer
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
E_typeErr startSingleTimer( E_timerEvent a_timerEvent, uint32_t a_time, timerCallback_t a_timerCallback );
/*****************************************************************
* DESCRIPTION: startReloadTimer
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
E_typeErr startReloadTimer( E_timerEvent a_timerEvent, uint32_t a_time, timerCallback_t a_timerCallback );
/*****************************************************************
* DESCRIPTION: getTimerType
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
E_timerType getTimerType( E_timerEvent a_timerEvent );
/*****************************************************************
* DESCRIPTION: stopTimer
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
t_timerList *stopTimer( E_timerEvent a_timerEvent, E_timerType a_timerType );
/*****************************************************************
* DESCRIPTION: clearTimer
*     
* INPUTS:
*     
* OUTPUTS:
*     
* NOTE:
*     null
*****************************************************************/
void clearTimer( E_timerEvent a_timerEvent, E_timerType a_timerType );

#ifdef __cplusplus
}
#endif
#endif /* OS_timers.h */
