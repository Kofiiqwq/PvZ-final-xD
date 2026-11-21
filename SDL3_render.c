/* ============================================================
   SDL3_render.c  -  modulo de graficos (SDL3)

   - Solo LEE el estado global de game.h. No modifica nada del juego.
   - Las medidas (OFFSET_X, ANCHO_CELDA, CARTA_X, ...) vienen de game.h,
     asi lo que se dibuja coincide con donde input.c detecta los clics.
   - Si una imagen no existe, se dibuja una figura basica en su lugar,
     asi el juego siempre se ve aunque todavia no haya arte.

   COMO CAMBIAR LAS IMAGENES:
     1) Edita la seccion "RUTAS DE ARCHIVOS" (justo abajo).
     2) Deja los archivos en esas rutas (por defecto, carpeta assets/).

   FORMATO DE IMAGEN:
     - Por defecto usa solo SDL3 y carga archivos .bmp
     - Para usar .png (u otros), compila con SDL3_image y la bandera
       -DUSAR_SDL_IMAGE  (ver comando de compilacion en main.c). En ese caso
       las extensiones por defecto pasan a ser .png automaticamente.
   ============================================================ */

#include <SDL3/SDL.h>
#ifdef USAR_SDL_IMAGE
#include <SDL3_image/SDL_image.h>
#endif
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"

/* ============================================================
   RUTAS DE ARCHIVOS  (aqui se cambian las imagenes)
   ============================================================ */
#ifdef USAR_SDL_IMAGE
    #define EXT_IMAGEN ".png"
#else
    #define EXT_IMAGEN ".bmp"
#endif

#define CARPETA_IMAGENES  "assets/"

/* escenario */
#define RUTA_FONDO              CARPETA_IMAGENES "fondo"             EXT_IMAGEN  /* ventana completa */

/* Zona de pasto DENTRO de la imagen de fondo, en pixeles de la imagen original
   (esquina superior izquierda + ancho/alto). El render escala y desplaza el fondo
   para que este rectangulo coincida exactamente con el tablero
   (OFFSET_X/OFFSET_Y + columnas_mapa*ANCHO_CELDA x filas_mapa*ALTO_CELDA).
   Si cambias fondo.*, mide el pasto de la nueva imagen y ajusta estos 4 numeros.
   Valores actuales: fondo de 1400x600 con 9 columnas x 5 filas. */
#define FONDO_PASTO_X   250.0f
#define FONDO_PASTO_Y    75.0f
#define FONDO_PASTO_W   745.0f
#define FONDO_PASTO_H   500.0f
#define RUTA_PANEL_TIENDA       CARPETA_IMAGENES "panel_tienda"      EXT_IMAGEN  /* barra superior */

/* plantas y obstaculos (se dibujan dentro de una celda) */
#define RUTA_GIRASOL            CARPETA_IMAGENES "girasol"           EXT_IMAGEN
#define RUTA_LANZAGUISANTES     CARPETA_IMAGENES "lanzaguisantes"    EXT_IMAGEN
#define RUTA_NUEZ               CARPETA_IMAGENES "nuez"              EXT_IMAGEN
#define RUTA_LAPIDA             CARPETA_IMAGENES "lapida"            EXT_IMAGEN

/* cartas de la tienda (si no existen, se usa la imagen de la planta) */
#define RUTA_CARTA_GIRASOL         CARPETA_IMAGENES "carta_girasol"        EXT_IMAGEN
#define RUTA_CARTA_LANZAGUISANTES  CARPETA_IMAGENES "carta_lanzaguisantes" EXT_IMAGEN
#define RUTA_CARTA_NUEZ            CARPETA_IMAGENES "carta_nuez"           EXT_IMAGEN

/* zombies (si falta la version "comiendo", se usa la de caminar) */
#define RUTA_ZOMBIE_NORMAL           CARPETA_IMAGENES "zombie_normal"           EXT_IMAGEN
#define RUTA_ZOMBIE_NORMAL_COMIENDO  CARPETA_IMAGENES "zombie_normal_comiendo"  EXT_IMAGEN
#define RUTA_ZOMBIE_FLAG             CARPETA_IMAGENES "zombie_flag"             EXT_IMAGEN
#define RUTA_ZOMBIE_FLAG_COMIENDO    CARPETA_IMAGENES "zombie_flag_comiendo"    EXT_IMAGEN

