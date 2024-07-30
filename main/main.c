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

/* Provides main entry point.  Initialise subsystems and enter GDB
 * protocol loop.
 */


#include "general.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#include "morse.h"
#define BIT0
#include "libopencm3/cm3/common.h"


#include "command.h"
#ifdef ENABLE_RTT
#include "rtt.h"
#endif


#if __has_include("esp_idf_version.h")
#include "esp_idf_version.h"
#else
#include "esp_event_loop.h"
#endif

#include "esp_wifi.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"

//
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

unsigned short gdb_port = 2345;
#include "platform.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

#define EXAMPLE_WIFI_SSID "change_this"
#define EXAMPLE_WIFI_PASS "secret"

#define EXAMPLE_ESP_WIFI_SSID "blackmagic"
#define EXAMPLE_ESP_WIFI_PASS "sesam1234"
#define EXAMPLE_ESP_WIFI_CHANNEL  7
#define  EXAMPLE_MAX_STA_CONN 3


#define AP_MODE 1

/* This has to be aligned so the remote protocol can re-use it without causing Problems */
static char pbuf[GDB_PACKET_BUFFER_SIZE + 1U] __attribute__((aligned(8)));

char *gdb_packet_buffer()
{
	return pbuf;
}

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD  WIFI_AUTH_WEP 
#define ESP_WIFI_SAE_MODE                  ESP_WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER             ""
static const char *TAG = "blackmagic";




// extern 
void set_gdb_socket(int socket);
void set_gdb_listen(int socket);

static int s_retry_num = 0;

static int already_connected=0;

#define EXAMPLE_ESP_MAXIMUM_RETRY  10


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static esp_err_t event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            if (already_connected==0) {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            }
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        already_connected=1;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}
static void initialise_wifi(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_UNSPECIFIED,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

}


