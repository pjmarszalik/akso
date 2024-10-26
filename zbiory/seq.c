#include "seq.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include<malloc.h>

typedef struct node{
	struct node *left, *middle, *right;
	struct node *next, *prev;
	char *name;
} node;

typedef struct seq{
	node *root;
} seq_t;

node* create_node(){
	node *w = (node *)malloc(sizeof(node));
	if(!w)
		return w;
	w -> right = NULL;
	w -> left = NULL;
	w -> middle = NULL;
	w -> next = w;
	w -> prev = w;
	w -> name = (char *)malloc(sizeof(char));
	if(!(w -> name)){
		return w;
	}
	w -> name[0] = '\0';
	return w;
}

seq_t *      seq_new(void){
	seq_t *s = (seq_t *)malloc(sizeof(seq_t));
	if(!s){
		errno = ENOMEM;
		return NULL;
	}
	(s -> root) = create_node();
	if(!(s -> root)){
		errno = ENOMEM;
		free(s);
		return NULL;
	}
	if(!(s -> root -> name)){
		errno = ENOMEM;
		free(s -> root);
		free(s);
		return NULL;
	}
	return s;
} 

void node_delete(node *w){
	if(w -> left)
		node_delete(w -> left);
	if(w -> middle)
		node_delete(w -> middle);
	if(w -> right)
		node_delete(w -> right);
	node *p = w -> next;
	node *q = w -> prev;
	if(p == w){
		if(w -> name){
			free(w -> name);
			w -> name = NULL;
		}
	}
	else{
		p -> prev = q;
		q -> next = p;
	}
	free(w);
	w = NULL;
}

void         seq_delete(seq_t *p){
	if(!p)
		return;
	node_delete(p -> root);
	free(p);
	p = NULL;
}

int max(int a, int b){
	if(a >= b)
		return a;
	return b;
}

void node_add(node *w, int p, int len, char const *s, int *x, int *first){
	if(p == len)
		return;
	node *v = NULL;
	if(s[p] == '0')
		v = w -> left;
	else if(s[p] == '1')
		v = w -> middle;
	else if(s[p] == '2')
		v = w -> right;
	
	if(!v){
		v = create_node();
		if(!v){
			(*x) = 1;
			return;
		}
		else if(!(v -> name)){
			if(s[p] == '0')
				w -> left = v;
			else if(s[p] == '1')
				w -> middle = v;
			else if(s[p] == '2')
				w -> right = v;
			(*x) = 1;
			if(*first == 0)
				*first = p + 1;
			return;
		}
		else if(*first == 0)
			*first = p + 1;
		
		if(s[p] == '0')
			w -> left = v;
		else if(s[p] == '1')
			w -> middle = v;
		else if(s[p] == '2')
			w -> right = v;
	}
	node_add(v, p + 1, len, s, x, first);
}

int empty(char const * s){
	if(!s)
		return 1;
	if(s[0] == '\0')
		return 1;
	else
		return 0;
}

int correct_seq(char const *s){
	if(empty(s))
		return 0;
	for(int i = 0; i < (int)strlen(s); i ++)
		if(s[i] != '0' && s[i] != '1' && s[i] != '2')
			return 0;
	return 1;
}

node* find(node *w, int p, int len, char const *s){
	if(p == len)
		return w;
	node *v = NULL;
	if(s[p] == '0')
		v = w -> left;
	else if(s[p] == '1')
		v = w -> middle;
	else if(s[p] == '2')
		v = w -> right;
	if(!v)
		return NULL;
	return find(v, p + 1, len, s);
}

int          seq_add(seq_t *p, char const *s){
	if(!s){
		errno = EINVAL;
		return -1;
	}
	if(!p || !correct_seq(s)){
		errno = EINVAL;
		return -1;
	}
	int d = (int)strlen(s);

	node *r = find(p -> root, 0, d, s);
	if(r != NULL)
		return 0;

	int *h = (int *)malloc(sizeof(int));
	if(!h){
		errno = ENOMEM;
		return -1;
	}
	*h = 0;
	int *fi = (int *)malloc(sizeof(int));
	if(!fi){
		errno = ENOMEM;
		free(h);
		return -1;
	}
	*fi = 0;
	node_add(p -> root, 0, d, s, h, fi);

	if(*h == 0){
		free(h);
		free(fi);
		return 1;
	}
	else{
		if(*fi != 0){
			node *r = find(p -> root, 0, *fi, s);
			node_delete(r);
			node *v = find(p -> root, 0, (*fi) - 1, s);
			if(s[(*fi) - 1] == '0')
				v -> left = NULL;
			else if(s[(*fi) - 1] == '1')
				v -> middle = NULL;
			else
				v -> right = NULL;
		}
		free(fi);
		free(h);
		errno = ENOMEM;
		return -1;
	}
}

