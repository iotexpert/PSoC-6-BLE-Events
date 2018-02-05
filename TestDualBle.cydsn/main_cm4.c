
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

//#define USE_RTOS

#ifdef USE_RTOS
SemaphoreHandle_t bleSemaphore;
#endif

void customEventHandler(uint32_t event, void *eventParameter)
{
    
    /* Take an action based on the current event */
    switch (event)
    {
        /* This event is received when the BLE stack is Started */
        case CY_BLE_EVT_STACK_ON:
            printf("CY_BLE_EVT_STACK_ON\r\n");
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        break;
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            printf("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n-------------\r\n",
            (unsigned int)(*(cy_stc_ble_gap_disconnect_param_t *)eventParameter).bdHandle,
            (unsigned int)(*(cy_stc_ble_gap_disconnect_param_t *)eventParameter).reason,
            (unsigned int)(*(cy_stc_ble_gap_disconnect_param_t *)eventParameter).status);
        
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
    
        break;
            
        case CY_BLE_EVT_GATT_CONNECT_IND:
            printf("CY_BLE_EVT_GATT_CONNECT_IND bdHandle=%x\r\n",((cy_stc_ble_conn_handle_t *)eventParameter)->bdHandle);
           
        break;
            
        case CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
             printf("CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE\r\n");
        break;
            
        case CY_BLE_EVT_TIMEOUT:
            printf("CY_BLE_EVT_TIMEOUT\r\n");
        break;
                
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            printf("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ\r\n");
        break;
                
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            printf("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ\r\n");
        break;
            
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            printf("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE\r\n");
        break;
            
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            printf("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE\r\n");
        break;
            
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            printf("CY_BLE_EVT_SET_TX_PWR_COMPLETE\r\n");
        break;
            
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            printf("CY_BLE_EVT_GATT_DISCONNECT_IND\r\n");
        break;
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            printf("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP = ");
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
                printf("CY_BLE_ADV_STATE_STOPPED");
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADV_INITIATED)
                printf("CY_BLE_ADV_STATE_ADV_INITIATED");
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
                printf("CY_BLE_ADV_STATE_ADVERTISING");
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOP_INITIATED)
                printf("CY_BLE_ADV_STATE_STOP_INITIATED");
             printf("\r\n");
        break;
            
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            printf("CY_BLE_EVT_GATTS_INDICATION_ENABLED\r\n");
        break;
   
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            printf("CY_BLE_EVT_GAP_DEVICE_CONNECTED\r\n");
        break;
                
        default:
            printf("Unknown Event = %X\n",(unsigned int)event);
        break;
    }
}

#ifdef USE_RTOS
void bleInterruptNotify()
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(bleSemaphore, &xHigherPriorityTaskWoken); 
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
#endif

void bleTask(void *arg)
{
    (void)arg;
    printf("\033[2J\033[H"); // Clear Screen
    printf("Started BLE Task\r\n");
    #ifdef USE_RTOS
    bleSemaphore = xSemaphoreCreateCounting(2^32-1,0);
    printf("Using RTOS\r\n");
    #else
        printf("Bare Metal\r\n");
    #endif
 
    printf("Cy_SysLib_GetDeviceRevision() %X \r\n", Cy_SysLib_GetDeviceRevision());
    
    Cy_BLE_Start(customEventHandler);
    #ifdef USE_RTOS
        
    Cy_BLE_IPC_RegisterAppHostCallback(bleInterruptNotify);
    //Cy_BLE_RegisterInterruptCallback(2^32-1,bleInterruptNotify);
    while(Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();
    }
    #endif
   
    for(;;)
    {
        #ifdef USE_RTOS
            xSemaphoreTake(bleSemaphore,portMAX_DELAY);
        #endif
        Cy_BLE_ProcessEvents();         
    }
}

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    UART_1_Start();
    printf("Started Project\r\n");
    #ifndef USE_RTOS
    bleTask(0);
    #endif
    
    xTaskCreate(bleTask,"bleTask",4*1024,0,1,0);
    vTaskStartScheduler();
 
}
