// Person 2: Insert operations

template <typename Type, int Ord>
void SplitLeaf(BTree<Type, Ord>& tree, LeafNode<Type, Ord>* leaf, LeafNode<Type, Ord>*& newLeaf)
{
    (void)tree;
    const int middle = Ord / 2;
    newLeaf = CreateLeaf<Type, Ord>();
    newLeaf->num_keys = leaf->num_keys - middle - 1;
    for (int i = 0; i < newLeaf->num_keys; ++i)
        newLeaf->key[i] = leaf->key[middle + 1 + i];
    leaf->num_keys = middle;
}

template <typename Type, int Ord>
void SplitInternal(BTree<Type, Ord>& tree, InternalNode<Type, Ord>* node, InternalNode<Type, Ord>*& newNode)
{
    (void)tree;
    const int middle = Ord / 2;
    newNode = CreateInternal<Type, Ord>();
    newNode->num_keys = node->num_keys - middle - 1;
    for (int i = 0; i < newNode->num_keys; ++i)
        newNode->key[i] = node->key[middle + 1 + i];
    for (int i = 0; i <= newNode->num_keys; ++i)
    {
        newNode->child[i] = node->child[middle + 1 + i];
        node->child[middle + 1 + i] = nullptr;
    }
    node->num_keys = middle;
}

template <typename Type, int Ord>
void Insert(BTree<Type, Ord>& tree, Type key)
{
    static_assert(Ord >= 2, "B-tree nodes must hold at least two keys");

    if (Search(tree, key))
        return;

    if (!tree.root)
        tree.root = CreateLeaf<Type, Ord>();

    if (tree.root->num_keys == Ord)
    {
        Node<Type, Ord>* oldRoot = tree.root;
        InternalNode<Type, Ord>* newRoot = CreateInternal<Type, Ord>();
        newRoot->child[0] = oldRoot;
        if (oldRoot->is_leaf)
        {
            LeafNode<Type, Ord>* sibling;
            SplitLeaf(tree, static_cast<LeafNode<Type, Ord>*>(oldRoot), sibling);
            newRoot->child[1] = sibling;
        }
        else
        {
            InternalNode<Type, Ord>* sibling;
            SplitInternal(tree, static_cast<InternalNode<Type, Ord>*>(oldRoot), sibling);
            newRoot->child[1] = sibling;
        }
        newRoot->key[0] = oldRoot->key[Ord / 2];
        newRoot->num_keys = 1;
        tree.root = newRoot;
    }

    Node<Type, Ord>* current = tree.root;
    while (!current->is_leaf)
    {
        InternalNode<Type, Ord>* parent = static_cast<InternalNode<Type, Ord>*>(current);
        int index = 0;
        while (index < parent->num_keys && key > parent->key[index])
            ++index;

        Node<Type, Ord>* child = parent->child[index];
        if (child->num_keys == Ord)
        {
            const Type promoted = child->key[Ord / 2];
            Node<Type, Ord>* sibling;
            if (child->is_leaf)
            {
                LeafNode<Type, Ord>* newLeaf;
                SplitLeaf(tree, static_cast<LeafNode<Type, Ord>*>(child), newLeaf);
                sibling = newLeaf;
            }
            else
            {
                InternalNode<Type, Ord>* newNode;
                SplitInternal(tree, static_cast<InternalNode<Type, Ord>*>(child), newNode);
                sibling = newNode;
            }

            for (int i = parent->num_keys; i > index; --i)
                parent->key[i] = parent->key[i - 1];
            for (int i = parent->num_keys + 1; i > index + 1; --i)
                parent->child[i] = parent->child[i - 1];
            parent->key[index] = promoted;
            parent->child[index + 1] = sibling;
            ++parent->num_keys;
            if (key > promoted)
                ++index;
        }
        current = parent->child[index];
    }

    LeafNode<Type, Ord>* leaf = static_cast<LeafNode<Type, Ord>*>(current);
    int index = leaf->num_keys;
    while (index > 0 && leaf->key[index - 1] > key)
    {
        leaf->key[index] = leaf->key[index - 1];
        --index;
    }
    leaf->key[index] = key;
    ++leaf->num_keys;
}
