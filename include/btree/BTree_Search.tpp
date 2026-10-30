// Person 1: Search and tree initialization

template <typename Type, int Ord>
void InitTree(BTree<Type, Ord> &tree)
{
    tree.root = nullptr;
}

template <typename Type, int Ord>
void DestroyNode(Node<Type, Ord> *node)
{
    if (!node)
        return;

    if (!node->is_leaf)
    {
        InternalNode<Type, Ord> *internal = static_cast<InternalNode<Type, Ord> *>(node);
        for (int i = 0; i <= internal->num_keys; ++i)
        {
            DestroyNode(internal->child[i]);
        }
    }

    if (node->is_leaf)
    {
        delete static_cast<LeafNode<Type, Ord> *>(node);
    }
    else
    {
        delete static_cast<InternalNode<Type, Ord> *>(node);
    }
}

template <typename Type, int Ord>
void DestroyTree(BTree<Type, Ord> &tree)
{
    DestroyNode(tree.root);
    tree.root = nullptr;
}

template <typename Type, int Ord>
LeafNode<Type, Ord> *CreateLeaf()
{
    LeafNode<Type, Ord> *node = new LeafNode<Type, Ord>();
    node->is_leaf = true;
    node->num_keys = 0;
    return node;
}

template <typename Type, int Ord>
InternalNode<Type, Ord> *CreateInternal()
{
    InternalNode<Type, Ord> *node = new InternalNode<Type, Ord>();
    node->is_leaf = false;
    node->num_keys = 0;
    for (int i = 0; i <= Ord; ++i)
    {
        node->child[i] = nullptr;
    }
    return node;
}

template <typename Type, int Ord>
bool SearchRecursive(Node<Type, Ord> *node, Type key)
{
    if (!node)
        return false;

    int i = 0;
    while (i < node->num_keys && key > node->key[i])
    {
        ++i;
    }

    if (i < node->num_keys && key == node->key[i])
    {
        return true;
    }

    if (node->is_leaf)
    {
        return false;
    }

    InternalNode<Type, Ord> *internal = static_cast<InternalNode<Type, Ord> *>(node);
    return SearchRecursive(internal->child[i], key);
}

template <typename Type, int Ord>
bool Search(BTree<Type, Ord> &tree, Type key)
{
    return SearchRecursive(tree.root, key);
}