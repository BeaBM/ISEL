#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>
#include <reactor.h>


#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6
/// añadir un gpio para identificar que he metido una moneda
#define GPIO_MONEDA 7



#define CUP_TIME	250
#define COFFEE_TIME	3000
#define MILK_TIME	3000
///////////////
#define PRECIOCAFE	50

//Periods in nanoseconds
#define SECONDARY_PERIOD_1 400000000
#define SECONDARY_PERIOD_2 800000000


fsm_t* cofm_fsm;
fsm_t* monedero_fsm;
enum cofm_state {
  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,
};

////////////////
enum monedero_state {
  MONEDERO,
};

///////////////////

static int button = 0; //global
//static void button_isr (void) { button = 1; }

static int timer = 0; //global
static void timer_isr (union sigval arg) { timer = 1; }


/////////////////////
static int cuenta = 0; //global

static int moneda=0;
static int flagBoton=0;

static struct timespec start, stop;
//static int t=0;
//////////////////////
static void timer_start (int ms)
{
  timer_t timerid;
  struct itimerspec value;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  value.it_value.tv_sec = ms / 1000;
  value.it_value.tv_nsec = (ms % 1000) * 1000000;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &value, NULL);
}

////////////////
static int button_pressed (fsm_t* this)
{
  int ret = button;
    //button=0;
    //si contador suficiente cambia de estado
    if(cuenta < PRECIOCAFE){
	//printf("INTRODUCE MONEDA \n SALDO INSUFICENTE \n");
       // flagBoton=0;
        ret = 0;
    }
    else if(ret==0){       
       //printf("SALDO SUFICIENTE: APRETA BOTON\n");
	return ret;
    }
  return ret;
}

static int timer_finished (fsm_t* this)
{
  int ret = timer;
  timer = 0;
  return ret;
}

static void cup (fsm_t* this)
{
  digitalWrite (GPIO_LED, LOW);
  digitalWrite (GPIO_CUP, HIGH);
  timer_start (CUP_TIME);
   //printf("CUP \n");
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
     //printf("COFFEE \n");
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
     //printf("MILK \n");
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
    flagBoton=1; //permito que devuelva dinero tras este estado
     //printf("FINISH \n");
}


// Explicit FSM description
static fsm_trans_t cofm[] = {
  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },
};

//////////////////////////////////
static int calcula_valor (fsm_t* this)
{
    cuenta+=moneda;
    //printf("Saldo= %d \n", cuenta);
     if((flagBoton==1)){
         flagBoton=0;
         return 1; //pasa a devolver
    }
    return 0;//vuelve a calcular
    
}

static void devolver (fsm_t* this){
    //int devuelto= cuenta- PRECIOCAFE;//sacar las monedas
    //printf("Devuelto %d \n",devuelto);
    cuenta=0;
    
}

//añadir otro para monedero, declarar un estado y una funcion como arriba

static fsm_trans_t monedero[] = {
  { MONEDERO, calcula_valor, MONEDERO, devolver},
  {-1, NULL, -1, NULL },
};





// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}

static void
fsm_moned (struct event_handler_t* eh)
{ 
clock_gettime(CLOCK_REALTIME, &start);
  static const struct timeval period = {0, SECONDARY_PERIOD_2/1000 };
  fsm_fire (monedero_fsm);
clock_gettime(CLOCK_REALTIME, &stop);
//t=( stop.tv_nsec - start.tv_nsec );
//printf("%d \n", t);
  timeval_add(&eh->next_activation, &eh->next_activation, &period);

}

static void
fsm_cofm (struct event_handler_t* eh)
{
  clock_gettime(CLOCK_REALTIME, &start);
 static const struct timeval period = { 0,  SECONDARY_PERIOD_1/1000};
 fsm_fire (cofm_fsm); 
clock_gettime(CLOCK_REALTIME, &stop);
//t=( stop.tv_nsec - start.tv_nsec );
//printf("%d \n", t); 
 timeval_add (&eh->next_activation, &eh->next_activation, &period);

}

int main (int argc, char *argv[])
{  
//  int numPeriodo=1;
 // double tiempos=0;
  double total[10];
  int i=0;
  
  for(i=0;i<10; i++){
	
	total[i]=0;
  }
  int col=0;

 // struct timeval clk_period = { 0, 250 * 1000 };
  //struct timeval next_activation;

  

  wiringPiSetup();
 
    //hacer lo mismo con el detector de monedas el gpio que usas
    
  pinMode (GPIO_CUP, OUTPUT);
  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);
  cofm_fsm = fsm_new (cofm);
  monedero_fsm = fsm_new (monedero);

  EventHandler tmoned, tcofm;

  reactor_init ();

  event_handler_init (&tcofm, 2, (eh_func_t)   fsm_cofm);
  reactor_add_handler (&tcofm);

  event_handler_init (&tmoned, 1, (eh_func_t)   fsm_moned);
  reactor_add_handler (&tmoned);
  
  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
	
	clock_gettime(CLOCK_REALTIME, &start);
	reactor_handle_events ();
	clock_gettime(CLOCK_REALTIME, &stop);
	total[col]=( stop.tv_nsec - start.tv_nsec );
	col++;

  }

	for(i=0;i<10; i++){
		printf("%f ",total[i]);
		printf("\n");
      }

  return 0;
}
