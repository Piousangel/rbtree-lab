#include "rbtree.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>



void rotate_left(rbtree *t, node_t *temp_x);     //왼로테이션
void rotate_right(rbtree *t, node_t *temp_x);    //오른로테이션
void delete_node(node_t *node);                  //tree 비울시 호출
void transplant(rbtree* t, node_t* node1, node_t* node2);  //successor빠진거 이어주기위해서
node_t *findsuccessor(rbtree *t, node_t *node);            //find successor
node_t *node_or_nil(node_t *node);              
void del_fixup(rbtree *t, node_t * node);
int in_order(node_t *node, key_t *arr, int i);

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));    //calloc heap memory of rbree same as linkedlist  
  return p;
}


//tree, node->parent
void rotate_left(rbtree *t, node_t *temp_x){
  node_t *temp_y = temp_x->right;  //오른쪽 자식이랑 부모랑 left rotation돌릴려고
  temp_x -> right = temp_y -> left; //자식의 왼쪽 자손이 부모의 오른쪽자식이 되야하므로
  
  if(temp_y -> left != NULL){
    temp_y -> left -> parent = temp_x; //양방관계로 걸어줘야함 (temp_y의 왼쪽 부가 temp_x가 되어야하므로)
  }

  temp_y -> parent = temp_x -> parent; // y를 x의 부모와 연결

  if(temp_x-> parent == NULL){         // x의 부모가 없다(x가 루트)
    t->root = temp_y;   //트리의 루트는 temp_y
  }
  else if(temp_x -> parent -> left == temp_x ){  //temp_x가 조부모의 왼쪽자식일 경우
    temp_x -> parent -> left = temp_y;  //조부모의 왼쪽 자식 자리에 로테이션한 값이 자식으로 들어가고
  }
  else{  // temp_x가 조부의 오른쪽 자식일 경우에
    temp_x -> parent -> right = temp_y;   //조부모의 오른쪽 자식 자리에 로테이션한 값이 자식으로 들어가고
  }

  temp_y -> left = temp_x;    //위치 바꾸고, 부모과 자식간의 포인터 바꾸기
  temp_x -> parent = temp_y;  //양방향으로 걸어줘야 하므로 여기도 바꾸기

}

void rotate_right(rbtree *t, node_t *temp_x){
  //왼쪽 자식이랑 부모랑 right rotation돌릴려고
  node_t *temp_y = temp_x -> left;

  temp_x -> left = temp_y -> right;

  if(temp_y -> right != NULL){
    temp_y -> right -> parent = temp_x;
  }

  temp_y -> parent = temp_x -> parent;

  if(temp_x -> parent == NULL){
    t -> root = temp_y;
  }
  else if(temp_x -> parent -> left == temp_x){
    temp_x -> parent -> left = temp_y;
  }
  else{
    temp_x -> parent -> right = temp_y;
  }

  temp_y -> right = temp_x;
  temp_x -> parent = temp_y;

}

node_t nd = {
  .color = RBTREE_BLACK,
  .left = NULL, 
  .right = NULL, 
  .parent = NULL, 
};
node_t *NIL = &nd;

void delete_node(node_t *node) {
  if (node == NULL){
    return;
  }
  delete_node(node->left);
  delete_node(node->right);
  free(node);
}

void delete_rbtree(rbtree *t) {

  delete_node(t->root);
  free(t);
}


