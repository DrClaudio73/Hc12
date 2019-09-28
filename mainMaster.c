//automatically pick up any incoming call after 1 ring: ATS0=1
//cfun=1,1 (full,rst)
//AT+CREG? if answer is 0,1 then is registered on home network
/*
int checkStatus() { // CHECK STATUS FOR RINGING or IN CALL

    sim800.println(“AT+CPAS”); // phone activity status: 0= ready, 2= unknown, 3= ringing, 4= in call
    delay(100);
    if (sim800.find(“+CPAS: “)) { // decode reply
        char c = sim800.read(); // gives ascii code for status number
        // simReply(); // should give ‘OK’ in Serial Monitor
        return c – 48; // return integer value of ascii code
    }
        else return -1;
}
*/

#include <stdio.h>
// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// UART driver
#include "driver/uart.h"

// Error library
#include "esp_err.h"

#include "string.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "simOK.h"

//RF24 componenets
//#include <RF24.h>
static char tag[] = "masterHC12";

//RF24 myRadio(SPI_nRF24_CE,SPI_nRF24_CSN); //ce , csn
int kkk=0;
int ledus=0;

void start_myRadioHC12(void) {
    /*myRadio.begin();
    myRadio.setPALevel(RF24_PA_MAX); //RF24_PA_MAX
    myRadio.setChannel(CH_No_nRF24); //0x4c
    myRadio.setCRCLength(RF24_CRC_8);
    myRadio.setDataRate(RF24_250KBPS); //RF24_250KBPS  RF24_1MBPS
    myRadio.setAddressWidth(5);
    myRadio.setRetries(5,15);
    myRadio.enableDynamicPayloads();
    uint8_t txAddress[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    myRadio.openWritingPipe(txAddress);
    myRadio.printDetails();*/

    //VERY LIKELY USELESS SINCE HC12 HOLDS CONFIGURATION IN EEPROM or FLASH
    //CONFIGURATION LIKE:
    // - BAUD 2400
    // - CHANNEL 0X01
    // - FU FU3
    // - POWER +20dBm
    // ARE ALREADY SET UP INTO HC12 MODULE 
    return;
}

bool write_nrf24(RF24 myRadio, int level) {
    bool success;
    char pino[50];
    for (int k=0; k<50; k++){
        pino[k]=0;
    }
    //rf24.write("Ciao Claudio te lo sei meritato", 32);
    //ESP_LOGD(tag, "Transmitted data");
    sprintf(pino,"Ciao: %d!",level);
    //level=1-level;
    //vTaskDelay(2000 / portTICK_PERIOD_MS);
    success=myRadio.write(pino, 12);
    vTaskDelay(200 / (1000*portTICK_RATE_MS)); //delayMicroseconds(128);
    //myRadio.printDetails();
    ESP_LOGD(tag, "Transmitted data");
    return(success);
}

int command_valid=0;
char command[LINE_MAX];

/*int quality=0;
int pending_DTMF=-1;
char parametro_feedback[LINE_MAX];*/
parsed_params parametri_globali;

void getCommand(void * parametro){
    int line_pos = 0;

    while(1) {
        int c = getchar();
        // nothing to read, give to RTOS the control
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        //if(c == '\r') continue;
        if(c == '\n') {
            // terminate the string and parse the command
            command[line_pos] = c;
            line_pos++;
            command[line_pos] = 0;
            //parse_command(command);

            // reset the buffer and print the prompt
            line_pos = 0;
            command_valid=1;
            printf("\nYou entered: %s",command);
            //print_prompt();
        }
        else {
            command_valid=0;
            putchar(c);
            command[line_pos] = c;
            line_pos++;
            
            // buffer full!
            if(line_pos == LINE_MAX) {
                ESP_LOGE(TAG,"\n*****************************Command buffer full!\n*****************************");
                // reset the buffer and print the prompt
                line_pos = 0;
                //print_prompt();
            }
        }
    }	
}

toggleLed
void toggleLed(void* parametro){
    uint32_t livello_led=0;
    while(1){
        if (parametri_globali.quality > 0){
            // Blink output led 
            gpio_set_level((gpio_num_t)BLINK_GPIO, livello_led);
            livello_led=1-livello_led;
            vTaskDelay(20*(32 - parametri_globali.quality) / portTICK_RATE_MS);
        } else {
            gpio_set_level((gpio_num_t)BLINK_GPIO, livello_led);
            livello_led=1-livello_led;
            vTaskDelay(3000 / portTICK_RATE_MS);
        }
    }
}

