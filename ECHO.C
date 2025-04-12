#include <stdio.h>
#include <string.h>

#include "xparameters.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "sleep.h"

#include "lwip/err.h"
#include "lwip/tcp.h"

#define GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_CHANNEL    1
#define TCP_PORT        7

XGpio gpio;

void print_app_header()
{
	xil_printf("\n\r----- lwIP TCP + GPIO Server ------\n\r");
	xil_printf("Send data to port %d to write to AXI GPIO\n\r", TCP_PORT);
}

err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	tcp_recved(tpcb, p->len);

	if (p->len >= 1) {
		u8 data = ((u8*)p->payload)[0];
		XGpio_DiscreteWrite(&gpio, GPIO_CHANNEL, 0x00);  // <-- This is the GPIO write line
		xil_printf("Received data: 0x%02x -> GPIO\n\r", data);
	}

	if (tcp_sndbuf(tpcb) >= p->len) {
		err = tcp_write(tpcb, p->payload, p->len, 1);
	} else {
		xil_printf("No space in tcp_sndbuf\n\r");
	}

	pbuf_free(p);
	return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;
	tcp_recv(newpcb, recv_callback);
	tcp_arg(newpcb, (void*)(UINTPTR)connection);
	connection++;
	return ERR_OK;
}

int start_application()
{
	struct tcp_pcb *pcb;
	err_t err;

	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", TCP_PORT, err);
		return -2;
	}

	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	tcp_accept(pcb, accept_callback);
	xil_printf("TCP server listening on port %d\n\r", TCP_PORT);
	return 0;
}

int init_gpio()
{
	int status;
	XGpio_Config *cfg;

	cfg = XGpio_LookupConfig(GPIO_DEVICE_ID);
	if (cfg == NULL) {
		xil_printf("ERROR: XGpio_LookupConfig failed.\n\r");
		return XST_FAILURE;
	}

	status = XGpio_CfgInitialize(&gpio, cfg, cfg->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("ERROR: XGpio_CfgInitialize failed.\n\r");
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&gpio, GPIO_CHANNEL, 0x00);  // Set as output
	return XST_SUCCESS;
}

void setup_tcp_server()
{
	struct tcp_pcb  *pcb = tcp_new();
	tcp_bind(pcb , IP_ADDR_ANY  , TCP_PORT);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, accept_callback);
}
