#include <stdlib.h>
#include "ast.h"

Node *mknode(NodeType t) {
    Node *n = calloc(1, sizeof(Node));
    n->type = t;
    return n;
}

Node *append_list(Node *list, Node *item) {
    if (!list) return item;
    Node *p = list;
    while (p->next) p = p->next;
    p->next = item;
    return list;
}
