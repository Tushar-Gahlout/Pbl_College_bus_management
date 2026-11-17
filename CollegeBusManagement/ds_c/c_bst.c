#include "c_bst.h"

CBinarySearchTree* c_bst_create(void) {
    CBinarySearchTree* tree = (CBinarySearchTree*)malloc(sizeof(CBinarySearchTree));
    if (!tree) return NULL;
    tree->root = NULL;
    tree->count = 0;
    return tree;
}

static CBSTNode* c_bst_create_node(const char* id, const char* name, const char* dept, int sem, const char* bus, int seat) {
    CBSTNode* node = (CBSTNode*)malloc(sizeof(CBSTNode));
    if (!node) return NULL;

    node->student_id = ds_strdup(id);
    node->student_name = ds_strdup(name);
    node->department = dept ? ds_strdup(dept) : ds_strdup("General");
    node->semester = sem;
    node->bus_number = bus ? ds_strdup(bus) : ds_strdup("None");
    node->seat_number = seat;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void c_bst_free_node(CBSTNode* node) {
    if (!node) return;
    free(node->student_id);
    free(node->student_name);
    free(node->department);
    free(node->bus_number);
    free(node);
}

static CBSTNode* c_bst_insert_rec(CBSTNode* root, CBSTNode* new_node, DS_Status* status) {
    if (!root) {
        *status = DS_SUCCESS;
        return new_node;
    }

    int cmp = strcmp(new_node->student_id, root->student_id);
    if (cmp < 0) {
        root->left = c_bst_insert_rec(root->left, new_node, status);
    } else if (cmp > 0) {
        root->right = c_bst_insert_rec(root->right, new_node, status);
    } else {
        *status = DS_ERR_DUPLICATE;
        c_bst_free_node(new_node);
    }
    return root;
}

DS_Status c_bst_insert(CBinarySearchTree* tree, const char* id, const char* name, const char* dept, int sem, const char* bus, int seat) {
    if (!tree || !id || !name) return DS_ERR_NULL_PTR;

    CBSTNode* new_node = c_bst_create_node(id, name, dept, sem, bus, seat);
    if (!new_node) return DS_ERR_ALLOC_FAILED;

    DS_Status status = DS_SUCCESS;
    tree->root = c_bst_insert_rec(tree->root, new_node, &status);
    if (status == DS_SUCCESS) {
        tree->count++;
    }
    return status;
}

const CBSTNode* c_bst_search(const CBinarySearchTree* tree, const char* id) {
    if (!tree || !id) return NULL;

    CBSTNode* curr = tree->root;
    while (curr) {
        int cmp = strcmp(id, curr->student_id);
        if (cmp == 0) return curr;
        curr = (cmp < 0) ? curr->left : curr->right;
    }
    return NULL;
}

static CBSTNode* c_bst_find_min_node(CBSTNode* node) {
    CBSTNode* curr = node;
    while (curr && curr->left) {
        curr = curr->left;
    }
    return curr;
}

static CBSTNode* c_bst_delete_rec(CBSTNode* root, const char* id, DS_Status* status) {
    if (!root) {
        *status = DS_ERR_NOT_FOUND;
        return NULL;
    }

    int cmp = strcmp(id, root->student_id);
    if (cmp < 0) {
        root->left = c_bst_delete_rec(root->left, id, status);
    } else if (cmp > 0) {
        root->right = c_bst_delete_rec(root->right, id, status);
    } else {
        *status = DS_SUCCESS;

        /* Case 1: Leaf node (no children) */
        if (!root->left && !root->right) {
            c_bst_free_node(root);
            return NULL;
        }

        /* Case 2: One child */
        if (!root->left) {
            CBSTNode* temp = root->right;
            c_bst_free_node(root);
            return temp;
        } else if (!root->right) {
            CBSTNode* temp = root->left;
            c_bst_free_node(root);
            return temp;
        }

        /* Case 3: Two children - find inorder successor (min in right subtree) */
        CBSTNode* successor = c_bst_find_min_node(root->right);

        /* Copy successor data to root */
        free(root->student_id);
        free(root->student_name);
        free(root->department);
        free(root->bus_number);

        root->student_id = ds_strdup(successor->student_id);
        root->student_name = ds_strdup(successor->student_name);
        root->department = ds_strdup(successor->department);
        root->semester = successor->semester;
        root->bus_number = ds_strdup(successor->bus_number);
        root->seat_number = successor->seat_number;

        /* Delete the inorder successor from right subtree */
        root->right = c_bst_delete_rec(root->right, successor->student_id, status);
    }
    return root;
}

DS_Status c_bst_delete(CBinarySearchTree* tree, const char* id) {
    if (!tree || !id) return DS_ERR_NULL_PTR;

    DS_Status status = DS_ERR_NOT_FOUND;
    tree->root = c_bst_delete_rec(tree->root, id, &status);
    if (status == DS_SUCCESS) {
        tree->count--;
    }
    return status;
}

const CBSTNode* c_bst_min(const CBinarySearchTree* tree) {
    if (!tree || !tree->root) return NULL;
    return c_bst_find_min_node(tree->root);
}

const CBSTNode* c_bst_max(const CBinarySearchTree* tree) {
    if (!tree || !tree->root) return NULL;
    CBSTNode* curr = tree->root;
    while (curr && curr->right) {
        curr = curr->right;
    }
    return curr;
}

static void c_bst_inorder_rec(const CBSTNode* node, CBSTVisitor visitor, void* user_data) {
    if (!node) return;
    c_bst_inorder_rec(node->left, visitor, user_data);
    if (visitor) visitor(node, user_data);
    c_bst_inorder_rec(node->right, visitor, user_data);
}

void c_bst_inorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data) {
    if (!tree) return;
    c_bst_inorder_rec(tree->root, visitor, user_data);
}

