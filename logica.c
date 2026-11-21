

#include <stdlib.h>
#include "game.h"

/* Variables internas de control de tiempo y oleadas */
static int timer_fase = 0;
int oleada_actual = 0;                 /* 0 = preparación inicial (global: la lee el render para el HUD) */
static int en_descanso = 1;            /* 1 = preparación o descanso entre oleadas */
static int zombies_por_spawnear = 0;
static int timer_spawneo = 0;
static int timer_soles_cielo = 0;

/* Función auxiliar para instanciar un sol en el pool */
static void crear_sol(int x, int y) {
    for (int s = 0; s < MAX_SOLES; s++) {
        if (soles_caidos[s].activo == 0) {
            soles_caidos[s].activo = 1;
            soles_caidos[s].x = x;
            soles_caidos[s].y = y;
            soles_caidos[s].timer_vida = (int)(segundos_vida_sol * FPS);
            break;
        }
    }
}

/* Revisa si existe al menos un zombie vivo en ese carril (para que Lanzaguisantes dispare) */
static int hay_zombie_en_carril(int carril, float desde_x) {
    for (int z = 0; z < max_zombies_oleada; z++) {
        if (horda[z].activo == 1 && horda[z].carril == carril && horda[z].x >= desde_x) {
            return 1;
        }
    }
    return 0;
}

void procesar_tick_cooldowns_y_soles(void) {
    /* Bajar cooldown de cartas */
    for (int c = 1; c <= 3; c++) {
        if (cooldown_cartas[c] > 0) {
            cooldown_cartas[c]--;
        }
    }

    /* Soles que caen periódicamente del cielo */
    timer_soles_cielo++;
    if (timer_soles_cielo >= (int)(segundos_entre_soles_cielo * FPS)) {
        int rand_x = OFFSET_X + rand() % (columnas_mapa * ANCHO_CELDA - 30);
        int rand_y = OFFSET_Y + rand() % (filas_mapa * ALTO_CELDA - 30);
        crear_sol(rand_x, rand_y);
        timer_soles_cielo = 0;
    }

    /* Descontar tiempo de vida de soles activos */
    for (int s = 0; s < MAX_SOLES; s++) {
        if (soles_caidos[s].activo == 1) {
            soles_caidos[s].timer_vida--;
            if (soles_caidos[s].timer_vida <= 0) {
                soles_caidos[s].activo = 0;
            }
        }
    }
}

void procesar_tick_oleadas_y_spawneo(void) {
    int limite_derecho = OFFSET_X + columnas_mapa * ANCHO_CELDA;

    /* Fase de preparación inicial o descanso entre oleadas */
    if (en_descanso == 1) {
        timer_fase++;
        float espera = (oleada_actual == 0) ? segundos_preparacion : segundos_descanso;
        if (timer_fase >= (int)(espera * FPS)) {
            oleada_actual++;
            if (oleada_actual > oleadas_totales) {
                estado_juego = 1; /* Victoria */
                return;
            }
            en_descanso = 0;
            timer_fase = 0;
            zombies_por_spawnear = zombies_base + (oleada_actual - 1) * zombies_extra_por_oleada;
            if (zombies_por_spawnear > max_zombies_oleada) {
                zombies_por_spawnear = max_zombies_oleada;
            }
            timer_spawneo = 0;
        }
        return;
    }

    /* Spawneo gradual durante la oleada activa */
    if (zombies_por_spawnear > 0) {
        timer_spawneo++;
        if (timer_spawneo >= 3 * FPS) { /* 1 zombie cada 3 segundos */
            for (int i = 0; i < max_zombies_oleada; i++) {
                if (horda[i].activo == 0) {
                    horda[i].activo = 1;
                    horda[i].carril = rand() % filas_mapa;
                    horda[i].x = (float)limite_derecho;
                    horda[i].estado = 0;

                    horda[i].tipo = (rand() % 5 == 0) ? 1 : 0;
                    if (horda[i].tipo == 1) {
                        horda[i].vida = vida_zombie_flag;
                        horda[i].velocidad = velocidad_zombie_flag;
                    } else {
                        horda[i].vida = vida_zombie_normal;
                        horda[i].velocidad = velocidad_zombie_normal;
                    }
                    zombies_por_spawnear--;
                    break;
                }
            }
            timer_spawneo = 0;
        }
    } else {
        /* Verificar si ya murieron todos los zombies de esta oleada */
        int vivos = 0;
        for (int i = 0; i < max_zombies_oleada; i++) {
            if (horda[i].activo == 1) vivos++;
        }
        if (vivos == 0) {
            if (oleada_actual >= oleadas_totales) {
                estado_juego = 1; /* Sobrevivió todas las oleadas: Victoria */
            } else {
                en_descanso = 1;
                timer_fase = 0;
            }
        }
    }
}

void procesar_tick_plantas(void) {
    int frames_sol = (int)(segundos_entre_soles_girasol * FPS);
    int frames_disparo = (int)(segundos_entre_disparos * FPS);

    for (int i = 0; i < filas_mapa; i++) {
        for (int j = 0; j < columnas_mapa; j++) {
            int tipo = mapa[i][j].id_tipo;

            if (tipo == TIPO_GIRASOL) {
                mapa[i][j].timer_accion++;
                if (mapa[i][j].timer_accion >= frames_sol) {
                    int sol_x = OFFSET_X + j * ANCHO_CELDA + 20;
                    int sol_y = OFFSET_Y + i * ALTO_CELDA + 30;
                    crear_sol(sol_x, sol_y);
                    mapa[i][j].timer_accion = 0;
                }
            } else if (tipo == TIPO_LANZAGUISANTES) {
                float pos_x = (float)(OFFSET_X + j * ANCHO_CELDA + ANCHO_CELDA / 2);
                if (hay_zombie_en_carril(i, pos_x)) {
                    mapa[i][j].timer_accion++;
                    if (mapa[i][j].timer_accion >= frames_disparo) {
                        for (int pr = 0; pr < MAX_PROYECTILES; pr++) {
                            if (disparos[pr].activo == 0) {
                                disparos[pr].activo = 1;
                                disparos[pr].carril = i;
                                disparos[pr].x = pos_x;
                                disparos[pr].daño = daño_proyectil;
                                break;
                            }
                        }
                        mapa[i][j].timer_accion = 0;
                    }
                }
            }
        }
    }
}