node_t *rbtree_insert(rbtree *t, const key_t key) {   //t -> insert node, key = value
  
  node_t *node = malloc(sizeof(node_t));   //following x = new node
  // node_t *temp = t -> root;   // save root info
  // node_t *p = t -> nil;       // save nil 

  node -> left = NULL;  //left, right NULL
  node -> right = NULL; 
  node -> parent = NULL; //부모 NULL
  node -> color = RBTREE_RED;  //maybe red
  node -> key = key;  //save key

  if(t->root == NULL){  //아무것도 없는 빈트리이면
    t -> root = node;
    node -> color = RBTREE_BLACK;  //다시 블랙으로
    return t-> root;  //node가 루트가 되어 반환
  }

  else{          //빈트리가 아니면
    node_t *prev = t->root; //t의 루트값을 저장
    int flag = 1;

    while(flag){
      if(prev -> key > key){  //부모value가 더 크면 left
        if(prev -> left != NULL){  //그리고 비어있지 않다면
          prev = prev -> left; //왼쪽자식
        }
        else{  //부모value가 더크지만 왼쪽이 비어있으면 즉, 비교 하고자 하는 대상이 없으면
          prev -> left = node;
          node -> parent = prev;
          node -> color = RBTREE_RED;
          break;
        }
      }
      else{      //부모value보다 자식의value가 더 크다면?
        if(prev -> right != NULL){
          prev = prev -> right;
        }
        else{    //부모value가 더작지만 오른쪽이 비어있으면 즉, 비교 하고자 하는 대상이 없으면
          prev -> right = node;
          node -> parent = prev;
          node -> color = RBTREE_RED;
          break;
        }
      }
    }
  }

  //부모가 있을 경우, 부모의 색이 레드일때 조건에 위반된다.
  //총 6가지case -> 왼쪽 3가지를 구해 반대로만 바꿔주면 됨
  while(node -> parent != NULL && node->parent->color == RBTREE_RED){
      
      //왼쪽 case(1,2,3)
      if(node -> parent == node -> parent -> parent -> left){
        node_t *uncle = node -> parent -> parent -> right;

        //부모노드와 삼촌노드가 빨간색인경우,
        //부모, 삼촌 노드를 검은색으로 바꾼다
        //그렇게 되면 5번규칙에 위배 -> 조부모의 색을 빨간색으로 바꾼다
        //case1 -> case2 or case3(더 위의 가지에서 일어나는 일)
        if(uncle != NULL && uncle -> color == RBTREE_RED){
          node -> parent -> color = RBTREE_BLACK; //부모
          uncle -> color = RBTREE_BLACK;   //삼촌
          uncle -> parent -> color = RBTREE_RED; //할아버지
          node = uncle -> parent;
        }

        //case 2 or 3 
        //부모노드 = 빨간색, 삼촌노드 = 검은색 (한 트리라고 가정하면 위에 가지)
        else{ //if(uncle != NULL && uncle -> color == RBTREE_BLACK){
          
          //case 2 현재노드가 오른쪽자식일 경우 -> left rotation을 한다(위치, 자손 변경)
          if(node -> parent -> right == node){
            node = node -> parent;
            rotate_left(t, node);
          }
          //case 3 현재노드가 왼쪽자식일 경우 -> right rotation을 한다(위치, 자손 변경)
            node -> parent -> parent -> color = RBTREE_RED;
            node -> parent -> color = RBTREE_BLACK;
            rotate_right(t, node->parent->parent); //트리와 노드의 할아버지를 넘겨
          }
        }
      
      //4,5,6 -> 오른쪽 3가지
      else{
        node_t *uncle = node -> parent -> parent -> left; //부모가 오른쪽, 삼촌이 왼쪽자식
        
        //case 4 부모 , 삼촌의 색이 다 빨간색일경우
        if(uncle != NULL && uncle -> color == RBTREE_RED){
          node -> parent -> color = RBTREE_BLACK;
          uncle -> color = RBTREE_BLACK;
          uncle -> parent -> color = RBTREE_RED;
          node = uncle -> parent;  //조부모가 레드니까 더 윗가지에서 문제가 발생 -> 1 or 2 or 3
        }
        //case 4 -> case5 or case6
        // if(uncle != NULL && uncle -> color == RBTREE_BLACK){
        else{
          //case 5 -> node_t가 부모의 왼쪽 자식일경우
          if(node -> parent -> left == node){
            node = node -> parent;
            rotate_right(t, node);
          }
          //case 6 -> node_t가 부모의 오른쪽 자식일때
          // else if(node -> parent -> right == node){
          node -> parent -> color = RBTREE_BLACK;
          node -> parent -> parent -> color = RBTREE_RED;
          rotate_left(t, node-> parent -> parent);
          // }
        }
      }
  }
  
  //tree의 root를 black으로
  t->root->color = RBTREE_BLACK;
  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  
  node_t *node = t -> root;

  while (node != NULL && key != node->key) {
    if (key < node->key) {
      node = node->left;
    } else {
      node = node->right;
    }
  }
  return node;

}

node_t *rbtree_min(const rbtree *t) {
  
  node_t *node = t->root;
  if(node == NULL){
    return t->root;
  }

  while(node->left != NULL){
    node = node -> left;
  }

  return node;
}

node_t *rbtree_max(const rbtree *t) {
  
  node_t *node = t -> root;

  if(node == NULL){
    return t -> root;
  }
  while(node -> right != NULL){
    node = node -> right;
  }

  return node;
}

 void transplant(rbtree* t, node_t *node1, node_t *node2){

   if (node1 -> parent == NULL){
     t -> root = node2;
   }
   else if(node1 == node1 -> parent -> left){
     node1 -> parent -> left = node2;
   }
   else{
     node1 -> parent -> right = node2;
   }

   if(node2 != NULL){
     node2 -> parent = node1 -> parent;
   }
 }

