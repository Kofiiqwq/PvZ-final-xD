
#ifndef GAME_H
#define GAME_H

/* Constantes de límites y Object Pools */
#define FPS 60
#define MAX_PROYECTILES 100
#define MAX_SOLES 50
#define MAXIMO_FILAS 10
#define MINIMO_COLUMNAS 5
#define MAXIMO_COLUMNAS 15

/* Nombres para los números del mapa */
#define TIPO_VACIO           0
#define TIPO_GIRASOL         1
#define TIPO_LANZAGUISANTES  2
#define TIPO_NUEZ            3
#define TIPO_LAPIDA          4   /* Obstáculo con vida: no se puede plantar ahí */

/* Medidas de pantalla y cartas (compartidas por input.c, logic.c y render.c) */
#define OFFSET_X          100
#define OFFSET_Y          120
#define ANCHO_CELDA        80
#define ALTO_CELDA        100

#define CARTA_X            20
#define CARTA_Y            10
#define CARTA_ANCHO        70
#define CARTA_ALTO         90
#define CARTA_SEPARACION   10

#define COOLDOWN_GIRASOL          5
#define COOLDOWN_LANZAGUISANTES   5
#define COOLDOWN_NUEZ            10

/* Variables globales de configuración (definidas en lectura.c) */
extern int filas_mapa;
extern int columnas_mapa;
extern int max_zombies_oleada;

extern int oleadas_totales;
extern int zombies_base;
extern int zombies_extra_por_oleada;
extern float segundos_preparacion;
extern float segundos_descanso;

extern int hay_podadoras;
extern float velocidad_podadora;

extern int vida_girasol;
extern int vida_lanzaguisantes;
extern int vida_nuez;
extern int costo_girasol;
extern int costo_lanzaguisantes;
extern int costo_nuez;
extern float segundos_entre_disparos;
extern float segundos_entre_soles_girasol;
extern int daño_proyectil;

extern int vida_lapida;

extern int soles_iniciales;
extern int valor_de_cada_sol;
extern float segundos_entre_soles_cielo;
extern float segundos_vida_sol;

extern int vida_zombie_normal;
extern int vida_zombie_flag;
extern float velocidad_zombie_normal;
extern float velocidad_zombie_flag;
extern int daño_zombie_por_segundo;

/* ---- STRUCTS ---- */
typedef struct {
    int id_tipo;       /* 0=vacio, 1=girasol, 2=lanzaguisantes, 3=nuez, 4=lapida */
    int vida;          /* También las lápidas tienen vida */
    int timer_accion;  /* Tick interno para disparar/generar sol */
} Planta;

typedef struct {
    int activo;        /* 0 = inactivo (memoria libre), 1 = vivo */
    int tipo;          /* 0 = normal, 1 = flag zombie */
    int carril;        /* Coincide con fila de matriz */
    float x;
    int vida;
    float velocidad;
    int estado;        /* 0 = caminando, 1 = comiendo */
} Zombie;

typedef struct {
    int activo;
    int carril;
    float x;
    int daño;
} Proyectil;

typedef struct {
    int activo;
    int x;
    int y;
    int timer_vida;
} Sol;

typedef struct {
    int disponible;    /* 1 = esperando en su lugar, 0 = ya se usó */
    int corriendo;     /* 1 = avanzando por la fila llevándose zombies */
    int carril;        /* Fila donde vive */
    float x;           /* Posición horizontal */
} Podadora;

/* ---- ESTADO GLOBAL DINÁMICO (definidos en main.c) ---- */
extern Planta** mapa;
extern Zombie* horda;
extern Proyectil* disparos;
extern Sol* soles_caidos;
extern Podadora* podadoras;

extern int soles_actuales;
extern int estado_juego;     /* 0 = jugando, 1 = victoria, -1 = derrota, 2 = salir */
extern int cooldown_cartas[4];

/* progreso de la partida (definido en logica.c, el render lo muestra en el HUD) */
extern int oleada_actual;            /* 0 = preparación, 1..oleadas_totales */

/* ---- PROTOTIPOS DE FUNCIONES ---- */
/* lectura.c */
int leer_archivos_configuracion(const char *archivo_config, const char *archivo_mapa);
int cargar_mapa_inicial(const char *archivo_mapa);
int generar_mapa_aleatorio(const char *nombre, int filas, int columnas,
                           int cuantas_lapidas, int cuantas_plantas);
void mostrar_configuracion_leida(void);

/* input.c */
int procesar_entradas(int* carta_seleccionada);

/* logic.c */
void actualizar_logica(void);

#endif