int checkPhoneActivityStatus(char* parametro_feedback) { // CHECK STATUS FOR RINGING or IN CALL
    char msg[8]="AT+CPAS";
    char condition[8]="+CPAS: ";
    // phone activity status: 0= ready, 2= unknown, 3= ringing, 4= in call
    if (verificaComando(UART_NUM_1,msg,condition,&parametri_globali)==0) { // decode reply
        //char c = sim800.read(); // gives ascii code for status number
        return atoi(parametri_globali.parametro_feedback); // return integer value of ascii code
    }
    else return -1;
}


int checkCallingNumber(){
    char *line1;
    char* token;
    char* token2;
    int is_calling_number_valid;
    is_calling_number_valid=+1; //-1=Number Calling not allowed ; 0=Number Calling allowed; ; +1=No calling number
    ESP_LOGI(TAG,"checkCallingNumber()\n");
    for(int k=0;k<10;k++){
        line1 = read_line(UART_NUM_1);
        printf("line read is:%s",line1);
        //+CLIP: "+3912345678",145,"",0,"ccccccc",0
        if (strncmp("+CLIP: ",line1,strlen("+CLIP: "))==0){
            token = strtok(line1, "CLIP: ");
            token = strtok(NULL, "CLIP: ");
            //printf("token: %s",token);
            token2 = strtok(token, "\"");
            sprintf(parametri_globali.parametro_feedback,"%s",token2);
            //printf("token2: %s",token2);
            if (strncmp(ALLOWED1,token2,sizeof(ALLOWED1))==0){
                is_calling_number_valid=0;
                //return(0); //OK: Valid Caller
            } else if (strncmp(ALLOWED2,token2,sizeof(ALLOWED2))==0) {
                is_calling_number_valid=0;
                //return(0); //OK: Valid Caller
            } else {
                is_calling_number_valid=-1;
                //return(-1); //Not valid Caller
            }
        }
    }

    return(is_calling_number_valid);
    
    if (is_calling_number_valid==0){
        return(is_calling_number_valid); // OK numbero valid
    }
    

    if (parametri_globali.calling_number_valid==0){
        sprintf(parametri_globali.parametro_feedback,"%s",parametri_globali.calling_number);
        parametri_globali.calling_number_valid=-1;
        parametri_globali.calling_number[0]=0;
        return(0); // OK number valid
    }

    return is_calling_number_valid; //Only different feedback
}

