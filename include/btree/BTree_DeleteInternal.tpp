// Person 4: Delete from internal and underflow helpers

template <typename Type, int Ord>
void DeleteFromInternal(BTree<Type, Ord> &tree, InternalNode<Type, Ord> *node, Type key)
{
    const int MIN_KEYS = Ord / 2;
    Node<Type, Ord> *current = node;

    while (true)
    {
        int index = 0;
        while (index < current->num_keys &&
               current->key[index] != key)
        {
            ++index;
        }

        if (current->is_leaf)
        {
            DeleteFromLeaf(tree, static_cast<LeafNode<Type, Ord> *>(current), key);
            return;
        }

        auto internal = static_cast<InternalNode<Type, Ord> *>(current);
        auto left = internal->child[index];
        auto right = internal->child[index + 1];

        bool usePredecessor = left->num_keys > MIN_KEYS;

        if (usePredecessor || right->num_keys > MIN_KEYS)
        {
            Node<Type, Ord> *temp = usePredecessor ? left : right;

            while (!temp->is_leaf)
            {
                auto parent = static_cast<InternalNode<Type, Ord> *>(temp);
                int childIndex = usePredecessor ? temp->num_keys : 0;
                temp = parent->child[childIndex];
            }
            
            int replacingIndex = usePredecessor ? temp->num_keys - 1 : 0;
            Type replacing = temp->key[replacingIndex];

            Delete(tree, replacing);
            internal->key[index] = replacing;
            return;
        }

        // Merge children into left
        Merge(tree, internal, key);

        // Shrink the root if the merge affected it
        if (internal == tree.root && internal->num_keys == 0)
        {
            FixUnderflow(tree, internal, key);
        }

        current = left;
    }
}

template <typename Type, int Ord>
void FixUnderflow(BTree<Type, Ord> &tree, Node<Type, Ord> *node, Type key)
{
    if (node == tree.root && node->num_keys == 0)
    {
        if (node->is_leaf)
        {
            DestroyNode(tree.root);
            tree.root = nullptr;
        }
        else
        {
            InternalNode<Type, Ord> *temp = static_cast<InternalNode<Type, Ord> *>(node);
            tree.root = temp->child[0];
            temp->child[0] = nullptr;
            DestroyNode(temp);
        }
        return;
    }
    
    const int MIN_KEYS = Ord / 2;
    if (node->is_leaf)
        return;

    int index = 0;
    while (index < node->num_keys && key > node->key[index])
        ++index;
    
    InternalNode<Type, Ord> *internal = static_cast<InternalNode<Type, Ord> *>(node);
    if (internal->child[index]->num_keys <= MIN_KEYS)
    {
        bool borrowLeft = index > 0 && internal->child[index - 1]->num_keys > MIN_KEYS;
        bool borrowRight = index < node->num_keys && internal->child[index + 1]->num_keys > MIN_KEYS;
        
        if (borrowLeft || borrowRight)
        {
            BorrowFromSibling(tree, node, key);
            return;
        }
        
        Type separator = (index < node->num_keys)
                             ? node->key[index]
                             : node->key[index - 1];
        Merge(tree, node, separator);

        if (node == tree.root && node->num_keys == 0)
            FixUnderflow(tree, node, key);
    }
}