/* otros objetos */
#define RUTA_GUISANTE           CARPETA_IMAGENES "guisante"          EXT_IMAGEN
#define RUTA_SOL                CARPETA_IMAGENES "sol"               EXT_IMAGEN
#define RUTA_PODADORA           CARPETA_IMAGENES "podadora"          EXT_IMAGEN

/* pantallas finales (ocupan toda la ventana, se dibujan sobre el juego) */
#define RUTA_VICTORIA           CARPETA_IMAGENES "victoria"          EXT_IMAGEN
#define RUTA_DERROTA            CARPETA_IMAGENES "derrota"           EXT_IMAGEN

/* ============================================================
   TEXTURAS: identificadores y tabla de rutas
   ============================================================ */
typedef enum {
    TEX_FONDO,
    TEX_PANEL_TIENDA,
    TEX_GIRASOL,
    TEX_LANZAGUISANTES,
    TEX_NUEZ,
    TEX_LAPIDA,
    TEX_CARTA_GIRASOL,
    TEX_CARTA_LANZAGUISANTES,
    TEX_CARTA_NUEZ,
    TEX_ZOMBIE_NORMAL,
    TEX_ZOMBIE_NORMAL_COMIENDO,
    TEX_ZOMBIE_FLAG,
    TEX_ZOMBIE_FLAG_COMIENDO,
    TEX_GUISANTE,
    TEX_SOL,
    TEX_PODADORA,
    TEX_VICTORIA,
    TEX_DERROTA,
    TEX_TOTAL
} IdTextura;

static const char *rutas_texturas[TEX_TOTAL] = {
    [TEX_FONDO]                   = RUTA_FONDO,
    [TEX_PANEL_TIENDA]            = RUTA_PANEL_TIENDA,
    [TEX_GIRASOL]                 = RUTA_GIRASOL,
    [TEX_LANZAGUISANTES]          = RUTA_LANZAGUISANTES,
    [TEX_NUEZ]                    = RUTA_NUEZ,
    [TEX_LAPIDA]                  = RUTA_LAPIDA,
    [TEX_CARTA_GIRASOL]           = RUTA_CARTA_GIRASOL,
    [TEX_CARTA_LANZAGUISANTES]    = RUTA_CARTA_LANZAGUISANTES,
    [TEX_CARTA_NUEZ]              = RUTA_CARTA_NUEZ,
    [TEX_ZOMBIE_NORMAL]           = RUTA_ZOMBIE_NORMAL,
    [TEX_ZOMBIE_NORMAL_COMIENDO]  = RUTA_ZOMBIE_NORMAL_COMIENDO,
    [TEX_ZOMBIE_FLAG]             = RUTA_ZOMBIE_FLAG,
    [TEX_ZOMBIE_FLAG_COMIENDO]    = RUTA_ZOMBIE_FLAG_COMIENDO,
    [TEX_GUISANTE]                = RUTA_GUISANTE,
    [TEX_SOL]                     = RUTA_SOL,
    [TEX_PODADORA]                = RUTA_PODADORA,
    [TEX_VICTORIA]                = RUTA_VICTORIA,
    [TEX_DERROTA]                 = RUTA_DERROTA
};

/* ============================================================
   MEDIDAS PROPIAS DEL DIBUJO (las del tablero vienen de game.h)
   ============================================================ */
#define MARGEN_CELDA        8     /* espacio entre la planta y el borde de su celda */
#define MARGEN_DERECHO      ANCHO_CELDA  /* zona a la derecha del pasto: ahi nacen los zombies */
#define MARGEN_INFERIOR     10
#define ZOMBIE_ANCHO        60    /* el zombie se dibuja desde horda[i].x hacia la derecha */
#define ZOMBIE_ALTO         90
#define SOL_TAM             40    /* coincide con el hitbox de 40x40 de input.c */
#define GUISANTE_RADIO      8
#define PODADORA_ANCHO      60
#define PODADORA_ALTO       60

/* ============================================================
   ESTADO INTERNO DEL MODULO
   ============================================================ */