static void c_bst_preorder_rec(const CBSTNode* node, CBSTVisitor visitor, void* user_data) {
    if (!node) return;
    if (visitor) visitor(node, user_data);
    c_bst_preorder_rec(node->left, visitor, user_data);
    c_bst_preorder_rec(node->right, visitor, user_data);
}

void c_bst_preorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data) {
    if (!tree) return;
    c_bst_preorder_rec(tree->root, visitor, user_data);
}

static void c_bst_postorder_rec(const CBSTNode* node, CBSTVisitor visitor, void* user_data) {
    if (!node) return;
    c_bst_postorder_rec(node->left, visitor, user_data);
    c_bst_postorder_rec(node->right, visitor, user_data);
    if (visitor) visitor(node, user_data);
}

void c_bst_postorder(const CBinarySearchTree* tree, CBSTVisitor visitor, void* user_data) {
    if (!tree) return;
    c_bst_postorder_rec(tree->root, visitor, user_data);
}

size_t c_bst_size(const CBinarySearchTree* tree) {
    return tree ? tree->count : 0;
}

static void default_print_visitor(const CBSTNode* node, void* user_data) {
    (void)user_data;
    printf("  [ID: %s] %s | Dept: %s | Bus: %s (Seat %d)\n",
           node->student_id, node->student_name, node->department,
           node->bus_number, node->seat_number);
}

void c_bst_print(const CBinarySearchTree* tree) {
    if (!tree) {
        printf("BST: (NULL)\n");
        return;
    }
    printf("Binary Search Tree (Inorder Traversal, Count = %zu):\n", tree->count);
    c_bst_inorder(tree, default_print_visitor, NULL);
}

static void c_bst_destroy_rec(CBSTNode* node) {
    if (!node) return;
    c_bst_destroy_rec(node->left);
    c_bst_destroy_rec(node->right);
    c_bst_free_node(node);
}

void c_bst_destroy(CBinarySearchTree* tree) {
    if (!tree) return;
    c_bst_destroy_rec(tree->root);
    free(tree);
}
