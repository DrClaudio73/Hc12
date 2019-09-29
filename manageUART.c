#include "manageUART.h"

void scriviUART(uart_port_t uart_controller, char* text){
    uart_write_bytes(uart_controller, text, strlen(text));    
    //printf("%s ---- %d",text,strlen(text));
    return;
}

// read a line from the UART controller
char* read_line(uart_port_t uart_controller) {
    static char line[2048];
    char *ptr = line;
    //printf("\nread_line on UART: %d\n", (int) uart_controller);
    while(1) {
        int num_read = uart_read_bytes(uart_controller, (unsigned char *)ptr, 1, 45/portTICK_RATE_MS);//portMAX_DELAY);
        if(num_read == 1) {
            // new line found, terminate the string and return 
            //printf("received char: %c", *ptr);
            if(*ptr == '\n') {
                ptr++;
                *ptr = '\0';
                return line;
            }
            // else move to the next char
            ptr++;
        } else {
            //printf("num_read=%d",num_read);
            *ptr=10;
            ptr++;
            *ptr=0;
            return line;
        } 
    }
}