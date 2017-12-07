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

/* Grupo:
 *      Gabriel de Paula
 *      Giovana Morais
 *      Mateus Abreu
 *      Thiago Borges
 */

#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096
#define LIVRE 	1
#define ULTIMO	2
#define A_FAT	3
#define A_DIR	4
#define DIR_LIVRE 0
#define DIR_USADO 1
#define TAM_FAT 65536
#define QTD_SETOR_DIR 8
#define INICIO_DIR 256
#define F_OPEN 0
#define F_READ 1
#define F_WRITE 2


unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];

typedef struct {
	unsigned short first_block; //-1 é vazio
	int mode;//-1 vazio
	int f_mode;	//f_mode será 0 para open, 1 para read , 2 para write e -1 pra vazio
} id_arquivo;// Estrutura utilizada como identificador de arquivos

id_arquivo id_arq[128];

/* Carrega informações existentes para as estruturas em memória e checa por
 * possíveis inconsistências. Nesse caso, avisa que o disco não está
 * formatado */
int fs_init() {
 	int i = 0, format = 0, j = 0;
 	char *buffer_fat = (char *) fat;
 	char *buffer_dir = (char *) dir;

    // carrega dados do disco
    for (i = 0; i < INICIO_DIR; i++){
		if(!bl_read(i, &buffer_fat[i*SECTORSIZE]))
			return 0;
    }

	// VERIFICA SE O DISCO ESTÁ FORMATADO
	for (i = 0; i < 32; i++){
		if(fat[i] != A_FAT){
			format = 1;
			break;
		}
	}

	if(format)
        printf("O disco não está formatado\n");
    else {
        for (i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++){
            if(!bl_read(j + i, &buffer_dir[i*SECTORSIZE]))
                return 0;
        }
    }
    for(i = 0; i < 128; i++){
      id_arq[i].first_block = -1;
      id_arq[i].mode = -1;
      id_arq[i].f_mode = -1;
    }
	return 1;
}

/* Formata o disco */
int fs_format() {
 	int i = 0, j;
 	char *buffer_fat = (char *) fat;
 	char *buffer_dir = (char *) dir;

  	// 70-77 SETA AS POSIÇÕES DA FAT, DO DIRETORIO E DOS ESPAÇOS LIVRES EM MEMÓRIA
  	for (i = 0; i < 32; i++)
  		fat[i] = A_FAT;

  	fat[32] = A_DIR;

  	for (i = 33; i < 65536; i++)
  		fat[i] = LIVRE;

	// INICIALIZA A STRUCT DIRETORIO
	for(i = 0; i < 128; i++){
	  	dir[i].used = DIR_LIVRE;
  		//dir[i].first_block = ULTIMO;
  		dir[i].size = 0;
  	}

  	// ESCREVE NO ARQUIVO IMAGEM A FAT E O DIRETORIO
	for (i = 0; i < INICIO_DIR; i++)
		if(!bl_write(i, &buffer_fat[i*SECTORSIZE]))
			return 0;

	for (i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++)
		if(!bl_write(j + i, &buffer_dir[i*SECTORSIZE]))
			return 0;

	return 1;
}

/* Faz a contagem da memória livre */
int fs_free() {
    int cont = 0, mem_livre;

    for(int i = 33; i < bl_size(); i++){
        if(fat[i] == LIVRE)
            cont++;
    }

    mem_livre = cont * SECTORSIZE;

    return mem_livre;
}

/* Lista o nome e o tamanho dos arquivos existentes */
int fs_list(char *buffer, int size) {
    char buffer_int[100];
    char buffer_mem[size];
    memset(buffer_int, '\0', 100);
    strcpy(buffer_mem, "\0");

    for(int i = 0; i < 128; i++){
        if(dir[i].used){
            strcat(buffer_mem, dir[i].name);
            strcat(buffer_mem, "\t\t");
            sprintf(buffer_int, "%d", dir[i].size);
            strcat(buffer_mem, buffer_int);
            strcat(buffer_mem, "\n");
        }
    }

    strcpy(buffer, buffer_mem);
    return 1;
}

