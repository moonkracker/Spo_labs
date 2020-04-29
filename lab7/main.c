#include "header.h"
struct Info info;                                                               //Глобальная структура, которая хранит начало свободного места

void Help()                                                                     //Вызов справки о функциях
{
    puts("");
    puts("Available commands:");
    puts("                    ls - Show all files and directories");
    puts("                    cd [directory] - open directory");
    puts("                    mkdir [name] - create directory");
    puts("                    rmdir [name] - remove directory");
    puts("                    touch [name] - create empty file");
    puts("                    writeFile - write data into file");
    puts("                    rm [name] - delete file");
    puts("                    lifs [name] - load file into filesystem");
    puts("                    lffs [name] - load file from filesystem");
    puts("                    exit - quit from program");
    puts("");
}

void initDir(struct Dir *dptr, struct Dir *prev, const char *name)              //Инициализация директории
{
    dptr->previous = prev;                                                      //Заполнение указателя на предыдущую директорию
    strcpy(dptr->name, name);                                                   //Заполнение имя директории
    dptr->dirs = NULL;                                                          //Зануление указатель на другие директории
    dptr->num_of_dirs = 0;
    dptr->num_of_files = 0;
    for (int i = 0; i < MAX_NUM_OF_FILES; i++)                                  //Заполнение информации о файлах
    {
        dptr->filesInfo[i].position = -1;
        dptr->filesInfo[i].size = -1;
        strcpy(dptr->filesInfo[i].name, "");
    }
}

//Добавление директории(возвращаемые значения: 1 - успешно, 0 - текущая директория содержим max число директорий,
//-1 - такая директория уже существует)
int addDir(struct Dir *dptr, const char *n)
{
    for (int i = 0; i < dptr->num_of_dirs; i++)                     //Проверка на то, существует ли директория с таким именем
    {
        if (!strcmp(dptr->dirs[i].name, n))
            return -1;
    }
    if ((dptr->num_of_dirs + 1) <= 5)
    {
        if (dptr->num_of_dirs == 0)                                 //Выделение памяти, если директорий было 0
        {
            dptr->dirs = (struct Dir *)malloc(sizeof(struct Dir));
        }
        else
        {                                                           //Перевыделение памяти
            dptr->dirs = (struct Dir *)realloc(dptr->dirs, (dptr->num_of_dirs + 1) * sizeof(struct Dir));
        }
        initDir(&(dptr->dirs[dptr->num_of_dirs]), dptr, n);         //Инициализация новой директории
        dptr->num_of_dirs++;                                        //Увличение числа директорий данной директории
        return 1;
    }
    else
    {
        return 0;
    }
}

//Функция вывода информации о директории
void showDir(struct Dir *dptr)
{
    for (int i = 0; i < dptr->num_of_dirs; i++) //Вывод каталогов текущего каталога
    {
        printf("dir: %s\n", dptr->dirs[i].name);
    }
    for (int i = 0; i < dptr->num_of_files; i++) //Вывод файлов текущего каталога
    {
        if (dptr->filesInfo[i].position >= 0)
        {
            printf("file: %s\n", dptr->filesInfo[i].name);
        }
    }
}

