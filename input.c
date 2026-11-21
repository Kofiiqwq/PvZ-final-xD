

#include <SDL3/SDL.h>
#include "game.h"

int cooldown_cartas[4] = {0, 0, 0, 0};

static int costo_de_la_carta(int id_carta) {
    if (id_carta == TIPO_GIRASOL)        return costo_girasol;
    if (id_carta == TIPO_LANZAGUISANTES) return costo_lanzaguisantes;
    if (id_carta == TIPO_NUEZ)           return costo_nuez;
    return 0;
}

static int vida_base_de_la_planta(int id_carta) {
    if (id_carta == TIPO_GIRASOL)        return vida_girasol;
    if (id_carta == TIPO_LANZAGUISANTES) return vida_lanzaguisantes;
    if (id_carta == TIPO_NUEZ)           return vida_nuez;
    return 0;
}

static int frames_de_cooldown(int id_carta) {
    if (id_carta == TIPO_GIRASOL)        return COOLDOWN_GIRASOL * FPS;
    if (id_carta == TIPO_LANZAGUISANTES) return COOLDOWN_LANZAGUISANTES * FPS;
    if (id_carta == TIPO_NUEZ)           return COOLDOWN_NUEZ * FPS;
    return 0;
}

static int esta_adentro(int x, int y, int rect_x, int rect_y, int ancho, int alto) {
    if (x < rect_x)          return 0;
    if (x >= rect_x + ancho) return 0;
    if (y < rect_y)          return 0;
    if (y >= rect_y + alto)  return 0;
    return 1;
}

static int que_carta_toco(int clic_x, int clic_y) {
    for (int id_carta = TIPO_GIRASOL; id_carta <= TIPO_NUEZ; id_carta++) {
        int posicion = id_carta - 1;
        int carta_x = CARTA_X + posicion * (CARTA_ANCHO + CARTA_SEPARACION);
        if (esta_adentro(clic_x, clic_y, carta_x, CARTA_Y, CARTA_ANCHO, CARTA_ALTO)) {
            return id_carta;
        }
    }
    return 0;
}

/* Revisa si el clic tocó algún sol activo y lo recoge */
static int recoger_sol(int clic_x, int clic_y) {
    for (int i = 0; i < MAX_SOLES; i++) {
        if (soles_caidos[i].activo == 1) {
            /* Hitbox de 40x40 píxeles alrededor del sol */
            if (esta_adentro(clic_x, clic_y, soles_caidos[i].x, soles_caidos[i].y, 40, 40)) {
                soles_caidos[i].activo = 0;
                soles_actuales += valor_de_cada_sol;
                return 1;
            }
        }
    }
    return 0;
}

static void elegir_carta(int id_carta, int *carta_seleccionada) {
    if (*carta_seleccionada == id_carta) {
        *carta_seleccionada = 0;
        return;
    }
    if (cooldown_cartas[id_carta] > 0) return;
    if (soles_actuales < costo_de_la_carta(id_carta)) return;

    *carta_seleccionada = id_carta;
}

static void plantar_en_el_pasto(int clic_x, int clic_y, int *carta_seleccionada) {
    int ancho_del_pasto = columnas_mapa * ANCHO_CELDA;
    int alto_del_pasto = filas_mapa * ALTO_CELDA;

    if (!esta_adentro(clic_x, clic_y, OFFSET_X, OFFSET_Y, ancho_del_pasto, alto_del_pasto)) {
        return;
    }

    int fila = (clic_y - OFFSET_Y) / ALTO_CELDA;
    int columna = (clic_x - OFFSET_X) / ANCHO_CELDA;

    if (mapa[fila][columna].id_tipo != TIPO_VACIO) {
        return;
    }

    int costo = costo_de_la_carta(*carta_seleccionada);
    if (soles_actuales < costo) {
        return;
    }

    mapa[fila][columna].id_tipo = *carta_seleccionada;
    mapa[fila][columna].vida = vida_base_de_la_planta(*carta_seleccionada);
    mapa[fila][columna].timer_accion = 0;

    soles_actuales -= costo;
    cooldown_cartas[*carta_seleccionada] = frames_de_cooldown(*carta_seleccionada);
    *carta_seleccionada = 0;
}

int procesar_entradas(int* carta_seleccionada) {
    SDL_Event evento;
    int quiere_salir = 0;

    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_EVENT_QUIT) {
            quiere_salir = 1;
        } else if (evento.type == SDL_EVENT_KEY_DOWN) {
            if (evento.key.key == SDLK_ESCAPE) {
                quiere_salir = 1;
            }
        } else if (evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            int clic_x = (int)evento.button.x;
            int clic_y = (int)evento.button.y;

            if (evento.button.button == SDL_BUTTON_RIGHT) {
                *carta_seleccionada = 0;
            } else if (evento.button.button == SDL_BUTTON_LEFT) {
                /* Prioridad 1: recoger soles en pantalla */
                if (recoger_sol(clic_x, clic_y)) {
                    continue;
                }
                /* Prioridad 2: seleccionar carta de la tienda */
                int carta_tocada = que_carta_toco(clic_x, clic_y);
                if (carta_tocada != 0) {
                    elegir_carta(carta_tocada, carta_seleccionada);
                } else if (*carta_seleccionada != 0) {
                    /* Prioridad 3: plantar en el jardín */
                    plantar_en_el_pasto(clic_x, clic_y, carta_seleccionada);
                }
            }
        }
    }

    if (quiere_salir) {
        estado_juego = 2;
        return 1;
    }
    return 0;
}
