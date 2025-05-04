
#include "stm_port.h"
#include "driver/uart.h"
#include "platform.h"
#include "esp_log.h"
#include <string.h>


static const char *TAG = "esp32_port";



int port_baudrate = 9600;
uart_port_t uart_num = UART_NUM_1;                                     //uart port number
QueueHandle_t uart_queue;
int driver_initiated=0;

static port_err_t esp_open(struct stm_port_interface *port,
			   struct port_options *ops)
{

	  unsigned char* data;

  if (driver_initiated==1) {
	return PORT_ERR_OK;
  }

  
  uart_config_t uart_config = {
      .baud_rate = port_baudrate,                    //baudrate
      .data_bits = UART_DATA_8_BITS,          //data bit mode
      .parity = UART_PARITY_EVEN,          //parity mode
      .stop_bits = UART_STOP_BITS_1,          //stop bit mode
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  //hardware flow control(cts/rts)
      .rx_flow_ctrl_thresh = 122,             //flow control threshold
  };
  ESP_LOGI(TAG, "Setting UART configuration number %d...", uart_num);
  ESP_ERROR_CHECK( uart_param_config(uart_num, &uart_config));

  ESP_ERROR_CHECK( uart_set_pin(uart_num, TRACESWO_DUMMY_TX, TRACESWO_PIN, -1, -1));
  ESP_ERROR_CHECK( uart_driver_install(uart_num, 512 * 2, 512 * 2, 10,  &uart_queue,0));

  ESP_LOGI(TAG,"ESP32 stm_probe pin %d baudrate %d\n",TRACESWO_PIN,port_baudrate);

  driver_initiated=1;
  return PORT_ERR_OK;
}

char merged_string[256];
void send_to_uart(int argc, const char **argv) {
	// Merge all input strings into one
	merged_string[0] = '\0';
	for (int i = 1; i < argc; ++i) {
		strcat(merged_string, argv[i]);
		if (i < argc - 1) {
			strcat(merged_string, " ");
		}
	}	
	uart_write_bytes(uart_num, (const char *) merged_string, strlen(merged_string));
	uart_flush(uart_num);
}

static port_err_t esp_close(struct stm_port_interface *port)
{
	//struct i2c_priv *h;
	

	//uart_driver_delete(uart_num);


	return PORT_ERR_OK;
}


static port_err_t esp_read(struct stm_port_interface *port, void *buf,
			   size_t nbyte)
{

	uint8_t *pos = (uint8_t *)buf;
	int r;
	ESP_LOGI(TAG,"Reading %d",nbyte);

	while (nbyte) {
		r = uart_read_bytes(uart_num, (unsigned char *)pos, nbyte, 8000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG,"Read %d %X",r,*pos);

		if (r == 0)
			return PORT_ERR_TIMEDOUT;
		if (r < 0)
			return PORT_ERR_UNKNOWN;

		nbyte -= r;
		pos += r;
	}

	return PORT_ERR_OK;
}

static port_err_t esp_flush(struct stm_port_interface __unused *port)
{
	/* We shouldn't need to flush */
	uart_flush(uart_num);
	return PORT_ERR_OK;
}

static port_err_t esp_write(struct stm_port_interface *port, void *buf,
			    size_t nbyte)
{	
	const uint8_t *pos = (const uint8_t *)buf;
	int r;
	ESP_LOGI(TAG,"writing %d bytes %X",nbyte,*pos);

	while (nbyte) {
		r = uart_write_bytes(uart_num, (const char *) pos, nbyte);
		if (r < 1)
			return PORT_ERR_UNKNOWN;

		nbyte -= r;
		pos += r;
	}
	uart_flush(uart_num);


	return PORT_ERR_OK;
}


#if 0
static port_err_t esp_gpio(struct port_interface __unused *port,
			   serial_gpio_t __unused n,
			   int __unused level)
{
	return PORT_ERR_OK;
}
#endif

static const char *esp_get_cfg_str(struct stm_port_interface *port)
{
	struct i2c_priv *h;
	static char str[11];

	h = (struct i2c_priv *)port->private;
	if (h == NULL)
		return "INVALID";
	//snprintf(str, sizeof(str), "addr 0x%2x", h->addr);
	return str;
}

static struct varlen_cmd esp_cmd_get_reply[] = {
	// {0x10, 11},
	// {0x11, 17},
	// {0x12, 18},
	// { /* sentinel */ }
};

struct stm_port_interface port_esp32 = {
	.name	= "esp32",
	.flags	= PORT_BYTE | PORT_GVR_ETX | PORT_CMD_INIT | PORT_RETRY,
	.open	= esp_open,
	.close	= esp_close,
	.flush  = esp_flush,
	.read	= esp_read,
	.write	= esp_write,
	.cmd_get_reply	= esp_cmd_get_reply,
	.get_cfg_str	= esp_get_cfg_str,
};



stm_port_interface_t *set_stm_port() {
	return &port_esp32;
}
