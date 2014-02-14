#include "list.h"

list* list_add (list* l, void* p, int pos) {
if (!l) {
l = malloc(sizeof(list));
l->ptr = malloc(sizeof(void*) * 3);
l->ptr[0] = p;
l->cap = 3;
l->len = 1;
return l;
}
else if (l->len>=l->cap) {
int nc = l->cap *3/2+1;
l->ptr = realloc(l->ptr, sizeof(void*) * nc);
l->cap = nc;
}
if (pos<0) pos += l->len +1;
if (pos<l->len) memmove(&(l->ptr[pos+1]), &(l->ptr[pos]), sizeof(void*) * (l->len-pos));
l->ptr[pos] = p;
l->len++;
return l;
}
