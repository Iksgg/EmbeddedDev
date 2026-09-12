/*Perjantai 28/8/26: Toteutettu ledien välkkyminen järjestyksessä (punainen, keltainen, vihreä)
ledit ovat päällä 1 sekunnin jonka jälkeen väri vaihtuu */

/*Keskiviikko 09/09/26: Huomasin virheen toteutuksessa erittelin Led_taskin eri taskeiksi nyt joka värillä on omansa
 ja lisätty keskeytys nappi*/

 /*Keskiviiko 09/09/26: Muokattu koodi alustavasti vastaamaan Viikon 2 1p vaatimuksia*/

//TODO: Lisää koodiin manuaalinen ohjaus toteuta tulevien viikkojen tavoitteet

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

// FIFO
struct uart_msg{
    void *fifo_reserved;
    char color;
    uint32_t duration_ms;
};

static struct k_fifo seq_fifo;

// Mutex and Condition variables
static struct k_mutex color_mutex;
static struct k_condvar red_cv;
static struct k_condvar yellow_cv;
static struct k_condvar green_cv;
static struct k_condvar release_cv;

// State and sync for dispatcher
static volatile char current_trgt = 0;
static volatile uint32_t current_duration = 1000;
static volatile bool task_release = true;

// Led pin configurations
static const struct gpio_dt_spec red   = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue  = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// UART configuration
static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

// Thread parameters
#define DISPATCHER_STACKSIZE 1024
#define STACKSIZE 500
#define PRIORITY 5

// Function prototypes
int init(void);
void red_task(void *, void *, void *);
void yellow_task(void *, void *, void *);
void green_task(void *, void *, void *);
void dispatcher_task(void *, void *, void *);
void uart_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(dispatcher_thread,DISPATCHER_STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);

// Main program
int main(void)
{
	init();
	return 0;
}

// Button interupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	printk("Button pressed\n");
	/*if (led_state != 4){
		prev_state = led_state
		led_state = 4
	} 
	else {
		led_state = prev_state;
	}*/
}

// Initialize leds
int  init(void) {

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
        k_mutex_lock(&color_mutex, K_FOREVER);
        // Wait to run
        while (current_trgt != 'R' && current_trgt != 'r') {
            k_condvar_wait(&red_cv, &color_mutex, K_FOREVER);
        }
        k_mutex_unlock(&color_mutex);

        // Change LED color
        gpio_pin_set_dt(&red, 1);
        gpio_pin_set_dt(&green, 0);
        gpio_pin_set_dt(&blue, 0);
        printk("led red\n");

        k_msleep(current_duration);

        // Send releas to dispatcher
        k_mutex_lock(&color_mutex, K_FOREVER);
        task_release = true;
        current_trgt = 0;
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&color_mutex);
    }
}

void yellow_task(void *, void *, void*) {
	while (true) {
        k_mutex_lock(&color_mutex, K_FOREVER);
        // Wait to run
        while (current_trgt != 'Y' && current_trgt != 'y') {
            k_condvar_wait(&yellow_cv, &color_mutex, K_FOREVER);
        }
        k_mutex_unlock(&color_mutex);

        // Change LED color
        gpio_pin_set_dt(&red, 1);
        gpio_pin_set_dt(&green, 1);
        gpio_pin_set_dt(&blue, 0);
        printk("led yellow\n");

        k_msleep(current_duration);

        // Send releas to dispatcher
        k_mutex_lock(&color_mutex, K_FOREVER);
        task_release = true;
        current_trgt = 0;
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&color_mutex);
    }
}

void green_task(void *, void *, void*) {
	while (true) {
        k_mutex_lock(&color_mutex, K_FOREVER);
        // Wait to run
        while (current_trgt != 'G' && current_trgt != 'g') {
            k_condvar_wait(&green_cv, &color_mutex, K_FOREVER);
        }
        k_mutex_unlock(&color_mutex);

        // Change LED color
        gpio_pin_set_dt(&red, 0);
        gpio_pin_set_dt(&green, 1);
        gpio_pin_set_dt(&blue, 0);
        printk("led green\n");

        k_msleep(current_duration);

        // Send releas to dispatcher
        k_mutex_lock(&color_mutex, K_FOREVER);
        task_release = true;
        current_trgt = 0;
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&color_mutex);
    }
}

// Dispatcher
void dispatcher_task(void *, void *, void *) {
    k_fifo_init(&seq_fifo);
    k_mutex_init(&color_mutex);
    k_condvar_init(&red_cv);
    k_condvar_init(&yellow_cv);
    k_condvar_init(&green_cv);
    k_condvar_init(&release_cv);

    while (true) {
        // wait for FIFO Buffer
        struct uart_msg *msg = k_fifo_get(&seq_fifo, K_FOREVER);
        if (!msg) continue;

        k_mutex_lock(&color_mutex, K_FOREVER);
        current_trgt = msg -> color;
        current_duration = msg -> duration_ms;
        task_release = false;

        // Releas memory
        k_free(msg);

        // Send signal to led tasks
        if (current_trgt == 'R' || current_trgt == 'r') {
            k_condvar_signal(&red_cv);
        }
        if (current_trgt == 'Y' || current_trgt == 'y') {
            k_condvar_signal(&yellow_cv);
        }
        if (current_trgt == 'G' || current_trgt == 'g') {
            k_condvar_signal(&green_cv);
        }

        // Wait for release signal
        while (!task_release) {
            k_condvar_wait(&release_cv, &color_mutex, K_FOREVER);
        }
        k_mutex_unlock(&color_mutex);

    }
}

void uart_task(void *, void *, void *) {
    char rx_buff[32];
    int buff_idx = 0;
    char c;
    char color = 0;
    uint32_t duration = 0;
    bool has_duration = false;

    while (true) {
        // Read sequense from serial
        if (uart_poll_in(uart_dev, &c) == 0) {
            printk("RX: 0x%02X ('%c')\n", c, (c >= 32 && c <= 126) ? c : '.'); // Debugg print
            if (c == '\n' || c == '\r'){
                if (buff_idx > 0){
                    rx_buff[buff_idx] = '\0';
                    printk("Ajetaan sekvenssi: %s\n", rx_buff); // Debugg print

                    // Run trough the sequense
                    for (int i = 0; i < buff_idx; i++){
                        char seq_char = rx_buff[i];
                        if (seq_char == 'R' || seq_char == 'r' || seq_char == 'Y' || seq_char == 'y' || seq_char == 'G' || seq_char == 'g') {
                            if (color != 0) {
                                if (!has_duration) duration = 1000;
                                struct uart_msg *msg = k_malloc(sizeof(struct uart_msg));
                                if (msg) {
                                    msg -> color = color;
                                    msg -> duration_ms = duration;
                                    k_fifo_put(&seq_fifo, msg);
                                }                            
                            
                            }
                            color = seq_char;
                            duration = 0;
                            has_duration = false;

                        } else if (seq_char >= '0' && seq_char <= '9') {
                            duration = (duration * 10) + (seq_char - '0');
                            has_duration = true;
                        }

                    }
                    if (color != 0) {
                        if (!has_duration) duration = 1000; // Fallback
                        
                        struct uart_msg *msg = k_malloc(sizeof(struct uart_msg));
                        if (msg) {
                            msg->color = color;
                            msg->duration_ms = duration;
                            k_fifo_put(&seq_fifo, msg);
                        }
                    }
                    buff_idx = 0;
                }
            } else {
                if(buff_idx < sizeof(rx_buff) - 1){
                    rx_buff[buff_idx++] = c;
                }
            }

        }
        k_msleep(10);
    }
}