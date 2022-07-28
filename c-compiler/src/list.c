#include "list.h"

#include "AST.h"
#include <memory.h>


struct List * init_list(size_t item_size) {

	struct List * list = calloc(1, sizeof(struct List));
	list->size = 0;
	list->capacity = 0;
	list->item_size = item_size;
	
	return list;
}

void free_list(struct List * list) {

	for(size_t i = 0; i < list->size; ++i) {
		memset(list->items[i], 0, list->item_size);
		free(list->items[i]);
	}
	free(list->items);
	free(list);

}

void list_push(struct List * list, void* item) {
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

void list_pop(struct List * list) {
	if(!list->size) 
		return;

	list->items[list->size] = NULL;
	free(list->items[list->size]);
}


void* list_at(struct List * list, int index) {

	if(0 <= index && index < list->size)
		return list->items[index];

	return NULL;

}

void print_list(struct List * list) {

	println("<size={u}, capacity={u}, item_size={u}>", list->size, list->capacity, list->item_size);
	for(size_t i = 0; i < list->size; ++i) {
		print_ast("\t{s}\n", list->items[i]);
	}

}
