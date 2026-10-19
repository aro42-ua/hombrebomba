# Memoria: enemigos e IA

Autor: Ángel Rubio
Hito: 1 (Alpha)
Incidencia: #10

## 1. Qué pide la asignatura

El enunciado del proyecto incluye la IA de los NPC entre los requisitos secundarios, los que suman nota: pide un sistema que gestione el comportamiento de los personajes no jugadores y recomienda mirar técnicas habituales en la industria, y nombra en concreto los árboles de decisión y los *steering behaviors*. También propone "IA" como uno de los papeles del reparto del grupo. El enunciado del Hito 1 no menciona enemigos.

Con eso he usado las dos técnicas que cita: un árbol de decisión para elegir qué hace cada enemigo y un *seek*, el *steering behavior* más sencillo, para moverlo. Además hacía falta buscar caminos por el laberinto, y para eso uso una búsqueda en anchura (BFS).

La incidencia #8 incluía los enemigos, pero se cerró con el modo de dos jugadores y sin ellos. Esta parte los añade sin cambiar ese modo.

## 2. Los enemigos en el juego

Hay dos tipos:

| Tipo | Aspecto | Velocidad | Comportamiento |
|---|---|---|---|
| Globo | círculo rosa | 60 px/s | pasea al azar; no reconoce las bombas |
| Perseguidor | rombo morado | 80 px/s | huye de las bombas, persigue al jugador cercano y si no ve a nadie pasea evitando el peligro |

Los jugadores van a 120 px/s, así que se puede escapar de los dos.

Cada partida empieza con un globo en la esquina inferior izquierda (1,11), un perseguidor en la inferior derecha (13,11) y un segundo globo en una celda vacía elegida con la semilla. Esa celda tiene que tener al menos una vecina libre (si no, el globo se quedaría encerrado hasta que alguien rompa un bloque), estar a 8 o más celdas (distancia Manhattan) de las dos salidas de los jugadores y a 3 o más de los otros dos enemigos, para que no aparezca encima de nadie.

Las reglas son las de Bomberman:

- Si un enemigo toca a un jugador, el jugador queda eliminado. La caja del enemigo se encoge 4 px por cada lado al comprobar el contacto, para que un roce de esquina no cuente.
- Si una llama llega a la celda de un enemigo, el enemigo muere.
- Los enemigos no atraviesan muros, bloques ni bombas, y no rompen bloques.

Los enemigos son un peligro para los dos jugadores por igual. El resultado de la partida no cambia: gana el último jugador vivo y, si caen los dos, es empate. El marcador muestra cuántos enemigos quedan.

## 3. Cómo decide un enemigo

### 3.1 Movimiento de celda en celda

Un enemigo siempre está en el centro de una celda o yendo del centro de una celda al de una vecina. Solo decide al llegar a un centro. Así la IA trabaja con celdas, igual que el tablero, y nunca se queda a medias en un cruce.

Entre dos centros se mueve con un *seek*: calcula el vector hacia el centro de la celda destino, lo normaliza y avanza a su velocidad. Si en ese fotograma recorrería más de lo que le falta, se coloca justo en el centro, así no se pasa ni oscila.

Si mientras va de camino alguien pone una bomba en la celda destino, da media vuelta y vuelve a la celda de la que salió.

### 3.2 Percepción: el mapa de peligro

En cada fotograma `MainGameState` construye un `MapaPeligro` con las bombas en cuenta atrás y las celdas en llamas. Una celda es peligrosa si está en llamas o si la va a barrer una bomba. Para saber qué barre una bomba uso la misma regla que `Bomba`: la llama avanza en cruz hasta su alcance, el muro la corta y el bloque la recibe pero no la deja pasar. Si esa regla cambia en `Bomba`, hay que cambiarla también en `MapaPeligro::calcular`.

Con el mapa, cada enemigo rellena una `Percepcion` al llegar a un centro:

- si su tipo reconoce el peligro y si persigue (son fijos para cada tipo);
- si su celda es peligrosa;
- si hay una celda segura a 10 pasos o menos;
- si hay un jugador vivo a 8 pasos o menos por celdas seguras;
- si tiene al menos una celda vecina a la que moverse.

Las distancias son pasos de camino real, no en línea recta: un jugador al otro lado de un muro está lejos aunque se vea cerca.

### 3.3 El árbol de decisión

La función `decidir` recibe la percepción y devuelve una acción:

```
¿Reconoce el peligro y está en peligro?
├─ sí → ¿hay una celda segura alcanzable?
│        ├─ sí → HUIR
│        └─ no → sigue por la rama de abajo
└─ no → ¿persigue y tiene a un jugador al alcance?
         ├─ sí → PERSEGUIR
         └─ no → ¿tiene alguna celda a la que moverse?
                  ├─ sí → DEAMBULAR
                  └─ no → ESPERAR
```

El orden importa: salvarse va antes que cazar. El globo entra siempre por la rama de abajo y acaba deambulando, porque ni reconoce el peligro ni persigue. Es lo que le hace fácil de matar.

