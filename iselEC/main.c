#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>

#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6
/// añadir un gpio para identificar que he metido una moneda
#define GPIO_MONEDA 7
//Para saber que moneda
//#define GPIO_M0   8
//#define GPIO_M1   9
//#define GPIO_M2   10
//


#define CUP_TIME	250
#define COFFEE_TIME	3000
#define MILK_TIME	3000
///////////////
#define PRECIOCAFE	50

//Periods in nanoseconds
#define SECONDARY_PERIOD_1 400000000
#define SECONDARY_PERIOD_2 800000000

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

static struct timespec start, stop, tv;
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
      // printf("SALDO SUFICIENTE: APRETA BOTON\n");
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
    // printf("COFFEE \n");
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
    int devuelto= cuenta- PRECIOCAFE;//sacar las monedas
   // printf("Devuelto %d \n",devuelto);
    cuenta=0;
    
}

//añadir otro para monedero, declarar un estado y una funcion como arriba

static fsm_trans_t monedero[] = {
  { MONEDERO, calcula_valor, MONEDERO, devolver},
  {-1, NULL, -1, NULL },
};


// Utility functions, should be elsewhere

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000; 
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}



int main (int argc, char *argv[])
{
    
    int numPeriodo=1;
    double tiempos=0;
    double total[10][2];
int i=0;
int j=0;
	for(i=0;i<10; i++){
		for(j=0; j<2; j++)
			total[i][j]=0;
        }
    int col=0;
  struct timeval clk_period = { 0, 250 * 1000 };
  struct timeval next_activation;
  fsm_t* cofm_fsm = fsm_new (cofm);
  fsm_t* monedero_fsm = fsm_new (monedero);

//  wiringPiSetup();
//  pinMode (GPIO_BUTTON, INPUT);
//  wiringPiISR (GPIO_BUTTON, INT_EDGE_FALLING, button_isr);
//
//  pinMode (GPIO_MONEDA, INPUT);
//  wiringPiISR (GPIO_MONEDA, INT_EDGE_FALLING, cuenta_isr);
//    //hacer lo mismo con el detector de monedas el gpio que usas
//    
//  pinMode (GPIO_MON0, INPUT);
//  pinMode (GPIO_MON1, INPUT);
//  pinMode (GPIO_MON2, INPUT);
//
//  pinMode (GPIO_CUP, OUTPUT);
//  pinMode (GPIO_COFFEE, OUTPUT);
//  pinMode (GPIO_MILK, OUTPUT);
//  pinMode (GPIO_LED, OUTPUT);
//  digitalWrite (GPIO_LED, HIGH);

  gettimeofday (&next_activation, NULL);
  
  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
      
      nanosleep(&tv,&tv);
      switch(numPeriodo){
      
          case 1:
              
              clock_gettime(CLOCK_REALTIME, &start);
              fsm_fire (cofm_fsm);
              fsm_fire(monedero_fsm);
              clock_gettime(CLOCK_REALTIME, &stop);
              
              tiempos=( stop.tv_nsec - start.tv_nsec );
                //printf("CASE 1\n");
                //printf("%f \n", tiempos);
		total[col][0]=tiempos;
              break;
              
          case 2:
              
              clock_gettime(CLOCK_REALTIME, &start);
              fsm_fire (cofm_fsm);
              clock_gettime(CLOCK_REALTIME, &stop);
              
              tiempos=( stop.tv_nsec - start.tv_nsec );
              total[col][1]=tiempos;
              numPeriodo=0;
               // printf("CASE 2\n");
               //printf("%f \n", tiempos);
              col++;
              break;
              
          default:
              break;
      
      }
      
      
      tv.tv_nsec = (time_t) (SECONDARY_PERIOD_1 - tiempos);
      
      numPeriodo++;

    
    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);
  }
for(i=0;i<10; i++){
		for(j=0; j<2; j++)
			printf("%f ",total[i][j]);
printf("\n");
        }

  return 0;
}