//노드를 삭제하기 위해 위의 find를 한 이후
node_t *findsuccessor(rbtree *t, node_t *node){ 

  // node_t *y = p -> left;  //삭제할 노드의 왼쪽 자식
  // node_t *parent = NULL;
  
  if(node-> right != NULL){  //왼쪽 자식의 오른쪽의 제일 오른쪽을 찾으려고
    node_t* child = node->right;
    while(child -> left != NULL){
      child = child -> left;
    }
    return child;
  }

  node_t *y = node -> parent;
  while(y != NULL && node == y -> right){
    node = y;
    y = y -> parent;
  }
  return y;
}

//노드가 없는 경우 -> 더미드 NIL을 리턴
node_t *node_or_nil(node_t *node){
  if(node == NULL){
    return NIL;
  }
  else{
    return node;
  }
}

void del_fixup(rbtree *t, node_t *node){
  node_t *sibling, *sibling_left, *sibling_right;

  while(node != t -> root && node -> color == RBTREE_BLACK){
    if(node == node->parent->left){
      sibling = node->parent->right;

      if(sibling -> color == RBTREE_RED){
        sibling -> color = RBTREE_BLACK;
        node->parent->color = RBTREE_RED;
        rotate_left(t, node->parent);
        sibling = node->parent->right;
      }

      sibling_left = node_or_nil(sibling -> left);
      sibling_right = node_or_nil(sibling -> right);

      if(sibling_left->color == RBTREE_BLACK && sibling_right -> color == RBTREE_BLACK){
        sibling -> color = RBTREE_RED;
        node = node->parent;
      }
      else{

        if(sibling_right->color == RBTREE_BLACK){
          sibling_left -> color = RBTREE_BLACK;
          sibling -> color = RBTREE_RED;
          rotate_right(t, sibling);
          sibling = node -> parent ->right;
        }

        sibling -> color = node -> parent -> color;
        node->parent->color = RBTREE_BLACK;
        sibling->right->color = RBTREE_BLACK;
        rotate_left(t, node -> parent);
        node = t -> root;
      }
    }
    else{

      sibling = node -> parent -> left;
      if(sibling->color == RBTREE_RED){
        sibling->color = RBTREE_BLACK;
        node->parent->color = RBTREE_RED;
        rotate_right(t, node->parent);
        sibling = node->parent->left;
      }
      //case2
      sibling_left = node_or_nil(sibling->left);
      sibling_right = node_or_nil(sibling->right);
      if(sibling_left->color == RBTREE_BLACK && sibling_right->color == RBTREE_BLACK){
        sibling -> color = RBTREE_RED;
        node = node -> parent;
      }
      else{
        //case3
        if(sibling_left->color == RBTREE_BLACK){
          sibling->right->color == RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          rotate_left(t, sibling);
          sibling = node->parent->left;
        }
        //case4
        sibling->color = node->parent->color;
        node->parent->color = RBTREE_BLACK;
        sibling->left->color = RBTREE_BLACK;
        rotate_right(t, node->parent);
        node = t->root;
      }
    }
  }
  node->color = RBTREE_BLACK;
}


int rbtree_erase(rbtree *t, node_t *p) {
  
  //삭제할 노드 y
  node_t *y = p;
  color_t past_color = y -> color;

  // 삭제될 노의 왼자식이 없으면 -> 오른자식으로 채움
  node_t *x;
  if(p -> left == NULL){

    x = node_or_nil(p->right);
    transplant(t, p, x);
    free(p);
  }
  //case 2 제삭제될 노드의 오른쪽 자식이 없는경우
  else if (p -> right == NULL){
    x = node_or_nil(p -> left);
    transplant(t,p,x);
    free(p);
  }
  //case 3
  else{
    y = findsuccessor(t, p);
    past_color = y -> color;
    x = node_or_nil(y -> right);
    if(y -> parent == p){
      x -> parent = y;
    }
    //y가 p의 오른쪽자식이 아닌경우
    else{
      transplant(t, y, x);
      y -> right = p -> right;
      y -> right -> parent = y;
    }

    transplant(t, p, y);
    
    y -> left = p -> left;
    y -> left -> parent = y;
    y -> color = p -> color;
    free(p);
  } 
  
  if(past_color == RBTREE_BLACK){
    del_fixup(t, x);
  }

  if(t -> root == NIL){
    t -> root = NULL;
  }
  else if(NIL -> parent != NULL){

    if(NIL -> parent -> left == NIL){
      NIL -> parent -> left = NULL;
    }
    else{
      NIL -> parent -> right = NULL;
    }

    NIL -> parent = NULL;
  }

  return 1;
}

