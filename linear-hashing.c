#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


// --- Constantes ---
// total de elementos = (espacio total de una página) / (tamaño de los elementos) = 1024 (bytes) / 8 (bytes) = 128
#define PAGE_SIZE 1024 // bytes
#define ELEMENT_SIZE sizeof(long long) // 8 bytes
#define ELEMENTS_PER_PAGE (PAGE_SIZE / ELEMENT_SIZE) // 128 elementos


// --- Variables Globales ---
// variable para contar los I/O totales para N inserciones
unsigned long long IOs = 0;

// Suma de la cantidad total de inserciones
unsigned long long inserciones = 0;

// costo promedio real
double C_real = 0;


// --- Funciones auxiliares ---
// Función auxiliar para calcular el dos elevado a un n eficientemente
// Debido a la cantidad de números que unsigned long long puede representar, 
// el n máximo es 63, pues luego se repite el valor de 2^(63)
unsigned long long exponent_base2(int n) {
    if (n >= 64) {
        return 18446744073709551615ULL;
    }
    else {
        return (1ULL << n);
    }
}


// --- Función de hash ---
unsigned long long h(long long y) {
    unsigned long long result = y % (18446744073709551615ULL);
    return result;
}


// --- Páginas ---
// Estructura para las páginas
typedef struct Page {
    long long elements[ELEMENTS_PER_PAGE];  // Array con los elementos de la página
    int size;                               // cantidad de elementos en la página
    struct Page* overflow;                  // Puntero a la página de rebalse
} Page;

// Crea una nueva página
Page *create_page() {
    Page *page = (Page *)malloc(sizeof(Page));
    page->size = 0;
    page->overflow = NULL;
    return page;
}

// Inserta un elemento en una página
// Si esta desbordada, se crea una nueva
void insert_into_page(Page *page, long long value) {
    IOs++;
    while(page->size == ELEMENTS_PER_PAGE) {
        if (page->overflow == NULL) {
            page->overflow = create_page();
            IOs++;
        }
        page = page->overflow;
        IOs++;
    }
    page->elements[page->size] = value;
    page->size++;
}

// Busca un elemento en una página (incluyendo las de desborde)
int search_in_page(Page *page, long long value) {
    int count = 0;
    IOs++;
    while(page) {
        if (count == page->size) {
            page = page->overflow;
            count = 0;
            IOs++;
        }
        else {
            if (page->elements[count] == value) {
                return 1;
            }
            else {
                count++;
            }
        }
    }
    IOs--;
    return 0;
}

// Libera la memoria de las páginas que no se usarán más
void free_pages(Page *page) {
    while(page) {
        Page *actual_page = page;
        page = page->overflow;
        free(actual_page);
        IOs++;
    }
}


// --- Tabla hash ---
// Celda de una tabla hash
typedef struct Cell {
    int index;               // Índice de la celda
    Page *page;              // Página almacenada
    struct Cell *prev;       // Puntero a la celda anterior
    struct Cell *next;       // Puntero a la celda siguiente
} Cell;

// Crea una celda
Cell *create_cell(int index, Page *page) {
    Cell* new_cell = (Cell*)malloc(sizeof(Cell));
    if (new_cell == NULL) {
        printf("Error al asignar memoria para la celda.\n");
        exit(1);
    }
    new_cell->index = index;
    new_cell->page = page;
    new_cell->prev = NULL;
    new_cell->next = NULL;
    return new_cell;
}

// Estructura de una tabla Hash
typedef struct {
    Cell *head;              // Puntero a la primera celda de la tabla hash
    Cell *tail;              // Puntero a la última celda de la tabla hash
    unsigned long long p;    // Cantidad actual de páginas (p)
    int t;    // Factor de expansión
} HashTable;

// Crea una tabla hash
HashTable *create_hashTable() {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));

    // hash parte con una celda y una página
    Page *page = create_page();
    Cell *cell0 = create_cell(0, page);

    // set de elementos iniciales del hash
    table->head = cell0;
    table->tail = cell0;
    table->p = 1;
    table->t = 0;

    return table;
}

// Función auxiliar para calcular el porcentaje promedio de llenado de las páginas de una tabla hash
double percent(HashTable *table) {
    Cell *temp_c = table->head;
    int sum_percent = 0;
    int pages = 0;
    while(temp_c) {
        Page *temp_p = temp_c->page;
        while(temp_p) {
            sum_percent += (temp_p->size * 100) / ELEMENTS_PER_PAGE;
            pages++;
            temp_p = temp_p->overflow;
        }
        Cell *temp = temp_c->next;
        temp_c = temp;
    }
    double prom_llenado = sum_percent / pages;
    return prom_llenado;
}

