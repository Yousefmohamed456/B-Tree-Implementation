// Person 3: Delete from leaf helpers

template <typename Type, int Ord>
void Delete(BTree<Type, Ord> &tree, Type key)
{
    if (tree.root == nullptr)
        return;

    Node<Type, Ord> *current = tree.root;

    int index = 0;
    while (index < current->num_keys && current->key[index] < key)
        ++index;

    if (current->key[index] == key)
    {
        if (current->is_leaf)
        {
            DeleteFromLeaf(tree, static_cast<LeafNode<Type, Ord> *>(current), key);
        }
        else
        {
            DeleteFromInternal(tree, static_cast<InternalNode<Type, Ord> *>(current), key);
        }
        return;
    }

    FixUnderflow(tree, current, key);
    current = tree.root;

    while (current != nullptr)
    {
        index = 0;
        while (index < current->num_keys && current->key[index] < key)
            ++index;

        if (current->key[index] == key)
        {
            if (current->is_leaf)
            {
                DeleteFromLeaf(tree, static_cast<LeafNode<Type, Ord> *>(current), key);
            }
            else
            {
                DeleteFromInternal(tree, static_cast<InternalNode<Type, Ord> *>(current), key);
            }
            return;
        }

        FixUnderflow(tree, current, key);
        index = 0;

        while (index < current->num_keys && current->key[index] < key)
            ++index;

        auto internal = static_cast<InternalNode<Type, Ord> *>(current);
        current = internal->child[index];
    }
}

template <typename Type, int Ord>
void DeleteFromLeaf(BTree<Type, Ord> &tree, LeafNode<Type, Ord> *leaf, Type key)
{
    int i = 0;

    while (i < leaf->num_keys && leaf->key[i] < key)
        ++i;

    if (i == leaf->num_keys || leaf->key[i] != key)
        return;

    for (int j = i; j < leaf->num_keys - 1; ++j)
        leaf->key[j] = leaf->key[j + 1];

    --leaf->num_keys;

    if (leaf == tree.root && leaf->num_keys == 0)
    {
        DestroyNode(tree.root);
        tree.root = nullptr;
    }
}

template <typename Type, int Ord>
bool BorrowFromSibling(BTree<Type, Ord> &tree, Node<Type, Ord> *node, Type key)
{
    if (node == nullptr || node->is_leaf)
        return false;

    const int MIN_KEYS = Ord / 2;

    int index = 0;
    while (index < node->num_keys && node->key[index] < key)
        ++index;

    InternalNode<Type, Ord> *parent = static_cast<InternalNode<Type, Ord> *>(node);
    Node<Type, Ord> *target = parent->child[index];

    if (target->num_keys > MIN_KEYS)
        return true;

    if (index > 0)
    {
        Node<Type, Ord> *left = parent->child[index - 1];
        if (left->num_keys > MIN_KEYS)
        {
            for (int i = target->num_keys; i > 0; i--)
            {
                target->key[i] = target->key[i - 1];
            }
            target->key[0] = parent->key[index - 1];
            parent->key[index - 1] = left->key[left->num_keys - 1];

            if (!left->is_leaf)
            {
                InternalNode<Type, Ord> *tI = static_cast<InternalNode<Type, Ord> *>(target);
                InternalNode<Type, Ord> *lI = static_cast<InternalNode<Type, Ord> *>(left);
                for (int i = target->num_keys + 1; i > 0; i--)
                {
                    tI->child[i] = tI->child[i - 1];
                }

                tI->child[0] = lI->child[left->num_keys];
                lI->child[left->num_keys] = nullptr;
            }
            target->num_keys++;
            left->num_keys--;

            return true;
        }
    }
    if (index < node->num_keys)
    {
        Node<Type, Ord> *right = parent->child[index - 1];
        if (right->num_keys > MIN_KEYS)
        {
            target->key[target->num_keys] = parent->key[index];
            parent->key[index] = right->key[0];
            for (int i = 0; i < right->num_keys - 1; i++)
            {
                right->key[i] = right->key[i + 1];
            }
            if (!right->is_leaf)
            {
                InternalNode<Type, Ord> *tI = static_cast<InternalNode<Type, Ord> *>(target);
                InternalNode<Type, Ord> *rI = static_cast<InternalNode<Type, Ord> *>(right);

                tI->child[target->num_keys] = rI->child[0];

                for (int i = 0; i < right->num_keys - 1; i++)
                {
                    rI->child[i] = rI->child[i + 1];
                }
                rI->child[right->num_keys] = nullptr;
            }
            target->num_keys++;
            right->num_keys--;

            return true;
        }
    }

    return false;
}

template <typename Type, int Ord>
void Merge(BTree<Type, Ord> &tree, Node<Type, Ord> *node, Type key)
{
    if (node == nullptr || node->is_leaf)
        return;

    InternalNode<Type, Ord> *parent = static_cast<InternalNode<Type, Ord> *>(node);

    int index = 0;
    while (index < node->num_keys && node->key[index] < key)
        ++index;

    Node<Type, Ord> *left = parent->child[index];
    Node<Type, Ord> *right = parent->child[index + 1];

    int leftKeys = left->num_keys;
    int rightKeys = right->num_keys;

    left->key[leftKeys] = node->key[index];

    for (int i = 0; i < rightKeys; i++)
        left->key[leftKeys + 1 + i] = right->key[i];

    for (int i = index; i < node->num_keys - 1; i++)
        node->key[i] = node->key[i + 1];

    for (int i = index; i < node->num_keys; i++)
        parent->child[i] = parent->child[i + 1];

    parent->child[parent->num_keys] = nullptr;

    if (!right->is_leaf)
    {
        InternalNode<Type, Ord> *lI = static_cast<InternalNode<Type, Ord> *>(left);
        InternalNode<Type, Ord> *rI = static_cast<InternalNode<Type, Ord> *>(right);

        for (int i = 0; i <= rightKeys; i++)
        {
            lI->child[leftKeys + 1 + i] = rI->child[i];
            rI->child[i] = nullptr;
        }
    }
    parent->num_keys--;
    left->num_keys += (1 + rightKeys);
    DestroyNode(right);
}