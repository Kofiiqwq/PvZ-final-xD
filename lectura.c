#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

#define LARGO_LINEA 200

/* definicion de variables globales de configuracion */
int filas_mapa = 0;
int columnas_mapa = 0;
int max_zombies_oleada = 0;

int oleadas_totales = 0;
int zombies_base = 0;
int zombies_extra_por_oleada = 0;
float segundos_preparacion = 0;
float segundos_descanso = 0;

int hay_podadoras = 0;
float velocidad_podadora = 0;

int vida_girasol = 0;
int vida_lanzaguisantes = 0;
int vida_nuez = 0;
int costo_girasol = 0;
int costo_lanzaguisantes = 0;
int costo_nuez = 0;
float segundos_entre_disparos = 0;
float segundos_entre_soles_girasol = 0;
int daño_proyectil = 0;

int vida_lapida = 0;

int soles_iniciales = 0;
int valor_de_cada_sol = 0;
float segundos_entre_soles_cielo = 0;
float segundos_vida_sol = 0;

int vida_zombie_normal = 0;
int vida_zombie_flag = 0;
float velocidad_zombie_normal = 0;
float velocidad_zombie_flag = 0;
int daño_zombie_por_segundo = 0;


/* 1. leer config.txt y dimensiones de mapa.txt*/
int leer_archivos_configuracion(const char *archivo_config, const char *archivo_mapa) {
    FILE *f_config = fopen(archivo_config, "r");
    if (!f_config) {
        printf("Error: no se pudo abrir el archivo de configuración '%s'\n", archivo_config);
        return 0;
    }

    char linea[LARGO_LINEA];
    char clave[60];
    float valor;

    while (fgets(linea, sizeof(linea), f_config) != NULL) {
        if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\r') continue;
        if (sscanf(linea, "%59s %f", clave, &valor) != 2) continue;

        if      (strcmp(clave, "oleadas_totales") == 0)          oleadas_totales = (int)valor;
        else if (strcmp(clave, "max_zombies_oleada") == 0)       max_zombies_oleada = (int)valor;
        else if (strcmp(clave, "zombies_base") == 0)             zombies_base = (int)valor;
        else if (strcmp(clave, "zombies_extra_por_oleada") == 0) zombies_extra_por_oleada = (int)valor;
        else if (strcmp(clave, "segundos_preparacion") == 0)     segundos_preparacion = valor;
        else if (strcmp(clave, "segundos_descanso") == 0)        segundos_descanso = valor;
        else if (strcmp(clave, "hay_podadoras") == 0)            hay_podadoras = (int)valor;
        else if (strcmp(clave, "velocidad_podadora") == 0)       velocidad_podadora = valor;
        else if (strcmp(clave, "vida_girasol") == 0)             vida_girasol = (int)valor;
        else if (strcmp(clave, "vida_lanzaguisantes") == 0)      vida_lanzaguisantes = (int)valor;
        else if (strcmp(clave, "vida_nuez") == 0)                vida_nuez = (int)valor;
        else if (strcmp(clave, "costo_girasol") == 0)            costo_girasol = (int)valor;
        else if (strcmp(clave, "costo_lanzaguisantes") == 0)     costo_lanzaguisantes = (int)valor;
        else if (strcmp(clave, "costo_nuez") == 0)               costo_nuez = (int)valor;
        else if (strcmp(clave, "segundos_entre_disparos") == 0)  segundos_entre_disparos = valor;
        else if (strcmp(clave, "segundos_entre_soles_girasol") == 0) segundos_entre_soles_girasol = valor;
        else if (strcmp(clave, "daño_proyectil") == 0)           daño_proyectil = (int)valor;
        else if (strcmp(clave, "vida_lapida") == 0)              vida_lapida = (int)valor;
        else if (strcmp(clave, "soles_iniciales") == 0)          soles_iniciales = (int)valor;
        else if (strcmp(clave, "valor_de_cada_sol") == 0)        valor_de_cada_sol = (int)valor;
        else if (strcmp(clave, "segundos_entre_soles_cielo") == 0) segundos_entre_soles_cielo = valor;
        else if (strcmp(clave, "segundos_vida_sol") == 0)        segundos_vida_sol = valor;
        else if (strcmp(clave, "vida_zombie_normal") == 0)       vida_zombie_normal = (int)valor;
        else if (strcmp(clave, "vida_zombie_flag") == 0)         vida_zombie_flag = (int)valor;
        else if (strcmp(clave, "velocidad_zombie_normal") == 0)  velocidad_zombie_normal = valor;
        else if (strcmp(clave, "velocidad_zombie_flag") == 0)    velocidad_zombie_flag = valor;
        else if (strcmp(clave, "daño_zombie_por_segundo") == 0)  daño_zombie_por_segundo = (int)valor;
    }
    fclose(f_config);

    /* leer solo las dimensiones del mapa para los malloc en main.c*/
    FILE *f_mapa = fopen(archivo_mapa, "r");
    if (!f_mapa) {
        printf("Error: no se pudo abrir el archivo de mapa '%s'\n", archivo_mapa);
        return 0;
    }
    if (fscanf(f_mapa, "%d %d", &filas_mapa, &columnas_mapa) != 2) {
        fclose(f_mapa);
        return 0;
    }
    fclose(f_mapa);
    return 1;
}
/* 2. cargar matriz del mapa y podadoras (despues de malloc)*/
int cargar_mapa_inicial(const char *archivo_mapa) {
    if (!mapa || !podadoras) return 0;

    FILE *f_mapa = fopen(archivo_mapa, "r");
    if (!f_mapa) return 0;

    int filas_arch, col_arch;
    if (fscanf(f_mapa, "%d %d", &filas_arch, &col_arch) != 2 || 
        filas_arch != filas_mapa || col_arch != columnas_mapa) {
        fclose(f_mapa);
        return 0;
    }

    for (int i = 0; i < filas_mapa; i++) {
        for (int j = 0; j < columnas_mapa; j++) {
            int codigo;
            if (fscanf(f_mapa, "%d", &codigo) != 1) {
                fclose(f_mapa);
                return 0;
            }
            mapa[i][j].id_tipo = codigo;
            mapa[i][j].timer_accion = 0;

            if      (codigo == TIPO_GIRASOL)        mapa[i][j].vida = vida_girasol;
            else if (codigo == TIPO_LANZAGUISANTES) mapa[i][j].vida = vida_lanzaguisantes;
            else if (codigo == TIPO_NUEZ)           mapa[i][j].vida = vida_nuez;
            else if (codigo == TIPO_LAPIDA)         mapa[i][j].vida = vida_lapida;
            else                                    mapa[i][j].vida = 0;
        }
    }
    fclose(f_mapa);

    /* configurar podadoras */
    for (int i = 0; i < filas_mapa; i++) {
        podadoras[i].disponible = hay_podadoras;
        podadoras[i].corriendo = 0;
        podadoras[i].carril = i;
        podadoras[i].x = 0.0f;
    }
    return 1;
}


