#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h> 


/* size of the stack for each thread */ 
#define STACKSIZE 1024

#define PRIORITY 7

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)


#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "the board doesn't have an allias that matches led0"
#endif

#if !DT_NODE_HAS_STATUS(LED1_NODE,okay)
#error "the board doesn't have an allias that matches led0"
#endif

//define the data package to be inserted into the fifo by each thread every time an event happens 
struct printk_data_t {
	void *fifo_reserved; //the first word most be reserved in the fifo <-- test this later
	uint32_t led;
	uint32_t cnt; 
};

//initialize the FIFO
K_FIFO_DEFINE(printk_fifo);

//define a structure that represents the LEDs 
struct led {
	struct gpio_dt_spec spec;
	const char *gpio_pin_name;
};

//initialize my leds structures
static const struct led led0 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED0_NODE,gpios,{0}),
	.gpio_pin_name = DT_PROP_OR(LED0_NODE,label,""),
};

static const struct led led1 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED1_NODE,gpios,{}),
	.gpio_pin_name = DT_PROP_OR(LED1_NODE,label,""),
};


void blink(const struct led * led, uint32_t sleep_time_ms, uint32_t id)
{
	const struct gpio_dt_spec *spec = &led->spec;
	int cnt = 0;
	int ret; 

	//check to see if the gpio peripherial has been turned on
	if(!device_is_ready(spec->port)){
		printk("Error: %s device is not ready", spec->port->name);
		return;
	}

	// check to see if the configuration of the port was successful 
	ret = gpio_pin_configure_dt(spec,GPIO_OUTPUT);
	if(ret != 0){
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
					ret,spec->pin,led->gpio_pin_name);
		return;
	}

	while(1){

		gpio_pin_set(spec->port,spec->pin,cnt % 2 );

		struct printk_data_t tx_data = {.led = id, .cnt = cnt};

		size_t size = sizeof(struct printk_data_t);

		char *memptr = k_malloc(size);

		__ASSERT_NO_MSG(memptr != 0 );

		memcpy(memptr,&tx_data,size);

		k_fifo_put(&printk_fifo,memptr);

		k_msleep(sleep_time_ms);

		cnt++;

	}
}


void blink0(void){
	blink(&led0,100,0);
}

void blink1(void){
	blink(&led1,500,1);
}

void uart_out(void){
	
	while(1){
		struct printk_data_t *rx_data = k_fifo_get(&printk_fifo,K_FOREVER); // pull the data out of the fifo

		printk("Toggled led %d. Counter = %d\n",rx_data->led,rx_data->cnt);

		k_free(rx_data);

	}

}

K_THREAD_DEFINE(blink0_id,STACKSIZE,blink0,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(blink1_id,STACKSIZE,blink1,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_out_id,STACKSIZE,uart_out,NULL,NULL,NULL,PRIORITY,0,0);