int          seq_remove(seq_t *p, char const *s){
	if(!s){
		errno = EINVAL;
		return -1;
	}
	if(!p || !correct_seq(s)){
		errno = EINVAL;
		return -1;
	}
	int d = strlen(s);

	node *w = find(p -> root, 0, d, s);
	if(!w)
		return 0;
	node_delete(w);
	node *v = find(p -> root, 0, d - 1, s);
	if(s[d - 1] == '0')
		v -> left = NULL;
	else if(s[d - 1] == '1')
		v -> middle = NULL;
	else
		v -> right = NULL;
	return 1;
}
int          seq_valid(seq_t *p, char const *s){
	if(!s){
		errno = EINVAL;
		return -1;
	}
	if(!p || !correct_seq(s)){
		errno = EINVAL;
		return -1;
	}
	int d = strlen(s);
	node *w = find(p -> root, 0, d, s);
	if(!w)
		return 0;
	else
		return 1;
}


void change_name(node *w, char *n){
	node *h = w;
	do{
		h -> name = n;
		h = h -> next;
	}while(w != h);
}

int          seq_set_name(seq_t *p, char const *s, char const *n){
	if(!s || !n){
		errno = EINVAL;
		return -1;
	}
	if(!p || !correct_seq(s) || empty(n)){
		errno = EINVAL;
		return -1;
	}
	int d = strlen(s);
	int q = strlen(n);
	node *w = find(p -> root, 0, d, s);
	if(!w)
		return 0;
	char *v = w -> name;
	if(empty(v) || strcmp(n, v)){
		char *h = (char *)malloc(sizeof(char) * (q + 1));
		if(!h){
			errno = ENOMEM;
			return -1;
		}

		h = strcpy(h, n);
		free(w -> name);
		w -> name = NULL;
		change_name(w, h);
		return 1;
	}
	else
		return 0;
}
char const * seq_get_name(seq_t *p, char const *s){
	if(!s){
		errno = EINVAL;
		return NULL;	
	}
	if(!p || !correct_seq(s)){
		errno = EINVAL;
		return NULL;
	}
	int d = strlen(s);
	node *w = find(p -> root, 0, d, s);
	if(!w){
		errno = 0;
		return NULL;
	}
	if(empty(w -> name)){
		errno = 0;
		return NULL;
	}
	return w -> name;
}

void join(node *w, node *v){
	node *h  = v -> next;
	v -> next = w;
	(w -> prev) -> next = h;
	h -> prev = w -> prev;
	w -> prev = v;
}

int          seq_equiv(seq_t *p, char const *s1, char const *s2){
	if(!s1){
		errno = EINVAL;
		return -1;
	}
	if(!s2){
		errno = EINVAL;
		return -1;
	}
	if(!p || !correct_seq(s1) || !correct_seq(s2)){
		errno = EINVAL;
		return -1;
	}	
	int d1 = strlen(s1);
	int d2 = strlen(s2);
	node *w1 = find(p -> root, 0, d1, s1);
	node *w2 = find(p -> root, 0, d2, s2);
	if(!w1 || !w2)
		return 0;
	char *n;
	char *t1 = w1 -> name;
	char *t2 = w2 -> name;
	if(t1 == t2)
		return 0;
	else if(empty(t1)){
		free(t1);
		t1 = NULL;
		n = t2;
	}
	else if(empty(t2)){
		free(t2);
		t2 = NULL;
		n = t1;
	}
	else if(!strcmp(t1, t2)){
		free(t2);
		t2 = NULL;
		n = t1;
	}
	else{
		d1 = (int)strlen(t1);
		d2 = (int)strlen(t2);
		n = (char *)malloc(sizeof(char) * (1 + d1 + d2));
		if(!n){
			errno = ENOMEM;
			return -1;
		}

		for(int i = 0; i < d1; i ++)
			n[i] = t1[i];

		for(int i = 0;i < d2; i ++)
			n[i + d1] = t2[i];

		n[d1 + d2] = '\0';
		free(t1);
		free(t2);
		t1 = t2 = NULL;
	}
	
	
	join(w1, w2);
	change_name(w1, n);
	return 1;
}