void procesar_tick_proyectiles(void) {
    float limite_derecho = (float)(OFFSET_X + columnas_mapa * ANCHO_CELDA + 50);

    for (int p = 0; p < MAX_PROYECTILES; p++) {
        if (disparos[p].activo == 1) {
            disparos[p].x += 5.0f;
            if (disparos[p].x > limite_derecho) {
                disparos[p].activo = 0;
            }
        }
    }
}

void procesar_tick_podadoras(void) {
    if (!hay_podadoras) return;
    float limite_derecho = (float)(OFFSET_X + columnas_mapa * ANCHO_CELDA + 40);

    for (int i = 0; i < filas_mapa; i++) {
        if (podadoras[i].corriendo == 1) {
            podadoras[i].x += velocidad_podadora;

            /* Elimina cualquier zombie en su carril al pasarle por encima */
            for (int z = 0; z < max_zombies_oleada; z++) {
                if (horda[z].activo == 1 && horda[z].carril == i) {
                    if (horda[z].x <= podadoras[i].x + 30.0f && horda[z].x >= podadoras[i].x - 20.0f) {
                        horda[z].activo = 0;
                    }
                }
            }

            if (podadoras[i].x > limite_derecho) {
                podadoras[i].corriendo = 0;
            }
        }
    }
}

void procesar_tick_zombies(void) {
    /* Daño por frame equivalente a daño_zombie_por_segundo (mínimo 1 para evitar truncamiento a 0) */
    int daño_frame = daño_zombie_por_segundo / FPS;
    if (daño_frame < 1) daño_frame = 1;

    for (int z = 0; z < max_zombies_oleada; z++) {
        if (horda[z].activo == 1) {
            /* Columna del zombie. Si ya cruzó el borde izquierdo del jardín no está en ninguna columna
               (la división entera trunca hacia 0 y dejaba a x entre 20 y 100 dentro de la columna 0) */
            int col_logica = -1;
            if (horda[z].x >= (float)OFFSET_X) {
                col_logica = (int)(horda[z].x - OFFSET_X) / ANCHO_CELDA;
            }

            /* 1. Colisión Zombie vs Planta o Lápida (cualquier id_tipo != TIPO_VACIO bloquea y se come) */
            if (col_logica >= 0 && col_logica < columnas_mapa &&
                mapa[horda[z].carril][col_logica].id_tipo != TIPO_VACIO) {

                horda[z].estado = 1; /* Comiendo */
                mapa[horda[z].carril][col_logica].vida -= daño_frame;

                if (mapa[horda[z].carril][col_logica].vida <= 0) {
                    mapa[horda[z].carril][col_logica].id_tipo = TIPO_VACIO;
                    mapa[horda[z].carril][col_logica].vida = 0;
                    mapa[horda[z].carril][col_logica].timer_accion = 0;
                    horda[z].estado = 0; /* Vuelve a caminar */
                }
            } else {
                horda[z].estado = 0;
                horda[z].x -= horda[z].velocidad;
            }

            /* 2. Colisión Proyectil vs Zombie */
            for (int p = 0; p < MAX_PROYECTILES; p++) {
                if (disparos[p].activo == 1 && disparos[p].carril == horda[z].carril) {
                    if (disparos[p].x >= horda[z].x && disparos[p].x <= horda[z].x + 35.0f) {
                        horda[z].vida -= disparos[p].daño;
                        disparos[p].activo = 0;

                        if (horda[z].vida <= 0) {
                            horda[z].activo = 0;
                            break;
                        }
                    }
                }
            }

            /* 3. Llegada al borde izquierdo del jardín (Podadora o Casa) */
            if (horda[z].activo == 1 && horda[z].x <= (float)OFFSET_X) {
                int c = horda[z].carril;
                if (hay_podadoras && podadoras[c].disponible == 1) {
                    podadoras[c].disponible = 0;
                    podadoras[c].corriendo = 1;
                    podadoras[c].x = (float)(OFFSET_X - 40);
                } else {
                    /* La podadora de la fila solo salva al zombie si todavía puede alcanzarlo:
                       va corriendo y su zona de choque (x - 20 .. x + 30) aún no lo dejó atrás.
                       Si ya pasó de largo (o no hay podadora), el zombie toca la casa: DERROTA inmediata.
                       El zombie queda activo para que la pantalla final lo muestre en la casa. */
                    int lo_alcanza_la_podadora = (podadoras[c].corriendo == 1 &&
                                                  podadoras[c].x - 20.0f <= horda[z].x);
                    if (!lo_alcanza_la_podadora) {
                        estado_juego = -1; /* Derrota */
                        return;
                    }
                }
            }
        }
    }
}

void actualizar_logica(void) {
    if (estado_juego != 0) return;

    procesar_tick_cooldowns_y_soles();
    procesar_tick_oleadas_y_spawneo();
    procesar_tick_plantas();
    procesar_tick_proyectiles();
    procesar_tick_podadoras();
    procesar_tick_zombies();
}
