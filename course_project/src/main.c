/*Perjantai 28/8/26: Toteutettu ledien välkkyminen järjestyksessä (punainen, keltainen, vihreä)
ledit ovat päällä 1 sekunnin jonka jälkeen väri vaihtuu */

/*Keskiviikko 09/09/26: Huomasin virheen toteutuksessa erittelin Led_taskin eri taskeiksi nyt joka värillä on omansa
 ja lisätty keskeytys nappi*/

//TODO: Lisää koodiin PAUSE-toiminti sekä manuaalinen ohjaus

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Pin configurations
static const struct gpio_dt_spec red    = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green  = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue   = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;

// Thread parameters
#define STACKSIZE 500
#define PRIORITY 5

// State machine variables
volatile int led_state = 0;
volatile int prev_state = 0;

// Function prototypes
void red_task(void *, void *, void *);
void yellow_task(void *, void *, void *);
void green_task(void *, void *, void *);
int init(void);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_task,NULL,NULL,NULL,PRIORITY,0,0);

// Main program
int main(void)
{
	init();
	return 0;
}

// Button interupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	printk("Button pressed\n");
	if (led_state != 4){
		prev_state = led_state;
		led_state = 4;
	} 
	else {
		led_state = prev_state;
	}
}

// Initialize leds
int  init(void) {
    // Button pin initialization
    if (!gpio_is_ready_dt(&button)) {
        printk("Error: button device not ready\n");
        return -1;
    }

    int ret_btn = gpio_pin_configure_dt(&button, GPIO_INPUT | button.dt_flags);
    if (ret_btn < 0) return ret_btn;

    ret_btn = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    if (ret_btn < 0) return ret_btn;

    gpio_init_callback(&button_cb_data, button_0_handler, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	int ret1 = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret1 < 0) {
		printk("Error: Green led configure failed\n");
		return ret1;
	}
	int ret2 = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
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

// LED Tasks to handle colors
void red_task(void *, void *, void*) {
	while (true) {
        if (led_state == 0) {
            gpio_pin_set_dt(&red, 1);
            gpio_pin_set_dt(&green, 0);
            gpio_pin_set_dt(&blue, 0);
            printk("led red\n");
            
            k_sleep(K_SECONDS(1));
            
            if (led_state == 0) { // Varmistetaan ettei tila ole muuttunut
                led_state = 1;    // Siirrytään suraavaan tilaan
            }
        } else {
            k_msleep(100); //Odottaa jos Pause
        }
    }
}

void yellow_task(void *, void *, void*) {
	while (true) {
        if (led_state == 1) {
            gpio_pin_set_dt(&red, 1);
            gpio_pin_set_dt(&green, 1);
            gpio_pin_set_dt(&blue, 0);
            printk("led yellow\n");
            
            k_sleep(K_SECONDS(1));
            
            if (led_state == 1) {
                led_state = 2;    // Siirrytään vihreään
            }
        } else {
            k_msleep(100);
        }
    }
}

void green_task(void *, void *, void*) {
	while (true) {
        if (led_state == 2) {
            gpio_pin_set_dt(&red, 0);
            gpio_pin_set_dt(&green, 1);
            gpio_pin_set_dt(&blue, 0);
            printk("led green\n");
            
            k_sleep(K_SECONDS(1));
            
            if (led_state == 2) {
                led_state = 0;    // Palataan punaiseen
            }
        } else {
            k_msleep(100);
        }
    }
}