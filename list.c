#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct lnode {
    struct lnode *next;
    void *val;
};

struct list_struct {
    struct lnode *head;
};

typedef struct list_struct* list;

void add_value(list li, void* val) {
    struct lnode* node = malloc(sizeof(struct lnode));
    node->val = val;
    node->next = NULL;
    if (li->head == NULL) {
        li->head = node;
        return;
    }
    struct lnode* current = li->head;
    while (current->next != NULL) { // O(N) search for last node
        current = current->next;
    }
    assert(current->next == NULL);
    current->next = node;
}

void print_list(list li) {
    printf("Pointers in list values:\n");
    if (li->head == NULL) {
        printf("EMPTY\n");
        return;
    }
    struct lnode* current = li->head;
    printf("%x\n",(int)current->val);
    while (current->next != NULL) {
        current = current->next;
        printf("%x\n", (int)current->val);
    }
    return;
}

int get_lsize(list li) {
    if (li->head == NULL)
        return 0;
    int sz = 1;
    struct lnode* current = li->head;
    while (current->next != NULL) { // O(N) search for last node
        current = current->next;
        sz++;
    }
    return sz;
}

int main() {
    list li = malloc(sizeof(struct list_struct));
    li->head = NULL;
    int values[] = { 1, 2, 3, 4, 5 };
    int* valptr = (int*)values;
    int i = 0;
    printf("adding values\n");
    for (i; i < 5; ++i) {
        printf("adding %d\n", *valptr); 
        add_value(li, (void*)valptr++);
    }
    assert(get_lsize(li) == 5);
    print_list(li);
}