int in_order(node_t *node, key_t *arr, int i){
 
  if(node != NULL){
    i = in_order(node->left, arr, i);
    arr[i] = node->key;
    i++;
    i = in_order(node->right, arr, i);
  }
  return i;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  int k;
  k = in_order(t->root, arr, 0);
  return 0;
}




// new_rbtree should return rbtree struct with null root node
void test_init(void) {
  rbtree *t = new_rbtree();
  assert(t != NULL);
#ifdef SENTINEL
  assert(t->nil != NULL);
  assert(t->root == t->nil);
#else
  assert(t->root == NULL);
#endif
  delete_rbtree(t);
}

// root node should have proper values and pointers
void test_insert_single(const key_t key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);
  // assert(p->color == RBTREE_BLACK);  // color of root node should be black
#ifdef SENTINEL
  assert(p->left == t->nil);
  assert(p->right == t->nil);
  assert(p->parent == t->nil);
#else
  assert(p->left == NULL);
  assert(p->right == NULL);
  assert(p->parent == NULL);
#endif
  delete_rbtree(t);
}

// find should return the node with the key or NULL if no such node exists
void test_find_single(const key_t key, const key_t wrong_key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);

  node_t *q = rbtree_find(t, key);
  assert(q != NULL);
  assert(q->key == key);
  assert(q == p);

  q = rbtree_find(t, wrong_key);
  assert(q == NULL);

  delete_rbtree(t);
}

// erase should delete root node
void test_erase_root(const key_t key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);

  rbtree_erase(t, p);
#ifdef SENTINEL
  assert(t->root == t->nil);
#else
  assert(t->root == NULL);
#endif

  delete_rbtree(t);
}

static void insert_arr(rbtree *t, const key_t *arr, const size_t n) {
  for (size_t i = 0; i < n; i++) {
    rbtree_insert(t, arr[i]);
  }
}

static int comp(const void *p1, const void *p2) {
  const key_t *e1 = (const key_t *)p1;
  const key_t *e2 = (const key_t *)p2;
  if (*e1 < *e2) {
    return -1;
  } else if (*e1 > *e2) {
    return 1;
  } else {
    return 0;
  }
};

// min/max should return the min/max value of the tree
void test_minmax(key_t *arr, const size_t n) {
  // null array is not allowed
  assert(n > 0 && arr != NULL);

  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);
#ifdef SENTINEL
  assert(t->root != t->nil);
#endif

  qsort((void *)arr, n, sizeof(key_t), comp);
  node_t *p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[0]);

  node_t *q = rbtree_max(t);
  assert(q != NULL);
  assert(q->key == arr[n - 1]);

  rbtree_erase(t, p);
  p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[1]);

  if (n >= 2) {
    rbtree_erase(t, q);
    q = rbtree_max(t);
    assert(q != NULL);
    assert(q->key == arr[n - 2]);
  }

  delete_rbtree(t);
}

void test_to_array(rbtree *t, const key_t *arr, const size_t n) {
  assert(t != NULL);

  insert_arr(t, arr, n);
  qsort((void *)arr, n, sizeof(key_t), comp);

  key_t *res = calloc(n, sizeof(key_t));
  rbtree_to_array(t, res, n);
  for (int i = 0; i < n; i++) {
    assert(arr[i] == res[i]);
  }
  free(res);
}

