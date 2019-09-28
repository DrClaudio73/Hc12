#include"credentials.h"
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define RESET_SIM800GPIO CONFIG_RESET_SIM800GPIO
#define UART_SIM800TXGPIO CONFIG_UART_SIM800TXGPIO
#define UART_SIM800RXGPIO CONFIG_UART_SIM800RXGPIO
#define HC12SETGPIO CONFIG_HC12SETGPIO
#define HC12TXGPIO CONFIG_HC12TXGPIO
#define HC12RXGPIO CONFIG_HC12RXGPIO
#define CH_NO_HC12  CONFIG_CH_NO_HC12

static const char *TAG = "SIM800-HC12App";
// max buffer length

#define SIM800MODULE 1
//#define SIM808MODULE 1
#ifdef SIM800MODULE
#define RESET_IDLE 1
#define RESET_ACTIVE 0
#endif
#ifdef SIM808MODULE
#define RESET_IDLE 0
#define RESET_ACTIVE 1
#endif

#ifndef LINE_MAX
#define LINE_MAX	123
#define MAX_ATTEMPTS 22
#define NUM_LINES_TO_FLUSH 4
#endif

#define NOT_ALLOWED_CALLER -1
#define ALLOWED_CALLER 0
#define NO_CALLER +1

typedef struct Parsed_Params
{
    int quality;
    char parametro_feedback[LINE_MAX];
    int pending_DTMF;
    char calling_number[LINE_MAX];
    int calling_number_valid; //-1=Number Calling not allowed ; 0=Number Calling allowed; ; +1=No calling number
} parsed_params; //struct gun arnies;

void scriviUART(uart_port_t uart_controller, char* text);

// read a line from the UART controller
char* read_line(uart_port_t uart_controller) ;

int checkDTMF(char* line);

char* parse_line(char* line, parsed_params* parsed_params);

void stampa_stringa(char* line);

int verificaComando(uart_port_t uart_controller, char* text, char* condition, parsed_params* parametri_globali);

void reset_module(uart_port_t uart_controller, parsed_params* parametri_globali);

void foreverRed() ;

int simOK(uart_port_t uart_controller, parsed_params* parametri_globali);

int checkPhoneActivityStatus(char* parametro_feedback);

int checkCallingNumber();