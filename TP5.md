1 How did you structure your transmit implementation? In particular, what do you do if the transmit ring is full?

La transmisión de paquetes se estructura con la siguiente lógica:
   * Se copia lo que se quiere enviar ( tamaño máximo 1516 bytes ) en un buffer estático.
   * Se crea el descriptor apropiado usando el tamaño y la dirección física de los datos previamente copiados.
   * Si el descriptor al que apunta el 'transmit descriptor tail' ya está procesado se asigna a esa posición el nuevo descriptor; si no está aún procesado ( el ring está lleno ) entonces se devuelve un error.

2  How did you structure your receive implementation? In particular, what do you do if the receive queue is empty and a user environment requests the next incoming packet?

La recepción de paquetes se estructura con la siguiente lógica:
   * Si el descriptor al que apunta el receive descriptor tail más uno no está procesado se devuelve error.
   * De lo contrario se copian los datos recividos ( alojados en una lista de buffers e indexado por el tdt + 1 ) en el buffer pasado por parámetro.

3   What does the web page served by JOS's web server say?
    This file came from jos
    cheesy web page!
4   How long approximately did it take you to do this lab?
    Entre 40hs y 60hs, pero perdí como 15 por no acordarme que nuestra versión de qemu no imprime "e1000: unicast match[0]: 52:54:00:12:34:56". También perdí mucho tiempo por tratar de instalar qemu en mi computadora y no sobre una maquina virtual corriendo ubuntu.