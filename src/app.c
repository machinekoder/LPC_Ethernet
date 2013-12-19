/*
 *  uC/OS-III - Getting Started Demo Application
 *
 **/
#include "app.h"
#include "taskStart.h"

#define printfData(x) Debug_printf(Debug_Level_1,x)
#define printfData2(x,y) Debug_printf(Debug_Level_1,x,y)

static uint8_t eth_mac[6] = {0x5a,0x01,0x02,0x03,0x04,0x05};

OS_SEM RX_SEM;

OS_SEM  ECHOSem;         // semaphore used for signaling incoming CHAT segment

OS_Q TXNicQ;

OS_Q RX_Q;

#define START_SRAM_BANK1 (0x20080000)
#define RX_BUF_START (START_SRAM_BANK1)
#define RX_BUF_LEN ((((EMAC_ETH_MAX_FLEN>>2)+2)*EMAC_NUM_RX_FRAG)+2/*+2 must be 32 bit aligned*/)

#define TX_BUF_START (RX_BUF_START+RX_BUF_LEN)
#define TX_BUF_LEN (((EMAC_ETH_MAX_FLEN>>2)+2)*EMAC_NUM_TX_FRAG)

OS_MEM PacketMemArea;

//ApplicationState applicationState = ApplicationState_Idle;

/*
************************************************************************************************
*                                            LOCAL VARIABLES
************************************************************************************************
*/

EMAC_CFG_Type emacConfigStruct;

uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x67,0x00u,0x05u};
uint8_t ipv4Address[4] = {10u,42u,0u,10u};

/*
************************************************************************************************
*                                         FUNCTION PROTOTYPES
************************************************************************************************
*/

/*
************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code. It is assumed that your code will
*               call main() once you have performed all necessary initialization.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Startup Code.
*
* Note(s)     : none.
************************************************************************************************
*/



int main (void)
{
    OS_ERR   os_err;
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif

    BSP_PreInit();                                 /* initialize basic board support routines */

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)CSP_DEV_NAME,
                (CPU_ERR  *)&cpu_err);
#endif

    Mem_Init();                                       /* Initialize memory management  module */

    OSInit(&os_err);                                                        /* Init uC/OS-III */

//    Debug_printf(Debug_Level_1, "Starting init");
#if 0
    /* Enable Timer0 Interrupt.*/
    CSP_IntVectReg((CSP_DEV_NBR   )CSP_INT_CTRL_NBR_MAIN,
                   (CSP_DEV_NBR   )CSP_INT_SRC_NBR_TMR_00,
                   (CPU_FNCT_PTR  )App_TMR0_IntHandler,
                   (void         *)0);
    CSP_IntEn(CSP_INT_CTRL_NBR_MAIN, CSP_INT_SRC_NBR_TMR_00);

    /* Enable Timer1 Interrupt.*/
    CSP_IntVectReg((CSP_DEV_NBR   )CSP_INT_CTRL_NBR_MAIN,
                   (CSP_DEV_NBR   )CSP_INT_SRC_NBR_TMR_01,
                   (CPU_FNCT_PTR  )App_TMR1_IntHandler,
                   (void         *)0);
    CSP_IntEn(CSP_INT_CTRL_NBR_MAIN, CSP_INT_SRC_NBR_TMR_01);

    /* Enable Timer2 Interrupt.*/
    CSP_IntVectReg((CSP_DEV_NBR   )CSP_INT_CTRL_NBR_MAIN,
                   (CSP_DEV_NBR   )CSP_INT_SRC_NBR_TMR_02,
                   (CPU_FNCT_PTR  )App_TMR2_IntHandler,
                   (void         *)0);
    CSP_IntEn(CSP_INT_CTRL_NBR_MAIN, CSP_INT_SRC_NBR_TMR_02);
    
    CSP_TmrInit();
    CSP_TmrCfg (CSP_TMR_NBR_00,TIMER_FREQ);
    CSP_TmrCfg (CSP_TMR_NBR_01,TIMER_FREQ);
    CSP_TmrCfg (CSP_TMR_NBR_02,TIMER_FREQ);
#endif

#if 0
    // Init the Ethernet PHY
    EMAC_PinCfg();
    emacConfigStruct.Mode = EMAC_MODE_100M_FULL;
    emacConfigStruct.pbEMAC_Addr = EthernetLinkLayer_macAddress();
    OS_EMAC_Init(EthernetLinkLayer_rxSemaphore());
    EMAC_Init(&emacConfigStruct);
    EMAC_SetFilterMode(EMAC_RFC_UCAST_EN, ENABLE);
    EMAC_SetFilterMode(EMAC_RFC_BCAST_EN, ENABLE);
    EMAC_SetFilterMode(EMAC_RFC_MCAST_EN, ENABLE);
    EMAC_SetFilterMode(EMAC_RFC_PERFECT_EN, ENABLE);
    EMAC_SetFilterMode(EMAC_RFC_MAGP_WOL_EN, DISABLE);
    EMAC_SetFilterMode(EMAC_RFC_PFILT_WOL_EN, DISABLE);
    
    // Init the Ethernt interrupt vecotrs
    CSP_IntVectReg((CSP_DEV_NBR   )CSP_INT_CTRL_NBR_MAIN,
                   (CSP_DEV_NBR   )CSP_INT_SRC_NBR_ETHER_00,
                   (CPU_FNCT_PTR  )CSP_IntETH_Handler,
                   (void         *)0);
    CSP_IntEn(CSP_INT_CTRL_NBR_MAIN, CSP_INT_SRC_NBR_ETHER_00);
    
    EthernetLinkLayer_setMacAddress(macAddress);
    Ip_initialize();
    Ip_setIPv4Address(ipv4Address);
#endif

#if 0
    USBInit();                                               /* USB Initialization */
    /* register descriptors */
    USBRegisterDescriptors(abDescriptors);                   /* USB Descriptor Initialization */

    /* register endpoint handlers */
    USBHwRegisterEPIntHandler(BULK_IN_EP, BulkIn);           /* register BulkIn Handler for EP */
    USBHwRegisterEPIntHandler(BULK_OUT_EP, BulkOut);         /* register BulkOut Handler for EP */
    USBHwRegisterEPIntHandler(INT_IN_EP, NULL);

    USBHwEPConfig(BULK_IN_EP, MAX_PACKET_SIZE);   /* Configure Packet Size for outgoing Transfer */
    USBHwEPConfig(BULK_OUT_EP, MAX_PACKET_SIZE);  /* Configure Packet Size for incoming Transfer */

    /* enable bulk-in interrupts on NAKs */
    USBHwNakIntEnable(INACK_BI);
#endif
    //Cb_initialize(&cncQueueBuffer, CNC_QUEUE_BUFFER_SIZE, sizeof(QueueItem), (void*)&cncQueueBufferData);
    
//    Debug_printf(Debug_Level_1, "Init finished");

    if(os_err != OS_ERR_NONE)
        for(;;);

    OSTaskCreate((OS_TCB      *)&App_TaskStartTCB,                  /* Create the Start Task */
                 (CPU_CHAR    *)"Start",
                 (OS_TASK_PTR  )App_TaskStart,
                 (void        *)0,
                 (OS_PRIO      )4,
                 (CPU_STK     *)App_TaskStartStk,
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY   )0u,
                 (OS_TICK      )0u,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&os_err);

    OSStart(&os_err);                                                   /* Start Multitasking */
    if(os_err != OS_ERR_NONE)                                         /* shall never get here */
        for(;;);
    return (0);
}

/*! EOF */