// Agrega una página al final de la tabla hash
void append(HashTable *table, Page *page) {
    Cell* new_cell = create_cell(table->p, page);  // El indice es el tamaño actual de la tabla hash
    
    // Agregar la celda al final de la tabla hash
    table->tail->next = new_cell;
    new_cell->prev = table->tail;
    table->tail = new_cell;
}

// Agrega una celda en una posición especifica en una tabla hash
void insert_at(HashTable *table, int index, Page *page) {
    if (index < 0 || index > table->p) {
        return;
    }

    Cell* new_cell = create_cell(index, page);

    Cell* temp = table->head;
    for (int i = 0; i < index; i++) {
        temp = temp->next;
    }

    new_cell->next = temp->next;
    new_cell->prev = temp->prev;
    if (temp->next != NULL) {
        temp->next->prev = new_cell;
    }

    if (temp->prev != NULL) {
        temp->prev->next = new_cell;
    }

    if (temp == table->head) {
        table->head = new_cell;
    }

    if (temp == table->tail) {
        table->tail = new_cell;
    }

    free_pages(temp->page);
    free(temp);

}

// Entrega la página en el indice pedido de la tabla hash
Page *get_page_by_index(HashTable *table, int index) {
    Cell *find = table->head;
    while(find) {
        if (find->index == index) {
            return find->page;
        }
        else {
            find = find->next;
        }
    }
    printf("Error al acceder a la celda %d.\n", index);
    return NULL;
}

// Expande la tabla y redistribuye los elementos siguiendo la idea del hashing lineal
void expand(HashTable *table) {
    // página p se agrega al final de la tabla
    Page *page_p = create_page();
    append(table, page_p);
    IOs++;

    // página compacta que guardará los elementos que no son de la página p
    Page *page_compacted = create_page();

    // página a expandir
    int index = table->p - exponent_base2(table->t);
    Page *page_to_expand = get_page_by_index(table, index);
    IOs++;

    while(page_to_expand) {
        long long *elements = page_to_expand->elements;
        for(int i=0; i<page_to_expand->size; i++) {
            long long value = elements[i];
            unsigned long long k = h(value) % exponent_base2(table->t + 1);
            if(k != index) {
                insert_into_page(page_p, value);
            }
            else {
                insert_into_page(page_compacted, value);
            }
        }
        page_to_expand = page_to_expand->overflow;
        IOs++;
    }
    
    insert_at(table, index, page_compacted);

    table->p++;

    if (table->p == exponent_base2(table->t + 1)) {
        table->t++;
    }
}

// Inserta un elemento en la tabla de hash
void insert(HashTable *table, long long value, int C_max) {
    unsigned long long k = h(value) % exponent_base2(table->t + 1);

    if (k < table->p) {
        Page *page = get_page_by_index(table, k);
        if (!search_in_page(page, value)) { // valor no se encuentra en la página
            insert_into_page(page, value);
        }
    }

    if (k >= table->p ) {
        Page *page = get_page_by_index(table, (k - exponent_base2(table->t)));
        if (!search_in_page(page, value)) { // valor no se encuentra en la página
            insert_into_page(page, value);
        }
    }

    // Si el costo de búsqueda promedio excede el permitido, expandir la tabla
    C_real = IOs / inserciones;
    if (C_real > C_max) {
        expand(table);
    }
}

// Liberar memoria de la tabla de hash
void free_table(HashTable *table) {
    Cell *actual_cell = table->head;
    unsigned long long limit = table->p;
    for (int i = 0; i < limit; i++) {
        Page *page = actual_cell->page;
        if(page) {
            free_pages(page);
        }
        Cell *temp = actual_cell;
        actual_cell = actual_cell->next;
        free(temp);
    }
}

// Función principal para probar el código
int main(int argc, char *argv[]) {
    srand(time(NULL));

    // Se crea la tabla de hash
    HashTable *table = create_hashTable();

    // Se eligeun número de largo de la secuencia
    unsigned long long N = exponent_base2(atoi(argv[2]));

    printf("N: %lld\n", N);

    // Insertar elementos
    for (long long i = 0; i < N; i++) {
        inserciones++;
        long long random_number = (long long)rand();
        insert(table, random_number, atoi(argv[1]));
    }

    double prom_llenado = percent(table);

    printf("Costo promedio máximo elegido: %d\n", atoi(argv[1]));
    printf("Costo promedio real: %f \n", C_real);
    printf("Porcentaje de llenado de páginas: %f\n", prom_llenado);
    printf("Largo de final de la tabla hash: %lld\n", table->p);

    // Liberar memoria
    free_table(table);

    return 0;
}