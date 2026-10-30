#include <cassert>
#include <iostream>
#include <cstdlib>
#include "btree/BTree.h"

// Live-allocation counter (Task 1 option c). Overrides count allocations
// and frees; after all cleanups the live count must be zero.
static long live_alloc = 0, live_free = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
void* operator new(std::size_t sz)
{
    void* p = std::malloc(sz);
    __sync_fetch_and_add(&live_alloc, 1);
    return p;
}
void operator delete(void* p) noexcept
{
    std::free(p);
    __sync_fetch_and_add(&live_free, 1);
}
void operator delete(void* p, std::size_t) noexcept
{
    std::free(p);
    __sync_fetch_and_add(&live_free, 1);
}
#pragma GCC diagnostic pop

static int g_pass = 0, g_fail = 0;

#define TEST(name, expr)                                          \
    do {                                                          \
        bool _r = (expr);                                         \
        if (_r) { g_pass++; }                                     \
        else {                                                    \
            g_fail++;                                             \
            std::cout << "FAIL: " #name << std::endl;             \
        }                                                       \
    } while (0)

// ---------- Helper to build a 2-level tree manually ----------
static BTree<int, 3>* Build2LevelTree()
{
    BTree<int, 3>* tree = new BTree<int, 3>();
    // Create root internal node with keys [10, 20]
    InternalNode<int, 3>* root = CreateInternal<int, 3>();
    root->is_leaf = false;
    root->num_keys = 2;
    root->key[0] = 10;
    root->key[1] = 20;
    root->child[0] = nullptr;
    root->child[1] = nullptr;
    root->child[2] = nullptr;

    // Create child leaves
    // child[0]: leaf with keys [5, 7]  (all < 10)
    LeafNode<int, 3>* leaf0 = CreateLeaf<int, 3>();
    leaf0->is_leaf = true;
    leaf0->num_keys = 2;
    leaf0->key[0] = 5;
    leaf0->key[1] = 7;
    root->child[0] = leaf0;

    // child[1]: leaf with keys [12, 15]  (>= 10 and < 20)
    LeafNode<int, 3>* leaf1 = CreateLeaf<int, 3>();
    leaf1->is_leaf = true;
    leaf1->num_keys = 2;
    leaf1->key[0] = 12;
    leaf1->key[1] = 15;
    root->child[1] = leaf1;

    // child[2]: leaf with keys [25, 30]  (>= 20)
    LeafNode<int, 3>* leaf2 = CreateLeaf<int, 3>();
    leaf2->is_leaf = true;
    leaf2->num_keys = 2;
    leaf2->key[0] = 25;
    leaf2->key[1] = 30;
    root->child[2] = leaf2;

    tree->root = static_cast<Node<int, 3>*>(root);
    return tree;
}

