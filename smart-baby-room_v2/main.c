/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Smart Baby Room application
 *
 * @author      Niccol� Fabrizio Pagano
 * @author      Enrico Verz�
 *
 * @}
 */

#include <stdio.h>
#include "periph/gpio.h"
#include "led.h"

#include "board.h"
#include "xtimer.h"
#include "periph_conf.h"
#include "adc_legacy.h"
#include <inttypes.h>
#include "dht.h"
#include "dht_params.h"

#define DELAY           (100 * 1000U)
#define GREEN_LED	GPIO_PIN(1, 3)
#define YELLOW_LED	GPIO_PIN(1, 4)
#define RED_LED 	GPIO_PIN(1, 5)
#define liv_sonoro 	GPIO_PIN(1, 10)
#define DHT_sensor 	GPIO_PIN(2, 7)

#define HUM_THRESH 	42
#define TEMP_THRESH 	30

enum {
    OK = 0,
    WARNING1,
    WARNING2,
    DANGER
} state;

int main(void)
{

   int i = 0;
   int result;
   int16_t intera_temp, decimale_temp, intera_hum, decimale_hum;
   int16_t temp_dht, hum_dht, temp, hum;
   //float hum1;
   //float hum2;
   int channelTemp = 1;
   int channelHum1 = 2;

   gpio_init(GPIO_PIN(2, 13), GPIO_IN_PD); //user blue button
   xtimer_init(); //inizializzazione per l'uso dei timer

   /*Inizializzo i pin relativi ai led*/
   gpio_init(GREEN_LED, GPIO_OUT);
   gpio_init(YELLOW_LED, GPIO_OUT);
   gpio_init(RED_LED, GPIO_OUT);

   gpio_init(liv_sonoro, GPIO_IN_PD); //il potenziometro simula il sensore acustico per la rilevazione del pianto

   state = OK;
   
   gpio_set(GREEN_LED);
   gpio_clear(YELLOW_LED);
   gpio_clear(RED_LED);


   //inizializzazione DHT
   dht_t dev;
   dht_params_t params;
   params.pin = DHT_sensor;
   params.type = DHT11;
   params.in_mode = GPIO_IN_PD;
 
   result = dht_init(&dev, &params);
   if(result == -1)printf("\nErrore inizializzazione DHT11..");


   printf("\nProgetto 'Smart Baby Room'\nStudenti: \n\tNiccolo' Fabrizio Pagano \n\tEnrico Verzi'");

   printf("\nYou are running RIOT on a(n) %s board.", RIOT_BOARD);
   printf("\nThis board features a(n) %s MCU.", RIOT_MCU);

    while(1){

	dht_read(&dev, &temp_dht, &hum_dht);

	//temperatura
	intera_temp = temp_dht/10;
	decimale_temp = (temp_dht-intera_temp*10);
	temp = intera_temp;

	//umidita'
	intera_hum = hum_dht/10;
	decimale_hum = (hum_dht-intera_hum*10);
	hum = intera_hum;

	i++;
	if(i % 15 == 0 && i % 30 != 0){
		printf("\n%d %d.%d", channelTemp, intera_temp, decimale_temp );
	}
	if(i % 30 == 0){
		i = 0;
		printf("\n%d %d.%d", channelHum1, intera_hum, decimale_hum );
	}

	switch(state){
		case OK:
			if(hum > HUM_THRESH){
				state = WARNING1;
				gpio_clear(GREEN_LED);
				gpio_set(YELLOW_LED);
			}
			if(gpio_read(liv_sonoro) == 0){
				state = WARNING2;
				gpio_clear(GREEN_LED);
				gpio_set(YELLOW_LED);
			}
			if(temp > TEMP_THRESH){
				state = DANGER;
				gpio_clear(GREEN_LED);
				gpio_clear(YELLOW_LED);
				gpio_set(RED_LED);
			}
			break;
		case WARNING1: //pannolino da cambiare
			if(hum < HUM_THRESH){
				state = OK;
				gpio_clear(YELLOW_LED);
				gpio_set(GREEN_LED);
			}
			if(gpio_read(liv_sonoro) == 0){
				state = WARNING2;
				gpio_clear(GREEN_LED);
				gpio_set(YELLOW_LED);
			}
			if(temp > TEMP_THRESH){
				state = DANGER;
				gpio_clear(GREEN_LED);
				gpio_clear(YELLOW_LED);
				gpio_set(RED_LED);
			}
			break;
		case WARNING2: //pianto
			gpio_toggle(YELLOW_LED);
			if(gpio_read(liv_sonoro) > 0){ //pianto terminato
				state = OK;
				gpio_clear(YELLOW_LED);
				gpio_set(GREEN_LED);
			
				if(hum > HUM_THRESH){
					state = WARNING1;
					gpio_clear(GREEN_LED);
					gpio_set(YELLOW_LED);
				}
			}
			if(temp > TEMP_THRESH){
				state = DANGER;
				gpio_clear(YELLOW_LED);
				gpio_clear(GREEN_LED);
				gpio_set(RED_LED);
			}
			break;
		case DANGER: //temperatura elevata
			if(temp < TEMP_THRESH){
				state = OK;
				gpio_clear(RED_LED);
				gpio_set(GREEN_LED);
				if(hum > HUM_THRESH){
					state = WARNING1;
					gpio_clear(GREEN_LED);
					gpio_set(YELLOW_LED);
				}
				if(gpio_read(liv_sonoro) == 0){
					state = WARNING2;
					gpio_clear(GREEN_LED);
					gpio_set(YELLOW_LED);
				}
			}
			break;
	}

	LED0_ON;
	xtimer_sleep(1);
	LED0_OFF;
	/*temp++;
	printf("%d %d", channelTemp, temp );*/
	//puts("\nCiao");
    }

    return 0;
}
