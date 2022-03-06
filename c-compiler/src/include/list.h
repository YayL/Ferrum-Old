#ifndef IRON_LIST_H
#define IRON_LIST_H

#include <stdlib.h>

typedef struct LIST_STRUCT {
	void** items;
	size_t size;
	size_t capacity;
	size_t item_size;
} list_t;

list_t* init_list(size_t item_size);

/**
 * @brief Push a new item to the back of the list
 * 
 * @param list 
 * @param item 
 */

void list_push(list_t* list, void* item);

/**
 * @brief Remove the last element in the list from memory but preserve capacity.
 * 
 * @param list 
 * @return void* 
 */

void* list_pop(list_t* list);

/**
 * @brief Return the item at a certain index in the list
 * 
 * @param list 
 * @param index 
 * @return void* 
 */

void* list_at(list_t* list, int index);

#endif