// ---------- Helper to build a 3-level tree manually ----------
static BTree<int, 3>* Build3LevelTree()
{
    BTree<int, 3>* tree = new BTree<int, 3>();
    // Root internal node with 1 key: [20]
    InternalNode<int, 3>* root = CreateInternal<int, 3>();
    root->is_leaf = false;
    root->num_keys = 1;
    root->key[0] = 20;
    root->child[0] = nullptr;
    root->child[1] = nullptr;

    // child[0]: internal node with keys [10]
    InternalNode<int, 3>* left = CreateInternal<int, 3>();
    left->is_leaf = false;
    left->num_keys = 1;
    left->key[0] = 10;
    left->child[0] = nullptr;
    left->child[1] = nullptr;

    // left->child[0]: leaf [1, 5]
    LeafNode<int, 3>* leafL0 = CreateLeaf<int, 3>();
    leafL0->is_leaf = true;
    leafL0->num_keys = 2;
    leafL0->key[0] = 1;
    leafL0->key[1] = 5;
    left->child[0] = leafL0;

    // left->child[1]: leaf [12, 15]
    LeafNode<int, 3>* leafL1 = CreateLeaf<int, 3>();
    leafL1->is_leaf = true;
    leafL1->num_keys = 2;
    leafL1->key[0] = 12;
    leafL1->key[1] = 15;
    left->child[1] = leafL1;

    // child[1]: internal node with keys [30]
    InternalNode<int, 3>* right = CreateInternal<int, 3>();
    right->is_leaf = false;
    right->num_keys = 1;
    right->key[0] = 30;
    right->child[0] = nullptr;
    right->child[1] = nullptr;

    // right->child[0]: leaf [22, 25]
    LeafNode<int, 3>* leafR0 = CreateLeaf<int, 3>();
    leafR0->is_leaf = true;
    leafR0->num_keys = 2;
    leafR0->key[0] = 22;
    leafR0->key[1] = 25;
    right->child[0] = leafR0;

    // right->child[1]: leaf [35, 40]
    LeafNode<int, 3>* leafR1 = CreateLeaf<int, 3>();
    leafR1->is_leaf = true;
    leafR1->num_keys = 2;
    leafR1->key[0] = 35;
    leafR1->key[1] = 40;
    right->child[1] = leafR1;

    // Connect root to its children
    root->child[0] = left;
    root->child[1] = right;

    tree->root = static_cast<Node<int, 3>*>(root);
    return tree;
}

// ---------- Helper to clean up a manually-built tree ----------
static void Delete2LevelTree(BTree<int, 3>* tree)
{
    if (!tree)
        return;
    // We built it with CreateInternal which allocates on heap; free node by node
    DestroyTree(*tree); // our DestroyTree handles recursive free
    delete tree;
    TEST(Live_after_2level_destroy, live_alloc == live_free);
}

static void Delete3LevelTree(BTree<int, 3>* tree)
{
    if (!tree)
        return;
    DestroyTree(*tree);
    delete tree;
    TEST(Live_after_3level_destroy, live_alloc == live_free);
}

