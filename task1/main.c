#include "project.h" 
#include <string.h> 

#define LINE_MAX_LEN    (48u) 
#define MAX_TOKENS      (4u) 
/* світлодіоди плати активні низьким рівнем */ 
#define LED_COUNT       (3u) 
#define LED_ON_LEVEL    (0u)   

typedef uint8 (*CommandHandler)(uint8 argc, char *argv[]); 

typedef struct 
{ 
    const char     *name; 
    const char     *help; 
    CommandHandler handler; 
} Command; 

static volatile uint32 uptimeMs = 0u; 
static char   lineBuf[LINE_MAX_LEN]; 
static uint8  lineLen = 0u; 
static uint8  lastWasCr = 0u; 
static uint8  ledOn[LED_COUNT]; 
static uint32 rxByteCount = 0u; 
static uint32 cmdOkCount = 0u; 
static uint32 cmdErrCount = 0u; 
static uint32 lineOverflowCount = 0u; 

static void SysTickTick(void) 
{ 
    uptimeMs++; 
} 

static void Print(const char *text) 
{ 
    UART_1_UartPutString(text); 
} 

static void PrintU32(uint32 value) 
{ 
    char   digits[11]; 
    uint8  pos = (uint8)sizeof(digits); 
    digits[--pos] = '\0'; 
    do 
    { 
        digits[--pos] = (char)('0' + (value % 10u)); 
        value /= 10u; 
    } while (value != 0u); 
    Print(&digits[pos]); 
} 

/* Розбір десяткового числа: 1 – успіх, 0 – помилка (не цифра або переповнення). */ 
static uint8 ParseU32(const char *text, uint32 *result) 
{ 
    uint32 value = 0u; 
    uint32 digit; 

    if (*text == '\0') 
    { 
        return 0u; 
    } 

    while (*text != '\0') 
    { 
        if ((*text < '0') || (*text > '9')) 
        { 
            return 0u; 
        } 
        digit = (uint32)(*text - '0'); 
        if (value > ((0xFFFFFFFFu - digit) / 10u)) 
        { 
            return 0u; 
        } 
        value = (value * 10u) + digit; 
        text++; 
    } 

    *result = value; 
    return 1u; 
} 

/* Друкує повідомлення про помилку й повертає 0 («команда не виконана»). */ 
static uint8 ErrorUsage(const char *usage) 
{ 
    Print("ERR usage: "); 
    Print(usage); 
    Print("\r\n"); 
    return 0u; 
} 

/* Керування виводом світлодіода: index 0 = R, 1 = G, 2 = B. */ 
static void LedWriteHw(uint8 index, uint8 on) 
{ 
    uint8 level = on ? LED_ON_LEVEL : (uint8)(1u - LED_ON_LEVEL); 

    switch (index) 
    { 
        case 0u: Pin_LED_R_Write(level); break; 
        case 1u: Pin_LED_G_Write(level); break; 
        default: Pin_LED_B_Write(level); break; 
    } 
} 

/* ---------- Готові команди ---------- */ 

static uint8 CmdHelp(uint8 argc, char *argv[]); 

static uint8 CmdPing(uint8 argc, char *argv[]) 
{ 
    (void)argc; (void)argv; 
    Print("pong\r\n"); 
    return 1u; 
} 

static uint8 CmdInfo(uint8 argc, char *argv[]) 
{ 
    (void)argc; (void)argv; 
    Print("Lab_2 UART console\r\nSYSCLK: "); 
    PrintU32((uint32)CYDEV_BCLK__SYSCLK__MHZ); 
    Print(" MHz\r\nSysTick: 1 ms\r\n"); 
    return 1u; 
} 

static uint8 CmdUptime(uint8 argc, char *argv[]) 
{ 
    (void)argc; (void)argv; 
    PrintU32(uptimeMs); 
    Print(" ms\r\n"); 
    return 1u; 
} 

static uint8 CmdReboot(uint8 argc, char *argv[]) 
{ 
    (void)argc; (void)argv; 
    Print("rebooting...\r\n"); 
    CyDelay(20u);   /* дати час на передавання тексту перед скиданням */ 
    CySoftwareReset(); 
    return 1u; 
} 

/* ===== ТУТ ДОПИШІТЬ ВАШІ КОМАНДИ (п. 6.3): CmdEcho, CmdLed, CmdStats, CmdBlink ===== */ 