void wifi_init_softap(void)
{

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
//#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
//            .authmode = WIFI_AUTH_WPA3_PSK,
//            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
//#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
//#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


void old_gdb_application_thread(void *pvParameters)
{
	int sock, new_sd;
	struct sockaddr_in address, remote;
	int size;

    ESP_LOGI(TAG, "create socket");
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(gdb_port);
	address.sin_addr.s_addr = INADDR_ANY;

    ESP_LOGI(TAG, "bind");
	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
		return;

	listen(sock, 0);
    set_gdb_listen(sock);

	size = sizeof(remote);

    ESP_LOGI(TAG, "accept");

	while (1) {
		if ((new_sd = accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0) {
			    printf("accepted new gdb connection 1\n");
                set_gdb_socket(new_sd);
                gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
	        }
	}
}


static void bmp_poll_loop(void)
{
	SET_IDLE_STATE(false);
	while (gdb_target_running && cur_target) {
		gdb_poll_target();

		// Check again, as `gdb_poll_target()` may
		// alter these variables.
		if (!gdb_target_running || !cur_target)
			break;
		char c = gdb_if_getchar_to(0);
		if (c == '\x03' || c == '\x04')
			target_halt_request(cur_target);
		platform_pace_poll();
#ifdef ENABLE_RTT
		if (rtt_enabled)
			poll_rtt(cur_target);
#endif
	}

	SET_IDLE_STATE(true);
	size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
	// If port closed and target detached, stay idle
	if (pbuf[0] != '\x04' || cur_target)
		SET_IDLE_STATE(false);
	gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
}

static void bad_bmp_poll_loop(void)
{
	SET_IDLE_STATE(false);
	while (gdb_target_running && cur_target) {
		gdb_poll_target();

		// Check again, as `gdb_poll_target()` may
		// alter these variables.
		if (!gdb_target_running || !cur_target)
			break;
		char c = gdb_if_getchar_to(0);
		if (c == '\x03' || c == '\x04')
			target_halt_request(cur_target);
		platform_pace_poll();
#ifdef ENABLE_RTT
		if (rtt_enabled)
			poll_rtt(cur_target);
#endif
	}

    while (1) {
        SET_IDLE_STATE(true);

        //char c = gdb_if_getchar_to(0);
		//if (c  == '\x03' || c  == '\x04') {
		//	target_halt_request(cur_target);
        //    printf("halt");
        //}
        //if (c!=0) {
        //    pbuf[0]=c;
        //}

        size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
		if (pbuf[0]  == '\x03' || pbuf[0]  == '\x04') {
			target_halt_request(cur_target);
            if (cur_target) printf("halting\n");
            int retries=6;

            target_addr_t watch;
	        target_halt_reason_e reason = target_halt_poll(cur_target, &watch);
	        while (!reason && retries-->0) {
                reason = target_halt_poll(cur_target, &watch);
            }
            gdb_poll_target();
        }
        // If port closed and target detached, stay idle
        if (pbuf[0] != '\x04' || cur_target)
            SET_IDLE_STATE(false);

        gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);

    }
}

static void main_loop(void)
{
     while (1) {
		volatile exception_s e;
		TRY_CATCH (e, EXCEPTION_ALL) {
			bmp_poll_loop();
		}
		if (e.type) {
			gdb_putpacketz("EFF");
			target_list_free();
			gdb_outf("Uncaught exception: %s\n", e.msg);
			morse("TARGET LOST.", true);
		}
     }

}


static void gdb_application_thread(void *pvParameters)
{
    char addr_str[128];
    int addr_family = AF_INET; // (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = 5;
    int keepInterval = 10;
    int keepCount = 4;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(gdb_port);
        ip_protocol = IPPROTO_TCP; // IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    //int opt = 1;
    //setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    // setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", gdb_port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        set_gdb_listen(listen_sock);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);
        printf("accepted new gdb connection\n");
        set_gdb_socket(sock);
        main_loop();
        //bmp_poll_loop();

        //SET_IDLE_STATE(true);
        //size_t size =  gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
        // If port closed and target detached, stay idle
	    //if (pbuf[0] != '\x04' || cur_target)
		//    SET_IDLE_STATE(false);
        //gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void main_task(void *parameters);


void app_main()
{

    esp_err_t ret; 
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
       ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        printf("no free pages in nvs");
    }

    ESP_ERROR_CHECK( ret );



#ifndef AP_MODE
    ESP_LOGI(TAG, "Normal wifi mode");
    initialise_wifi();
#else
    ESP_LOGI(TAG, "Soft AP mode");
    wifi_init_softap();
#endif

	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
						false, true, portMAX_DELAY);

	EventBits_t uxBits=xEventGroupWaitBits(wifi_event_group, WIFI_FAIL_BIT,
						false, true, 4000 / portTICK_PERIOD_MS);

    if (( uxBits & WIFI_FAIL_BIT ) != 0) {

        ESP_LOGI(TAG, "Late fail");

        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                            false, true, portMAX_DELAY);

    }

	ESP_LOGI(TAG, "Connected to AP");

	platform_init();

    xTaskCreate(&gdb_application_thread, "gdb_thread", 4*4096, NULL, 17, NULL);


   //xTaskCreate(&main_task, "main_task", 4*4096, NULL, 17, NULL);

    for(;;) {
        	EventBits_t uxBits=xEventGroupWaitBits(wifi_event_group, WIFI_FAIL_BIT,
						false, true, 20000);

        if (( uxBits & WIFI_FAIL_BIT ) != 0) {
            s_retry_num=0;
            esp_wifi_stop();
            esp_wifi_start();

            uxBits=xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
						false, true, portMAX_DELAY);

            ESP_LOGI(TAG, "Restarted WIFI");

            //platform_init();

           // ESP_ERROR_CHECK(esp_wifi_start() );


        }
    }
#if 0
	while (true) {
		volatile struct exception e;
		TRY_CATCH(e, EXCEPTION_ALL) {
			gdb_main();
		}
		if (e.type) {
			gdb_putpacketz("EFF");
			target_list_free();
			morse("TARGET LOST.", 1);
		}
	}
#endif
	/* Should never get here */
	return ;
}

