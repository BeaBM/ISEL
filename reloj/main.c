#include <stdio.h>
#include <time.h>
#include "fsm.h"

int segundos;
int minutos;
int horas;

enum relojLed {
 INICIO,
 PINTA,
 TERMINA,
};

int sensor;
int flagLlegada;
int tiempos[8];

void actualizaTiempo(){

	time_t initTiempos = time(NULL); //epoch 00:00 del 1 de enero de 1970 cuenta los segundos desde entonces.

	struct tm fecha = *localtime(&initTiempos); // Seprara minutos horas etc..
	
	segundos = fecha.tm_sec;
	minutos = fecha.tm_min;
	horas = fecha.tm_hour;

	tiempos[0] = '0' + horas/10;
	tiempos[1] = '0' + horas%10;
	tiempos[2] = ':';
	tiempos[3] = '0' + minutos/10;
	tiempos[4] = '0' + minutos%10;
	tiempos[5] = ':';
	tiempos[6] = '0' + segundos/10;
	tiempos[7] = '0' + segundos%10;
	
}

static int sensorSalida ()
{
	if (sensor == 1){
		return 1;
	} else{
		return 0;
	}
}
static void wait ()
{
	struct timespec tWait = {0, 140000000};
	nanosleep(&tWait, NULL);

}

static void pintar ()
{
	int i;
	char t;
	

	actualizaTiempo();
	
	for(i=0; i<8; i++){
		wait();
		t = tiempos[i];
		printf("%c", t);
		fflush(stdout);
	}

	flagLlegada = 1;

}

static int sensorLlegada ()
{
	if (flagLlegada == 1){
		flagLlegada = 0;
		return 1;
	} else{
		return 0;
	}
	
}

static void fin ()
{
	printf("\n");
}

static fsm_trans_t relojLed[] = {
 { INICIO, sensorSalida, PINTA,     pintar    },
 { PINTA,  sensorLlegada, INICIO,    fin   },
 {-1, NULL, -1, NULL },
};

int main (){
	fsm_t* fsm_relojLed = fsm_new(relojLed);

	while(scanf("%d", &sensor)==1){
		fsm_fire(fsm_relojLed);	
	}
return 0;
}