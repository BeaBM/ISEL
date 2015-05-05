#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>
#include "tasks.h"


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

static  pthread_mutex_t m_cuenta;
static  pthread_mutex_t m_boton;


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
static int t=0;
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
  clock_gettime(CLOCK_REALTIME, &start);
   pthread_mutex_lock(&m_cuenta);
      //button=0;
    //si contador suficiente cambia de estado
    if(cuenta < PRECIOCAFE){
        //printf("INTRODUCE MONEDA \n SALDO INSUFICENTE \n");
       // flagBoton=0;
        ret = 0;
    }

     else if(ret==0){       
	pthread_mutex_unlock(&m_cuenta);
       //printf("SALDO SUFICIENTE: APRETA BOTON\n");
	return ret;
    }

pthread_mutex_unlock(&m_cuenta);
   clock_gettime(CLOCK_REALTIME, &stop);
    t=( stop.tv_nsec - start.tv_nsec );
    printf("%d cuenta \n", t);
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
   printf("CUP \n");
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
     printf("COFFEE \n");
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
     printf("MILK \n");
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
  clock_gettime(CLOCK_REALTIME, &start);
   pthread_mutex_lock(&m_cuenta);
      flagBoton=1; //permito que devuelva dinero tras este estado
   pthread_mutex_unlock(&m_cuenta);
   clock_gettime(CLOCK_REALTIME, &stop);
    t=( stop.tv_nsec - start.tv_nsec );
    printf("%d button\n", t);
     printf("FINISH \n");
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
 clock_gettime(CLOCK_REALTIME, &start);
    pthread_mutex_lock(&m_cuenta);
    	cuenta+=moneda;
    pthread_mutex_unlock(&m_cuenta);
    clock_gettime(CLOCK_REALTIME, &stop);
    t=( stop.tv_nsec - start.tv_nsec );
    printf("%d cuenta\n", t);
    //printf("Saldo= %d \n", cuenta);
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_mutex_lock(&m_boton);

     if((flagBoton==1)){
         flagBoton=0;
	pthread_mutex_unlock(&m_boton);
   	 clock_gettime(CLOCK_REALTIME, &stop);
        t=( stop.tv_nsec - start.tv_nsec );
   	 printf("%d button\n", t);        
	 return 1; //pasa a devolver
    }
    pthread_mutex_unlock(&m_boton);
    clock_gettime(CLOCK_REALTIME, &stop);
        t=( stop.tv_nsec - start.tv_nsec );
    printf("%d button\n", t);
    return 0;//vuelve a calcular
    
}

static void devolver (fsm_t* this){
    clock_gettime(CLOCK_REALTIME, &start);
    	pthread_mutex_lock(&m_cuenta);
    		int devuelto= cuenta- PRECIOCAFE;//sacar las monedas
    	pthread_mutex_unlock(&m_cuenta);
    clock_gettime(CLOCK_REALTIME, &stop);
    t=( stop.tv_nsec - start.tv_nsec );
    printf("%d cuenta \n", t);
    printf("Devuelto %d \n",devuelto);
   clock_gettime(CLOCK_REALTIME, &start);
        pthread_mutex_lock(&m_cuenta);
	 cuenta=0;
	pthread_mutex_unlock(&m_cuenta);
    clock_gettime(CLOCK_REALTIME, &stop);
    t=( stop.tv_nsec - start.tv_nsec );
    printf("%d cuenta \n", t);
    
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
fsm_moned (void)
{ 
clock_gettime(CLOCK_REALTIME, &start);
  static const struct timeval period = {0, SECONDARY_PERIOD_2/1000 };
  fsm_fire (monedero_fsm);
clock_gettime(CLOCK_REALTIME, &stop);
//t=( stop.tv_nsec - start.tv_nsec );
//printf("%d \n", t);
 

}

static void
fsm_cofm (void)
{
  clock_gettime(CLOCK_REALTIME, &start);
 static const struct timeval period = { 0,  SECONDARY_PERIOD_1/1000};
 fsm_fire (cofm_fsm); 
 clock_gettime(CLOCK_REALTIME, &stop);
//t=( stop.tv_nsec - start.tv_nsec );
//printf("%d \n", t); 
 

}

int main (int argc, char *argv[])
{  
 pthread_t t_cofm;
 pthread_t t_moned;
 
 void* ret;
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

init_mutex (&m_boton, 2);
init_mutex (&m_cuenta, 2);
 
    //hacer lo mismo con el detector de monedas el gpio que usas
    
  pinMode (GPIO_CUP, OUTPUT);
  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);
  cofm_fsm = fsm_new (cofm);
  monedero_fsm = fsm_new (monedero);

  
  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
	
	clock_gettime(CLOCK_REALTIME, &start);
	create_task (&t_cofm, fsm_cofm, NULL, 400, 2, 1024);
  	create_task (&t_moned, fsm_moned, NULL, 800, 1, 1024);
	clock_gettime(CLOCK_REALTIME, &stop);
	total[col]=( stop.tv_nsec - start.tv_nsec );
	col++;

  }

	for(i=0;i<10; i++){
		printf("%f ",total[i]);
		printf("\n");
      }
//  pthread_join(t_cofm, &ret);
//  pthread_join(t_moned, &ret);
  return 0;
}
