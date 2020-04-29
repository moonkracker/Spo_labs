#include <stdio.h>
#include <stdbool.h>

#define HEAP_SIZE 256                               //Полный размер heap раздела

typedef struct mem_header                           //Структура данных о разделе памяти
{
    struct mem_header *next;                        //Указатель на следующий элемент
    int block_size;                                 //Выделенная память для блока
    bool available;                                 //Если true то блок свободен, если нет то занят
    int available_bytes;                            //Число доступных байт после выделения памяти
    void *heap_pointer;                             //Указатель на память в heap разделе
    void *old;
} mem_head;

int heap_size = HEAP_SIZE;                          //Доступные байты для выделения
char heap_block[HEAP_SIZE];                         //раздел памяти размером HEAP_SIZE
mem_head mem_addr;                                  //Стартовая точка структуры данных о блоках памяти

void initialize()                                   //Инициализация данных
{
    mem_addr.next = NULL;
    mem_addr.block_size = 0;
    mem_addr.available = 1;
    mem_addr.available_bytes = HEAP_SIZE;
    mem_addr.heap_pointer = heap_block;             //heap_pointer указывает на 1 байт в heap
    mem_addr.old = heap_block;
}

void my_mem_move(void *dest, void *src, int n)
{
    for (int i = 0; i < n; i++)
        ((char *)dest)[i] = ((char *)src)[i];
}

void *my_malloc(size_t size)
{
    if (size < 0 || size > HEAP_SIZE)               //Размер должен быть положительным и меньше чем доступный размер HEAP_SIZE
    {
        printf("SIZE INPUT ERROR\n");
        return NULL;                               
    }

    mem_head *temp = &mem_addr;                     //Временный указатель связного списка

    while (temp)                                    
    {
        if (temp->available == 1)
        {
            if (temp->available_bytes > size)       //Выделение памяти заданного размера
            {
                heap_size -= size;                  //Обновляем кол-во доступных байт
                /*обновляем данные указателя памяти*/
                temp->block_size = size;
                temp->available = 0;
                temp->next = (temp) + sizeof(mem_head);

                /*Обновляем следующий указатель*/
                mem_head *next_mem_addr = temp->next;
                next_mem_addr->available = 1;
                next_mem_addr->available_bytes = heap_size;
                next_mem_addr->heap_pointer = ((temp->heap_pointer) + size); //Сдвигаем указатель на следующие свободные байты
                next_mem_addr->old = next_mem_addr->heap_pointer;
                next_mem_addr->next = NULL;
                return temp->heap_pointer;
            }
            return NULL;
        }
        temp = temp->next;                                                   //Сдвигаемся на следующее значение
    }

    return NULL;
}

void my_free(void *ptr)
{
    if (ptr != 0)
    {
        mem_head *tmp = &mem_addr;

        while (tmp->heap_pointer != ptr)                                    //Ищем указатель который хранить данные о памяти ptr        
            tmp = tmp->next;

        tmp->available = 1;                                                 //Делаем блок доступным для выделения
        heap_size += tmp->block_size;                                       //Обновляем кол-во свободных байт
        char *t = (char *)tmp->heap_pointer;
        for (int i = 0; i < tmp->block_size; i++, t++)
            *t = 0;
    }
    else
        printf("memory is not allocated to this pointer\n");
}

size_t free_space_in_my_heap()
{
    return heap_size;
}

void show()                                                                     
{
    printf("\nyour heap: \n");
    for (int i = 0; i < HEAP_SIZE; i++)
        printf("%c", heap_block[i]);
    printf("\nfree space=%d\n", free_space_in_my_heap());

    mem_head *tmp = &mem_addr;
    printf("---------------------------------------------------------------------\n");
    printf("|  POINTER  |    BLOCK SIZE   |  AVL_MEM   |  AVL_BYTES |  NEXT_PTR |\n");
    printf("---------------------------------------------------------------------\n");
    while (tmp)
    {
        printf("|%10x | %15d | %10d | %10d | %10x|\n", tmp->heap_pointer,
               tmp->block_size, tmp->available, tmp->available_bytes, tmp->next);
        tmp = tmp->next;
    }
    printf("---------------------------------------------------------------------\n");
}