static SDL_Window   *ventana  = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture  *texturas[TEX_TOTAL];
static int ancho_ventana = 0;
static int alto_ventana  = 0;
static Uint8 alfa_actual = 255;   /* transparencia global del dibujo (para la planta "fantasma") */

/* ============================================================
   CARGA DE TEXTURAS
   ============================================================ */
static SDL_Texture *cargar_textura(const char *ruta) {
#ifdef USAR_SDL_IMAGE
    return IMG_LoadTexture(renderer, ruta);
#else
    SDL_Surface *superficie = SDL_LoadBMP(ruta);
    if (!superficie) return NULL;
    SDL_Texture *textura = SDL_CreateTextureFromSurface(renderer, superficie);
    SDL_DestroySurface(superficie);
    return textura;
#endif
}

static void cargar_todas_las_texturas(void) {
    int faltan = 0;
    for (int i = 0; i < TEX_TOTAL; i++) {
        texturas[i] = cargar_textura(rutas_texturas[i]);
        if (texturas[i]) {
            SDL_SetTextureBlendMode(texturas[i], SDL_BLENDMODE_BLEND);
        } else {
            faltan++;
            SDL_Log("[render] no se encontro '%s': se dibuja una figura basica", rutas_texturas[i]);
        }
    }
    if (faltan > 0) {
        SDL_Log("[render] %d de %d imagenes no se cargaron (el juego funciona igual)", faltan, TEX_TOTAL);
    }
}

/* ============================================================
   AYUDANTES DE DIBUJO
   ============================================================ */
static void color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, (Uint8)((a * alfa_actual) / 255));
}

static void rellenar(float x, float y, float w, float h) {
    SDL_FRect r = { x, y, w, h };
    SDL_RenderFillRect(renderer, &r);
}

static void borde(float x, float y, float w, float h) {
    SDL_FRect r = { x, y, w, h };
    SDL_RenderRect(renderer, &r);
}

static void elipse(float cx, float cy, float rx, float ry) {
    if (rx < 1.0f || ry < 1.0f) return;
    for (int dy = -(int)ry; dy <= (int)ry; dy++) {
        float t = 1.0f - ((float)(dy * dy)) / (ry * ry);
        if (t < 0.0f) continue;
        float dx = rx * sqrtf(t);
        rellenar(cx - dx, cy + (float)dy, 2.0f * dx, 1.0f);
    }
}

static void circulo(float cx, float cy, float r) {
    elipse(cx, cy, r, r);
}