void test_multi_instance() {
  rbtree *t1 = new_rbtree();
  assert(t1 != NULL);
  rbtree *t2 = new_rbtree();
  assert(t2 != NULL);

  key_t arr1[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
  insert_arr(t1, arr1, n1);
  qsort((void *)arr1, n1, sizeof(key_t), comp);

  key_t arr2[] = {4, 8, 10, 5, 3};
  const size_t n2 = sizeof(arr2) / sizeof(arr2[0]);
  insert_arr(t2, arr2, n2);
  qsort((void *)arr2, n2, sizeof(key_t), comp);

  key_t *res1 = calloc(n1, sizeof(key_t));
  rbtree_to_array(t1, res1, n1);
  for (int i = 0; i < n1; i++) {
    assert(arr1[i] == res1[i]);
  }

  key_t *res2 = calloc(n2, sizeof(key_t));
  rbtree_to_array(t2, res2, n2);
  for (int i = 0; i < n2; i++) {
    assert(arr2[i] == res2[i]);
  }

  free(res2);
  free(res1);
  delete_rbtree(t2);
  delete_rbtree(t1);
}


static bool search_traverse(const node_t *p, key_t *min, key_t *max,
                            node_t *nil) {
  if (p == nil) {
    return true;
  }

  *min = *max = p->key;

  key_t l_min, l_max, r_min, r_max;
  l_min = l_max = r_min = r_max = p->key;

  const bool lr = search_traverse(p->left, &l_min, &l_max, nil);
  if (!lr || l_max > p->key) {
    return false;
  }
  const bool rr = search_traverse(p->right, &r_min, &r_max, nil);
  if (!rr || r_min < p->key) {
    return false;
  }

  *min = l_min;
  *max = r_max;
  return true;
}

void test_search_constraint(const rbtree *t) {
  assert(t != NULL);
  node_t *p = t->root;
  key_t min, max;
#ifdef SENTINEL
  node_t *nil = t->nil;
#else
  node_t *nil = NULL;
#endif
  assert(search_traverse(p, &min, &max, nil));
}


bool touch_nil = false;
int max_black_depth = 0;

static void init_color_traverse(void) {
  touch_nil = false;
  max_black_depth = 0;
}

static bool color_traverse(const node_t *p, const color_t parent_color,
                           const int black_depth, node_t *nil) {
  if (p == nil) {
    if (!touch_nil) {
      touch_nil = true;
      max_black_depth = black_depth;
    } else if (black_depth != max_black_depth) {
      return false;
    }
    return true;
  }
  if (parent_color == RBTREE_RED && p->color == RBTREE_RED) {
    return false;
  }
  int next_depth = ((p->color == RBTREE_BLACK) ? 1 : 0) + black_depth;
  return color_traverse(p->left, p->color, next_depth, nil) &&
         color_traverse(p->right, p->color, next_depth, nil);
}

void test_color_constraint(const rbtree *t) {
  assert(t != NULL);
#ifdef SENTINEL
  node_t *nil = t->nil;
#else
  node_t *nil = NULL;
#endif
  node_t *p = t->root;
  assert(p == nil || p->color == RBTREE_BLACK);

  init_color_traverse();
  assert(color_traverse(p, RBTREE_BLACK, 0, nil));
}

// rbtree should keep search tree and color constraints
void test_rb_constraints(const key_t arr[], const size_t n) {
  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);

  test_color_constraint(t);
  test_search_constraint(t);

  delete_rbtree(t);
}

// rbtree should manage distinct values
void test_distinct_values() {
  const key_t entries[] = {10, 5, -8, 34, -67, 23, -156, 24, 2, 12, 0, };
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

// rbtree should manage values with duplicate
void test_duplicate_values() {
  const key_t entries[] = {10, 5, 5, 34, -6, 23, 12, 12, -6, 12, 10, 10, 6};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

void test_minmax_suite() {
  key_t entries[] = {10, 5, -8, 34, 67, 0, -23, 156, 24, 2, -12, 26, 35};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_minmax(entries, n);
}

void test_to_array_suite() {
  rbtree *t = new_rbtree();
  assert(t != NULL);

  key_t entries[] = {0, -5, 8, 34, 67, -22, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_to_array(t, entries, n);

  delete_rbtree(t);
}

void test_find_erase(rbtree *t, const key_t *arr, const size_t n) {
  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    // printf("arr[%d] = %d\n", i, arr[i]);
    assert(p != NULL);
    assert(p->key == arr[i]);
    rbtree_erase(t, p);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    assert(p == NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
    node_t *q = rbtree_find(t, arr[i]);
    assert(q != NULL);
    assert(q->key == arr[i]);
    assert(p == q);
    rbtree_erase(t, p);
    q = rbtree_find(t, arr[i]);
    assert(q == NULL);
  }
}

void test_find_erase_fixed() {
  const key_t arr[] = {10, -5, 8, 34, 67, 33, 156, -24, 2, 12, 24, 36, 990, 25, 127, -77, 0};
  const size_t n = sizeof(arr) / sizeof(arr[0]);
  rbtree *t = new_rbtree();
  assert(t != NULL);

  test_find_erase(t, arr, n);

  delete_rbtree(t);
}

void test_find_erase_rand(const size_t n, const unsigned int seed) {
  srand(seed);
  rbtree *t = new_rbtree();
  key_t *arr = calloc(n, sizeof(key_t));
  for (int i = 0; i < n; i++) {
    arr[i] = rand();
  }

  test_find_erase(t, arr, n);

  free(arr);
  delete_rbtree(t);
}

int main(void) {
  test_init();
  test_insert_single(0);
  test_find_single(512, -512);
  test_erase_root(-1);
  test_find_erase_fixed();
  test_minmax_suite();
  test_to_array_suite();
  test_distinct_values();
  test_duplicate_values();
  test_multi_instance();
  test_find_erase_rand(1000000, 55);
  printf("Passed all tests!\n");
}