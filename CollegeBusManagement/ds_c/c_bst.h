#ifndef C_BST_H
#define C_BST_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CBSTNode {
    char* student_id;
    char* student_name;
    char* department;
    int semester;
    char* bus_number;
    int seat_number;
    struct CBSTNode* left;
    struct CBSTNode* right;
} CBSTNode;

typedef struct {
    CBSTNode* root;
    size_t count;
} CBinarySearchTree;

typedef void (*CBSTVisitor)(const CBSTNode* node, void* user_data);

CBinarySearchTree* c_bst_create(void);
DS_Status c_bst_insert(CBinarySearchTree* tree, const char* id, const char* name, const char* dept, int sem, const char* bus, int seat);
const CBSTNode* c_bst_search(const CBinarySearchTree* tree, const char* id);
DS_Status c_bst_delete(CBinarySearchTree* tree, const char* id);
const CBSTNode* c_bst_min(const CBinarySearchTree* tree);
const CBSTNode* c_bst_max(const CBinarySearchTree* tree);

void c_bst_inorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data);
void c_bst_preorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data);
void c_bst_postorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data);

size_t c_bst_size(const CBinarySearchTree* tree);
void c_bst_print(const CBinarySearchTree* tree);
void c_bst_destroy(CBinarySearchTree* tree);

#ifdef __cplusplus
}
#endif

#endif /* C_BST_H */
