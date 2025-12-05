#define __linked_list_init__(typename, attributes) \
typedef struct typename {struct typename* next; attributes; } typename; \
typename *typename##_head = NULL;

#define __linked_list_creator__(typename, arguments, set_attrs) \
typename *typename##_create arguments { \
    typename *item = (typename *) malloc(sizeof(*item)); \
    if (item) { \
        item->next = NULL; \
        set_attrs; \
    } \
    return item; \
}

#define __linked_list_creator_add__(typename, arguments, set_attrs) \
typename *typename##_create arguments { \
    typename *item = (typename *) malloc(sizeof(*item)); \
    if (item) { \
        item->next = typename##_head; \
        typename##_head = item; \
        set_attrs; \
    } \
    return item; \
}

#define __linked_list_destroy_all__(typename) \
void typename##_destroy_all(void) { \
    typename *temp = typename##_head; \
    typename *next; \
    while (temp) { \
        next = temp->next; \
        free(temp); \
        temp = next; \
    } \
    typename##_head = NULL; \
}

#define __linked_list_all_add__(typename, attributes, creator_arguments, creator_set_attrs) \
__linked_list_init__(typename, attributes) \
__linked_list_creator_add__(typename, creator_arguments, creator_set_attrs) \
__linked_list_destroy_all__(typename)

#define __linked_list_all__(typename, attributes, creator_arguments, creator_set_attrs) \
__linked_list_init__(typename, attributes) \
__linked_list_creator__(typename, creator_arguments, creator_set_attrs) \
__linked_list_destroy_all__(typename)