# Entregable 1: Makefiles y ccache

## 1. Compilación en paralelo

Para analizar cómo afecta el número de trabajos en paralelo al tiempo de compilación, se han realizado pruebas con `make -jN`, utilizando los valores N = 1, 2, 4, 8 y 16.

Para cada valor se han realizado tres ejecuciones, limpiando previamente el proyecto con `make clean`. Posteriormente, se ha calculado el tiempo medio de compilación.

### Resultados

| Comando | Tiempo medio |
|---|---:|
| `make -j1` | 7,323 s |
| `make -j2` | 4,270 s |
| `make -j4` | 2,773 s |
| `make -j8` | 2,663 s |
| `make -j16` | 2,673 s |

### Conclusión

Se observa que el tiempo de compilación disminuye considerablemente al aumentar el paralelismo, especialmente al pasar de 1 a 4 trabajos.

A partir de `-j4`, las mejoras son muy pequeñas. De hecho, con `-j16` el tiempo aumenta ligeramente respecto a `-j8`.

Por tanto, se considera que `-j4` es suficiente para este proyecto, ya que aumentar el número de trabajos no supone una mejora significativa.


## 2. Errores en la compilación paralela

Se han realizado pruebas de compilación utilizando distintos niveles de paralelismo: `-j2`, `-j4`, `-j8` y `-j16`.

En todas las ejecuciones la compilación ha finalizado correctamente, sin producirse errores.

Por tanto, no se han detectado problemas de dependencias ni conflictos entre las tareas de compilación. No ha sido necesario realizar modificaciones en el Makefile para solucionar errores de paralelismo.

## 3. Comparación de compilación con ccache

Se ha utilizado `ccache` para acelerar la compilación del proyecto, ejecutando dos compilaciones con `make -j4`.

Los tiempos obtenidos han sido:

| Tipo de compilación | Tiempo |
|---|---:|
| Sin ccache | 2,773 s |
| Primera compilación con ccache | 5,21 s |
| Segunda compilación con ccache | 0,35 s |

En la primera compilación, `ccache` no tenía archivos guardados, por lo que tuvo que compilar todos los archivos. Por eso tardó más que la compilación normal.

En la segunda compilación, `ccache` reutilizó los resultados guardados anteriormente y el tiempo bajó a 0,35 segundos.

Las estadísticas muestran que hubo 13 aciertos de caché en la segunda compilación, correspondientes a todos los archivos que se podían almacenar en caché.

Por tanto, `ccache` resulta muy útil cuando se realizan compilaciones repetidas, ya que reduce considerablemente el tiempo de compilación.

## 4. Limpieza y análisis de las estadísticas de ccache

Primero se borró la caché y se reiniciaron las estadísticas con los comandos `ccache --clear` y `ccache --zero-stats`.

Después se realizaron dos compilaciones:

| Estadística | Primera compilación | Segunda compilación |
|---|---:|---:|
| Tiempo | 5,49 s | 0,36 s |
| Aciertos (Hits) | 0 | 13 |
| Fallos (Misses) | 13 | 13 acumulados |

En la primera compilación no hubo aciertos porque la caché estaba vacía. Los 13 archivos compilables se guardaron en la caché.

En la segunda compilación se produjeron 13 aciertos, ya que `ccache` pudo reutilizar los resultados anteriores. Esto redujo el tiempo de compilación de 5,49 a 0,36 segundos.

Las estadísticas finales muestran 13 aciertos y 13 fallos, con un 50 % de aciertos sobre las llamadas que se podían almacenar en caché.

Por tanto, `ccache` permite reducir considerablemente el tiempo cuando se realizan compilaciones repetidas.

## Conclusión final

Las pruebas muestran que la compilación en paralelo reduce considerablemente el tiempo de construcción del proyecto. Además, ccache permite acelerar las compilaciones repetidas al reutilizar los resultados almacenados en caché. Para este proyecto, -j4 ofrece un buen equilibrio entre tiempo de compilación y número de trabajos.