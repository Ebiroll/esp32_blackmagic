/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2019  Olof Åstrand.
 * Written by Olof Åstrand <olof.astrand@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "traceswo.h"

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
//#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <sys/time.h>
#include "platform.h"
#include "driver/uart.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#ifdef PLATFORM_HAS_TRACESWO

static const char *TAG = "traceswo";

char serial_no[9];

int g_baudrate=115200; // 

#define BUF_SIZE 4

#if 0

static uart_dev_t* UART[UART_NUM_MAX] = {&UART0, &UART1, &UART2};


static esp_err_t uart_reset_rx_fifo(uart_port_t uart_num)
{
    //UART_CHECK((uart_num < UART_NUM_MAX), "uart_num error", ESP_FAIL);
    //Due to hardware issue, we can not use fifo_rst to reset uart fifo.
    //See description about UART_TXFIFO_RST and UART_RXFIFO_RST in <<esp32_technical_reference_manual>> v2.6 or later.

    // we read the data out and make `fifo_len == 0 && rd_addr == wr_addr`.
    while(UART[uart_num]->status.rxfifo_cnt != 0 || (UART[uart_num]->mem_rx_status.wr_addr != UART[uart_num]->mem_rx_status.rd_addr)) {
        READ_PERI_REG(UART_FIFO_REG(uart_num));
    }
    return ESP_OK;
}

uint32_t uart_baud_detect(uart_port_t uart_num)
{

    int low_period = 0;
    int high_period = 0;
    uint32_t intena_reg = UART[uart_num]->int_ena.val;
    //Disable the interruput.
    UART[uart_num]->int_ena.val = 0;
    UART[uart_num]->int_clr.val = ~0;
    //Filter
    UART[uart_num]->auto_baud.glitch_filt = 4;
    //Clear the previous result
    UART[uart_num]->auto_baud.en = 0;
    UART[uart_num]->auto_baud.en = 1;
    while(UART[uart_num]->rxd_cnt.edge_cnt < 100) {
        ets_delay_us(10);
    }
    low_period = UART[uart_num]->lowpulse.min_cnt;
    high_period = UART[uart_num]->highpulse.min_cnt;
    // disable the baudrate detection
    UART[uart_num]->auto_baud.en = 0;
    //Reset the fifo;
    uart_reset_rx_fifo(uart_num);
    UART[uart_num]->int_ena.val = intena_reg;
    //Set the clock divider reg
    //UART[uart_num]->clk_div.div_int = (low_period > high_period) ? high_period : low_period;

    //Return the divider. baud = APB / divider;
    return (low_period > high_period) ? high_period : low_period;;
}
#endif

// This task receives serial data over the SWO pin, and prints on serial output for now.. Later to socket
static void routeTask(void *inpar) {
//        rxPin = 1;
//        txPin = 3;
  unsigned char* data;

  uart_port_t uart_num = UART_NUM_1;                                     //uart port number
  uart_config_t uart_config = {
      .baud_rate = g_baudrate,                    //baudrate
      .data_bits = UART_DATA_8_BITS,          //data bit mode
      .parity = UART_PARITY_DISABLE,          //parity mode
      .stop_bits = UART_STOP_BITS_1,          //stop bit mode
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  //hardware flow control(cts/rts)
      .rx_flow_ctrl_thresh = 122,             //flow control threshold
  };
  ESP_LOGI(TAG, "Setting UART configuration number %d...", uart_num);
  ESP_ERROR_CHECK( uart_param_config(uart_num, &uart_config));
  QueueHandle_t uart_queue;

  ESP_ERROR_CHECK( uart_set_pin(uart_num, TRACESWO_DUMMY_TX, TRACESWO_PIN, -1, -1));
  ESP_ERROR_CHECK( uart_driver_install(uart_num, 512 * 2, 512 * 2, 10,  &uart_queue,0));
  data = (uint8_t*) malloc(BUF_SIZE);

  printf("ESP32 traceswo pin %d baudrate %d\n",TRACESWO_PIN,g_baudrate);
  int size;       

  while(1) {
      //uint32_t bd=uart_baud_detect(1);
      //printf("baud %d\n",bd);
      size = uart_read_bytes(uart_num, (unsigned char *)data, 3, 10 / portTICK_PERIOD_MS);

      // TODO, read channel or whatever is in data[0] & data[1] 
      if (size == 1) {
            //printf("%c\n",data[0]);
            printf("1 bytes received %d\n",data[0]);
      }
      if (size == 2) {
            //printf("%c%c\n",data[0],data[1]);
            printf("2 bytes received %d\n",data[0]);
      }
      if (size == 3) {
            printf("%c%c%c",data[0],data[1],data[2]);
            //printf("3\n");
      }
      data[size]=0;
      gdb_outf("%s",data);
      // Keep watchdog happy
      timg_wdtfeed_reg_t feed_me;
      timg_wdtwprotect_reg_t protect_me;
      protect_me.val=1356348065;
      feed_me.val=1;
      //protect_me.val=TIMG_WDT_WKEY_VALUE;
      TIMERG0.wdtwprotect=protect_me;
      TIMERG0.wdtfeed=feed_me;
      protect_me.val=0;
      TIMERG0.wdtwprotect=protect_me;
  }
}

static int task_started=0;



#define OUTPUT_PIN_SEL  (1<<9)

// Test of pin capabilities
void pins_test() {
    int times=20;

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/17
    io_conf.pin_bit_mask = OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;

    //io_conf.mode GPIO_PULLUP_ONLY
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_pull_mode(9, GPIO_PULLUP_ONLY);
    gpio_drive_cap_t strength=GPIO_DRIVE_CAP_DEFAULT;
    gpio_get_drive_capability(8, &strength);

/////  
    if (task_started) {
        //pins_test();
        gpio_set_direction(8, GPIO_MODE_OUTPUT);
        ESP_LOGI(TAG, "Toggling pin 8 %d...", times);
        while(times-->0) {

#if 0
            esp_err_t ret=gpio_set_level(9,1);
            if (ret!=ESP_OK) {
                ESP_LOGI(TAG, "Err setting 15 %d...", ret);
            }
            gpio_set_level(6,0);
            gpio_set_level(7,0);

            vTaskDelayMs(100);
            ret=gpio_set_level(9,0);
            if (ret!=ESP_OK) {
                ESP_LOGI(TAG, "Err resetting pin 15 %d...", ret);
            }
            gpio_set_level(16,6);
            gpio_set_level(14,7);
#endif
            vTaskDelayMs(100);       
         }
    }


}

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_PERIOD_MS)

void traceswo_init(uint32_t baudrate, uint32_t swo_chan_bitmask) {

    if (task_started) {

        ESP_LOGI(TAG, "Trace already started...");
        return;
    }

    if (baudrate>0) {
        g_baudrate=baudrate;       
    }
    strcpy(serial_no,"esp32");

    task_started = 1;
    xTaskCreate(&routeTask, "swo_thread", 4*4096, NULL, 8, NULL);

}

#endif

//void trace_buf_drain(usbd_device *dev, uint8_t ep);