int main()
{
    std::cout << "=== Person 1 Tests ===" << std::endl;

    // ----- InitTree: root is null -----
    {
        BTree<int, 3> tree;
        InitTree(tree);
        TEST(InitTree_empty, tree.root == nullptr);
    }

    // ----- CreateLeaf / CreateInternal -----
    {
        LeafNode<int, 3>* leaf = CreateLeaf<int, 3>();
        TEST(CreateLeaf_init, leaf->is_leaf == true && leaf->num_keys == 0);
        delete leaf;

        InternalNode<int, 3>* internal = CreateInternal<int, 3>();
        TEST(CreateInternal_init, internal->is_leaf == false && internal->num_keys == 0);
        // child pointers should be null per CreateInternal implementation
        bool children_null = true;
        for (int i = 0; i <= 3; ++i)
            if (internal->child[i] != nullptr)
                children_null = false;
        TEST(CreateInternal_children_null, children_null);
        delete internal;
    }

    // ----- Search on empty tree -----
    {
        BTree<int, 3> tree;
        InitTree(tree);
        TEST(Search_empty_false, Search(tree, 10) == false);
    }

    // ----- Single leaf tests -----
    {
        BTree<int, 3> tree;
        InitTree(tree);
        LeafNode<int, 3>* leaf = CreateLeaf<int, 3>();
        leaf->is_leaf = true;
        leaf->num_keys = 3;
        leaf->key[0] = 10;
        leaf->key[1] = 20;
        leaf->key[2] = 30;
        tree.root = static_cast<Node<int, 3>*>(leaf);

        TEST(Search_single_first, Search(tree, 10) == true);
        TEST(Search_single_middle, Search(tree, 20) == true);
        TEST(Search_single_last, Search(tree, 30) == true);
        TEST(Search_single_missing, Search(tree, 25) == false);
        TEST(Search_single_smaller, Search(tree, 5) == false);
        TEST(Search_single_larger, Search(tree, 35) == false);

        DestroyTree(tree);
        TEST(Live_after_single_leaf_destroy, live_alloc == live_free);
    }

    // ----- 2-level tree tests -----
    {
        BTree<int, 3>* tree = Build2LevelTree();

        // Found keys
        TEST(Search_2level_found_first_child_5, Search(*tree, 5) == true);
        TEST(Search_2level_found_last_in_first, Search(*tree, 7) == true);
        TEST(Search_2level_found_first_in_mid, Search(*tree, 12) == true);
        TEST(Search_2level_found_mid_in_mid, Search(*tree, 15) == true);
        TEST(Search_2level_found_first_in_last, Search(*tree, 25) == true);
        TEST(Search_2level_found_last_in_last, Search(*tree, 30) == true);

        // Separator keys (in internal node)
        TEST(Search_2level_separator_10, Search(*tree, 10) == true);
        TEST(Search_2level_separator_20, Search(*tree, 20) == true);

        // Not found: smaller than all
        TEST(Search_2level_smaller_than_all, Search(*tree, 1) == false);
        // Not found: larger than all
        TEST(Search_2level_larger_than_all, Search(*tree, 35) == false);
        // Not found: between keys
        TEST(Search_2level_between_10_20_11, Search(*tree, 11) == false);
        TEST(Search_2level_between_20_25_21, Search(*tree, 21) == false);
        TEST(Search_2level_between_12_15_13, Search(*tree, 13) == false);

        // Boundary: exactly at separator (already covered by 10/20, but explicit)
        TEST(Search_2level_separator_root_10, Search(*tree, 10) == true);
        TEST(Search_2level_separator_root_20, Search(*tree, 20) == true);

        Delete2LevelTree(tree);
    }

    // ----- 3-level tree tests -----
    {
        BTree<int, 3>* tree = Build3LevelTree();

        // Keys in left subtree (< 20)
        TEST(Search_3level_leftmost, Search(*tree, 1) == true);
        TEST(Search_3level_left_5, Search(*tree, 5) == true);
        TEST(Search_3level_left_internal_10, Search(*tree, 10) == true);
        TEST(Search_3level_left_12, Search(*tree, 12) == true);
        TEST(Search_3level_left_15, Search(*tree, 15) == true);

        // Keys in right subtree (>= 20)
        TEST(Search_3level_rightmost, Search(*tree, 40) == true);
        TEST(Search_3level_right_35, Search(*tree, 35) == true);
        TEST(Search_3level_right_internal_30, Search(*tree, 30) == true);
        TEST(Search_3level_right_25, Search(*tree, 25) == true);
        TEST(Search_3level_right_22, Search(*tree, 22) == true);

        // Root/internal separators
        TEST(Search_3level_root_20, Search(*tree, 20) == true);

        // Not found
        TEST(Search_3level_not_found_11, Search(*tree, 11) == false);
        TEST(Search_3level_not_found_21, Search(*tree, 21) == false);
        TEST(Search_3level_not_found_55, Search(*tree, 55) == false);

        // Boundary: smaller than all, larger than all
        TEST(Search_3level_boundary_small, Search(*tree, 0) == false);
        TEST(Search_3level_boundary_large, Search(*tree, 99) == false);

        Delete3LevelTree(tree);
    }

    // ----- DestroyTree: safe on empty -----
    {
        BTree<int, 3> tree;
        InitTree(tree);
        // Should not crash
        DestroyTree(tree);
        TEST(Destroy_twice_safe, tree.root == nullptr); // double-call nulls root
        TEST(Live_after_empty_destroy, live_alloc == live_free);
        DestroyTree(tree); // second call, root already null
        TEST(Live_after_double_destroy, live_alloc == live_free);
    }

    std::cout << "\n=== Results: " << g_pass << " passed, " << g_fail << " failed ===" << std::endl;
    return (g_fail > 0) ? 1 : 0;
}