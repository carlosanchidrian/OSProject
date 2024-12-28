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
int ComprobarComando(char *comando, char *orden, char *argumento1, char *argumento2);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);
void GrabarBloque(void *data, size_t size, int block_number, FILE *fich);
int fetchfile(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreorigen);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
unsigned int FindFirstFreeInode(EXT_BYTE_MAPS *ext_bytemaps);
int removefiles (EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombrearchivo, FILE *fich);

int main() {
	 char comando[LONGITUD_COMANDO];
	 char orden[LONGITUD_COMANDO];
	 char argumento1[LONGITUD_COMANDO];
	 char argumento2[LONGITUD_COMANDO];
	 
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

    /*
    //Debug: Check directory entries
    printf("Directory:\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            printf("File: %s, Inode: %d\n", directorio[i].dir_nfich, directorio[i].dir_inodo);
        }
    }

    //Debug: Check loaded inodes
    printf("Inodes:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        EXT_SIMPLE_INODE inode = ext_blq_inodos.blq_inodos[i];
        printf("Inode %d: Size = %u bytes, Blocks = ", i, inode.size_fichero);
        for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            printf("%u ", inode.i_nbloque[j]);
        }
        printf("\n");
    }
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
        } else if (strcmp(orden, "exit") == 0) {
            //Write back the modified data to the partition file
            GrabarDatos(memdatos, fent); //Write data blocks
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent); //Write directory and inodes
            GrabarByteMaps(&ext_bytemaps, fent); //Write bytemaps
            GrabarSuperBloque(&ext_superblock, fent); //Write superblock
            fclose(fent);
            return 0;
        } 
        else if (strcmp(orden, "copy") == 0) { 
            int copyCheck = Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
    
            switch (copyCheck) {
                case 0:
                    printf("File '%s' successfully copied to '%s'.\n", argumento1, argumento2);
                    break;
                case -1:
                    printf("Error: Source file '%s' not found or destination file '%s' already exists.\n", argumento1, argumento2);
                    break;
                case -2:
                    printf("Error: No free inodes available to copy the file.\n");
                    break;
                case -3:
                    printf("Error: No free directory entries available to copy the file.\n");
                    break;
                case -4:
                    printf("Error: No free blocks available to copy the file.\n");
                    break;
                case -5:
                    printf("Error: FILENAMES!! The file does not exist or the file you are trying to create already exists.\n");
                    break;
                default:
                    printf("Unknown error occurred during file copy , Error Code %d.\n", copyCheck);
                    break;
            }
        } 
                else if (strcmp(orden, "remove") == 0) {
                    // Call the remove function and check the result
                    int removecheck = removefiles(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, fent);

                    if (removecheck == 0) {
                        // Successful removal
                        printf("File '%s' successfully removed.\n", argumento1);
                    } else if (removecheck == -1) {
                        // Error: File not found
                        printf("Error: File not found.\n");
                    } else {
                    // Unexpected error
                    printf("An unknown error occurred while trying to remove the file.\n");
            }

        }else {
            printf("Unknown command.\n");
        }
    
    //return 0;
    }
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
    //Print the inode and block bitmaps
    printf("Inode Bytemap:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Block Bytemap:\n");
    for (int i = 0; i < sizeof(ext_bytemaps->bmap_bloques); i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    //Print directory listing (file names, sizes, inode and block numbers)
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
    //Remove posible newline chars from the old and new name
    nombreantiguo[strcspn(nombreantiguo, "\n")] = '\0';
    nombrenuevo[strcspn(nombrenuevo, "\n")] = '\0';
    //Search the directory entry with the old name
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) {
            for (int j = 0; j < MAX_FICHEROS; j++) {
                //Verify file with the new name doesn't exist
                if (strcmp(directorio[j].dir_nfich, nombrenuevo) == 0) {
                    printf("Error: File with the new name already exists.\n");
                    return -1;
                }
            }
            //If it doesn't exist we change the name of the entry in the directory
            strncpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH);
            printf("File renamed successfully.\n");
            return 0;
        }
    }
    printf("Error: File not found.\n");
    return -1;
}


int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    //Remove newline char from the filename
    nombre[strcspn(nombre, "\n")] = '\0';
        
    //Find the directory entry for the specified file
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            unsigned short int inodo = directorio[i].dir_inodo;
            
            //Check that inode number is valid
            if (inodo >= MAX_INODOS) {
                printf("Inode not valid.\n");
                return -1;
            }

            //Get inode information
            EXT_SIMPLE_INODE inode = inodos->blq_inodos[inodo];

            printf("Contents of %s:\n", nombre);
            //Iterate through the blocks 
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                unsigned short int block_num = inode.i_nbloque[j];
                //Check if we have reached the end of the file's block
                if (block_num == NULL_BLOQUE) break;

                //Access the data block using memdatos and the block number
                printf("%s", (char *)memdatos[block_num - PRIM_BLOQUE_DATOS].dato);
            }
            
            printf("\n");
            
            return 0;
        }
    }

    printf("File not found.\n");
    
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
    if (strcmp(orden, "info") == 0 || strcmp(orden, "bytemaps") == 0 || strcmp(orden, "dir") == 0 || strcmp(orden, "exit" ) == 0) {
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
    }  else if (strcmp(orden, "copy") == 0) {
        if (argumento1[0] == '\0' || argumento2[0] == '\0') {
            printf("Error: Command 'copy' requires two arguments: old name and name of the copy.\n");
            return 1;  // Invalid if missing arguments
        } else {
            return 0; // Valid command
        }
            } else if (strcmp(orden, "remove") == 0) {
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

void GrabarBloque(void *data, size_t size, int block_number, FILE *fich) {
    fseek(fich, block_number * SIZE_BLOQUE, SEEK_SET);
    fwrite(data, size, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    // Start writing from the correct block (after the reserved blocks)
    for (int i = 0; i < MAX_BLOQUES_DATOS; i++) {
        GrabarBloque(&memdatos[i], sizeof(EXT_DATOS), PRIM_BLOQUE_DATOS + i, fich);
    }
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    GrabarBloque(directorio, sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS, 3, fich);
    GrabarBloque(inodos, sizeof(EXT_BLQ_INODOS), 2, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    // Write the inode bitmap
    GrabarBloque(ext_bytemaps->bmap_inodos, sizeof(ext_bytemaps->bmap_inodos), 1, fich);

    // Write the block bitmap
    GrabarBloque(ext_bytemaps->bmap_bloques, sizeof(ext_bytemaps->bmap_bloques), 1, fich);
}


void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    GrabarBloque(ext_superblock, sizeof(EXT_SIMPLE_SUPERBLOCK), 0, fich);
}
int fetchfile(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreorigen) {
    // Iterate through the directory entries
    for (int i = 0; i < MAX_FICHEROS; i++) {
        // Check if the directory entry is valid (not empty)
        if (directorio[i].dir_inodo != NULL_INODO) {
            // Compare the file name and return index if names match
            if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
                return i; // Return the position in the 'directorio' array
            }
        }
    }
    // If the file was not found, return 0
    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {

    int i, j, k, size;
    int fileFound;
    unsigned int originBlock, freeInode, freeBlock, copiedblock, dirEntry, originInode;
    unsigned short int blockNumber;
    unsigned short int copiedBlocks[MAX_NUMS_BLOQUE_INODO];
    int namecheck = 0;
    nombreorigen[strcspn(nombreorigen, "\n")] = '\0';
    nombredestino[strcspn(nombredestino, "\n")] = '\0';

    originBlock = 0; // Initialize originBlock
    freeInode = NULL_INODO;
    dirEntry = 0;
    fileFound = 0;
    

    // Check if the destination file already exists
    i = 0;

    i = fetchfile(directorio, inodos, nombreorigen);
    namecheck = fetchfile(directorio, inodos, nombredestino);

    if (i != 0 && namecheck == 0) {
        fileFound = 1;
        j = 0;
        do {
            blockNumber = inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j];
            if (blockNumber != NULL_BLOQUE) {
                originBlock++;
            }
            j++;
        } while ((blockNumber != NULL_BLOQUE) && (j < MAX_NUMS_BLOQUE_INODO));

        // Search for the first free inode
        freeInode = FindFirstFreeInode(ext_bytemaps);
        if (freeInode == NULL_INODO) {
            return -1; // No free inode
        }

        // Search for the first free directory entry
        dirEntry == 0;
        while (dirEntry < MAX_FICHEROS && directorio[dirEntry].dir_inodo != NULL_INODO) {
            dirEntry++;
        }

        if (dirEntry == MAX_FICHEROS) {
            return -2; // No free directory entry
        }
        //Mark inode as used
        ext_bytemaps->bmap_inodos[freeInode] = 1;
        ext_superblock->s_free_inodes_count--;


        // Get the source inode
        originInode = directorio[i].dir_inodo;
        EXT_SIMPLE_INODE *sourceInode = &inodos->blq_inodos[originInode];
        EXT_SIMPLE_INODE *newInode = &inodos->blq_inodos[freeInode];

        // Copy the file size and initialize the new inode
        newInode->size_fichero = sourceInode->size_fichero;
        for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            newInode->i_nbloque[j] = NULL_BLOQUE;
        }

        // Copy the data blocks
        for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            blockNumber = sourceInode->i_nbloque[j];
            newInode->i_nbloque[j];
            if (blockNumber == NULL_BLOQUE) {
                break;
            }
            else{
                // Find the first free block
                freeBlock = 0;
                while (freeBlock < MAX_BLOQUES_PARTICION && ext_bytemaps->bmap_bloques[freeBlock] != 0) {
                freeBlock++;
                }
                if (freeBlock == MAX_BLOQUES_PARTICION) {
                    return -3; // No free blocks available
                }

                // Mark the block as occupied
                ext_bytemaps->bmap_bloques[freeBlock] = 1;

                // Copy the data block
                memcpy(&memdatos[(freeBlock - PRIM_BLOQUE_DATOS)],
                &memdatos[(blockNumber - PRIM_BLOQUE_DATOS)],
                SIZE_BLOQUE);

                // Assign the block to the new inode
                newInode->i_nbloque[j] = freeBlock;

                // Update the superblock
                ext_superblock->s_free_blocks_count--;
            }

            // Debugging: Print allocation info
            printf("Free blocks remaining: %u\n",
            ext_superblock->s_free_blocks_count);
            printf("Free inodes remaining: %u\n",
            ext_superblock->s_free_inodes_count);
        }

        // Create the new directory entry
        strncpy(directorio[dirEntry].dir_nfich, nombredestino, LEN_NFICH);
        directorio[dirEntry].dir_inodo = freeInode;

        // Write back all changes to the file
        Grabarinodosydirectorio(directorio, inodos, fich);
        fflush(fich); // Ensure metadata is written
        GrabarByteMaps(ext_bytemaps, fich);
        fflush(fich); // Ensure bytemaps are written
        GrabarDatos(memdatos, fich);
        fflush(fich); // Ensure data is written
        GrabarSuperBloque(ext_superblock, fich);

        return 0; // File successfully copied
    }

    else if (i == 0)
    {
        return -5;
    }
    
}

unsigned int FindFirstFreeInode(EXT_BYTE_MAPS *ext_bytemaps) {
    // Iterate through the inode bitmap
    for (unsigned int i = 3; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) { // Inode is free
            return i; // Return the index of the first free inode
        }
    }
    return NULL_INODO; // No free inode found
}

int removefiles(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombrearchivo, FILE *fich)
{
    int i, j;
    int file_position;  // Store the directory position of the file
    int inode_index;    // Index of the inode corresponding to the file
    int occupiedBlocks = 0;
    nombrearchivo[strcspn(nombrearchivo, "\n")] = '\0';

    // Fetch the file position in the directory using the provided fetchfile function
    file_position = fetchfile(directorio, inodos, nombrearchivo);
    if (file_position == 0) { // fetchfile returns 0 if the file is not found
        
        return -1;
    }
    else{
    // Get the inode index from the directory entry
    inode_index = directorio[file_position].dir_inodo;

    // Get inode corresponding to the file
    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[inode_index];

    // Mark the data blocks as free in the bytemaps
    for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        if (inode->i_nbloque[j] != NULL_BLOQUE) {
            ext_bytemaps->bmap_bloques[inode->i_nbloque[j]] = 0;  // Mark block as free
            inode->i_nbloque[j] = NULL_BLOQUE;                     // Reset block pointer
            occupiedBlocks++;
        }
    }

    // Update the inode: size to 0, mark as free
    inode->size_fichero = 0;
    ext_bytemaps->bmap_inodos[inode_index] = 0;  // Mark inode as free

    // Update the directory entry: clear name and mark inode as free
    strcpy(directorio[file_position].dir_nfich, "");  // Clear file name
    directorio[file_position].dir_inodo = NULL_INODO;    // Mark inode number as free

    // Update the superblock: increment free blocks and inodes counters
    ext_superblock->s_free_blocks_count += occupiedBlocks;  // Free all file blocks
    ext_superblock->s_free_inodes_count++;

    // Write updates back to the file system image
    GrabarBloque(directorio, sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS, 1, fich);
    GrabarBloque(inodos, sizeof(EXT_BLQ_INODOS), 2, fich);
    GrabarBloque(ext_bytemaps, sizeof(EXT_BYTE_MAPS), 3, fich);
    GrabarBloque(ext_superblock, sizeof(EXT_SIMPLE_SUPERBLOCK), 0, fich);


    return 0;
    }
}
