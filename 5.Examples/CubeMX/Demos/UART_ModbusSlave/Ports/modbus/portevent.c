
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Variables ----------------------------------------*/
static eMBEventType eQueuedEvent;
static bool         xEventInQueue;

/* ----------------------- Start implementation -----------------------------*/
bool xMBPortEventInit(void)
{
    xEventInQueue = false;
    return true;
}

bool xMBPortEventPost(eMBEventType eEvent)
{
    xEventInQueue = true;
    eQueuedEvent  = eEvent;
    return true;
}

bool xMBPortEventGet(eMBEventType* eEvent)
{
    bool xEventHappened = false;

    if (xEventInQueue) {
        *eEvent        = eQueuedEvent;
        xEventInQueue  = false;
        xEventHappened = true;
    }
    return xEventHappened;
}
