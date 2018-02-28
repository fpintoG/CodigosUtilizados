#!/bin/bash

#este script elimina los archivos con un tiempo mayor a 2 dias
#en el directorio 'caidas', si se requiere modificar el tiempo de
#antiguedad de por ejemplo 2 dias a 3 dias simeplente se debe cambiar 
#el +1 por +2
path="caidas/"
find $path -name "*.txt"  -type f -mtime +1 -print -delete 



