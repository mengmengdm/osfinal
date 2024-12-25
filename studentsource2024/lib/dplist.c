//
// Created by fhr on 12/22/24.
//
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"
/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

//free_element if true call element_free() on the element of the list node to remove
void dpl_free(dplist_t **list, bool free_element) {
    if (list == NULL || *list ==NULL) {
        return;
    }
    else if ((*list)->head == NULL) {
      *list = NULL;
      return;
    }
    else if ((*list)->head != NULL && (*list)->head->next == NULL) {
        if (free_element) { (*list)->element_free(&((*list)->head->element));}
        free((*list)->head);
        *list = NULL;
        return;
    }
    else {
        //int count = 0;
        dplist_node_t *current_node, *next_node;
        current_node = (*list)->head;
        next_node = current_node;
        while (next_node->next != NULL) {
            current_node = next_node;
            next_node = current_node->next;
            if (free_element) { (*list)->element_free(&(current_node->element));}
            free(current_node);
        }
        //free((*list)->head);
        free(next_node);
        *list = NULL;
        return;
    }
}


//insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    //If 'list' is is NULL, NULL is returned.
    if (list == NULL){
        return NULL;
    }

    void *inserted_element;

    if (insert_copy) {
        inserted_element = list->element_copy(element);
    }
    else {
      inserted_element = element;
    }

    dplist_node_t *ref_at_index, *inserted_node;
    inserted_node = malloc(sizeof(dplist_node_t));
    inserted_node->element = inserted_element;

    if (list->head == NULL) { // covers case 1
        inserted_node->prev = NULL;
        inserted_node->next = NULL;
        list->head = inserted_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        inserted_node->prev = NULL;
        inserted_node->next = list->head;
        list->head->prev = inserted_node;
        list->head = inserted_node;
        // pointer drawing breakpoint
    } else {
        int list_size = dpl_size(list);
        //if the index >= size of the list
        if (index >= list_size) {
            index = list_size;
            ref_at_index = dpl_get_reference_at_index(list, index);
            assert(ref_at_index->next == NULL);
            inserted_node->next = NULL;
            inserted_node->prev = ref_at_index;
            ref_at_index->next = inserted_node;
            // pointer drawing breakpoint
        }

        // pointer drawing breakpoint
        else if (index < list_size) { // covers case 4
            ref_at_index = dpl_get_reference_at_index(list, index);
            assert(ref_at_index != NULL);
            ref_at_index->prev->next = inserted_node;
            inserted_node->prev = ref_at_index->prev;
            inserted_node->next = ref_at_index;
            ref_at_index->prev= inserted_node;
            // pointer drawing breakpoint
        }
    }
    return list;



}

//1. 'free_element' is true: remove the list node containing the element and use the callback to free the memory of the removed element;
//2. 'free_element' is false: remove the list node containing the element without freeing the memory of the removed element;
dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL || list->head == NULL) {
      return list;
    }

    dplist_node_t *node_at_index = dpl_get_reference_at_index(list, index);

    if (node_at_index == list->head && node_at_index->next == NULL) {
        list->head = NULL;
    }
    else if (node_at_index == list->head && node_at_index->next != NULL) {
        node_at_index->next->prev = NULL;
        list->head = node_at_index->next;
    }
    else if (node_at_index->next == NULL) {
      node_at_index->prev->next = NULL;
    }
    else {
      node_at_index->next->prev = node_at_index->prev;
      node_at_index->prev->next = node_at_index->next;
    }

    if (free_element) {
      list->element_free(&(node_at_index->element));
    }

    //do nothing if free_element is false
    free(node_at_index);
    return list;
}

int dpl_size(dplist_t *list) {

    //checking if the list is null or the head of list is null
    int count = 0;
    if(list==NULL||list->head==NULL) {
        return 0;
    }
    //define a test node as dummy, if dummy->next == null means it comes to the end
    else {
        dplist_node_t *dummy = list->head;
        while(dummy->next!=NULL) {
            dummy = dummy->next;
            count++;
        }
        return ++count;
    }
}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    void *element = NULL;
    dplist_node_t *node_at_index = dpl_get_reference_at_index(list, index);
    if(node_at_index != NULL) {
      element = node_at_index->element;
    }
    return element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    int index = 0;

    // Check for null list or empty list
    if (list==NULL || list->head==NULL) {
      return -1;
    }

    //check if the is the head of the list (only 1 element condition)
    if (list->element_compare(element, list->head->element) == 0) {
      return 0;
    }

    //default check for the whole list
    dplist_node_t *dummy = list->head;
    while(dummy->next!=NULL) {
        if(list->element_compare(element, dummy->element) == 0) {
            return index;
        }
        else {
            dummy = dummy->next;
            index++;
        }
    }

    //If 'element' is found at the end of the list
    if (dummy->next == NULL && list->element_compare(element, dummy->element) == 0) {
        return index;
    }

    //If 'element' is not found in the list, -1 is returned.
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    // Check for null list or empty list
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    // If index is less than or equal to 0, return the head of the list
    if (index <= 0) {
        return list->head;
    }

    // If index is greater than or equal to the size of the list, return the end of the list
    if (index >= dpl_size(list)) {
        index = dpl_size(list) - 1;
    }

    // Traverse the list to find the node at the specified index
    dplist_node_t *dummy = list->head;
    int count = 0;
    while (count < index) {
        dummy = dummy->next;
        count++;
    }

    return dummy;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    // Check for null list or empty list
    if (list == NULL || list->head == NULL || reference==NULL) {
        return NULL;
    }

    dplist_node_t *dummy = list->head;
    while (dummy->next != NULL) {

        if (dummy == reference) {
            return dummy->element;
        }
        dummy = dummy->next;
    }
    if (dummy->next == NULL && dummy ==reference) {
        return dummy->element;
    }

    return NULL;
}