//Запись директорий в файл
void writeInFile(struct Dir *root)
{
    FILE *FS = fopen("FS", "r+b");
    if (!FS)
    {
        puts("ERROR WHILE OPEN FILESYSTEM");
        exit(1);
    }
    fwrite(&info, sizeof(struct Info), 1, FS);                     //переместить курсор на sizeof( struct Info)(fseek не работает)
    fwrite(root, sizeof(struct Dir), 1, FS);                       //Запись в файл корневого каталога
    fwrite(root->dirs, sizeof(struct Dir), root->num_of_dirs, FS); //Запись каталогов корневого коталого в файл
    struct Dir *level = root->dirs;                                //Указатель на массив второго уровня дерева каталогов
    struct Dir *next_level = NULL;                                 //Указатель на массив следующего уровня дерева каталогов
    int level_size = root->num_of_dirs;                            //Размер текущего уровня в дереве каталогов
    int next_level_size = 0;                                       //Размер следующего уровня в дереве каталогов
    int cur_dir_num = 0;                                           //Счетчик для записи элементов на следующий уровень
    while (1)
    {
        for (int i = 0; i < level_size; i++) //Прозодим по текущему уровню в дереве каталогов
        {
            if (level[i].num_of_dirs > 0) //Если у каталога из текущего уровня есть подкоталоги, то
            {                             //добавляем из в след. уровень
                if (next_level_size == 0)
                {
                    next_level = (struct Dir *)malloc(sizeof(struct Dir) * level[i].num_of_dirs);
                    next_level_size = level[i].num_of_dirs;
                }
                else
                {
                    cur_dir_num = next_level_size; //Можно удалить
                    next_level_size += level[i].num_of_dirs;
                    next_level = (struct Dir *)realloc(next_level, next_level_size * sizeof(struct Dir));
                }
                for (int j = 0; cur_dir_num < next_level_size; cur_dir_num++, j++) //Добавление каталогов текущего уровня
                    next_level[cur_dir_num] = level[i].dirs[j];                    //в массив каталогов след. уровня
            }
        }
        if (next_level_size == 0) //Если следующего уровня нет, то конец записи
        {
            break;
        }
        level = next_level; //Теперь следующий уровень будет предыдущим
        level_size = next_level_size;
        fwrite(level, sizeof(struct Dir), level_size, FS); //Запись текущего уровня в файл
        next_level_size = 0;                               //Зануление следущего уровня
        next_level = NULL;
        cur_dir_num = 0;
    }
    fclose(FS);
    FS = fopen("FS", "ab"); //Приходитя закрывать и открывать, т.к fseek не работает
    fseek(FS, 0, SEEK_SET);
    fwrite(&info, sizeof(struct Info), 1, FS); //Запись информации о файловой системе в файл
    fclose(FS);
}

//Чтение директорий из файла
void readFromFile(struct Dir *root)
{
    FILE *FS = fopen("FS", "rb");
    if (FS == NULL) //Если файла не создано, то инициализируем корневой каталог
    {
        initDir(root, NULL, "root");
        return;
    }
    long int size = filelength(fileno(FS)); //Если файл 0-го размера, то инициализируем корневой каталог
    if (size == 0)
    {
        initDir(root, NULL, "root");
        fclose(FS);
        return;
    }
    fread(&info, sizeof(struct Info), 1, FS); //Чтение информации о файловой сиситеме
    fread(root, sizeof(struct Dir), 1, FS);   //Чтение корневого каталога
    if (root->num_of_dirs > 0)                //Если у корневого каталога есть подкаталоги, то читаем их
    {
        root->dirs = (struct Dir *)malloc(sizeof(struct Dir) * root->num_of_dirs); //Выделение памяти на подкаталоги
        fread(root->dirs, sizeof(struct Dir), root->num_of_dirs, FS);              //Непосредственно чтение подкаталогов
    }
    else
    {
        return;
    }
    for (int i = 0; i < root->num_of_dirs; i++) //Даем ссылку подкаталогам на из предыдущий каталог
    {
        root->dirs[i].previous = root;
    }
    struct Dir **level; //Текущий уровень в дереве каталогов(массив указателей на каталоги)
    level = (struct Dir **)malloc(sizeof(struct Dir *) * root->num_of_dirs);
    for (int i = 0; i < root->num_of_dirs; i++) //Инициализируем текущий уровень подкаталогами
    {
        level[i] = &(root->dirs[i]);
    }
    struct Dir **next_level = NULL; //Следующий уровень в дереве каталогов
    int level_size = root->num_of_dirs;
    int next_level_size = 0;
    int cur_dir_num = 0; //Показывает номер в массиве следущего уровня
    while (1)
    {
        for (int i = 0; i < level_size; i++) //Проход по текущему уровню
        {
            if ((*level[i]).num_of_dirs > 0) //Если у элемента текущего уровня есть подкаталоги, то добавляем их в
            {                                //массив следующего уровня дерева
                (*level[i]).dirs = (struct Dir *)malloc(sizeof(struct Dir) * (*level[i]).num_of_dirs);
                fread((*level[i]).dirs, sizeof(struct Dir), (*level[i]).num_of_dirs, FS); //Чтение подкаталогов
                for (int k = 0; k < (*level[i]).num_of_dirs; k++)                         //Проходимся по подкоталогам
                {
                    (*level[i]).dirs[k].dirs = NULL;         //Зануляем их указатели на их подкоталоги
                    (*level[i]).dirs[k].previous = level[i]; //Даем им указатель на предыдущий каталог
                }
                if (next_level_size == 0) //Елси след уровень был пуст, то выделение памяти
                {
                    next_level = (struct Dir **)malloc(sizeof(struct Dir) * (*level[i]).num_of_dirs);
                    next_level_size = (*level[i]).num_of_dirs;
                }
                else
                {                                  //Если след. уровень не пуст то перевыделение памяти
                    cur_dir_num = next_level_size; //Можно удалить
                    next_level_size += (*level[i]).num_of_dirs;
                    next_level = (struct Dir **)realloc(next_level, next_level_size * sizeof(struct Dir *));
                } //Добавление подкаталогов в массив следующего уровня
                for (int j = 0; cur_dir_num < next_level_size; cur_dir_num++, j++)
                    next_level[cur_dir_num] = &((*level[i]).dirs[j]);
            }
        }
        if (next_level_size == 0) //Если след. уровень пуст, то конец чтения
        {
            break;
        }
        level = next_level; //Подготовка к обработке след. уровня
        level_size = next_level_size;
        next_level_size = 0;
        next_level = NULL;
        cur_dir_num = 0;
    }
    fclose(FS);
}

