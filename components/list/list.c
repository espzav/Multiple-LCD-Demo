/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "list.h"

/*******************************************************************************
* Public API functions
*******************************************************************************/

esp_err_t list_create(list_t * list)
{
	assert(list != NULL);

	memset(list, 0, sizeof(list_t));

	return ESP_OK;
}

esp_err_t list_delete(list_t * list)
{
	list_item_t * next_item;
	assert(list != NULL);

	next_item = list->first;

	/* Free all items */
	while(next_item != NULL)
	{
		list_item_t *i = next_item;
		if(i->item != NULL)
			free(i->item);

		next_item = i->next;
		free(i);
	}

	memset(list, 0, sizeof(list_t));

	return ESP_OK;
}

esp_err_t list_clear(list_t * list)
{
	return list_delete(list);
}

esp_err_t list_add_after(list_t * list, list_item_t * after, void *item, uint32_t size, void ** added_item)
{
	esp_err_t err = ESP_OK;
	list_item_t * list_item;
	assert(list != NULL);

	/* Create list item */
	list_item = calloc(sizeof(list_item_t), 1);
	if(list_item == NULL)
		return ESP_ERR_NO_MEM;

	/* Create size in item */
	list_item->item = calloc(size, 1);
	if(list_item->item == NULL)
	{
		err = ESP_ERR_NO_MEM;
		goto ERROR;
	}

	/* Copy data */
	memcpy(list_item->item, item, size);

	/* Add on the first in list */
	if(after == NULL)
	{
		if(list->first)
			list->first->prev = list_item;

		list_item->next = list->first;
		list->first = list_item;

		list_item->prev = NULL;

		if(list->last == NULL)
			list->last = list_item;

	}
	else
	{
		/* Save next */
		list_item_t * next = after->next;

		/* Set next of before item */
		after->next = list_item;
		/* Set preview of created item */
		list_item->prev = after;
		/* Set next of created item */
		list_item->next = next;

		/* If next item exists, then set created item as preview, othervise set created item as last in list */
		if(next != NULL)
			next->prev = list_item;
		else
			list->last = list_item;
	}

	if(added_item)
		*added_item = list_item->item;


ERROR:
	if(err != ESP_OK)
	{
		if(list_item != NULL)
			free(list_item);
	}

	return err;
}

esp_err_t list_add_before(list_t * list, list_item_t * before, void *item, uint32_t size, void ** added_item)
{
	esp_err_t err = ESP_OK;
	list_item_t * list_item;
	assert(list != NULL);

	/* Create list item */
	list_item = calloc(sizeof(list_item_t), 1);
	if(list_item == NULL)
		return ESP_ERR_NO_MEM;

	/* Create size in item */
	list_item->item = calloc(size, 1);
	if(list_item->item == NULL)
	{
		err = ESP_ERR_NO_MEM;
		goto ERROR;
	}

	/* Copy data */
	memcpy(list_item->item, item, size);

	/* Add on the first in list */
	if(before == NULL)
	{
		if(list->first)
			list->first->prev = list_item;

		list_item->next = list->first;
		list->first = list_item;

		list_item->prev = NULL;

		if(list->last == NULL)
			list->last = list_item;

	}
	else
	{
		/* Save prev */
		list_item_t * prev = before->prev;

		/* Set prev of before item */
		before->prev = list_item;
		/* Set preview of created item */
		list_item->prev = prev;
		/* Set next of created item */
		list_item->next = before;

        if(list->first == before)
            list->first = list_item;

		/* If prev item exists, then set created item as preview, othervise set created item as last in list */
		if(prev != NULL)
			prev->next = list_item;
		else
			list->last = list_item;
	}

	if(added_item)
		*added_item = list_item->item;


ERROR:
	if(err != ESP_OK)
	{
		if(list_item != NULL)
			free(list_item);
	}

	return err;
}

esp_err_t list_add(list_t * list, void *item, uint32_t size)
{
	esp_err_t err = ESP_OK;
	list_item_t * list_item;
	assert(list != NULL);

	/* Create list item */
	list_item = calloc(sizeof(list_item_t), 1);
	if(list_item == NULL)
		return ESP_ERR_NO_MEM;

	/* Create size in item */
	list_item->item = calloc(size, 1);
	if(list_item->item == NULL)
	{
		err = ESP_ERR_NO_MEM;
		goto ERROR;
	}

	/* Copy data */
	memcpy(list_item->item, item, size);

	if(list->first == NULL)
		list->first = list_item;

	list_item->prev = list->last;

	if(list->last != NULL)
		list->last->next = list_item;

	list->last = list_item;

ERROR:
	if(err != ESP_OK)
	{
		if(list_item != NULL)
			free(list_item);
	}

	return err;
}

esp_err_t list_remove(list_t * list, list_item_t * item)
{
	list_item_t * prev;
	list_item_t * next;
	assert(list != NULL);
	assert(item != NULL);

	if(item->item != NULL)
		free(item->item);

	prev = item->prev;
	next = item->next;

	if(prev != NULL)
		prev->next = next;

	if(next != NULL)
		next->prev = prev;

	if(item == list->first)
		list->first = next;

	if(item == list->last)
		list->last = prev;

	free(item);

	return ESP_OK;
}

void * list_get_first(list_t * list, list_item_t ** item)
{
	assert(list != NULL);
	assert(item != NULL);

	*item = list->first;

	if((*item) == NULL)
		return NULL;

	return (*item)->item;
}

void * list_get_last(list_t * list, list_item_t ** item)
{
	assert(list != NULL);
	assert(item != NULL);

	*item = list->last;

	if((*item) == NULL)
		return NULL;

	return (*item)->item;
}

void * list_get_next(list_t * list, list_item_t ** item)
{
	assert(list != NULL);
	assert(item != NULL);

	*item = (*item)->next;

	if((*item) == NULL)
		return NULL;

	return (*item)->item;
}

void * list_get_prev(list_t * list, list_item_t ** item)
{
	assert(list != NULL);
	assert(item != NULL);

	*item = (*item)->prev;

	if((*item) == NULL)
		return NULL;

	return (*item)->item;
}

uint32_t list_get_count(list_t * list)
{
	list_item_t * next_item;
	uint32_t count = 0;
	assert(list != NULL);

	next_item = list->first;

	/* Free all items */
	while(next_item != NULL)
	{
		count++;
		next_item = next_item->next;
	}

	return count;
}
