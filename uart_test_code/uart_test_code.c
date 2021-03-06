#include "project.h"
#include "uart_test.h"

#define TASKSTACKSIZE     512
#define uart_buffersize 8

Void task_init(UArg arg0, UArg arg1);
Void task_uart(UArg arg0, UArg arg1);

UInt32 sleepTickCount;

Task_Struct task1Struct, task2Struct;
Char task1Stack[TASKSTACKSIZE], task2Stack[TASKSTACKSIZE];

Semaphore_Struct uart_semStruct;
Semaphore_Handle uart_semHandle;

Semaphore_Struct init_semStruct;
Semaphore_Handle init_semHandle;

UART_Handle uart;
UART_Params uartParams;

unsigned char pcBuffer[128], uartBuf[uart_buffersize], pcData[],uartData[128];

Int main()
{
    /* Construct BIOS objects */
    Task_Params taskParams;
    Semaphore_Params semParams;

    /* Construct writer/reader Task threads */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;

    taskParams.stack = &task1Stack;
    taskParams.priority = 1;
    Task_construct(&task1Struct, (Task_FuncPtr)task_init, &taskParams, NULL);

    taskParams.stack = &task2Stack;
    taskParams.priority = 2;
    Task_construct(&task2Struct, (Task_FuncPtr)task_uart, &taskParams, NULL);

    Semaphore_Params_init(&semParams);
    Semaphore_construct(&uart_semStruct, 0, &semParams);

    Semaphore_Params_init(&semParams);
    Semaphore_construct(&init_semStruct, 0, &semParams);

    uart_semHandle = Semaphore_handle(&uart_semStruct);
    init_semHandle = Semaphore_handle(&init_semStruct);

    /* We want to sleep for 10000 microseconds */
    sleepTickCount = 10000 / Clock_tickPeriod;

    BIOS_start();
    return(0);
}
/**
 * task_init fonksiyonu gerekli kurulumlar� yap�p uart'�n echo yapmas�i�in semaphore'u post etmesi i�in �al��an os task�d�r.
 */
Void task_init(UArg arg0, UArg arg1)
{
    Board_initGeneral();
    Board_initGPIO();
    Board_initUART();
    UART_Kurulum(Baud_115200, readCallback, Parite_NONE,Stop_ONE);
    //UART_read(uart, uartBuf, uart_buffersize);
    Semaphore_post(init_semHandle);
    for(;;)
    {
        UART_read(uart, uartBuf, uart_buffersize);
        Task_sleep(sleepTickCount);
    }
}
/**
 * task_uart fonksiyonu ringbuffer'a yaz�lm�� verileri uarta tekrar g�ndermek i�in �al��mas� gereken bir i�letim sistemi task�d�r.
 */
Void task_uart(UArg arg0, UArg arg1)
{
    Semaphore_pend(init_semHandle, BIOS_WAIT_FOREVER);
    for (;;)
    {
        UART_Step();
        Task_sleep(sleepTickCount);
    }
}