//Получение текущего пути(рекурсивно)(возвращает строку с путем)
char *getCurrentPath(struct Dir *dir)
{
    static char path[1000];
    if (dir->previous != NULL) //Если НЕ дошли до корневого каталога, то
    {
        strcat(getCurrentPath(dir->previous), dir->name); //добавление именя текущей директории к общему имени, возвращенным
        strcat(path, "/");                                //еще одним вызовом функции
    }
    else
    { //Если дошли до корневого каталога, то запить его имени в путь
        strcpy(path, dir->name);
        strcat(path, "/");
    }
    return path;
}

//Удаление директории(рекурсивно)
void delDir(struct Dir *pdir)
{
    for (int j = 0; j < pdir->num_of_files; j++)
    {
        delFile(pdir, pdir->filesInfo[j].name/*, j++*/);
    }
    if (pdir->dirs != NULL)
    {
        for (int i = 0; i < pdir->num_of_dirs; i++) //Вызов удаления для всех подкаталогов
            delDir(&(pdir->dirs[i]));
    }
    //Удаление файлов директории(можно не писать, т.к. в данной реализации при потери информации о файле, сам файл недосягаем)
    //(при удалении файла память не освобождатся)
    int j = 0;
    struct Dir *prevDirPtr = pdir->previous; //Получение указателя на предыдущий каталог
    for (; j < prevDirPtr->num_of_dirs; j++) //Поиск данного каталога их предыдущего каталога
    {
        if (!strcmp(pdir->name, prevDirPtr->dirs[j].name))
            break;
    }
    if (j == prevDirPtr->num_of_dirs)
    {
        puts("DELETE ERROR");
        return;
    }
    while (j < (prevDirPtr->num_of_dirs - 1)) //Смещение всех подкаталогов у предыдущего каталога влево, чтобы удалить
    {                                         //информацию о данном каталоге
        prevDirPtr->dirs[j] = prevDirPtr->dirs[j + 1];
        j++;
    }
    prevDirPtr->num_of_dirs--;
    prevDirPtr->dirs = (struct Dir *)realloc(prevDirPtr->dirs, sizeof(struct Dir) * prevDirPtr->num_of_dirs); //Перевыделение памяти
}

//Чтение файла(возвращаемые значения: 1 - успешно, 0 - такого файла нет)
int readFile(struct Dir *pdir, const char *n, char *buf)
{
    int i = 0;
    for (; i < pdir->num_of_files; i++) //Поиск файла в массиве файлов
    {
        if (!strcmp(pdir->filesInfo[i].name, n))
            break;
    }
    if (i == pdir->num_of_files)
    {
        return 0;
    }
    FILE *FS = fopen("FS", "r+b");
    fsetpos(FS, &(pdir->filesInfo[i].position)); //Перемещение указателя в "диске" на место начало файла

    for (int j = 0; j < pdir->filesInfo[i].size; j++)
        buf[j] = fgetc(FS);

    fclose(FS);
    return 1;
}

