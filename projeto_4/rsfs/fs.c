/*
 * RSFS - Really Simple File System
 *
 * Copyright © 2010 Gustavo Maciel Dias Vieira
 * Copyright © 2010 Rodrigo Rocco Barbieri
 *
 * This file is part of RSFS.
 *
 * RSFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096

unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];

int fs_init() {
    //printf("Função não implementada: fs_init\n");
    // checa se está formatado
    // carrega dados do disco
    char *buffer; 
    int i;

    buffer = (char *) fat;
    bl_read(0, buffer);

    // nada dá certo 
    for(i = 0; i < 65536; i++)
        printf("%c", buffer[i]);

    return 1;
}

int fs_format() {
  printf("Função não implementada: fs_format\n");
  return 0;
}

int fs_free() {
  printf("Função não implementada: fs_free\n");
  return 0;
}

int fs_list(char *buffer, int size) {
  printf("Função não implementada: fs_list\n");
  return 0;
}

int fs_create(char* file_name) {
  printf("Função não implementada: fs_create\n");
  return 0;
}

int fs_remove(char *file_name) {
  printf("Função não implementada: fs_remove\n");
  return 0;
}

int fs_open(char *file_name, int mode) {
  printf("Função não implementada: fs_open\n");
  return -1;
}

int fs_close(int file)  {
  printf("Função não implementada: fs_close\n");
  return 0;
}

int fs_write(char *buffer, int size, int file) {
  printf("Função não implementada: fs_write\n");
  return -1;
}

int fs_read(char *buffer, int size, int file) {
  printf("Função não implementada: fs_read\n");
  return -1;
}