/* 3. generador de mapa aleatorio */
int generar_mapa_aleatorio(const char *nombre, int filas, int columnas,
                           int cuantas_lapidas, int cuantas_plantas) {

    if (filas < 1 || filas > MAXIMO_FILAS) {
        printf("error: las filas deben ir de 1 a %d\n", MAXIMO_FILAS);
        return 0;
    }
    if (columnas < MINIMO_COLUMNAS || columnas > MAXIMO_COLUMNAS) {
        printf("error: las columnas deben ir de %d a %d\n", MINIMO_COLUMNAS, MAXIMO_COLUMNAS);
        return 0;
    }

    /* cuanto espacio hay para las lapidas: todas las filas por las columnas del medio */
    int espacio_para_lapidas = filas * (columnas - 2);
    if (cuantas_lapidas < 0 || cuantas_lapidas > espacio_para_lapidas) {
        printf("error: la cantidad de lapidas debe ir de 0 a %d\n", espacio_para_lapidas);
        return 0;
    }

    /* para las plantas: todo menos la ultima columna, y menos lo que ya ocupan las lapidas */
    int espacio_para_plantas = filas * (columnas - 1) - cuantas_lapidas;
    if (cuantas_plantas < 0 || cuantas_plantas > espacio_para_plantas) {
        printf("error: con %d lapidas, la cantidad de plantas debe ir de 0 a %d\n",
               cuantas_lapidas, espacio_para_plantas);
        return 0;
    }

    /* un tablero chico de prueba, todo en 0 (vacio) */
    int tablero[MAXIMO_FILAS][MAXIMO_COLUMNAS] = {{0}};

    /* primero pongo las lapidas */
    int lapidas_puestas = 0;
    while (lapidas_puestas < cuantas_lapidas) {
        int fila_al_azar = rand() % filas;
        int columna_al_azar = 1 + rand() % (columnas - 2);   /* de la 1 a la columnas-2 */

        /* solo la pongo si esa celda esta vacia, si no, sorteo otra vez */
        if (tablero[fila_al_azar][columna_al_azar] == TIPO_VACIO) {
            tablero[fila_al_azar][columna_al_azar] = TIPO_LAPIDA;
            lapidas_puestas++;
        }
    }

    /* despues las plantas, en las celdas que sigan vacias */
    int plantas_puestas = 0;
    while (plantas_puestas < cuantas_plantas) {
        int fila_al_azar = rand() % filas;
        int columna_al_azar = rand() % (columnas - 1);        /* nunca la ultima */
        int tipo_al_azar = 1 + rand() % 3;                    /* 1, 2 o 3 */

        if (tablero[fila_al_azar][columna_al_azar] == TIPO_VACIO) {
            tablero[fila_al_azar][columna_al_azar] = tipo_al_azar;
            plantas_puestas++;
        }
    }

    /* ahora lo escribo en el archivo ("w" = write, crea el archivo o lo reemplaza) */
    FILE *archivo = fopen(nombre, "w");
    if (archivo == NULL) {
        printf("error: no pude crear el archivo '%s'\n", nombre);
        return 0;
    }

    fprintf(archivo, "%d %d\n", filas, columnas);   /* primera linea: las medidas */

    int f = 0;
    int c = 0;
    for (f = 0; f < filas; f++) {
        for (c = 0; c < columnas; c++) {
            fprintf(archivo, "%d ", tablero[f][c]);
        }
        fprintf(archivo, "\n");   /* salto de linea al terminar cada fila */
    }

    fclose(archivo);
    printf("mapa guardado en '%s' con %d lapidas y %d plantas\n",
           nombre, cuantas_lapidas, cuantas_plantas);
    return 1;
}

/*  4. mostrar configuracion leida*/
void mostrar_configuracion_leida(void) {
    printf("Configuración cargada correctamente. Filas: %d, Columnas: %d, Oleadas: %d\n", 
           filas_mapa, columnas_mapa, oleadas_totales);
}
