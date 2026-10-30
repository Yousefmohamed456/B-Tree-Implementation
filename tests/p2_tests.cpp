#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>
#include "btree/BTree.h"

template <int Ord>
void CheckNode(Node<int, Ord>* node, int lower, int upper, int depth,
               int& leafDepth, std::vector<int>& keys)
{
    assert(node != nullptr);
    assert(node->num_keys >= (depth == 0 ? 1 : (Ord - 1) / 2));
    assert(node->num_keys <= Ord);
    for (int i = 0; i < node->num_keys; ++i)
    {
        assert(lower < node->key[i] && node->key[i] < upper);
        if (i)
            assert(node->key[i - 1] < node->key[i]);
    }
    if (node->is_leaf)
    {
        if (leafDepth == -1)
            leafDepth = depth;
        assert(leafDepth == depth);
        for (int i = 0; i < node->num_keys; ++i)
            keys.push_back(node->key[i]);
        return;
    }
    auto* internal = static_cast<InternalNode<int, Ord>*>(node);
    for (int i = 0; i <= node->num_keys; ++i)
    {
        CheckNode(internal->child[i], i ? node->key[i - 1] : lower,
                  i < node->num_keys ? node->key[i] : upper,
                  depth + 1, leafDepth, keys);
        if (i < node->num_keys)
            keys.push_back(node->key[i]);
    }
}

template <int Ord>
void CheckTree(BTree<int, Ord>& tree, const std::vector<int>& expected)
{
    assert(tree.root != nullptr);
    int leafDepth = -1;
    std::vector<int> actual;
    CheckNode(tree.root, -10000, 10000, 0, leafDepth, actual);
    assert(actual == expected);
    for (int value : expected)
        assert(Search(tree, value));
    assert(!Search(tree, -9999));
    assert(!Search(tree, 9999));
}

template <int Ord>
void RunInsertionCases()
{
    for (int mode = 0; mode < 3; ++mode)
    {
        std::vector<int> input(250);
        std::iota(input.begin(), input.end(), 0);
        if (mode == 1)
            std::reverse(input.begin(), input.end());
        if (mode == 2)
            std::shuffle(input.begin(), input.end(), std::mt19937(42));

        BTree<int, Ord> tree;
        InitTree(tree);
        std::vector<int> expected;
        for (int value : input)
        {
            Insert(tree, value);
            expected.insert(std::lower_bound(expected.begin(), expected.end(), value), value);
            CheckTree(tree, expected);
            Node<int, Ord>* root = tree.root;
            Insert(tree, value);
            assert(tree.root == root);
            CheckTree(tree, expected);
        }
        assert(!tree.root->is_leaf);
        DestroyTree(tree);
        assert(tree.root == nullptr);
    }
}

int main()
{
    BTree<int, 3> tree;
    InitTree(tree);
    Insert(tree, 20);
    Insert(tree, 10);
    Insert(tree, 30);
    assert(tree.root->is_leaf && tree.root->num_keys == 3);
    Insert(tree, 25);
    assert(!tree.root->is_leaf && tree.root->key[0] == 20);
    assert(Search(tree, 20) && Search(tree, 25));
    DestroyTree(tree);

    RunInsertionCases<3>();
    RunInsertionCases<4>();
    RunInsertionCases<5>();
}