/* texto con la fuente de depuracion incluida en SDL3 (8x8 px por letra), con escala */
static void texto(float x, float y, float escala, const char *cadena) {
    SDL_SetRenderScale(renderer, escala, escala);
    SDL_RenderDebugText(renderer, x / escala, y / escala, cadena);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

static float ancho_texto(float escala, const char *cadena) {
    return (float)SDL_strlen(cadena) * 8.0f * escala;
}

/* dibuja la textura si existe. devuelve 1 si la dibujo, 0 si no hay textura */
static int dibujar_textura(IdTextura id, const SDL_FRect *destino) {
    if (!texturas[id]) return 0;
    SDL_SetTextureAlphaMod(texturas[id], alfa_actual);
    SDL_RenderTexture(renderer, texturas[id], NULL, destino);
    return 1;
}

static void barra_de_vida(float x, float y, float ancho, int vida, int vida_maxima) {
    if (vida_maxima <= 0 || vida >= vida_maxima) return;   /* solo se ve si recibio dano */
    if (vida < 0) vida = 0;
    float fraccion = (float)vida / (float)vida_maxima;
    color(0, 0, 0, 180);
    rellenar(x, y, ancho, 5.0f);
    if (fraccion > 0.5f)       color(60, 200, 60, 255);
    else if (fraccion > 0.25f) color(230, 200, 40, 255);
    else                       color(220, 50, 50, 255);
    rellenar(x + 1.0f, y + 1.0f, (ancho - 2.0f) * fraccion, 3.0f);
}

static int vida_maxima_de_planta(int tipo) {
    switch (tipo) {
        case TIPO_GIRASOL:        return vida_girasol;
        case TIPO_LANZAGUISANTES: return vida_lanzaguisantes;
        case TIPO_NUEZ:           return vida_nuez;
        case TIPO_LAPIDA:         return vida_lapida;
        default:                  return 0;
    }
}

static int costo_de_planta(int tipo) {
    switch (tipo) {
        case TIPO_GIRASOL:        return costo_girasol;
        case TIPO_LANZAGUISANTES: return costo_lanzaguisantes;
        case TIPO_NUEZ:           return costo_nuez;
        default:                  return 0;
    }
}

static int cooldown_total_de_carta(int tipo) {
    switch (tipo) {
        case TIPO_GIRASOL:        return COOLDOWN_GIRASOL * FPS;
        case TIPO_LANZAGUISANTES: return COOLDOWN_LANZAGUISANTES * FPS;
        case TIPO_NUEZ:           return COOLDOWN_NUEZ * FPS;
        default:                  return 1;
    }
}

/* ============================================================
   DIBUJO DE OBJETOS (imagen si existe, figura basica si no)
   ============================================================ */

/* dibuja una planta o lapida dentro del rectangulo "r" */
static void dibujar_planta(int tipo, SDL_FRect r) {
    float cx = r.x + r.w / 2.0f;
    float cy = r.y + r.h / 2.0f;

    switch (tipo) {
    case TIPO_GIRASOL:
        if (dibujar_textura(TEX_GIRASOL, &r)) return;
        color(40, 140, 40, 255);
        rellenar(cx - 3.0f, cy, 6.0f, r.h / 2.0f);                    /* tallo */
        color(250, 210, 30, 255);
        circulo(cx, cy - r.h * 0.12f, r.w * 0.38f);                   /* petalos */
        color(130, 80, 20, 255);
        circulo(cx, cy - r.h * 0.12f, r.w * 0.2f);                    /* centro */
        break;

    case TIPO_LANZAGUISANTES:
        if (dibujar_textura(TEX_LANZAGUISANTES, &r)) return;
        color(40, 140, 40, 255);
        rellenar(cx - 3.0f, cy, 6.0f, r.h / 2.0f);                    /* tallo */
        color(90, 200, 70, 255);
        circulo(cx - 4.0f, cy - r.h * 0.12f, r.w * 0.32f);            /* cabeza */
        color(60, 160, 50, 255);
        rellenar(cx + r.w * 0.15f, cy - r.h * 0.2f, r.w * 0.3f, r.h * 0.18f); /* canon */
        color(20, 60, 20, 255);
        circulo(cx - 8.0f, cy - r.h * 0.18f, 3.0f);                   /* ojo */
        break;

    case TIPO_NUEZ:
        if (dibujar_textura(TEX_NUEZ, &r)) return;
        color(170, 120, 60, 255);
        elipse(cx, cy, r.w * 0.38f, r.h * 0.45f);
        color(60, 30, 10, 255);
        circulo(cx - 8.0f, cy - 6.0f, 3.0f);                          /* ojos */
        circulo(cx + 8.0f, cy - 6.0f, 3.0f);
        rellenar(cx - 7.0f, cy + 8.0f, 14.0f, 2.0f);                  /* boca */
        break;

    case TIPO_LAPIDA:
        if (dibujar_textura(TEX_LAPIDA, &r)) return;
        color(130, 130, 140, 255);
        rellenar(r.x + r.w * 0.15f, cy - r.h * 0.05f, r.w * 0.7f, r.h * 0.5f);
        elipse(cx, cy - r.h * 0.05f, r.w * 0.35f, r.h * 0.3f);        /* parte redondeada */
        color(70, 70, 80, 255);
        rellenar(cx - 2.0f, cy - r.h * 0.15f, 4.0f, r.h * 0.4f);      /* cruz */
        rellenar(cx - 9.0f, cy - r.h * 0.05f, 18.0f, 4.0f);
        break;

    default:
        break;
    }
}

static void dibujar_zombie(const Zombie *z) {
    float x = z->x;
    float y = (float)OFFSET_Y + (float)z->carril * ALTO_CELDA + (ALTO_CELDA - ZOMBIE_ALTO) / 2.0f;
    SDL_FRect r = { x, y, ZOMBIE_ANCHO, ZOMBIE_ALTO };
    int es_flag  = (z->tipo == 1);
    int comiendo = (z->estado == 1);

    /* elegir textura: si falta la de "comiendo", usar la de caminar */
    IdTextura caminando = es_flag ? TEX_ZOMBIE_FLAG : TEX_ZOMBIE_NORMAL;
    IdTextura comer     = es_flag ? TEX_ZOMBIE_FLAG_COMIENDO : TEX_ZOMBIE_NORMAL_COMIENDO;
    IdTextura elegida   = (comiendo && texturas[comer]) ? comer : caminando;

    if (!dibujar_textura(elegida, &r)) {
        /* figura basica */
        if (comiendo) color(150, 130, 90, 255); else color(120, 140, 110, 255);
        rellenar(x + 12.0f, y + 35.0f, 36.0f, 55.0f);                 /* cuerpo */
        color(150, 170, 130, 255);
        circulo(x + 30.0f, y + 22.0f, 17.0f);                         /* cabeza */
        color(20, 20, 20, 255);
        circulo(x + 23.0f, y + 20.0f, 3.0f);                          /* ojos */
        circulo(x + 34.0f, y + 20.0f, 3.0f);
        color(100, 70, 50, 255);
        rellenar(x + 2.0f, y + 45.0f, 14.0f, 6.0f);                   /* brazo al frente */
        if (es_flag) {
            color(110, 80, 40, 255);
            rellenar(x + 46.0f, y - 6.0f, 3.0f, 60.0f);               /* asta */
            color(200, 40, 40, 255);
            rellenar(x + 49.0f, y - 6.0f, 20.0f, 14.0f);              /* bandera */
        }
    }
    barra_de_vida(x + 8.0f, y - 6.0f, 44.0f, z->vida,
                  es_flag ? vida_zombie_flag : vida_zombie_normal);
}

static void dibujar_guisante(const Proyectil *p) {
    float cy = (float)OFFSET_Y + (float)p->carril * ALTO_CELDA + ALTO_CELDA * 0.35f;
    SDL_FRect r = { p->x - GUISANTE_RADIO, cy - GUISANTE_RADIO,
                    GUISANTE_RADIO * 2.0f, GUISANTE_RADIO * 2.0f };
    if (dibujar_textura(TEX_GUISANTE, &r)) return;
    color(70, 190, 60, 255);
    circulo(p->x, cy, GUISANTE_RADIO);
    color(150, 240, 130, 255);
    circulo(p->x - 2.0f, cy - 2.0f, 3.0f);
}

static void dibujar_sol(const Sol *s) {
    /* parpadea durante el ultimo segundo y medio para avisar que va a desaparecer */
    if (s->timer_vida < (FPS * 3) / 2 && ((SDL_GetTicks() / 120) % 2 == 0)) return;
    SDL_FRect r = { (float)s->x, (float)s->y, SOL_TAM, SOL_TAM };
    if (dibujar_textura(TEX_SOL, &r)) return;
    float cx = r.x + SOL_TAM / 2.0f;
    float cy = r.y + SOL_TAM / 2.0f;
    color(255, 230, 80, 140);
    circulo(cx, cy, SOL_TAM / 2.0f);                                  /* halo */
    color(255, 210, 30, 255);
    circulo(cx, cy, SOL_TAM / 2.0f - 6.0f);
    color(255, 245, 160, 255);
    circulo(cx - 3.0f, cy - 3.0f, 5.0f);
}

static void dibujar_podadora(float x, int carril) {
    float y = (float)OFFSET_Y + (float)carril * ALTO_CELDA + (ALTO_CELDA - PODADORA_ALTO) / 2.0f;
    SDL_FRect r = { x, y, PODADORA_ANCHO, PODADORA_ALTO };
    if (dibujar_textura(TEX_PODADORA, &r)) return;
    color(200, 40, 40, 255);
    rellenar(x, y, PODADORA_ANCHO, PODADORA_ALTO - 8.0f);             /* cuerpo */
    color(60, 60, 60, 255);
    rellenar(x - 6.0f, y - 8.0f, 4.0f, PODADORA_ALTO);                /* manilla */
    circulo(x + 8.0f, y + PODADORA_ALTO - 6.0f, 6.0f);                /* ruedas */
    circulo(x + PODADORA_ANCHO - 8.0f, y + PODADORA_ALTO - 6.0f, 6.0f);
}

/* ============================================================
   ESCENA
   ============================================================ */
static void dibujar_fondo(void) {
    /* con imagen de fondo: se escala para que su zona de pasto calce con el tablero */
    if (texturas[TEX_FONDO]) {
        float ancho_img = 0, alto_img = 0;
        SDL_GetTextureSize(texturas[TEX_FONDO], &ancho_img, &alto_img);

        /* factor para que el pasto de la imagen mida lo mismo que el tablero */
        float escala_x = (float)(columnas_mapa * ANCHO_CELDA) / FONDO_PASTO_W;
        float escala_y = (float)(filas_mapa * ALTO_CELDA)     / FONDO_PASTO_H;

        SDL_FRect destino = {
            (float)OFFSET_X - FONDO_PASTO_X * escala_x,   /* el pasto empieza en OFFSET_X */
            (float)OFFSET_Y - FONDO_PASTO_Y * escala_y,   /* ...y en OFFSET_Y */
            ancho_img * escala_x,
            alto_img  * escala_y
        };
        dibujar_textura(TEX_FONDO, &destino);
        return;
    }

    /* sin imagen de fondo: cielo, casa (franja izquierda) y pasto cuadriculado */
    color(120, 190, 235, 255);
    SDL_RenderClear(renderer);
    color(150, 110, 80, 255);
    rellenar(0, OFFSET_Y, OFFSET_X, (float)filas_mapa * ALTO_CELDA);            /* casa */
    for (int i = 0; i < filas_mapa; i++) {
        for (int j = 0; j < columnas_mapa; j++) {
            if ((i + j) % 2 == 0) color(95, 175, 70, 255);
            else                  color(110, 190, 80, 255);
            rellenar((float)(OFFSET_X + j * ANCHO_CELDA), (float)(OFFSET_Y + i * ALTO_CELDA),
                     ANCHO_CELDA, ALTO_CELDA);
        }
    }
    color(60, 120, 50, 255);
    borde((float)OFFSET_X, (float)OFFSET_Y,
          (float)(columnas_mapa * ANCHO_CELDA), (float)(filas_mapa * ALTO_CELDA));
}

static void dibujar_tienda_y_hud(int carta_seleccionada) {
    /* panel superior */
    SDL_FRect panel = { 0, 0, (float)ancho_ventana, OFFSET_Y - 10 };
    if (!dibujar_textura(TEX_PANEL_TIENDA, &panel)) {
        color(90, 60, 35, 255);
        rellenar(panel.x, panel.y, panel.w, panel.h);
    }

    /* cartas */
    for (int id = TIPO_GIRASOL; id <= TIPO_NUEZ; id++) {
        float cx = (float)(CARTA_X + (id - 1) * (CARTA_ANCHO + CARTA_SEPARACION));
        float cy = (float)CARTA_Y;
        SDL_FRect carta = { cx, cy, CARTA_ANCHO, CARTA_ALTO };
        int costo = costo_de_planta(id);

        color(235, 220, 170, 255);
        rellenar(carta.x, carta.y, carta.w, carta.h);

        /* icono: textura de la carta, o de la planta, o figura basica */
        SDL_FRect icono = { cx + 6.0f, cy + 6.0f, CARTA_ANCHO - 12.0f, CARTA_ALTO - 32.0f };
        IdTextura tex_carta = (IdTextura)(TEX_CARTA_GIRASOL + (id - 1));
        if (!dibujar_textura(tex_carta, &icono)) {
            dibujar_planta(id, icono);
        }

        /* costo */
        char buffer[16];
        SDL_snprintf(buffer, sizeof(buffer), "%d", costo);
        color(40, 25, 10, 255);
        texto(cx + (CARTA_ANCHO - ancho_texto(2.0f, buffer)) / 2.0f, cy + CARTA_ALTO - 22.0f, 2.0f, buffer);

        /* no alcanzan los soles: oscurecer */
        if (soles_actuales < costo) {
            color(0, 0, 0, 110);
            rellenar(carta.x, carta.y, carta.w, carta.h);
        }
        /* cooldown: cortina oscura que baja segun el tiempo que falta */
        if (cooldown_cartas[id] > 0) {
            float fraccion = (float)cooldown_cartas[id] / (float)cooldown_total_de_carta(id);
            if (fraccion > 1.0f) fraccion = 1.0f;
            color(0, 0, 0, 160);
            rellenar(carta.x, carta.y, carta.w, carta.h * fraccion);
        }
        /* borde: amarillo grueso si esta en la mano */
        if (carta_seleccionada == id) {
            color(255, 235, 60, 255);
            borde(carta.x - 1.0f, carta.y - 1.0f, carta.w + 2.0f, carta.h + 2.0f);
            borde(carta.x - 2.0f, carta.y - 2.0f, carta.w + 4.0f, carta.h + 4.0f);
        } else {
            color(60, 40, 20, 255);
            borde(carta.x, carta.y, carta.w, carta.h);
        }
    }

    /* contador de soles */
    float sx = (float)(CARTA_X + 3 * (CARTA_ANCHO + CARTA_SEPARACION) + 20);
    float sy = (float)CARTA_Y;
    color(235, 220, 170, 255);
    rellenar(sx, sy, 130.0f, 50.0f);
    color(60, 40, 20, 255);
    borde(sx, sy, 130.0f, 50.0f);
    SDL_FRect icono_sol = { sx + 6.0f, sy + 5.0f, 40.0f, 40.0f };
    if (!dibujar_textura(TEX_SOL, &icono_sol)) {
        color(255, 210, 30, 255);
        circulo(sx + 26.0f, sy + 25.0f, 17.0f);
        color(255, 245, 160, 255);
        circulo(sx + 22.0f, sy + 21.0f, 5.0f);
    }
    char soles[16];
    SDL_snprintf(soles, sizeof(soles), "%d", soles_actuales);
    color(40, 25, 10, 255);
    texto(sx + 54.0f, sy + 16.0f, 3.0f, soles);

    /* progreso: oleada actual */
    char progreso[32];
    color(255, 255, 255, 255);
    if (oleada_actual == 0) SDL_snprintf(progreso, sizeof(progreso), "PREPARACION");
    else                    SDL_snprintf(progreso, sizeof(progreso), "OLEADA %d/%d", oleada_actual, oleadas_totales);
    texto(sx, sy + 56.0f, 2.0f, progreso);

    /* ayuda */
    color(255, 255, 255, 255);
    color(255, 255, 255, 255);
    texto(sx + 150.0f, sy + 8.0f,  1.0f, "Clic: elegir/plantar");
    texto(sx + 150.0f, sy + 22.0f, 1.0f, "Clic der: soltar");
    texto(sx + 150.0f, sy + 36.0f, 1.0f, "ESC: salir");
}

/* planta "fantasma" en la celda bajo el mouse cuando hay una semilla en la mano */
static void dibujar_fantasma(int carta_seleccionada) {
    if (carta_seleccionada == 0) return;

    float mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);

    if (mx < OFFSET_X || my < OFFSET_Y) return;
    int columna = ((int)mx - OFFSET_X) / ANCHO_CELDA;
    int fila    = ((int)my - OFFSET_Y) / ALTO_CELDA;
    if (columna < 0 || columna >= columnas_mapa || fila < 0 || fila >= filas_mapa) return;

    float x = (float)(OFFSET_X + columna * ANCHO_CELDA);
    float y = (float)(OFFSET_Y + fila * ALTO_CELDA);
    int libre = (mapa[fila][columna].id_tipo == TIPO_VACIO);

    if (libre) color(255, 255, 255, 70); else color(230, 40, 40, 90);
    rellenar(x, y, ANCHO_CELDA, ALTO_CELDA);

    if (libre) {
        Uint8 guardado = alfa_actual;
        alfa_actual = 150;
        SDL_FRect r = { x + MARGEN_CELDA, y + MARGEN_CELDA,
                        ANCHO_CELDA - 2 * MARGEN_CELDA, ALTO_CELDA - 2 * MARGEN_CELDA };
        dibujar_planta(carta_seleccionada, r);
        alfa_actual = guardado;
    }
}