static const Command commandTable[] = 
{ 
    { "help",   "list commands",          CmdHelp   }, 
    { "ping",   "reply pong",             CmdPing   }, 
    { "info",   "clock and build info",   CmdInfo   }, 
    { "uptime", "time since reset, ms",   CmdUptime }, 
    { "reboot", "software reset",         CmdReboot }, 
    /* ===== ТУТ ДОДАЙТЕ РЯДКИ ДЛЯ ВАШИХ КОМАНД ===== */ 
}; 

#define COMMAND_COUNT (sizeof(commandTable) / sizeof(commandTable[0])) 

static uint8 CmdHelp(uint8 argc, char *argv[]) 
{ 
    uint8 i; 

    (void)argc; (void)argv; 
    for (i = 0u; i < COMMAND_COUNT; i++) 
    { 
        Print("  "); 
        Print(commandTable[i].name); 
        Print(" - "); 
        Print(commandTable[i].help); 
        Print("\r\n"); 
    } 
    return 1u; 
} 

/* ---------- Командний рядок ---------- */ 

/* Розбиває lineBuf на токени (замінює пробіли на '\0') і викликає обробник. */ 
static void ExecuteLine(void) 
{ 
    char  *argv[MAX_TOKENS]; 
    uint8  argc = 0u; 
    uint8  inToken = 0u; 
    uint8  i; 
    uint8  ok = 0u; 
    uint8  found = 0u; 
    char  *p = lineBuf; 

    for (; *p != '\0'; p++) 
    { 
        if (*p == ' ') 
        { 
            *p = '\0'; 
            inToken = 0u; 
        } 
        else if (!inToken) 
        { 
            if (argc == MAX_TOKENS) 
            { 
                Print("ERR too many arguments\r\n"); 
                cmdErrCount++; 
                return; 
            } 
            argv[argc++] = p; 
            inToken = 1u; 
        } 
    } 

    if (argc == 0u) 
    { 
        return; 
    } 

    for (i = 0u; i < COMMAND_COUNT; i++) 
    { 
        if (strcmp(argv[0], commandTable[i].name) == 0) 
        { 
            found = 1u; 
            ok = commandTable[i].handler(argc, argv); 
            break; 
        } 
    } 

    if (!found) 
    { 
        Print("ERR unknown command, type help\r\n"); 
    } 

    if (found && ok) 
    { 
        cmdOkCount++; 
    } 
    else 
    { 
        cmdErrCount++; 
    } 
} 

/* Обробка одного прийнятого байта: ехо, Backspace, Enter, переповнення рядка. */ 
static void ProcessChar(uint8 c) 
{ 
    rxByteCount++; 

    if ((c == '\n') && lastWasCr) 
    { 
        lastWasCr = 0u;   /* друга половина CR+LF */ 
        return; 
    } 

    if ((c == '\r') || (c == '\n')) 
    { 
        lastWasCr = (uint8)(c == '\r'); 
        Print("\r\n"); 
        lineBuf[lineLen] = '\0'; 
        ExecuteLine(); 
        lineLen = 0u; 
        Print("> "); 
        return; 
    } 

    lastWasCr = 0u; 

    if ((c == 0x08u) || (c == 0x7Fu)) 
    { 
        if (lineLen > 0u) 
        { 
            lineLen--; 
            Print("\b \b"); 
        } 
    } 
    else if ((c >= 0x20u) && (c < 0x7Fu)) 
    { 
        if (lineLen < (LINE_MAX_LEN - 1u)) 
        { 
            lineBuf[lineLen++] = (char)c; 
            UART_1_UartPutChar(c);   /* ехо символу з боку плати */ 
        } 
        else 
        { 
            lineOverflowCount++; 
        } 
    } 
} 

int main(void) 
{ 
    uint8 i; 
    CyGlobalIntEnable; 
    CySysTickStart(); 
    (void)CySysTickSetCallback(0u, SysTickTick); 
    for (i = 0u; i < LED_COUNT; i++) 
    { 
        ledOn[i] = 0u; 
        LedWriteHw(i, 0u); 
    } 
    UART_1_Start(); 
    Print("\r\n=== Lab_2 UART console ===\r\nType help\r\n> "); 
    for (;;) 
    { 
        while (UART_1_SpiUartGetRxBufferSize() > 0u) 
        { 
            ProcessChar((uint8)UART_1_SpiUartReadRxData()); 
        } 
        /* ТУТ викличете BlinkService() після того, як напишете команду blink */ 
    } 
}