//Запись данных в файл(возвращаемые значения: 1 - успешно, 0 - такого файла нет)
int writeDataInFile(struct Dir *pdir, const char *n, const char *buf)
{
    int i = 0;
    for (; i < pdir->num_of_files; i++) //Поиск файла в массиве файлов
    {
        if (!strcmp(pdir->filesInfo[i].name, n))
            break;
    }
    if (i == pdir->num_of_files)
    {
        return 0;
    }
    FILE *FS = fopen("FS", "r+b");
    fsetpos(FS, &(pdir->filesInfo[i].position)); //Перемещение указателя в "диске" на место начала файла

    for (int j = 0; j < pdir->filesInfo[i].size; j++)
        fputc(buf[j], FS);

    fclose(FS);
    return 1;
}

//Добавление файла(возвращаемые значения: 1 - успешно, 0 - места нет, -1 - файл с таким именем уже существует,
//-2 - в директории уже максимальное количество файлов)
int addFile(struct Dir *pdir, const char *n, int s)
{

    int i = 0;
    for (; i < pdir->num_of_files; i++)
    {
        if (!strcmp(pdir->filesInfo[i].name, n))
            return -1;
    }
    if (pdir->num_of_files == MAX_NUM_OF_FILES)
        return -2;
    FILE *FS = fopen("FS", "r+b");
    fsetpos(FS, &info.begin_of_free_space); //Премещение указателя в "диске" на начало свободного места
    fseek(FS, s, SEEK_CUR);
    fgetpos(FS, &info.begin_of_free_space); //Получение указателя на новое свободное место
    pdir->filesInfo[pdir->num_of_files].position = info.begin_of_free_space;
    pdir->filesInfo[pdir->num_of_files].size = s;
    strcpy(pdir->filesInfo[pdir->num_of_files].name, n);
    pdir->num_of_files++;
    fclose(FS);
    char *zeroMem = (char *)calloc(s, sizeof(char));
    writeDataInFile(pdir, n, zeroMem); //Зануление памяти
    free(zeroMem);
    return 1;
}

//Удаление файла(возвращаемые значения: 1 - успешно, 0 - такого файла нет)
int delFile(struct Dir *pdir, const char *n)
{
    int i = 0;
    for (; i < pdir->num_of_files; i++) //Ищем файл в директории
    {
        if (!strcmp(pdir->filesInfo[i].name, n))
            break;
    }
    if (i == pdir->num_of_files)
    {
        return 0;
    }

    while (i < (pdir->num_of_files - 1)) //Смещение инфы о фалах в массиве на 1 влево, чтобы удалить ифнормацию
    {                                    //об удаляемом файле
        pdir->filesInfo[i] = pdir->filesInfo[i + 1];
        i++;
    }
    strcpy(pdir->filesInfo[i].name, "");
    pdir->filesInfo[i].position = -1;
    pdir->filesInfo[i].size = -1;
    pdir->num_of_files--;
    return 1;
}

int LoadFileFromFS(struct Dir *pdir, char *path, char *filename)
{
    int i = 0;
    for (; i < pdir->num_of_files; i++)
        if (!strcmp(pdir->filesInfo[i].name, filename))
            break;
    if (i == pdir->num_of_files)
        return 0;
    char *buf = (char *)calloc(pdir->filesInfo[i].size, sizeof(char));
    if (!readFile(pdir, filename, buf))
        return 0;
    char fullPath[MAX_NAME];
    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, filename);
    FILE *file = fopen(fullPath, "wb");
    if (!file)
        return -1;

    for (int j = 0; j < pdir->filesInfo[i].size; j++)
        fputc(buf[j], file);

    chsize(fileno(file), pdir->filesInfo[i].size);
    fclose(file);
    return 1;
}

int LoadFileInFS(struct Dir *pdir, char *path, char *filename)
{
    char fullPath[MAX_NAME];
    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, filename);
    FILE *file = fopen(fullPath, "rb");
    if (!file)
        return -3;
    int filesize = filelength(fileno(file));

    char *buf = (char *)calloc(filesize, sizeof(char));

    size_t q = 0, bytes;
    while ((bytes = fread(buf + q, sizeof(char), 1000, file)) != (size_t)EOF && bytes > 0)
        q += bytes;

    printf("%d\n", filesize);

    fclose(file);
    int err = addFile(pdir, filename, filesize);
    if (err <= 0)
        return err;
    writeDataInFile(pdir, filename, buf);
    return 1;
}

