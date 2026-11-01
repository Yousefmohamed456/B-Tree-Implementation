#include <iostream>
#include "btree/BTree.h"

void RunTests();

template <typename Type, int Ord>
void runMenu()
{
    BTree<Type, Ord> tree;
    InitTree(tree);

    while (true)
    {
        std::cout << "********* B-Tree *********" << std::endl;
        std::cout << "1. Insert" << std::endl;
        std::cout << "2. Delete" << std::endl;
        std::cout << "3. Search" << std::endl;
        std::cout << "4. Display All" << std::endl;
        std::cout << "0. Exit" << std::endl;

        int choice;
        std::cout << "Enter your choice" << std::endl;
        std::cin >> choice;

        if (choice == 0)
        {
            break;
        }

        if (choice == 4)
        {
            DisplayAll(tree);
            continue;
        }

        if (choice > 3 || choice < 1)
        {
            std::cout << "Invalid choice." << std::endl;
            continue;
        }
        std::cout << "Enter a value." << std::endl;
        Type value;
        std::cin >> value;
        bool exists = Search(tree, value);
        switch (choice)
        {
        case 1:
            if (exists)
                std::cout << "The value [" << value << "] already exists." << std::endl;
            else
            {
                Insert(tree, value);
                std::cout << "The value [" << value << "] was inserted successfully." << std::endl;
            }

            break;
        case 2:
            if (exists)
            {
                Delete(tree, value);
                std::cout << "The value [" << value << "] was deleted successfully." << std::endl;
            }
            else
                std::cout << "The value [" << value << "] was not found." << std::endl;

            break;
        case 3:
            if (exists)
                std::cout << "The value [" << value << "] was found." << std::endl;
            else
                std::cout << "The value [" << value << "] was not found." << std::endl;
            break;
        }
    }
    DestroyTree(tree);
}

int main()
{
    runMenu<char, 5>();
    return 0;
}