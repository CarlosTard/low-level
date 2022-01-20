#include <stdio.h>
#include "44b.h"
#include "button.h"
#include "leds.h"
#include "utils.h"
#include "D8Led.h"
#include "intcontroller.h"
#include "timer.h"
#include "gpio.h"
#include "keyboard.h"

struct RLstat {
	int moving;
	int speed;
	int direction;
	int position;
};

static struct RLstat RL = {
	.moving = 1,
	.speed = 5,
	.direction = 0,
	.position = 0,
};
static int count = 0;

void timer_ISR(void) __attribute__ ((interrupt ("IRQ")));
void button_ISR(void) __attribute__ ((interrupt ("IRQ")));
void keyboard_ISR(void) __attribute__ ((interrupt ("IRQ")));

void timer_ISR(void)
{
	//COMPLETAR: tomar el código de avance de posición del led rotante de la práctica anterior
	// COMPLETAR: debemos hacer avanzar el led rotante una posici�n en
	// la direcci�n indicada por el campo direction de la variable RL.
	// La posici�n actual est� representada en el campo position.
	// Recordar que queremos un movimiento circular, representado por
	// las 6 primeras posiciones en el array Segmentes del display de 8
	// segmentos, por lo que position debe estar siempre entre 0 y 5.
	if (RL.moving) {
		if(RL.direction){
			RL.position = (RL.position + 1) % 6;
		}else{
			RL.position = (RL.position - 1 + 6) % 6;
		}
	}

	D8Led_segment(RL.position);
	ic_cleanflag(INT_TIMER0);
}

void button_ISR(void)
{
	unsigned int whicheint = rEXTINTPND;
	unsigned int buttons = (whicheint >> 2) & 0x3;

	if (buttons & BUT1) {
		// COMPLETAR: utilizando la interfaz para los leds definida en leds.h
		// hay que apagar ambos leds
		// Tambi�n hay que comutar la direcci�n del movimiento del led rotante
		// representado por el campo direction de la variable RL
		led1_off();
		led2_off();
		if(RL.direction)RL.direction = 0;
		else RL.direction = 1;
	}

	if (buttons & BUT2) {
		// COMPLETAR: utilizando la interfaz para los leds definida en leds.h
		// Incrementar contador de pulsaciones. Si es par, conumtar led1. Si es impar, conmutar el led2.
		// Tambi�n hay que comutar el estado de movimiento del led rotante
		// representado por el campo moving de la variable RL, y en caso de
		// ponerlo en marcha debemos reiniciar el campo iter al valor del campo
		// speed.
		count++;
		if(count % 2 == 0)
			led1_switch();
		else   led2_switch();
		if(RL.moving == 1) RL.moving = 0;
		else RL.moving = 1;

	}
	// eliminamos rebotes
	Delay(2000);
	// borramos el flag en extintpnd
	rEXTINTPND = rEXTINTPND | buttons << 2;//COMPLETAR: debemos borrar las peticiones de interrupción en
		         //EXTINTPND escribiendo un 1 en los flags que queremos borrar (los
				 //correspondientes a los pulsadores pulsados)
	ic_cleanflag(INT_EINT4567);
}

void keyboard_ISR(void)
{
	int key;

	/* Eliminar rebotes de presión */
	Delay(200);
	
	/* Escaneo de tecla */
	key = kb_scan();

	if (key != -1) {
		/* Visualizacion en el display */
		//COMPLETAR: mostrar la tecla en el display utilizando el interfaz
		//definido en D8Led.h

		switch (key) {
			case 0:
				//COMPLETAR: poner en timer0 divisor 1/8 y contador 62500
				break;
			case 1:
				//COMPLETAR: poner en timer0 timer divisor 1/8 y contador 31250
				break;
			case 2:
				//COMPLETAR: poner en timer0 timer divisor 1/8 y contador 15625
				break;
			case 3:
				//COMPLETAR: poner en timer0 timer divisor 1/4 y contador 15625
				break;
			default:
				break;
		}
		
		/* Esperar a que la tecla se suelte, consultando el registro de datos */		
		while (/*COMPLETAR: true si está pulsada la tecla (leer del registro rPDATG)*/);
	}

    /* Eliminar rebotes de depresión */
    Delay(200);
     
    /* Borrar interrupciones pendientes */
	//COMPLETAR
	//borrar la interrupción por la línea EINT1 en el registro rI_ISPC
    ic_cleanflag(INT_EINT1);
}

int setup(void)
{
	leds_init();
	D8Led_init();
	D8Led_segment(RL.position);

	/* Port G: configuración para generación de interrupciones externas,
	 *         botones y teclado
	 **/

	//COMPLETAR: utilizando el interfaz para el puerto G definido en gpio.h
	//configurar los pines 1, 6 y 7 del puerto G para poder generar interrupciones
	//externas por flanco de bajada por ellos y activar las correspondientes
	//resistencias de pull-up.
	portG_conf(1,EINT);
	portG_conf(6,EINT);
	portG_conf(7,EINT);
	portG_conf_pup(1,ENABLE);
	portG_conf_pup(6,ENABLE);
	portG_conf_pup(7,ENABLE);
	portG_eint_trig(1,FALLING);
	portG_eint_trig(6,FALLING);
	portG_eint_trig(7,FALLING);

	/********************************************************************/

	/* Configuración del timer */

	//COMPLETAR: tomar el código de la segunda parte
	tmr_set_prescaler(0,255);
	tmr_set_divider(0,8);
	tmr_set_count(TIMER0,62500,0);
	tmr_set_mode(TIMER0,RELOAD);

	if (RL.moving)
		tmr_start(TIMER0);
	/***************************/

	// Registramos las ISRs
	pISR_TIMER0   = (unsigned) timer_ISR; //COMPLETAR: registrar la RTI del timer
	pISR_EINT4567 = (unsigned) button_ISR; //COMPLETAR: registrar la RTI de los botones
	pISR_EINT1    = (unsigned) keyboard_ISR; //COMPLETAR: registrar la RTI del teclado

	/* Configuración del controlador de interrupciones
	 * Habilitamos la línea IRQ, en modo vectorizado 
	 * Configuramos el timer 0 en modo IRQ y habilitamos esta línea
	 * Configuramos la línea EINT4567 en modo IRQ y la habilitamos
	 * Configuramos la línea EINT1 en modo IRQ y la habilitamos
	 */

	ic_init();
	//COMPLETAR: utilizando el interfaz definido en intcontroller.h
	ic_conf_irq(ENABLE,VEC);//	habilitar la línea IRQ en modo vectorizado
	ic_conf_fiq(DISABLE);//		deshabilitar la línea FIQ
	ic_conf_line(INT_TIMER0,IRQ);//	configurar la línea INT_TIMER0 en modo IRQ
	ic_conf_line(INT_EINT4567,IRQ);//	configurar la línea INT_EINT4567 en modo IRQ
	ic_conf_line(INT_EINT1,IRQ);//	configurar la línea INT_EINT1 en modo IRQ
	ic_enable(INT_TIMER0);//	habilitar la línea INT_TIMER0
	ic_enable(INT_EINT4567);//	habilitar la línea INT_EINT4567
	ic_enable(INT_EINT1);//	habilitar la línea INT_EINT1

	/***************************************************/

	Delay(0);
	return 0;
}

int loop(void)
{
	return 0;
}


int main(void)
{
	setup();

	while (1) {
		loop();
	}
}
