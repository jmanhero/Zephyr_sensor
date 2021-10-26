#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS 500

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0 	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN 	DT_GPIO_PIN(LED0_NODE,gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE,gpios)
#else

//any build error here mean something in the board isn't set up to blink an led
#error "unsupported board: LED devicetree alias is no defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif




void main(void)
{



	const struct device *dev;
	bool led_is_on = true; 
	int ret;

	dev = device_get_binding(LED0);
	if( dev == NULL)
	{
		return;
	}

		ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if(ret < 0)
	{
		return;
	}

	while(1)
	{
		gpio_pin_set(dev,PIN,(int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}


