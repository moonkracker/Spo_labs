#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <io.h>

#define MAX_NAME 255                                    //Максимальное имя файла или директории
#define MAX_NUM_OF_DIRS 5                               //Максимальное количество директорий
#define MAX_NUM_OF_FILES 10                             //Максимальное количество файлов
#define SIZE_OF_FILE 64

struct Info                                             //Информация о том, где начинается свободное место на диске
{
    fpos_t begin_of_free_space;                         //Позиция в файле, указывающая на начало свободного места
};

struct FileInfo                                         //Информация о файле
{
    char name[MAX_NAME];                                //Имя файла
    fpos_t position;                                    //Позиция начала файла в файловой системе
    int size;                                           //Размер файла
};

struct Dir                                              //Директория
{
    char name[MAX_NAME];                                //Имя директории
    struct Dir *previous;                               //Указатель на директорю, в которой находится текущая директория
    struct Dir *dirs;                                   //Директории, на которые ссылается текущая директория
    struct FileInfo filesInfo[MAX_NUM_OF_FILES];        //Массив с информацией о файлах
    int num_of_dirs;                                    //Количество директорий в данной директории
    int num_of_files;                                   //Количество файлов в данной директории
};

void initDir(struct Dir *, struct Dir *, const char *);
int addDir(struct Dir *, const char *);
void showDir(struct Dir *);
void writeInFile(struct Dir *);
void readFromFile(struct Dir *);
char *getCurrentPath(struct Dir *);
void delDir(struct Dir *);
int readFile(struct Dir *, const char *, char *);
int writeDataInFile(struct Dir *, const char *, const char *);
int addFile(struct Dir *, const char *, int);
int delFile(struct Dir *, const char *);
int LoadFileFromFS(struct Dir *, char *, char *);
int LoadFileInFS(struct Dir *, char *, char *);
void Help();