void *my_calloc(size_t nmemb, size_t size)                                              //Принимает кол-во элементов и их размер
{
    void *heap_ptr_mem = my_malloc((size)*nmemb);
    char *temp = (char *)heap_ptr_mem;
    for (int i = 0; i < size; i++, temp++)
        *temp = 0;

    return heap_ptr_mem;
}

void defragment()
{
    mem_head *t, *tmp = &mem_addr;

    while (tmp)
    {
        if (tmp->available)
        {
            t = tmp->next;
            while (t && t->available)
                t = t->next;
            if (t)
            {
                my_mem_move(tmp->heap_pointer, t->heap_pointer, t->block_size);
                tmp->available = 0;
                tmp->block_size = t->block_size;
                t->available = 1;
                t->available_bytes = heap_size;
                tmp->old = t->old;
                t->heap_pointer = (char *)tmp->heap_pointer + tmp->block_size;
                tmp->next = t;
            }
        }
        tmp = tmp->next;
    }
}

void *my_realloc(void *ptr, size_t size)
{
    int minimum_size;
    void *newptr;

    mem_head *tmp = &mem_addr;
    while (tmp->heap_pointer != ptr)
        tmp = tmp->next;
    newptr = my_malloc(size);                                               //Выделяем память под newptr и проверяем размер
    if (newptr == NULL)                                                     //if memory not available return NULL
        return NULL;
    mem_head *new_tmp = &mem_addr;
    while (new_tmp->heap_pointer != newptr)
        new_tmp = new_tmp->next;

    if (tmp != NULL)                                                         
    {
        minimum_size = tmp->block_size;
        if (size < minimum_size)
            minimum_size = size;                                             //Находим меньший из размеров
        my_mem_move(new_tmp->heap_pointer, tmp->heap_pointer, minimum_size); //Перемещаем данные в основной указатель если доступна память
        my_free(tmp->heap_pointer);                                          //Очищаем временный указатель
    }
    return tmp->heap_pointer;
}

void *reload(void *ptr)
{
    if (ptr != 0)
    {
        mem_head *tmp = &mem_addr;

        while (tmp && tmp->old != ptr)                                      //Ищем указатель хранящий данные о ptr
            tmp = tmp->next;

        if (tmp)
        {
            tmp->old = tmp->heap_pointer;
            ptr = tmp->heap_pointer;
            return ptr;
        }
    }
    return NULL;
}

int main()
{
    initialize();

    char *s1 = (char *)my_malloc(sizeof(char) * 6);
    for (int i = 0; i < 6; i++)
        s1[i] = 'a';
    char *s2 = (char *)my_malloc(sizeof(char) * 8);
    for (int i = 0; i < 8; i++)
        s2[i] = 'b';
    char *s3 = (char *)my_malloc(sizeof(char) * 25);
    for (int i = 0; i < 25; i++)
        s3[i] = 'c';
    char *s4 = (char *)my_malloc(sizeof(char) * 16);
    for (int i = 0; i < 16; i++)
        s4[i] = 'd';
    show();

    printf("after realloc: ");
    s2 = (char *)my_realloc(s2, sizeof(char) * 4);
    show();

    printf("after free: ");
    my_free(s1);
    show();

    printf("after defragment: ");
    defragment();
    show();

    s3 = (char *)reload(s3);
    s4 = (char *)reload(s4);
    s2 = (char *)reload(s2);

    printf("\n%x\n", s3);

    printf("\n");
    for (int i = 0; i < 25; i++)
    {
        printf("%c", s3[i]);
    }
    printf("\n");

    printf("after malloc: ");
    char *s5 = (char *)my_malloc(sizeof(char) * 20);
    for (int i = 0; i < 20; i++)
        s5[i] = 'e';
    show();

    char *s6 = (char *)my_malloc(sizeof(char) * 5);
    for (int i = 0; i < 5; i++)
        s6[i] = 'z';
    show();
    return 0;
}