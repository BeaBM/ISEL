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

static struct timespec start,stop;


int sensor;
int flagLlegada;
int tiempos[8];
int tmp[24];
int cnt =0;
int i;
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

static void numeros(char c){
    switch(c){
        case '0':
            printf("11111111\n");
            printf("10000001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '1':
            printf("00000100\n");
            printf("00000010\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '2':
            printf("11111001\n");
            printf("10001001\n");
            printf("10001111\n");
            printf("00000000\n");
            break;
        case '3':
            printf("10001001\n");
            printf("10001001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '4':
            printf("00001111\n");
            printf("00001001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '5':
            printf("10001111\n");
            printf("10001001\n");
            printf("11111001\n");
            printf("00000000\n");
            break;
            
        case '6':
            printf("11111111\n");
            printf("10001000\n");
            printf("11111000\n");
            printf("00000000\n");
            break;
        case '7':
            printf("00000001\n");
            printf("00000001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '8':
            printf("11111111\n");
            printf("10001001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        case '9':
            printf("00001111\n");
            printf("00001001\n");
            printf("11111111\n");
            printf("00000000\n");
            break;
        default:
            clock_gettime(CLOCK_REALTIME, &start);
            printf("10000001\n");
            clock_gettime(CLOCK_REALTIME, &stop);
            printf("00000000\n");
            tmp[cnt]=( stop.tv_nsec - start.tv_nsec );
            cnt++;
         }

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
	struct timespec tWait = {0, 139000000};
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
        numeros(t);
		//printf("%c", t);
		//fflush(stdout);
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
    	for(i=0;i<24; i++){
       	 printf("%d ",tmp[i]);
       	 printf("\n");
   	 }
	return 0;
}
