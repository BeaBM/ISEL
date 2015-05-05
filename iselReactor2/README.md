Seguimos con los mismos periodos definidos para cada máquina de estados, definidos para el caso del ejecutivo cíclico.

En este caso la planificación se basa en el reactor.

Asignamos un evento a cada a cada máquina de estado, es decir cada evento incluirá el fire correspondiente a 
la máquina que le corresponde. También asignamos prioridades a cada tipo de vento. Los eventos relacionados con la máquina de café serán más
prioritarios que los del monedero.

Para este tipo de planificación puede existir bloqueos epara los eventos. Es decir, un evento menos
prioritario puede bloquear a otro evento más prioritario justo en el 
inicio de  la ejecución del evento.
En este caso el monedero puede bloquear a la máquina de café y, el bloqueo durará como mucho 0,4 segundos,
es decir, el periodo de su event handler

Resultado práctico:
Al ejecutar el programa con el fichero inputs obtenemos que la ejecución en caso peor
dura 445049 nanosegundos y se da en el caso en el que se sirve el café.
Dado que es menor que el periodo estableciedo, el sistema funciona correctamente.
