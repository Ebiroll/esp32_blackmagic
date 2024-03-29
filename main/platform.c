/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
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
#include "general.h"
#include "gdb_if.h"
#include "version.h"

#include "gdb_packet.h"
#include "gdb_main.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#include "morse.h"
#include "driver/gpio.h"

#include <assert.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "platform.h"

//#include <dhcpserver.h>

//#define ACCESS_POINT_MODE
#define AP_SSID	 "blackmagic"
#define AP_PSK	 "blackmagic"

//static char pbuf[GDB_PACKET_BUFFER_SIZE + 1U] __attribute__((aligned(8)));


//  | (1<<TMS_PIN) | (1<<TDI_PIN) | (1<<TDO_PIN) | (1<<TCK_PIN)
#define GPIO_OUTPUT_PIN_SEL  ((1<<SWCLK_PIN) | (1<<SWDIO_PIN))

uint32_t target_clk_divider = 0;

void pins_init() {

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/17
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void platform_init()
{

	pins_init();

}


void platform_nrst_set_val(bool assert)
{
	//gpio_set_val(NRST_PORT, NRST_PIN, !assert);
}

bool platform_nrst_get_val(void)
{
	return (gpio_get(NRST_PORT, NRST_PIN)) ? false : true;
}

void platform_srst_set_val(bool assert)
{
	(void)assert;
}

bool platform_srst_get_val(void) { return false; }

const char *platform_target_voltage(void)
{
	return "not supported on this platform.";
}

uint32_t platform_time_ms(void)
{
	//return xTaskGetTickCount() / portTICK_PERIOD_MS;
	int64_t time_milli=esp_timer_get_time()/1000;
	return((uint32_t)time_milli);
}

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_PERIOD_MS)

void platform_delay(uint32_t ms)
{
	vTaskDelayMs(ms);
}

int platform_hwversion(void)
{
	return 0;
}

void platform_target_clk_output_enable(bool enable)
{
	(void)enable;
}


void platform_max_frequency_set(const uint32_t frequency)
{
}

uint32_t platform_max_frequency_get(void)
{
	uint32_t result = 0;
	//uint32_t result = rcc_ahb_frequency;
	//result /= USED_SWD_CYCLES + CYCLES_PER_CNT * target_clk_divider;
	return result;
}


/* This is a transplanted main() from main.c */
void main_task(void *parameters)
{
	(void) parameters;

	platform_init();

	while (true) {

		volatile struct exception e;
		TRY_CATCH(e, EXCEPTION_ALL) {
  			    char* pbuf=gdb_packet_buffer();
			    SET_IDLE_STATE(true);
				size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
				// If port closed and target detached, stay idle				
				if (pbuf[0] != '\x04' || cur_target)
					SET_IDLE_STATE(false);
				gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
			}
		if (e.type) {
			gdb_putpacketz("EFF");
			target_list_free();
			morse("TARGET LOST.", 1);
		}
	}

	/* Should never get here */
}

void user_init(void)
{

#if 0
	uart_set_baud(0, 460800);
	printf("SDK version:%s\n", sdk_system_get_sdk_version());

#ifndef ACCESS_POINT_MODE
	struct sdk_station_config config = {
		.ssid = WIFI_SSID,
		.password = WIFI_PASS,
	};

	sdk_wifi_set_opmode(STATION_MODE);
	sdk_wifi_station_set_config(&config);
#else

	/* required to call wifi_set_opmode before station_set_config */
	sdk_wifi_set_opmode(SOFTAP_MODE);

	struct ip_info ap_ip;
	IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
	IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
	IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
	sdk_wifi_set_ip_info(1, &ap_ip);

	struct sdk_softap_config ap_config = {
		.ssid = AP_SSID,
		.ssid_hidden = 0,
		.channel = 3,
		.ssid_len = strlen(AP_SSID),
		.authmode = AUTH_OPEN, //AUTH_WPA_WPA2_PSK,
		.password = AP_PSK,
		.max_connection = 3,
		.beacon_interval = 100,
	};
	sdk_wifi_softap_set_config(&ap_config);

	ip_addr_t first_client_ip;
	IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
	dhcpserver_start(&first_client_ip, 4);

#endif
#endif
	xTaskCreate(&main_task, "main", 4*256, NULL, 2, NULL);
}
