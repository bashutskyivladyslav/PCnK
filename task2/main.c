#include "project.h"
#include <string.h>

#define LINE_MAX_LEN (48)
#define MAX_TOKENS   (4u)
#define LED_COUNT    (3u)
#define LED_ON_LEVEL (0) 

typedef uint8 (*CommandHandler) (uint8 argc, char *argv[]);

typedef struct
{
    const char *name;
    const char *help;
    CommandHandler handler;
} Command;

static volatile uint32 uptimeMs = 0u;
static char lineBuf[LINE_MAX_LEN];
static uint8 lineLen = 0u;
static uint8 lastWasCr = 0u;
static uint8 ledOn[LED_COUNT];
static uint32 rxByteCount = 0u;
static uint32 cmdOkCount = 0u;
static uint32 cmdErrCount = 0u;
static uint32 lineOverflowCount = 0u;

static uint32 blinkIntervalMs[LED_COUNT] = {0u, 0u, 0u};
static uint32 blinkNextMs[LED_COUNT] = {0u, 0u, 0u};

static void SysTick_Tick(void)
{
    uptimeMs++;
}

static void Print(const char *text)
{
    UART_1_UartPutString(text);
}

static void PrintU32(uint32 value)
{
    char digits[11];
    uint8 pos = (uint8) sizeof(digits);
    digits[--pos] = '\0';
    do
    {
        digits[--pos] = (char) ('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u);
    Print(&digits[pos]);
}

static uint8 ParseU32 (const char *text, uint32 *result)
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
        digit = (uint32) (*text - '0');
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

static uint8 ErrorUsage(const char *usage)
{
    Print("ERR usage: ");
    Print(usage);
    Print("\r\n");
    return 0u;
}

static void LedWriteHw(uint8 index, uint8 on)
{
    uint8 level = on ? LED_ON_LEVEL : (uint8) (1u - LED_ON_LEVEL);
    switch (index)
    {
        case 0u: Pin_LED_R_Write(level); break;
        case 1u: Pin_LED_G_Write(level); break;
        default: Pin_LED_B_Write(level); break;
    }
}

static int8 ParseLedIndex(const char *name, uint8 *isAll)
{
    *isAll = 0u;
    if (strcmp(name, "r") == 0) return 0;
    if (strcmp(name, "g") == 0) return 1;
    if (strcmp(name, "b") == 0) return 2;
    if (strcmp(name, "all") == 0) {
        *isAll = 1u;
        return -1;
    }
    return -1;
}

static void BlinkService(void)
{
    uint8 i;
    for (i = 0u; i < LED_COUNT; i++)
    {
        if (blinkIntervalMs[i] > 0u)
        {
            if ((int32)(uptimeMs - blinkNextMs[i]) >= 0)
            {
                ledOn[i] = !ledOn[i];
                LedWriteHw(i, ledOn[i]);
                blinkNextMs[i] += blinkIntervalMs[i];
            }
        }
    }
}

static uint8 CmdHelp(uint8 argc, char *argv[]);

static uint8 CmdPing (uint8 argc, char *argv[])
{
    (void)argc; (void)argv;
    Print("pong\r\n");
    return 1u;
}

static uint8 CmdInfo(uint8 argc, char *argv[])
{
    (void)argc; (void)argv;
    Print("Lab_2 UART console\r\nSYSCLK: 24 MHz\r\nSysTick: 1 ms\r\n");
    return 1u;
}

static uint8 CmdUptime (uint8 argc, char *argv[])
{
    (void)argc; (void)argv;
    PrintU32(uptimeMs);
    Print(" ms\r\n");
    return 1u;
}

static uint8 CmdReboot (uint8 argc, char *argv[])
{
    (void)argc; (void)argv;
    Print("rebooting...\r\n");
    CyDelay(20u);
    CySoftwareReset();
    return 1u;
}

static uint8 CmdEcho(uint8 argc, char *argv[])
{
    uint8 i;
    if (argc < 2)
    {
        return ErrorUsage("echo <text>");
    }
    for (i = 1; i < argc; i++)
    {
        Print(argv[i]);
        if (i < argc - 1)
        {
            Print(" ");
        }
    }
    Print("\r\n");
    return 1u;
}

static uint8 CmdLed(uint8 argc, char *argv[])
{
    uint8 isAll = 0u;
    int8 chIdx;
    uint8 newState;
    uint8 i;

    if (argc != 3)
    {
        return ErrorUsage("led <r|g|b|all> <on|off|toggle>");
    }

    chIdx = ParseLedIndex(argv[1], &isAll);
    if (!isAll && (chIdx < 0 || chIdx >= (int8)LED_COUNT))
    {
        return ErrorUsage("led <r|g|b|all> <on|off|toggle>");
    }

    if (strcmp(argv[2], "on") == 0)
    {
        newState = 1u;
    }
    else if (strcmp(argv[2], "off") == 0)
    {
        newState = 0u;
    }
    else if (strcmp(argv[2], "toggle") == 0)
    {
        if (isAll)
        {
            for (i = 0u; i < LED_COUNT; i++)
            {
                ledOn[i] = !ledOn[i];
                blinkIntervalMs[i] = 0u;
                LedWriteHw(i, ledOn[i]);
            }
            Print("OK\r\n");
            return 1u;
        }
        else
        {
            newState = !ledOn[chIdx];
        }
    }
    else
    {
        return ErrorUsage("led <r|g|b|all> <on|off|toggle>");
    }

    if (isAll)
    {
        for (i = 0u; i < LED_COUNT; i++)
        {
            ledOn[i] = newState;
            blinkIntervalMs[i] = 0u;
            LedWriteHw(i, newState);
        }
    }
    else
    {
        ledOn[chIdx] = newState;
        blinkIntervalMs[chIdx] = 0u;
        LedWriteHw((uint8)chIdx, newState);
    }

    Print("OK\r\n");
    return 1u;
}

static uint8 CmdStats(uint8 argc, char *argv[])
{
    (void)argc; (void)argv;
    Print("rx bytes: ");
    PrintU32(rxByteCount);
    Print("\r\ncmd ok: ");
    PrintU32(cmdOkCount);
    Print("\r\ncmd err: ");
    PrintU32(cmdErrCount);
    Print("\r\nline overflow: ");
    PrintU32(lineOverflowCount);
    Print("\r\n");
    return 1u;
}

static uint8 CmdBlink(uint8 argc, char *argv[])
{
    uint8 isAll = 0u;
    int8 chIdx;
    uint32 interval;

    if (argc != 3)
    {
        return ErrorUsage("blink <r|g|b> <10..60000 ms|off>");
    }

    chIdx = ParseLedIndex(argv[1], &isAll);
    if (isAll || chIdx < 0 || chIdx >= (int8)LED_COUNT)
    {
        return ErrorUsage("blink <r|g|b> <10..60000 ms|off>");
    }

    if (strcmp(argv[2], "off") == 0)
    {
        blinkIntervalMs[chIdx] = 0u;
        ledOn[chIdx] = 0u;
        LedWriteHw((uint8)chIdx, 0u);
        Print("OK\r\n");
        return 1u;
    }

    if (!ParseU32(argv[2], &interval))
    {
        return ErrorUsage("blink <r|g|b> <10..60000 ms|off>");
    }

    if (interval < 10u || interval > 60000u)
    {
        return ErrorUsage("blink <r|g|b> <10..60000 ms|off>");
    }

    blinkIntervalMs[chIdx] = interval;
    blinkNextMs[chIdx] = uptimeMs + interval;
    Print("OK\r\n");
    return 1u;
}

static const Command commandTable[] =
{
    { "help",   "list commands",         CmdHelp },
    { "ping",   "reply pong",            CmdPing },
    { "info",   "clock and build info",  CmdInfo },
    { "uptime", "time since reset, ms",  CmdUptime },
    { "reboot", "software reset",        CmdReboot },
    { "echo",   "echo text",             CmdEcho },
    { "led",    "control led",           CmdLed },
    { "stats",  "show statistics",       CmdStats },
    { "blink",  "blink led",             CmdBlink },
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

static void ExecuteLine(void)
{
    char *argv[MAX_TOKENS];
    uint8 argc = 0u;
    uint8 inToken = 0u;
    uint8 i;
    uint8 ok = 0u;
    uint8 found = 0u;
    char *p = lineBuf;

    for (; *p != '\0'; p++)
    {
        if (*p == ' ')
        {
            *p = '\0';
            inToken = 0u;
        }
        else if (!inToken)
        {
            if (argc >= MAX_TOKENS)
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

static void ProcessChar(uint8 c)
{
    rxByteCount++;

    if ((c == '\n') && lastWasCr)
    {
        lastWasCr = 0;
        return;
    }

    if ((c == '\r') || (c == '\n'))
    {
        lastWasCr = (uint8) (c == '\r');
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
            UART_1_UartPutChar(c);
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
    (void) CySysTickSetCallback(0u, SysTick_Tick);

    for (i = 0u; i < LED_COUNT; i++)
    {
        ledOn[i] = 0u;
        LedWriteHw(i, 0u);
    }

    UART_1_Start();
    Print("\r\n=== Lab_2 UART console ===\r\nType help\r\n> ");

    for (;;)
    {
        while (UART_1_SpiUartGetRxBufferSize() > 0)
        {
            ProcessChar((uint8) UART_1_SpiUartReadRxData());
        }
        
        BlinkService();
    }
}