#ifndef _DEFLIST_H9
#define _DEFLIST_H9

typedef struct {
int len, cap;
void** ptr;
} list;

#define l_item(l,n) (l->ptr[n])
#define l_len(l) (l->len)
#define l_add(l,x,p) (l=list_add(l,x,p))

list* list_add (list* l, void* x, int pos) ;

#endif
