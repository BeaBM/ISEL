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

static void numeros(int num){
    switch(num){
        case 0:
            printf("11111111");
            printf("10000001");
            printf("11111111");
            printf("00000000");
            break;
        case 1:
            printf("00000100");
            printf("00000010");
            printf("11111111");
            printf("00000000");
            break;
        case 2:
            printf("11111001");
            printf("10001001");
            printf("10001111");
            printf("00000000");
            break;
        case 3:
            printf("10001001");
            printf("10001001");
            printf("11111111");
            printf("00000000");
            break;
        case 4:
            printf("00001111");
            printf("00001001");
            printf("11111111");
            printf("00000000");
            break;
        case 5:
            printf("10001111");
            printf("10001001");
            printf("11111001");
            printf("00000000");
            break;
            
        case 6:
            printf("11111111");
            printf("10001000");
            printf("11111000");
            printf("00000000");
            break;
        case 7:
            printf("00000001");
            printf("00000001");
            printf("11111111");
            printf("00000000");
            break;
        case 8:
            printf("11111111");
            printf("10001001");
            printf("11111111");
            printf("00000000");
            break;
        case 9:
            printf("00001111");
            printf("00001001");
            printf("11111111");
            printf("00000000");
            break;
        default:
            printf("10000001");
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
return 0;
}