He dejado `decidir` como una función que solo depende de sus datos de entrada, sin raylib ni estado de la partida. Así se prueba cada rama del árbol con una percepción hecha a mano.

### 3.4 Búsqueda de caminos

Huir y perseguir necesitan el primer paso del camino más corto. `primerPasoHacia` hace una búsqueda en anchura desde la celda del enemigo con un límite de pasos y devuelve la celda vecina por la que empieza el camino.

He usado BFS y no A* por dos motivos. El tablero tiene 15 × 13 = 195 celdas, así que recorrerlo entero cuesta muy poco. Y al huir no hay un destino concreto: se busca la celda segura más cercana, sea cual sea, y BFS lo resuelve parándose en la primera que cumple la condición. A* necesita un destino para su heurística.

Al huir, el enemigo puede cruzar celdas peligrosas porque ya está en una. Al perseguir, no: el perseguidor prefiere no alcanzar al jugador antes que meterse en una llama.

### 3.5 Deambular

Cuando deambula, el enemigo sigue recto con un 70 % de probabilidad si puede. Si no, elige al azar entre las celdas vecinas libres y solo da media vuelta cuando no le queda otra salida. Esto hace que recorra pasillos en vez de temblar entre dos celdas. El perseguidor, además, descarta las celdas peligrosas; si todas lo son y la suya no, se queda quieto esperando a que pase la explosión.

Cada enemigo tiene su propio `std::mt19937` sembrado a partir de la semilla de la partida. Por eso el paseo también se reproduce con la misma semilla, igual que el tablero y los power-ups.

### 3.6 Por qué no una máquina de estados

Una máquina de estados guardaría la acción actual y cambiaría con transiciones. Aquí el árbol se vuelve a evaluar en cada celda con la percepción de ese momento, así que no hace falta recordar nada entre decisiones y el comportamiento sale de una sola función que se puede probar. Si en hitos posteriores los enemigos necesitan memoria (por ejemplo, seguir buscando al jugador unos segundos después de perderlo), sería el momento de añadir estados.

## 4. Reparto del código

| Fichero | Contenido |
|---|---|
| `IAEnemigo.hpp/.cpp` | `MapaPeligro`, `Percepcion`, `decidir` y `primerPasoHacia`. No depende del estado de la partida. |
| `Enemigo.hpp/.cpp` | La clase `Enemigo`: movimiento, percepción, paseo y dibujo. |
| `MainGameState` | Crea los enemigos, calcula el mapa de peligro, los actualiza y comprueba llamas y contactos. |
| `Constantes.hpp` | Sección "Enemigos": tamaños, velocidades, radios y probabilidades. |

Para ajustar la dificultad basta con tocar las constantes: `RADIO_PERSECUCION`, las dos velocidades y `PROB_SEGUIR_RECTO`.

## 5. Depuración

Con F1, además de lo que ya había, se ven:

- las celdas peligrosas, en rojo translúcido;
- sobre cada enemigo, la letra de su acción (H huir, P perseguir, D deambular, E esperar);
- una línea amarilla hasta la celda a la que va.

## 6. Pruebas

`tests/test_ia_enemigos.cpp` hace 19 comprobaciones sin abrir ventana. Casi todas usan un tablero sin bloques (se genera uno y se rompen todos), donde los caminos se pueden contar a mano.

- Árbol de decisión: huir va antes que perseguir, sin refugio se pasa a la rama de perseguir, el globo solo deambula y sin salida espera.
- Mapa de peligro: la cruz de una bomba llega hasta su alcance y no más, un pilar corta la llama, un bloque la recibe pero la frena y las llamas cuentan como peligro.
- BFS: encuentra el camino recto, rodea una bomba que lo corta y respeta el límite de pasos.
- Enemigos: el perseguidor llega hasta la celda del jugador, no persigue a uno fuera de su radio y sale de la zona de una bomba; un globo encerrado entre bombas se queda quieto; si le ponen una bomba delante vuelve a su celda; con la misma semilla dos globos hacen el mismo paseo; y un globo pasea 30 segundos sin meterse en un muro.

Se compila con:

```bash
g++ -o bin/test_ia_enemigos tests/test_ia_enemigos.cpp src/Enemigo.cpp src/IAEnemigo.cpp src/Tablero.cpp -I src/ -I vendor/include/ -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
./bin/test_ia_enemigos
```

Pasan las 19. También he dejado el juego 40 segundos sin tocar nada con la semilla 7 y F1 activado para comprobar que los enemigos no se atascan ni se salen de las celdas libres.

## 7. Limitaciones conocidas

- Las bombas no se encadenan (una llama no hace explotar otra bomba), y el mapa de peligro sigue esa misma regla. Si el Paquete 3 añade las reacciones en cadena, el mapa se queda corto y hay que ampliarlo.
- Los enemigos pueden solaparse entre sí. En el Bomberman original también pasa y no afecta al juego.
- El perseguidor elige al jugador más cercano por camino. Si los dos están a la misma distancia, se queda con el primero que encuentra la búsqueda, que depende del orden de las direcciones (arriba, abajo, izquierda, derecha).
