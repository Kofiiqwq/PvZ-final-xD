#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "render.h"

/* archivos que lee el juego (cambiar aqui si se renombran) */
#define RUTA_CONFIG          "config.txt"
#define RUTA_MAPA            "mapa.txt"
#define RUTA_MAPA_ALEATORIO  "mapa_aleatorio.txt"

/* parametros del mapa aleatorio (opcion --aleatorio) */
#define ALEATORIO_FILAS      5
#define ALEATORIO_COLUMNAS   9
#define ALEATORIO_LAPIDAS    4
#define ALEATORIO_PLANTAS    3

/* duracion de un frame en nanosegundos (FPS viene de game.h) */
#define NS_POR_FRAME  (1000000000ULL / FPS)

/* ---- ESTADO GLOBAL (declarado extern en game.h) ---- */
Planta**   mapa         = NULL;
Zombie*    horda        = NULL;
Proyectil* disparos     = NULL;
Sol*       soles_caidos = NULL;
Podadora*  podadoras    = NULL;

int soles_actuales = 0;
int estado_juego   = 0;   // 0 = jugando, 1 = victoria, -1 = derrota, 2 = salir 

/* ------------------------------------------------------------
   memoria: se pide DESPUES de leer config y mapa (necesita las medidas)
   ------------------------------------------------------------ */
static void liberar_memoria(void) {
    if (mapa) {
        for (int i = 0; i < filas_mapa; i++) {
            free(mapa[i]);
        }
        free(mapa);
        mapa = NULL;
    }
    free(horda);        horda = NULL;
    free(disparos);     disparos = NULL;
    free(soles_caidos); soles_caidos = NULL;
    free(podadoras);    podadoras = NULL;
}

/* calloc deja todo en 0: todos los objetos parten inactivos / vacios */
static int inicializar_memoria(void) {
    if (filas_mapa < 1 || filas_mapa > MAXIMO_FILAS ||
        columnas_mapa < MINIMO_COLUMNAS || columnas_mapa > MAXIMO_COLUMNAS) {
        printf("Error: medidas de mapa invalidas (%d filas x %d columnas). "
               "Filas: 1-%d, columnas: %d-%d\n",
               filas_mapa, columnas_mapa, MAXIMO_FILAS, MINIMO_COLUMNAS, MAXIMO_COLUMNAS);
        return 0;
    }
    if (max_zombies_oleada < 1) {
        printf("Error: max_zombies_oleada debe ser al menos 1 en config.txt\n");
        return 0;
    }

    mapa = calloc((size_t)filas_mapa, sizeof(Planta*));
    if (!mapa) return 0;
    for (int i = 0; i < filas_mapa; i++) {
        mapa[i] = calloc((size_t)columnas_mapa, sizeof(Planta));
        if (!mapa[i]) { liberar_memoria(); return 0; }
    }

    horda        = calloc((size_t)max_zombies_oleada, sizeof(Zombie));
    disparos     = calloc(MAX_PROYECTILES, sizeof(Proyectil));
    soles_caidos = calloc(MAX_SOLES, sizeof(Sol));
    podadoras    = calloc((size_t)filas_mapa, sizeof(Podadora));
    if (!horda || !disparos || !soles_caidos || !podadoras) {
        printf("Error: no hay memoria suficiente\n");
        liberar_memoria();
        return 0;
    }

    soles_actuales = soles_iniciales;
    estado_juego = 0;
    for (int c = 0; c < 4; c++) cooldown_cartas[c] = 0;
    return 1;
}

/* ------------------------------------------------------------
   pantalla final: deja la escena congelada con el cartel de
   victoria/derrota hasta que el jugador presione ESC/Enter o cierre
   ------------------------------------------------------------ */
static void esperar_cierre_en_pantalla_final(void) {
    int salir = 0;
    while (!salir) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) {
                salir = 1;
            } else if (evento.type == SDL_EVENT_KEY_DOWN) {
                if (evento.key.key == SDLK_ESCAPE || evento.key.key == SDLK_RETURN) {
                    salir = 1;
                }
            }
        }
        renderizar_pantalla(0);
        SDL_Delay(16);
    }
}

/* ------------------------------------------------------------
   main
   ------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    const char *ruta_mapa = RUTA_MAPA;
    srand((unsigned int)time(NULL));

    /* opcional: generar un mapa al azar y jugar con el */
    if (argc > 1 && strcmp(argv[1], "--aleatorio") == 0) {
        if (generar_mapa_aleatorio(RUTA_MAPA_ALEATORIO, ALEATORIO_FILAS, ALEATORIO_COLUMNAS,
                                   ALEATORIO_LAPIDAS, ALEATORIO_PLANTAS)) {
            ruta_mapa = RUTA_MAPA_ALEATORIO;
        } else {
            printf("No se pudo generar el mapa aleatorio, se usa '%s'\n", RUTA_MAPA);
        }
    }

    /* 1. lectura: config.txt + medidas del mapa (antes de pedir memoria) */
    if (!leer_archivos_configuracion(RUTA_CONFIG, ruta_mapa)) {
        return 1;
    }
    mostrar_configuracion_leida();

    /* 2. memoria dinamica (con las medidas ya leidas) */
    if (!inicializar_memoria()) {
        return 1;
    }

    /* 3. llenar mapa y podadoras desde el archivo */
    if (!cargar_mapa_inicial(ruta_mapa)) {
        printf("Error: no se pudo cargar el mapa '%s'\n", ruta_mapa);
        liberar_memoria();
        return 1;
    }

    /* 4. graficos (ventana del tamano del tablero) */
    if (!inicializar_sdl()) {
        liberar_memoria();
        return 1;
    }

    int carta_seleccionada = 0;   /* 0 = ninguna, 1 = girasol, 2 = lanzaguisantes, 3 = nuez */

    /* ---- GAME LOOP ---- */
    while (estado_juego == 0) {
        Uint64 inicio = SDL_GetTicksNS();

        procesar_entradas(&carta_seleccionada);   /* mouse y teclado */
        actualizar_logica();                      /* reglas del juego */
        renderizar_pantalla(carta_seleccionada);  /* dibujo */

        /* control de FPS */
        Uint64 duracion = SDL_GetTicksNS() - inicio;
        if (duracion < NS_POR_FRAME) {
            SDL_DelayNS(NS_POR_FRAME - duracion);
        }
    }

    /* ---- FIN ---- */
    if (estado_juego == 1)  printf("VICTORIA: sobreviviste las %d oleadas\n", oleadas_totales);
    if (estado_juego == -1) printf("DERROTA: un zombie llego a la casa\n");
    if (estado_juego == 1 || estado_juego == -1) {
        esperar_cierre_en_pantalla_final();
    }

    cerrar_sdl();
    liberar_memoria();
    return 0;
}