/* Cria um arquivo de tamanho 0 na primeira posição livre encontrada */
int fs_create(char* file_name) {
    int pos_dir = -1, pos_fat, i, j, flag = 0;
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;

    // verifica se o arquivo já existe na memória
    for(i = 0; i < 128; i++){
        if(!strcmp(dir[i].name, file_name)){
            pos_dir = i;
            pos_fat = dir[i].first_block;
            flag = 1;
            break;
        }
    }

    if(flag){
        printf("Arquivo já existe\n");
        return 0;
    }
    /* ----------- MANIPULAÇÃO EM MEMÓRIA ------------- */
    // busca a primeira posição livre na fat
    for(i = 33; i < TAM_FAT; i++){
        if(fat[i] == LIVRE){
            pos_fat = i;
            break;
        }
    }

    // busca a primeira posição livre no vetor de diretórios
    for(i = 0; i < 128; i++){
        if(dir[i].used == 0){
            pos_dir = i;
            break;
        }
    }

    /* VERIFICAÇÕES */
    if(i == 127 && pos_dir == -1){
        printf("Não há espaço livre!\n");
        return 0;
    }

    if(strlen(file_name) > 25){
        printf("Nome do arquivo precisa ter menos de 25 caracteres!\n");
        return 0;
    }

    // seta infos dir
    dir[pos_dir].used = DIR_USADO;
    dir[pos_dir].size = 0;
    dir[pos_dir].first_block = pos_fat;
    strcpy(dir[pos_dir].name, file_name);

    // marca a fat com o último agrupamento do diretório
    fat[pos_fat] = ULTIMO;

    /* ----------- MANIPULAÇÃO EM DISCO ------------- */
    for(i = 0; i < INICIO_DIR; i++){
        if(!bl_write(i, &buffer_fat[i * SECTORSIZE]))
            return 0;
    }

    for(i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++){
        if(!bl_write(j + i, &buffer_dir[i * SECTORSIZE]))
            return 0;
    }

    return 0;
}

/* Remove um arquivo de nome file_name */
int fs_remove(char *file_name) {
    int pos_dir, pos_fat, i, j, flag = 0;
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;

    /* ----------- MANIPULAÇÃO EM MEMÓRIA ------------- */
    for(i = 0; i < 128; i++){
        if(!strcmp(dir[i].name, file_name)){
            pos_dir = i;
            pos_fat = dir[i].first_block;
            flag = 1;
            break;
        }
    }

    if(!flag){
        printf("Arquivo não encontrado\n");
        return 0;
    }

    memset(dir[pos_dir].name, '\0', strlen(dir[pos_dir].name));
    dir[pos_dir].first_block = 0;
    dir[pos_dir].used = DIR_LIVRE;
    dir[pos_dir].size = 0;

    fat[pos_fat] = LIVRE;

    /* ----------- MANIPULAÇÃO EM DISCO ------------- */
    for(i = 0; i < INICIO_DIR; i++) {
        if(!bl_write(i, &buffer_fat[i * SECTORSIZE]))
            return 0;
    }

    for(i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++) {
        if(!bl_write(j + i, &buffer_dir[i * SECTORSIZE]))
            return 0;
    }

    return 0;
}

/*
 *  Manter um buffer interno pra gerenciar os bytes que o usuário quer ler
 *  independente dos 4K que temos que puxar do disco.
 */
int fs_open(char *file_name, int mode) {
	//printf("Função não implementada: fs_open\n");
	//open prepara estruturas para serem utilizadas no read e write, gerar um identificador pro arquivo.
	int i = 0, pos_dir, pos_fat, flag = 0;

	//Nesse for iremos percorrer todo o diretório em busca de um arquivo com o nome file_name
	for(i = 0; i < 128; i++){
		if(!strcmp(dir[i].name, file_name)){
		    pos_dir = i;
		    pos_fat = dir[i].first_block;
		    flag = 1;							//Caso esse arquivo seja encontrado, iremos sinalizar
		    break;
		}
	}

	if(mode == FS_R){
		//Se entrar aqui, significa que o arquivo não existe e um erro deve ser gerado
		if (!flag){
			printf("Arquivo nao existe\n");
			return -1;
		} else {	//Caso entre aqui, significa que o arquivo existe
          for(i = 0; i < 128; i++){
            if(id_arq[i].first_block = -1){
              id_arq[i].first_block = pos_fat;
              id_arq[i].mode = mode;
              id_arq[i].f_mode = 1;
              return i;
            }
          }
			//Implementar fopen (blread e blwrite)
		}
	} else if (mode == FS_W){
		//Se entrar aqui, significa que o arquivo não existe e então iremos criá-lo.
		if (!flag){
			fs_create(file_name);
		} else {
			fs_remove(file_name);
			fs_create(file_name);
		}
	}
  return -1;
}

int fs_close(int file)  {
  printf("Função não implementada: fs_close\n");
  return 0;
}

/*  Caso o buffer tiver cheio, ir na fat e procurar o próximo bloco livre.
 *  Aí é só recarregar o buffer e continuar a escrita.
 *  A posição de escrita é sempre na última posição porque não tem seek.
 */
int fs_write(char *buffer, int size, int file) {
  printf("Função não implementada: fs_write\n");
  return -1;
}

/*  > Lê dados do disco
 *  > Guarda no buffer
 *  > Cria um marcador de leitura pra esse buffer pq cada leitura vai
 *  retornar esses bytes e ver se existem
 *      > Ficar atento pra quando estiver perto do fim pq se passar do número
 *      de bytes do final, vai ter que recarregar o buffer com os prox 4096
 *      bytes do disco
 *      > Checar fim do buffer do usuário, fim do arquivo e fim do buffer do
 *      usuário
 */
int fs_read(char *buffer, int size, int file) {
  printf("Função não implementada: fs_read\n");
  return -1;
}