void setup(void){
    parametri_globali.quality=0;
    parametri_globali.pending_DTMF=-1;
    parametri_globali.calling_number[0]=0;
    parametri_globali.calling_number_valid=NO_CALLER;

    gpio_pad_select_gpio((gpio_num_t)BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t)BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 1);

    gpio_pad_select_gpio((gpio_num_t)RESET_SIM800GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t)RESET_SIM800GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RESET_SIM800GPIO, RESET_IDLE); //does not reset right now
    
    gpio_pad_select_gpio((gpio_num_t)HC12SETGPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t)HC12SETGPIO, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)HC12SETGPIO, 1); //enter transparent mode

    // configure the UART1 controller
    uart_config_t uart_configSIM800 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick= false
    };
    
    uart_config_t uart_configHC12 = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick= false
    };

    uart_param_config(UART_NUM_1, &uart_configSIM800);
    uart_set_pin(UART_NUM_1, UART_SIM800RXGPIO, UART_SIM800TXGPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); //TX , RX
    uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0);
    //uint8_t *data1 = (uint8_t *) malloc(1024);

    // configure the UART2 controller 
    uart_param_config(UART_NUM_2, &uart_configHC12);
    uart_set_pin(UART_NUM_2, HC12RXGPIO, HC12TXGPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); //TX , RX
    uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0);
    //uint8_t *data2 = (uint8_t *) malloc(1024);

    ESP_LOGW(TAG,"==============================");
    ESP_LOGW(TAG,"Welcome to SIM800L control App");
    ESP_LOGW(TAG,"==============================\r\n");

    ESP_LOGI(TAG,"First allowed sim number is %s\r\n",ALLOWED1);
    ESP_LOGI(TAG,"Second allowed sim number is %s\r\n",ALLOWED2);
    //presentation blinking
    gpio_set_level((gpio_num_t)BLINK_GPIO, 1);
    vTaskDelay(500 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 0);
    vTaskDelay(500 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 1);
    vTaskDelay(500 / portTICK_RATE_MS);
    gpio_set_level((gpio_num_t)BLINK_GPIO, 0);

    //Initializing Radio RF24
    start_myRadioHC12();
    //setUpScannermyRadioRF24();
    //while(1) {
    //    loopScannermyRadio();
        //printf("\r\nCIAOOOOOOOOOOOOOOOOOOOOOOOOOO\r\n");
    //}
    if (simOK(UART_NUM_1,&parametri_globali)==-1){
        ESP_LOGE(TAG,"\nSim808 module not found, stop here!");
        foreverRed();
    } 
    
    ESP_LOGW(TAG,"calling_number_valid; %d",parametri_globali.calling_number_valid);
    //Launching Task for getting MANUAL commands to be sent to SIM800L
    //xTaskCreate(&getCommand, "getCommand", 8192, NULL, 5, NULL); QUESTA LINEA IN PRODUZIONE LA TOGLIAMO AD EVITARE PROBLEMI
    //Launching Task for blinking led according to signal quality report 
    //xTaskCreate(&toggleLed, "toggleLed", 4096, NULL, 5, NULL);
}
/*
void test_performance(void){
    int num_sent=0;
    int num_ok=0;
    int num_ko=0;
    int NUM_MAX_TEST = 10000;
    int DTMF=0;
    while (num_sent<NUM_MAX_TEST){
        num_sent++;
        DTMF++;
        if (DTMF > 3) DTMF=0;
        if(write_nrf24(myRadio,DTMF)==true){
            num_ok++;
        } else {
            num_ko++;
        }
        printf_P("DTMF: %d, sent: %d, ok: %d, ko: %d, per_ok: %d, perc_ko: %d\r\n", DTMF, num_sent, num_ok, num_ko, (100*num_ok)/num_sent, (100*num_ko)/num_sent);
        vTaskDelay(1500 / portTICK_RATE_MS);
    }

}
*/
void loop(void){
    // Read data from the UART1
    char *line1;
    line1 = read_line(UART_NUM_1);
    char *subline1=parse_line(line1,&parametri_globali);
    //char debugstr[150];
    stampa_stringa(subline1);

    /*
    ESP_LOGW(TAG,"=======Parametri Globali=======");
    //sprintf(debugstr,"Quality; %d",parametri_globali.quality);
    ESP_LOGW(TAG,"Quality; %d",parametri_globali.quality);
    //sprintf(debugstr,"parametro_feedback; %s",parametri_globali.parametro_feedback);
    ESP_LOGW(TAG,"parametro_feedback; %s",parametri_globali.parametro_feedback);
    //sprintf(debugstr,"pending_DTMF; %d",parametri_globali.pending_DTMF);
    ESP_LOGW(TAG,"pending_DTMF; %d",parametri_globali.pending_DTMF);
    //sprintf(debugstr,"calling_number; %s",parametri_globali.calling_number);
    ESP_LOGW(TAG,"calling_number; %s",parametri_globali.calling_number);
    //sprintf(debugstr,"calling_number_valid; %d",parametri_globali.calling_number_valid);
    ESP_LOGW(TAG,"calling_number_valid; %d",parametri_globali.calling_number_valid);
    ESP_LOGW(TAG,"==============================");
    
    //return;
    if (command_valid){
        ESP_LOGW(TAG,"\n********Command valid, sending %s",command);
        //uart_write_bytes(UART_NUM_1, command, strlen(command));
        scriviUART(UART_NUM_1, command);
        //verificaComando(UART_NUM_1,command,"OK\r\n");
        command_valid=0;
    }
    */
    vTaskDelay(100 / portTICK_RATE_MS);

    //test_performance();

    if (parametri_globali.calling_number_valid==ALLOWED_CALLER){
        ESP_LOGI(TAG,"\n********Valid calling number: %s. Answering call.....",parametri_globali.parametro_feedback);;
        char msg1[4]="ATA";
        char msg_tmp[19]="AT+DDET=1,1000,0,0";
        char condition1[5]="OK\r\n";
        parametri_globali.calling_number_valid=NO_CALLER;
        parametri_globali.calling_number[0]=0;
        
        ESP_LOGW(TAG,"\n********Enabling DTMF recognition....");
        if (verificaComando(UART_NUM_1,msg_tmp,condition1,&parametri_globali)==0){
            ESP_LOGI(TAG,"\n********DTMF recognition is on !!!");
        } else {
            ESP_LOGE(TAG,"\n********Error in setting DDET mode !!!");
        }
        
        if (verificaComando(UART_NUM_1,msg1,condition1,&parametri_globali)==0){
                ESP_LOGI(TAG,"\n********Answered call!!!");
                char msg2[15]="AT+VTS=\"1,0,6\"";
                char condition2[5]="OK\r\n";
                //if (verificaComando(UART_NUM_1,"AT+VTS=\"{1,1},{0,2},{6,1}\"","OK\r\n",&parametri_globali)==0){
                if (verificaComando(UART_NUM_1,msg2,condition2,&parametri_globali)==0){
                    ESP_LOGI(TAG,"\n********Answer tones sent!!!");
                    vTaskDelay(500 / portTICK_RATE_MS);
                } else {
                    ESP_LOGE(TAG,"\n********ERROR in sending answering tones!!!");
                }
            } else {
                ESP_LOGE(TAG,"\n********Error in answering allowed call!!!");
            }
    } else if (parametri_globali.calling_number_valid==NOT_ALLOWED_CALLER) {
        parametri_globali.calling_number_valid=NO_CALLER;
        parametri_globali.calling_number[0]=0;
        ESP_LOGE(TAG,"\n********Calling number %s NOT valid!!! Answering and hanging right after.",parametri_globali.parametro_feedback);
        char msg3[4]="ATA";
        char condition3[5]="OK\r\n";
        if (verificaComando(UART_NUM_1,msg3,condition3,&parametri_globali)==0){
                ESP_LOGI(TAG,"\n********Answered NOT allowed call!!!");
                vTaskDelay(2000 / portTICK_RATE_MS);
                char msg4[4]="ATH";
                char condition4[5]="OK\r\n";                       
                if (verificaComando(UART_NUM_1,msg4,condition4,&parametri_globali)==0){
                    ESP_LOGI(TAG,"\n********Hanged call!!!");
                    vTaskDelay(1000 / portTICK_RATE_MS);
                } else {
                    ESP_LOGE(TAG,"\n********ERROR in hanging NOT allowed call!!!");
                }
            } else {
                ESP_LOGE(TAG,"\n********Error in answering NOT allowed call!!!");
            }
    }

    //int parametri_globali.pending_DTMF = checkDTMF();
    if (parametri_globali.pending_DTMF>=0){
        int DTMF=parametri_globali.pending_DTMF;
        parametri_globali.pending_DTMF=-1;
        ESP_LOGW(TAG,"Tone detected equals %d. Replying with the same tone.",DTMF);;
        char replyTone[30];
        sprintf(replyTone,"AT+VTS=\"{%d,2}\"",DTMF);
        vTaskDelay(2000 / portTICK_RATE_MS);
        char condition5[5]="OK\r\n";
        if (verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali)==0){
                    ESP_LOGI(TAG,"\nReply tone sent!!!");
                    if(write_nrf24(myRadio,DTMF)==true){
                        ESP_LOGI(TAG,"\nMessage sent to SlaveStation was succesffully delivered!!!");   
                        sprintf(replyTone,"AT+VTS=\"{%d,1}\"",DTMF);
                        verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                        verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                        verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali); 
                    } else {
                        ESP_LOGE(TAG,"\nMessage sent to SlaveStation was NOT delivered!!!");    
                        //verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                    }
                } else {
                    ESP_LOGE(TAG,"\nERROR in sending reply tone!!!");
                }
    } else 
    {
        ESP_LOGI(TAG,"nop");
    }

    kkk++;
    if (kkk>=9999) {
        ledus=1-ledus;
        gpio_set_level((gpio_num_t)BLINK_GPIO, ledus);
        kkk=0;
        int current_phone_status = checkPhoneActivityStatus(parametri_globali.parametro_feedback);
        if (current_phone_status == 0) {
            ESP_LOGI(TAG,"No call in progress");
            //digitalWrite(red, LOW); // red LED off
        } else if (current_phone_status == -1){
            ESP_LOGE(TAG,"Not possible to determine Phone Activity Status!!!");
            // QUI PUOI METTERE UN TENTATUVI DI RESET AGENDO SU D9 e CON UN reset_module (che forse la farai già che agisce su D9)
            if (simOK(UART_NUM_1,&parametri_globali)==-1){
                ESP_LOGE(TAG,"\nSim808 module not found, stop here!");
                foreverRed();
            } 
        } else if (current_phone_status == 3){ 
            printf("RINGING\n");
            //RINGING
            /*if (checkCallingNumber()==0){
                ESP_LOGI(TAG,"\n********Valid calling number: %s. Answering call.....",parametri_globali.parametro_feedback);;
                char msg1[4]="ATA";
                char condition1[5]="OK\r\n";
                parametri_globali.calling_number_valid=-1;
                parametri_globali.calling_number[0]=0;
                if (verificaComando(UART_NUM_1,msg1,condition1,&parametri_globali)==0){
                        ESP_LOGI(TAG,"\n********Answered call!!!");
                        char msg2[15]="AT+VTS=\"1,0,6\"";
                        char condition2[5]="OK\r\n";
                        //if (verificaComando(UART_NUM_1,"AT+VTS=\"{1,1},{0,2},{6,1}\"","OK\r\n",&parametri_globali)==0){
                        if (verificaComando(UART_NUM_1,msg2,condition2,&parametri_globali)==0){
                            ESP_LOGI(TAG,"\n********Answer tones sent!!!");
                            vTaskDelay(500 / portTICK_RATE_MS);
                        } else {
                            ESP_LOGE(TAG,"\n********ERROR in sending answering tones!!!");
                        }
                    } else {
                        ESP_LOGE(TAG,"\n********Error in answering allowed call!!!");
                    }
            } else {
                parametri_globali.calling_number_valid=-1;
                parametri_globali.calling_number[0]=0;
                ESP_LOGE(TAG,"\n********Calling number %s NOT valid!!! Answering and hanging right after.",parametri_globali.parametro_feedback);
                char msg3[4]="ATA";
                char condition3[5]="OK\r\n";
                if (verificaComando(UART_NUM_1,msg3,condition3,&parametri_globali)==0){
                        ESP_LOGI(TAG,"\n********Answered NOT allowed call!!!");
                        vTaskDelay(2000 / portTICK_RATE_MS);
                        char msg4[4]="ATH";
                        char condition4[5]="OK\r\n";                       
                        if (verificaComando(UART_NUM_1,msg4,condition4,&parametri_globali)==0){
                            ESP_LOGI(TAG,"\n********Hanged call!!!");
                            vTaskDelay(1000 / portTICK_RATE_MS);
                        } else {
                            ESP_LOGE(TAG,"\n********ERROR in hanging NOT allowed call!!!");
                        }
                    } else {
                        ESP_LOGE(TAG,"\n********Error in answering NOT allowed call!!!");
                    }
            }
        } else if (current_phone_status == 4) { // in call 
                ESP_LOGW(TAG,"Handling Call....");
                //digitalWrite(red, HIGH); // red LED on
                if (parametri_globali.pending_DTMF>=0){
                    int DTMF=parametri_globali.pending_DTMF;
                    parametri_globali.pending_DTMF=-1;
                    ESP_LOGW(TAG,"Tone detected equals %d. Replying with the same tone.",DTMF);;
                    char replyTone[30];
                    sprintf(replyTone,"AT+VTS=\"{%d,2}\"",DTMF);
                    vTaskDelay(2000 / portTICK_RATE_MS);
                    char condition5[5]="OK\r\n";
                    if (verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali)==0){
                                ESP_LOGI(TAG,"\nReply tone sent!!!");
                                if(write_nrf24(myRadio,DTMF)==true){
                                    ESP_LOGI(TAG,"\nMessage sent to SlaveStation was succesffully delivered!!!");   
                                    sprintf(replyTone,"AT+VTS=\"{%d,1}\"",DTMF);
                                    verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                                    verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                                    verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali); 
                                } else {
                                    ESP_LOGE(TAG,"\nMessage sent to SlaveStation was NOT delivered!!!");    
                                    //verificaComando(UART_NUM_1,replyTone,condition5,&parametri_globali);
                                }
                            } else {
                                ESP_LOGE(TAG,"\nERROR in sending reply tone!!!");
                            }
                } else 
                {
                    printf("\nnop\n");
                }*/
        }
    }

}

// Main application
void app_main() {
    setup();
    ESP_LOGE(TAG,"Entering while loop!!!!!\r\n");
    while(1)
        loop();
    fflush(stdout);
    }
