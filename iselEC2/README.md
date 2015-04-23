En este caso, el cliente prefija un periodos para cada maquina de estado:

Máquina de café: 0.4 segundos.
Monedero=0.8 segundos.

Siendo de esta manera, la máquina de café más prioritaria que la del monedero.

A partir de estos datos, sabiendo que estamos en una planificación basada en ejecutivo cíclico obtenemos:

periodo secundario= m.cd.(0,4; 0,8)= 0,4 seundos
hiperperiodo= m.c.m(0,4;0.8)=0.8 segundos

Con esto nuestra máquina de café debe funcionar de la siguiente manera:
En la primera trama se ejecutarán ambas máquinas de estados (café y monedero).
En la segunda trama se ejecutará sólo la máquina de estados de café.

Este funcionamiento se repite indefinidamente mientras existan datos de entrada.

Ejecución práctica:
En este caso al introducir el fichero inputs obtenemos que el tiempo de ejecución en caso peor tarda 359164 nanosegundos. Este caso corresponde a la ejecución de la máquina de café, cuando sirve el café. Comprobamos que efectivamente este tiempo es menor que el periodo establecido y por lo tanto el sistema funciona correctamente.