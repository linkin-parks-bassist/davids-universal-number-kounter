#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define DECLARE_LINKED_LIST(X) struct X##_linked_list;\
typedef struct X##_linked_list {\
	X data;\
	struct X##_linked_list *next;\
} X##_linked_list;\
\
X##_linked_list *X##_linked_list_new(X value);\
void free_X##_linked_list(X##_linked_list *list);\
X##_linked_list *X##_linked_list_tail(X##_linked_list *list);\
int X##_linked_list_append(X##_linked_list *list, X value);\
int X##_linked_list_remove_next(X##_linked_list *list);

#define IMPLEMENT_LINKED_LIST(X)\
X##_linked_list *X##_linked_list_new(X value)\
{\
	X##_linked_list *result = malloc(sizeof(X##_linked_list)); \
	\
	if (result == NULL)\
		return NULL;\
	\
	result->data = value;\
	\
	return result;\
}\
\
void free_X##_linked_list(X##_linked_list *list)\
{\
	if (list == NULL)\
		return;\
	\
	X##_linked_list *tail = X##_linked_list_tail(list);\
	\
	while (tail != NULL) {\
		free(tail);\
		tail = X##_linked_list_tail(list);\
	}\
}\
\
X##_linked_list *X##_linked_list_tail(X##_linked_list *list)\
{\
	if (list == NULL)\
		return NULL;\
	\
	X##_linked_list *current = list;\
	\
	while (current->next != NULL)\
		current = current->next;\
	\
	return current;\
}\
\
int X##_linked_list_append(X##_linked_list *list, X value) \
{ \
	X##_linked_list *current = list; \
	\
	if (current == NULL) return 1;\
	\
	while (current->next != NULL) current = current->next;\
	\
	current->next = malloc(sizeof(X##_linked_list));\
	if (current->next == NULL) {\
		return 2;\
	} else {\
		current->next->next = NULL;\
		current->next->data = value;\
	}\
}\
\
int X##_linked_list_remove_next(X##_linked_list *list) \
{	\
	if (list == NULL)\
		return 1;\
	\
	if (list->next == NULL)\
		return 0;\
	\
	list->next = list->next->next;\
	\
	return 0;\
}

#define DECLARE_LINKED_PTR_LIST(X) struct X##_ptr_linked_list;\
typedef struct X##_ptr_linked_list {\
	X *data;\
	struct X##_ptr_linked_list *next;\
} X##_ptr_linked_list;\
\
X##_ptr_linked_list *X##_ptr_linked_list_new(X *value);\
void free_X##_ptr_linked_list(X##_ptr_linked_list *list);\
X##_ptr_linked_list *X##_ptr_linked_list_tail(X##_ptr_linked_list *list);\
int X##_ptr_linked_list_append(X##_ptr_linked_list *list, X *value);\
int X##_ptr_linked_list_remove_next(X##_ptr_linked_list *list);\
void X##_ptr_linked_list_free_all(X##_ptr_linked_list *list);

#define IMPLEMENT_LINKED_PTR_LIST(X)\
X##_ptr_linked_list *X##_ptr_linked_list_new(X *value) \
{\
	X##_ptr_linked_list *result = malloc(sizeof(X##_ptr_linked_list)); \
	\
	if (result == NULL)\
		return NULL;\
	\
	result->data = value;\
	\
	return result;\
}\
\
void free_X##_ptr_linked_list(X##_ptr_linked_list *list)\
{\
	if (list == NULL)\
		return;\
	\
	X##_ptr_linked_list *tail = X##_ptr_linked_list_tail(list);\
	\
	while (tail != NULL) {\
		free(tail);\
		tail = X##_ptr_linked_list_tail(list);\
	}\
}\
\
X##_ptr_linked_list *X##_ptr_linked_list_tail(X##_ptr_linked_list *list)\
{\
	if (list == NULL)\
		return NULL;\
	\
	X##_ptr_linked_list *current = list;\
	\
	while (current->next != NULL)\
		current = current->next;\
	\
	return current;\
}\
\
int X##_ptr_linked_list_append(X##_ptr_linked_list *list, X *value) \
{ \
	X##_ptr_linked_list *current = list; \
	\
	if (current == NULL) return 1;\
	\
	while (current->next != NULL) current = current->next;\
	\
	current->next = malloc(sizeof(X##_ptr_linked_list));\
	if (current->next == NULL) {\
		return 2;\
	} else {\
		current->next->next = NULL;\
		current->next->data = value;\
	}\
}\
\
int X##_ptr_linked_list_remove_next(X##_ptr_linked_list *list) \
{	\
	if (list == NULL)\
		return 1;\
	\
	if (list->next == NULL)\
		return 0;\
	\
	list->next = list->next->next;\
	\
	return 0;\
}\
\
void X##_ptr_linked_list_free_all(X##_ptr_linked_list *list) \
{\
	if (list == NULL)\
		return;\
	\
	X##_ptr_linked_list *tail = X##_ptr_linked_list_tail(list);\
	\
	while (tail != NULL) {\
		if (tail->data != NULL)\
			free(tail->data);\
		free(tail);\
		tail = X##_ptr_linked_list_tail(list);\
	}\
}

#endif
