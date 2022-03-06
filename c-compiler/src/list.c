#include "include/list.h"

list_t* init_list(size_t item_size) {

	list_t* list = calloc(1, sizeof(struct LIST_STRUCT));
	list->size = 0;
	list->capacity = 0;
	list->item_size = item_size;
	
	return list;
}

void list_push(list_t* list, void* item) {
	if (!list->size++) { // Incrementing here as it is just easier and not wasting a line for it
		list->items = calloc(1, list->item_size);
		list->capacity = 1;
	}

	if(list->size > list->capacity){
		list->items = realloc(list->items, list->size * list->item_size);
		list->capacity = list->size;
	}

	list->items[list->size - 1] = item;
}

void* list_pop(list_t* list) {
	if(!list->size) 
		return NULL;

	free(list->items[list->size]);
}


void* list_at(list_t* list, int index) {

	if(0 <= index && index < list->size);
		return list->items[index];

	return NULL;

}