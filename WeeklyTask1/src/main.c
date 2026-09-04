/*Perjantai 28/8/26: Toteutettu ledien välkkyminen järjestyksessä (punainen, keltainen, vihreä)
ledit ovat päällä 1 sekunnin jonka jälkeen väri vaihtuu */



#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
void led_task(void *, void *, void*);
int init_led();
int led_state = 0;

K_THREAD_DEFINE(led_thread,STACKSIZE,led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(init_thread,STACKSIZE,init_led,NULL,NULL,NULL,PRIORITY,0,0);

// Main program
int main(void)
{
	init_led();

	return 0;
}

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	int ret1 = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	int ret2 = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);

	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	if (ret1 < 0) {
		printk("Error: Green led configure failed\n");
		return ret1;
	}
	if (ret2 < 0) {
		printk("Error: Blue led configure failed\n");
		return ret2;
	}
	// set led off
	gpio_pin_set_dt(&red,0);
	gpio_pin_set_dt(&green,0);
	gpio_pin_set_dt(&blue,0);

	printk("Led initialized ok\n");
	
	return 0;
}

// Task to handle led blinking and sequenceing
void led_task(void *, void *, void*) {

	//LED Changing in sequence (red, yellow, green)
	printk("led thread started\n");
	while (true) {
		switch (led_state) {
			case 0:
				led_state = 1;	
				gpio_pin_set_dt(&red,1);
				gpio_pin_set_dt(&green,0);
				gpio_pin_set_dt(&blue,0);
				printk("led red\n");
				k_sleep(K_SECONDS(1));
				break;
			case 1:
				led_state = 2;
				gpio_pin_set_dt(&red,1);
				gpio_pin_set_dt(&green,1);
				gpio_pin_set_dt(&blue,0);
				printk("led yellow\n");
				k_sleep(K_SECONDS(1));
				break;
			case 2:
				led_state = 0;
				gpio_pin_set_dt(&red,0);
				gpio_pin_set_dt(&green,1);
				gpio_pin_set_dt(&blue,0);
				printk("led green\n");
				k_sleep(K_SECONDS(1));
				break;
		}
	}


	/* LED blinking white
	printk("led thread started\n");
	while (true) {
		// 1. set led on 
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		gpio_pin_set_dt(&blue,1);
		printk("led on\n");
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		// 3. set led off
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&blue,0);
		printk("led off\n");
		// 4. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
	
	}*/
}