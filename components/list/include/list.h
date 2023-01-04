/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Types definitions
*******************************************************************************/

typedef struct list_item_s
{
	void 				*item;
	uint32_t 			size;
	struct list_item_s 	*prev;
	struct list_item_s 	*next;
} list_item_t;

typedef struct list_s
{
	list_item_t * first;
	list_item_t * last;
} list_t;

/*******************************************************************************
* Functions definitions
*******************************************************************************/
esp_err_t list_create(list_t * list);
esp_err_t list_delete(list_t * list);
esp_err_t list_clear(list_t * list);
esp_err_t list_add(list_t * list, void *item, uint32_t size);
esp_err_t list_add_after(list_t * list, list_item_t * after, void *item, uint32_t size, void ** added_item);
esp_err_t list_add_before(list_t * list, list_item_t * before, void *item, uint32_t size, void ** added_item);
esp_err_t list_remove(list_t * list, list_item_t * item);
void * list_get_first(list_t * list, list_item_t ** item);
void * list_get_last(list_t * list, list_item_t ** item);
void * list_get_next(list_t * list, list_item_t ** item);
void * list_get_prev(list_t * list, list_item_t ** item);
uint32_t list_get_count(list_t * list);


#ifdef __cplusplus
}
#endif