static void dibujar_pantalla_final(void) {
    SDL_FRect completo = { 0, 0, (float)ancho_ventana, (float)alto_ventana };
    int victoria = (estado_juego == 1);

    if (dibujar_textura(victoria ? TEX_VICTORIA : TEX_DERROTA, &completo)) return;

    color(0, 0, 0, 170);
    rellenar(0, 0, (float)ancho_ventana, (float)alto_ventana);

    const char *titulo = victoria ? "VICTORIA!" : "DERROTA";
    const char *ayuda  = "Presiona ESC o Enter para salir";
    if (victoria) color(120, 255, 120, 255); else color(255, 90, 90, 255);
    texto((ancho_ventana - ancho_texto(6.0f, titulo)) / 2.0f, alto_ventana / 2.0f - 40.0f, 6.0f, titulo);
    color(255, 255, 255, 255);
    texto((ancho_ventana - ancho_texto(2.0f, ayuda)) / 2.0f, alto_ventana / 2.0f + 30.0f, 2.0f, ayuda);
}

/* ============================================================
   FUNCIONES PUBLICAS (render.h)
   ============================================================ */
int inicializar_sdl(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: no se pudo iniciar SDL: %s\n", SDL_GetError());
        return 0;
    }

    ancho_ventana = OFFSET_X + columnas_mapa * ANCHO_CELDA + MARGEN_DERECHO;
    alto_ventana  = OFFSET_Y + filas_mapa * ALTO_CELDA + MARGEN_INFERIOR;

    if (!SDL_CreateWindowAndRenderer("Plants vs Zombies - UdeC", ancho_ventana, alto_ventana,
                                     0, &ventana, &renderer)) {
        printf("Error: no se pudo crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    cargar_todas_las_texturas();
    return 1;
}

void renderizar_pantalla(int carta_seleccionada) {
    alfa_actual = 255;

    /* 1. limpiar + 2. fondo estatico */
    color(0, 0, 0, 255);
    SDL_RenderClear(renderer);
    dibujar_fondo();

    /* 3. podadoras (detras de plantas y zombies) */
    if (hay_podadoras && podadoras) {
        for (int i = 0; i < filas_mapa; i++) {
            if (podadoras[i].corriendo) {
                dibujar_podadora(podadoras[i].x, i);
            } else if (podadoras[i].disponible) {
                dibujar_podadora((float)(OFFSET_X - PODADORA_ANCHO), i);   /* en su lugar, junto a la casa */
            }
        }
    }

    /* 4. celdas: plantas y lapidas */
    for (int i = 0; i < filas_mapa; i++) {
        for (int j = 0; j < columnas_mapa; j++) {
            int tipo = mapa[i][j].id_tipo;
            if (tipo == TIPO_VACIO) continue;
            SDL_FRect r = { (float)(OFFSET_X + j * ANCHO_CELDA + MARGEN_CELDA),
                            (float)(OFFSET_Y + i * ALTO_CELDA + MARGEN_CELDA),
                            (float)(ANCHO_CELDA - 2 * MARGEN_CELDA),
                            (float)(ALTO_CELDA - 2 * MARGEN_CELDA) };
            dibujar_planta(tipo, r);
            barra_de_vida(r.x, r.y - 4.0f, r.w, mapa[i][j].vida, vida_maxima_de_planta(tipo));
        }
    }

    /* 5. zombies, guisantes y soles */
    for (int z = 0; z < max_zombies_oleada; z++) {
        if (horda[z].activo) dibujar_zombie(&horda[z]);
    }
    for (int p = 0; p < MAX_PROYECTILES; p++) {
        if (disparos[p].activo) dibujar_guisante(&disparos[p]);
    }
    for (int s = 0; s < MAX_SOLES; s++) {
        if (soles_caidos[s].activo) dibujar_sol(&soles_caidos[s]);
    }

    /* 6. planta fantasma bajo el mouse y UI (tienda + soles) */
    dibujar_fantasma(carta_seleccionada);
    dibujar_tienda_y_hud(carta_seleccionada);

    /* 7. pantalla final (solo si el juego termino) */
    if (estado_juego == 1 || estado_juego == -1) {
        dibujar_pantalla_final();
    }

    SDL_RenderPresent(renderer);
}

void cerrar_sdl(void) {
    for (int i = 0; i < TEX_TOTAL; i++) {
        if (texturas[i]) {
            SDL_DestroyTexture(texturas[i]);
            texturas[i] = NULL;
        }
    }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (ventana)  { SDL_DestroyWindow(ventana);    ventana  = NULL; }
    SDL_Quit();
}
