/**********************************************************************/
/**
 * @file hello_world/main.c
 * @author Stephan Nolting
 * @brief Classic 'hello world' demo prog
 **************************************************************************/
#include <neorv32.h>
#include <unistd.h>
// #include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "fileproto.pb.h"

/**********************************************************************/
/**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/** Number of RGB LEDs in stripe (24-bit data) */
#define NUM_LEDS_24BIT (12)
#define MAX_INTENSITY (16)
/**@}*/

// #define BUFFER_SIZE 256

int message_length;
bool status;
char key[30];

bool handle_request(pb_istream_t *istream);

int setLED(uint32_t color, int32_t id, char *token);
/**********************************************************************/
/**
 * Main function; prints some fancy stuff via UART.
 *
 * @note This program requires the UART interface to be synthesized.
 *
 * @return 0 if execution was successful
 **************************************************************************/
int main()
{
    // char read_buffer[BUFFER_SIZE];
    /* capture all exceptions and give debug info via UART
    // this is not required, but keeps us safe */
    neorv32_rte_setup();

    /* setup UART at default baud rate, no interrupts */
    neorv32_uart0_setup(BAUD_RATE, 0);

    /* print project logo via UART */
    /* neorv32_aux_print_logo(); */
    // size_t length = 0;
    neorv32_cpu_delay_ms(3000);
    // handle_request();
    char buffer[256];
    size_t length = 0;

    for (;;)
    {
        neorv32_uart0_printf("waiting for message");
        length = neorv32_uart0_scan(buffer, sizeof(buffer), 1);
        neorv32_uart0_printf("\n");

        if (!length)
        {
            neorv32_uart0_printf("empty \n");
            continue;
        } // nothing to be done

        /*neorv32_uart0_printf("Received something %d \n", length);
        for (int i = 0; i < 200; i++)
        {
            neorv32_uart0_printf("%d\n", i);
        }  */
        neorv32_cpu_delay_ms(3000);
        uint8_t message[256];

        /*for (int j = 0; j < length; j++)
        {
            neorv32_uart0_printf("%c", buffer[j]);
        }
        neorv32_uart0_printf("\n"); */
        // Convert the buffer to uint8_t values
        int msg_index = 0;
        int num = 0;
        for (int j = 0; j < length; j++)
        {
            if (buffer[j] >= '0' && buffer[j] <= '9')
            {
                num = num * 10 + (buffer[j] - '0');
            }
            else if (buffer[j] == ' ' || j == length - 1)
            {
                if (buffer[j] != ' ')
                {
                    num = num * 10 + (buffer[j] - '0');
                }
                message[msg_index++] = (uint8_t)num;
                num = 0;
            }
        }

        // Print the converted values
        /*for (int j = 0; j < msg_index; j++)
        {
            neorv32_uart0_printf("%d\n", message[j]);
        }
        neorv32_uart0_printf("\n");

        for (int j = 0; j < msg_index; j++)
        {
            neorv32_uart0_printf("%d \n", message[j]);
        }
        neorv32_uart0_printf("\n");

        // Print raw bytes of the message
        for (int j = 0; j < length; j++)
        {
            neorv32_uart0_printf("%X ", message[j]);
        }

        neorv32_uart0_printf("%d", length); */

        pb_istream_t istream = pb_istream_from_buffer(message, length);
        // pb_ostream_t ostream = pb_ostream_from_buffer(message, sizeof(message));
        neorv32_cpu_delay_ms(5000);
        handle_request(&istream);
    }
    return 0;
}

bool handle_request(pb_istream_t *istream)
{
    uint8_t response_buffer[256];
    char response_message[256];
    Request request = Request_init_zero;
    Response response = Response_init_zero;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t ostream = pb_ostream_from_buffer(response_buffer, sizeof(response_buffer));

    if (!pb_decode_delimited(istream, Request_fields, &request))
    {
        neorv32_uart0_printf("Failed to decode response: %s\n", PB_GET_ERROR(istream));
        return 1;
    }
    neorv32_uart0_putc('a');

    if (request.which_request_type == Request_get_device_info_tag)
    {
        // neorv32_uart0_printf("Device Info\n");
        strcpy(response.response_type.get_device_info.type, "x3xx");
        strcpy(response.response_type.get_device_info.product, "X310");
        strcpy(response.response_type.get_device_info.ip, "192.10.1.1");
        response.which_response_type = Response_get_device_info_tag;
    }
    else if (request.which_request_type == Request_claim_tag)
    {
        // neorv32_uart0_printf("claim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_reclaim_tag)
    {
        // neorv32_uart0_printf("reclaim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_unclaim_tag)
    {
        // neorv32_uart0_printf("unclaim\n");
        strcpy(response.response_type.claim.token, "0000");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_set_smartled_tag)
    {
        // neorv32_uart0_printf("samrtled\n");
        response.which_response_type = Response_claim_tag;
        uint32_t color = request.request_type.set_smartled.color;
        int32_t id = request.request_type.set_smartled.id;
        char token[20];
        strcpy(token, request.request_type.set_smartled.token);
        setLED(color,id,token);
    }
    else
    {
        neorv32_uart0_printf("Wrong request \n");
    }

    status = pb_encode(&ostream, Response_fields, &response);
    message_length = ostream.bytes_written;
    if (!status)
    {
        neorv32_uart0_printf("Encoding failed: %s\n", PB_GET_ERROR(&ostream));
        return 1;
    }
    // neorv32_uart0_printf("%d", message_length);
    neorv32_uart0_putc((char)message_length);

    for (size_t i = 0; i < message_length; i++)
    {
        response_message[i] = (char)response_buffer[i];
    }
    neorv32_uart0_puts(response_message);

    return 0;
}

int setLED(uint32_t color, int32_t id, char *token)
{
    // check if NEOLED unit is implemented at all, abort if not
    if (neorv32_neoled_available() == 0)
    {
        //neorv32_uart0_printf("Error! No NEOLED unit synthesized!\n");
        return 1;
    }
    set_smartledRequest led = set_smartledRequest_init_zero;
    if (strcmp(key, "password"))
    {
        //neorv32_uart0_printf("\n Device is not claimed. Token doesn't match \n\n");
    }
    else
    {
      
        neorv32_neoled_setup_ws2812(0); // interrupt configuration = fire IRQ if TX FIFO is empty (not used here)
        neorv32_neoled_set_mode(0);     // mode = 0 = 24-bit
        // clear all LEDs
        //neorv32_uart0_printf("\n Clearing all LEDs...\n");
        int i;
        for (i = 0; i < NUM_LEDS_24BIT; i++)
        {
            neorv32_neoled_write_blocking(0);
        }

        neorv32_cpu_delay_ms(500);
        led.id = id;
        led.color = color;
        strcpy(led.token, key);
        for (int i = 0; i < NUM_LEDS_24BIT; i++)
        {
            if (i == led.id)
            {
                // Set the specific LED to the desired color
                neorv32_neoled_write_blocking(led.color);
            }
            else
            {
                neorv32_neoled_write_blocking(0); // Turn off other LEDs or set them to a default color
            }
        }
        neorv32_neoled_strobe_blocking();
    }
    return 0;
}