int main()
{
    system("cls");
    struct Dir root;            //Корень дерева каталогов
    readFromFile(&root);        //Читаем каталоги из файла с инфой
    struct Dir *curDir = &root; //Инициализация указателя на текущую директорию
    char buf[MAX_NAME], q;
    while (1)
    {
        printf("[ %s ] # ", getCurrentPath(curDir));
        fflush(stdin);
        scanf("%s%c", buf, &q);
        if (!strcmp(buf, "exit"))
            break;
        else if (!strcmp(buf, "ls"))
            showDir(curDir);
        else if (!strcmp(buf, "help"))
            Help();
        else if (!strcmp(buf, "mkdir"))
        {
            gets(buf);
            int n = addDir(curDir, buf);
            switch (n)
            {
            case 1:
                break;
            case 0:
                puts("this directory is full");
                break;
            case -1:
                puts("directory with this name has already exists");
                break;
            }
        }
        else if (!strcmp(buf, "rmdir"))
        {
            gets(buf);
            int i = 0;
            for (; i < curDir->num_of_dirs; i++)
            {
                if (!strcmp(curDir->dirs[i].name, buf))
                    break;
            }
            if (i == curDir->num_of_dirs)
            {
                strcat(buf, " no such directory");
                puts(buf);
            }
            else
            {
                delDir(&(curDir->dirs[i]));
            }
        }
        else if (!strcmp(buf, "touch"))
        {
            gets(buf);
            int n;
            n = addFile(curDir, buf, 0);
            switch (n)
            {
            case 1:
                break;
            case 0:
                puts("There are no free space");
                break;
            case -1:
                puts("File with this name is already exist");
                break;
            case -2:
                puts("This directory is full");
                break;
            }
        }
        else if (!strcmp(buf, "writeFile"))
        {
            char fileData[SIZE_OF_FILE];
            printf("Enter name: ");
            gets(buf);
            printf("Enter data: ");
            gets(fileData);
            if (!writeDataInFile(curDir, buf, fileData))
            {
                puts("There is no such file");
            }
        }
        else if (!strcmp(buf, "rm"))
        {
            gets(buf);
            if (!delFile(curDir, buf))
            {
                puts("There are no such file");
            }
        }
        else if (!strcmp(buf, "cd"))
        {
            gets(buf);
            if (!strcmp(buf, ".."))
            {
                if (curDir->previous)
                    curDir = curDir->previous;
            }
            else
            {
                int i = 0;
                for (; i < curDir->num_of_dirs; i++)
                {
                    if (!strcmp(curDir->dirs[i].name, buf))
                        break;
                }
                if (i == curDir->num_of_dirs)
                {
                    strcat(buf, " is wrong name");
                    puts(buf);
                }
                else
                {
                    curDir = &(curDir->dirs[i]);
                }
            }
        }
        else if (!strcmp(buf, "lifs"))
        {
            char path[MAX_NAME];
            int err;
            puts("Write path:");
            gets(path);
            puts("Write name of file:");
            gets(buf);
            err = LoadFileInFS(curDir, path, buf);
            if (err == 0)
                puts("There is no such file");
            else if (err == -1)
                puts("Can't create file in file system");
        }
        else if (!strcmp(buf, "lffs"))
        {
            char path[MAX_NAME];
            int err;
            puts("Write path to directory:");
            gets(path);
            puts("Write name of file:");
            gets(buf);
            err = LoadFileFromFS(curDir, path, buf);
            switch (err)
            {
            case 1:
                puts("File added");
                break;
            case 0:
                puts("There is no free space");
                break;
            case -1:
                puts("There is file with this name");
                break;
            case -2:
                puts("Directory is full");
                break;
            case -3:
                puts("Can't open file in file system");
                break;
            }
        }
        else
        {
            strcat(buf, " is wrong command");
            puts(buf);
        }
    }
    writeInFile(&root); //Запись информации в файл
    return 0;
}