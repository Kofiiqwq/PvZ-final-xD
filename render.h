#ifndef RENDER_H
#define RENDER_H

/* ============================================================
   render.h - interfaz del modulo grafico (SDL3_render.c)

   El render SOLO LEE el estado global de game.h (mapa, horda,
   disparos, soles_caidos, podadoras, soles_actuales, estado_juego,
   cooldown_cartas). Nunca modifica vida, posiciones ni soles.
   ============================================================ */

/* Crea la ventana, el renderer y carga las texturas.
   Debe llamarse DESPUES de leer config/mapa (necesita filas_mapa y
   columnas_mapa para calcular el tamano de la ventana).
   Devuelve 1 si salio bien, 0 si hubo error (ya se avisa en consola). */
int  inicializar_sdl(void);

/* Dibuja un frame completo. carta_seleccionada: 0 = ninguna,
   1 = girasol, 2 = lanzaguisantes, 3 = nuez.
   Si estado_juego es 1 o -1 dibuja ademas la pantalla de victoria/derrota. */
void renderizar_pantalla(int carta_seleccionada);

/* Libera texturas, renderer y ventana, y cierra SDL. */
void cerrar_sdl(void);

#endif
