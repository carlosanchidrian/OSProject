#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "headers.h"

#define LONGITUD_COMANDO 100

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);

int main() {
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
    /*
    LeeSuperBloque(&ext_superblock);
    printf ("\n");
    Printbytemaps(&ext_bytemaps);
    printf ("\n");
    Directorio(&directorio, &ext_blq_inodos);
    printf ("\n");
    Imprimir(&directorio, &ext_blq_inodos, memdatos, "HOLA.txt");
    printf ("\n");
    Renombrar(&directorio, &ext_blq_inodos, "HOLA.txt", "Hola.txt");
    printf ("\n");
    Directorio(&directorio, &ext_blq_inodos);
    Imprimir(&directorio, &ext_blq_inodos, memdatos, "Hola.txt");
    */
    for (;;) {
        do {
            printf(">> ");
            fflush(stdin);
            fgets(comando, LONGITUD_COMANDO, stdin);

        }while(ComprobarComando(comando,orden,argumento1,argumento2) !=0);

        if (strcmp(orden, "info") == 0) {
                LeeSuperBloque(&ext_superblock);
        } else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "rename") == 0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
        } else if (strcmp(orden, "print") == 0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        } else if (strcmp(orden, "salir") == 0) {
            //GrabarDatos(memdatos, fent);
            fclose(fent);
            return 0;
        } else {
            printf("Unknown command.\n");
        }
    }
    //return 0;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Superblock Information:\n");
    printf("Total inodes: %u\n", psup->s_inodes_count);
    printf("Total blocks: %u\n", psup->s_blocks_count);
    printf("Free blocks: %u\n", psup->s_free_blocks_count);
    printf("Free inodes: %u\n", psup->s_free_inodes_count);
    printf("Block size: %u bytes\n", psup->s_block_size);
    printf("First data block: %u\n", psup->s_first_data_block);
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    int i;

    printf("Inode Bytemap:\n");
    for (i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Block Bytemap:\n");
    for (i = 0; i < sizeof(ext_bytemaps->bmap_bloques); i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("Directory Listing:\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE inode = inodos->blq_inodos[directorio[i].dir_inodo];
            printf("File: %s, Size: %u bytes, Inode: %d, Blocks: ",
                   directorio[i].dir_nfich, inode.size_fichero, directorio[i].dir_inodo);
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inode.i_nbloque[j] != NULL_BLOQUE) {
                    printf("%u ", inode.i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    nombreantiguo[strcspn(nombreantiguo, "\n")] = '\0';
    nombrenuevo[strcspn(nombrenuevo, "\n")] = '\0';
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) {
            for (int j = 0; j < MAX_FICHEROS; j++) {
                if (strcmp(directorio[j].dir_nfich, nombrenuevo) == 0) {
                    printf("Error: File with the new name already exists.\n");
                    return -1;
                }
            }
            strncpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH);
            printf("File renamed successfully.\n");
            return 0;
        }
    }
    printf("Error: File not found.\n");
    return -1;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    nombre[strcspn(nombre, "\n")] = '\0';
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            EXT_SIMPLE_INODE inode = inodos->blq_inodos[directorio[i].dir_inodo];
            printf("Contents of %s:\n", nombre);
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inode.i_nbloque[j] != NULL_BLOQUE) {
                    printf("%.*s", SIZE_BLOQUE, memdatos[inode.i_nbloque[j]].dato);
                }
            }
            printf("\n");
            return 0;
        }
    }
    printf("%s",nombre);
    printf("Error: File not found.\n");
    return -1;
}

int ComprobarComando(char *comando, char *orden, char *argumento1, char *argumento2) {
 

        // Clear the argument buffers
    argumento1[0] = '\0';
    argumento2[0] = '\0';

   char *token = strtok(comando, " "); // Tokenize the command string

    comando[strcspn(comando, "\n")] = '\0';
    
    if (token == NULL) {
        return 1; // No command provided
    } else {
        strcpy(orden, token); // Copy command
    }

    // Get arguments
    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(argumento1, token); // 1st argument
    } else {
        argumento1[0] = '\0';  // No argument
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(argumento2, token); // 2nd argument
    } else {
        argumento2[0] = '\0';  // No argument
    }

    // Check the command and arguments
    if (strcmp(orden, "info") == 0 || strcmp(orden, "bytemaps") == 0 || strcmp(orden, "dir") == 0 || strcmp(orden, "exit") == 0) {
        if (argumento1[0] != '\0' || argumento2[0] != '\0') {
            printf("Error: Command '%s' does not take arguments.\n", orden);
            return 1;  // Invalid if arguments are provided for these commands
        } else {
            return 0; // Valid command
        }
    } else if (strcmp(orden, "rename") == 0) {
        if (argumento1[0] == '\0' || argumento2[0] == '\0') {
            printf("Error: Command 'rename' requires two arguments: old name and new name.\n");
            return 1;  // Invalid if missing arguments
        } else {
            return 0; // Valid command
        }
    } else if (strcmp(orden, "print") == 0) {
        if (argumento1[0] == '\0') {
            printf("Error: Command '%s' requires a file name as an argument.\n", orden);
            return 1;  // Invalid if no argument is given
        } else {
            return 0; // Valid command
        }
    } else {
        printf("Error: Unknown command '%s'.\n", orden);
        return 1;  // Invalid if the command is not recognized
    }
}
