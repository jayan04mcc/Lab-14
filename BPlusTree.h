#pragma once
#include <vector>
#include <iostream>
#include <iterator>
using namespace std;

template <class key_type, class val_type>
class Node
{
public:
    Node() {
        next_leaf = nullptr;
        prev_leaf = nullptr;
    };

    vector <key_type> keys;
    vector <val_type> vals;
    vector <Node<key_type, val_type>*> nodes;
    class Node <key_type, val_type>* next_leaf;
    class Node <key_type, val_type>* prev_leaf;
};

template <class key_type, class val_type, size_t max_children = 3>
class BPlusTree
{
public:

    class Iterator
    {

    public:
        key_type get_key() const {
            if (node == nullptr) throw std::out_of_range("B+Tree: iterator is out of range");
            return node->keys[idx];
        }

        val_type get_val() const {
            if (node == nullptr) throw std::out_of_range("B+Tree: iterator is out of range");
            return node->vals[idx];
        }

        void set_val(val_type v) {
            if (node == nullptr) throw std::out_of_range("B+Tree: iterator is out of range");
            node->vals[idx] = v;
        }

        void advance(int distance) {

            if (distance < 0) {
                while (distance != 0) {
                    --(*this);
                    distance++;
                }
            }
            else {
                while (distance != 0) {
                    ++(*this);
                    distance--;
                }
            }
        }

        Iterator operator--(int) {
            Iterator it = *this;
            --(*this);
            return it;
        }

        const Iterator& operator--() {

            if (node == nullptr) throw std::out_of_range("B+Tree: iterator is out of range");
            if (idx == 0) {
                node = node->prev_leaf;
                idx = node->vals.size() - 1;
            }
            else {
                idx--;
            }

            return *this;
        }

        Iterator operator++(int) {
            Iterator it = *this;
            ++(*this);
            return it;
        }

        const Iterator& operator++() {
            if (node == nullptr) throw std::out_of_range("B+Tree: iterator is out of range");
            if (idx + 1 < node->vals.size()) {
                idx++;
            }
            else {
                idx = 0;
                node = node->next_leaf;
            }
            return *this;
        }

        bool operator!=(const Iterator& it) const {
            return !(*this == it);
        }

        bool operator==(const Iterator& it) const {
            return (this->node == it.node && this->idx == it.idx);
        }

    public:
        friend class Tree;
        Node<key_type, val_type>* node;
        size_t idx;
    }; 

    BPlusTree() {
        if (max_children < 3) throw std::runtime_error("B+Tree - max_degree must > 3");
        this->max_degree = max_children;
        root = new Node<key_type, val_type>;
        num_elements = 0;
    }

    ~BPlusTree() {
        clear();
        delete root;
    }

    void insert(const key_type& key, val_type val) {

        size_t i, j, traverse_index;
        key_type median_key;
        vector <size_t> traverse_indices;
        vector <Node<key_type, val_type>*> parents;

        Node<key_type, val_type>* right;
        Node<key_type, val_type>* n = root;
        Node<key_type, val_type>* parent;
        bool records = true;

        while (1) {

            for (i = 0; i < n->keys.size(); i++) {
                if (key < n->keys[i]) break;
            }
            if (n->nodes.size() != 0) {
                traverse_indices.push_back(i);
                parents.push_back(n);
                n = n->nodes[i];
            }
            else break;

        }

        /* llaves existentes */
        for (j = 0; j < n->keys.size(); j++) {
            if (n->keys[j] == key) {
                n->vals[j] = val;
                return;
            }
        }

        num_elements++;
        /* poner el valor y la clave en la posición adecuada */
        n->keys.insert(n->keys.begin() + i, key);
        n->vals.insert(n->vals.begin() + i, val);


        /* dividir el nodo hasta que el cubo (clave) ya no esté lleno */
        while (n->keys.size() == max_degree) {
            median_key = n->keys[max_degree / 2];

            right = new Node<key_type, val_type>;
            if (records) j = max_degree / 2;
            else j = max_degree / 2 + 1;

            for (i = j; i < max_degree; i++) {
                if (records) right->vals.push_back(n->vals[i]);
                right->keys.push_back(n->keys[i]);
            }

            for (i = (n->nodes.size() + 1) / 2; i < n->nodes.size(); i++) {
                right->nodes.push_back(n->nodes[i]);
            }

            // cuando dividimos el nodo raíz, creamos el nuevo nodo principal.
            // El nodo original se convirtió en el nodo "izquierdo".

            if (traverse_indices.size() == 0) {

                parent = new Node<key_type, val_type>;

                parent->nodes.push_back(n);
                parent->nodes.push_back(right);
                parent->keys.push_back(median_key);

                /* conectar los nodos hoja */

                right->next_leaf = n->next_leaf;
                if (n->next_leaf != nullptr) n->next_leaf->prev_leaf = right;
                n->next_leaf = right;
                right->prev_leaf = n;

                root = parent;
                n->keys.resize(max_degree / 2);
                if (records) n->vals.resize(max_degree / 2);

                if (n->nodes.size() != 0) n->nodes.resize((n->nodes.size() + 1) / 2);
            }
            else {

                /* cuando dividimos el nodo interno, el nodo original mantiene la mitad de la 
                capacidad del nodo izquierdo. Además, la clave mediana se agregó a su padre. */

                if (records) n->vals.resize(max_degree / 2);
                n->keys.resize(max_degree / 2);

                if (n->nodes.size() != 0) n->nodes.resize((n->nodes.size() + 1) / 2); // nodo interno

                /* conectar los nodos hoja */
                right->next_leaf = n->next_leaf;
                if (n->next_leaf != nullptr) n->next_leaf->prev_leaf = right;
                n->next_leaf = right;
                right->prev_leaf = n;

                parent = parents[parents.size() - 1];
                parents.pop_back();

                traverse_index = traverse_indices[traverse_indices.size() - 1];
                traverse_indices.pop_back();

                parent->keys.insert(parent->keys.begin() + traverse_index, median_key);
                parent->nodes.insert(parent->nodes.begin() + traverse_index + 1, right);

                n = parent;
                records = false;
            }
        }
        return;
    }

    Iterator find(const key_type& key) const {

        Node<key_type, val_type>* n = root;
        Iterator it;
        size_t i;

        /* find the leaf node first */
        while (n->nodes.size() != 0) {
            for (i = 0; i < n->keys.size(); i++) {
                if (key < n->keys[i]) break;
            }

            if (n->nodes.size() != 0) n = n->nodes[i];
        }

        /* check to see if we find the key */
        for (i = 0; i < n->keys.size(); i++) {
            if (key == n->keys[i]) {
                it.idx = i;
                it.node = n;
                return it;
            }
        }
        return end();
    }

    void erase(const key_type& key) {

        Node<key_type, val_type>* n = root;
        size_t i;
        int delete_index = -1;
        size_t min_keys = (max_degree - 1) / 2;
        size_t size;
        Node<key_type, val_type>* left, * right;
        Node<key_type, val_type>* parent;
        size_t traverse_index;
        vector <size_t> traverse_indices;
        vector <Node<key_type, val_type>*> parents;
        bool records = true;

        /* remember an internal node along with index, whose key is euqal to param "key"
           when we delete the leftmost key in the subtree, we will update the internal node's key,
           which has the same value as param "key". The new key will be the new replaced element.
        */
        Node<key_type, val_type>* same_value_node = nullptr;
        int same_value_index = -1;

        /* encontrar el nodo hoja primero */
        while (n->nodes.size() != 0) {
            for (i = 0; i < n->keys.size(); i++) {
                if (key == n->keys[i]) {
                    same_value_node = n;
                    same_value_index = i;
                }
                if (key < n->keys[i]) break;
            }

            if (n->nodes.size() != 0) {
                traverse_indices.push_back(i);
                parents.push_back(n);
                n = n->nodes[i];
            }
        }

        /* encuentra el indice */
        for (i = 0; i < n->keys.size(); i++) {
            if (key == n->keys[i]) {
                delete_index = i;
                break;
            }
        }

        /* la clave no se encuentra en el árbol */
        if (delete_index == -1) return;


        num_elements--;
        /* borra el registro */
        n->keys.erase(n->keys.begin() + delete_index);
        n->vals.erase(n->vals.begin() + delete_index);
        if (n == root) return;


        left = n->prev_leaf;
        right = n->next_leaf;

        /* caso1: el depósito es lo suficientemente grande después de la operación de eliminación */
        if (n->keys.size() >= min_keys) {

            /* the leftmost key is deleted */
            if (same_value_node != nullptr) {
                same_value_node->keys[same_value_index] = n->keys[0];
            }
            return;
        }

        /* fusionar o tomar prestado el nodo de los vecinos
        hasta que el tamaño del depósito actual sea >= min_keys o llegue al nodo raíz
        */
        while (n->keys.size() < min_keys && n != root) {


            left = n->prev_leaf;
            right = n->next_leaf;
            parent = parents[parents.size() - 1];
            parents.pop_back();
            traverse_index = traverse_indices[traverse_indices.size() - 1];
            traverse_indices.pop_back();

            /* caso:2 prestado del nodo izquierdo
               cuando no es el nodo más a la izquierda en el subárbol y el tamaño del nodo izquierdo es lo suficientemente grande.
               traverse_index menos 1 se debe a que el índice de la clave es uno menos que el índice de los nodos.
            */
            if (left != nullptr && traverse_index != 0) {
                size = left->keys.size();

                if (size > min_keys) {

                    traverse_index--;
                    /* si son nodos hoja, robamos la clave más a la derecha y val en el nodo izquierdo
                    y actualice la clave del padre con su clave.
                    De lo contrario, bajamos la clave principal al nodo actual y
                    mostrar la clave más a la derecha en el nodo izquierdo.
                    */
                    if (records) {
                        n->keys.insert(n->keys.begin(), left->keys[size - 1]);
                        parent->keys[traverse_index] = n->keys[0];

                        n->vals.insert(n->vals.begin(), left->vals[size - 1]);
                        left->vals.pop_back();
                    }
                    else {
                        n->keys.insert(n->keys.begin(), parent->keys[traverse_index]);
                        parent->keys[traverse_index] = left->keys[size - 1];
                        n->nodes.insert(n->nodes.begin(), left->nodes[left->nodes.size() - 1]);
                        left->nodes.pop_back();
                    }

                    left->keys.pop_back();

                    return;
                }

                /* case3: préstamo del nodo derecho */
            }
            else if (right != nullptr && traverse_index != parent->nodes.size() - 1) {


                size = right->keys.size();

                if (size > min_keys) {
                    /* the leftmost key in the subtree could be deleted */
                    if (same_value_node != nullptr) {
                        same_value_node->keys[same_value_index] = n->keys[0];
                    }

                    /* si son nodos hoja, robamos la clave más a la izquierda y val en el nodo derecho
                    y actualice la clave del padre con su clave.
                    De lo contrario, bajamos la clave principal al nodo actual y
                    mostrar la clave más a la izquierda en el nodo derecho.
                    */

                    if (records) {
                        n->keys.push_back(right->keys[0]);
                        parent->keys[traverse_index] = right->keys[1];
                        n->vals.push_back(right->vals[0]);
                        right->vals.erase(right->vals.begin());

                    }
                    else {
                        n->keys.push_back(parent->keys[traverse_index]);
                        parent->keys[traverse_index] = right->keys[0];

                        n->nodes.push_back(right->nodes[0]);
                        right->nodes.erase(right->nodes.begin());
                    }
                    right->keys.erase(right->keys.begin());
                    return;
                }
            }

            if (left != nullptr && traverse_index != 0) {

                left->next_leaf = n->next_leaf;
                if (n->next_leaf != nullptr) n->next_leaf->prev_leaf = left;

                if (!records) {
                    left->keys.push_back(parent->keys[traverse_index - 1]);
                    for (i = 0; i < n->nodes.size(); i++) {
                        left->nodes.push_back(n->nodes[i]);
                    }
                }

                for (i = 0; i < n->keys.size(); i++) {
                    left->keys.push_back(n->keys[i]);
                    if (records) left->vals.push_back(n->vals[i]);
                }

                parent->keys.erase(parent->keys.begin() + traverse_index - 1);
                parent->nodes.erase(parent->nodes.begin() + traverse_index);

                if (parent->keys.size() == 0 && parent == root) {

                    delete n;
                    delete root;
                    root = left;
                    return;
                }

                delete n;
                n = parent;

            }
            else if (right != nullptr && traverse_index != parent->nodes.size() - 1) {

                if (same_value_node != nullptr) {
                    same_value_node->keys[same_value_index] = n->keys[0];
                }

                n->next_leaf = right->next_leaf;
                if (right->next_leaf != nullptr) right->next_leaf->prev_leaf = n;

                if (!records) {
                    n->keys.push_back(parent->keys[traverse_index]);
                    for (i = 0; i < right->nodes.size(); i++) {
                        n->nodes.push_back(right->nodes[i]);
                    }
                }

                for (i = 0; i < right->keys.size(); i++) {
                    n->keys.push_back(right->keys[i]);
                    if (records) n->vals.push_back(right->vals[i]);
                }

                parent->nodes[traverse_index + 1] = n;
                parent->keys.erase(parent->keys.begin() + traverse_index);
                parent->nodes.erase(parent->nodes.begin() + traverse_index);


                if (parent->keys.size() == 0 && parent == root) {
                    delete right;
                    delete root;
                    root = n;
                    return;
                }
                delete right;
                n = parent;
            }
            records = false;
            same_value_node = nullptr;
        }
    }

    size_t size() const { return num_elements; };

    bool empty() const { return (num_elements == 0); };

    void clear() {
        recursive_clear_tree(root);
        root = new Node<key_type, val_type>;
        num_elements = 0;
    }

    vector<key_type> get_keys() const {
        Node<key_type, val_type>* n = root;
        vector <key_type> rv;
        size_t i;

        // go to the leftmost node.
        while (n->nodes.size() != 0) {
            n = n->nodes[0];
        }

        do {
            for (i = 0; i < n->keys.size(); i++) {
                rv.push_back(n->keys[i]);
            }
            n = n->next_leaf;
        } while (n != nullptr);

        return rv;
    }

    vector<val_type> get_vals() const {
        Node<key_type, val_type>* n = root;
        vector <val_type> rv;
        size_t i;

        // go to the leftmost node.
        while (n->nodes.size() != 0) {
            n = n->nodes[0];
        }

        do {
            for (i = 0; i < n->vals.size(); i++) {
                rv.push_back(n->vals[i]);
            }
            n = n->next_leaf;
        } while (n != nullptr);

        return rv;
    }

    val_type at(const key_type& key) const {
        Iterator it = find(key);
        return it.get_val();
    }

    val_type& operator[] (const key_type& key) {

        static val_type dummy;
        if (find(key) == end()) insert(key, dummy);


        Node<key_type, val_type>* n = root;
        size_t i;

        /* find the leaf node first */
        while (n->nodes.size() != 0) {
            for (i = 0; i < n->keys.size(); i++) {
                if (key < n->keys[i]) break;
            }

            if (n->nodes.size() != 0) n = n->nodes[i];
        }

        /* check to see if we find the key */
        for (i = 0; i < n->keys.size(); i++) {
            if (key == n->keys[i]) {
                break;
            }
        }

        if (n->keys.size() == i) throw std::runtime_error("B+tree [] internal error");
        return n->vals[i];
    }

    Iterator begin() const {

        Iterator it;
        Node<key_type, val_type>* n = root;

        if (num_elements == 0) {
            it.node = nullptr;
            it.idx = 0;
            return it;
        }

        /* encuentra el nodo más a la izquierda */
        while (n->nodes.size() != 0) {
            n = n->nodes[0];
        }

        it.node = n;
        it.idx = 0;

        return it;
    }

    Iterator end() const {
        Iterator it;
        it.node = nullptr;
        it.idx = 0;
        return it;
    }

    Node<key_type, val_type>* get_root() {
        return this->root;
    }

private:
    size_t num_elements;
    Node<key_type, val_type>* root;
    size_t max_degree;

    void recursive_clear_tree(const Node<key_type, val_type>* n) {
        size_t i;
        for (i = 0; i < n->nodes.size(); i++) {
            recursive_clear_tree(n->nodes[i]);
        }
        delete n;
    }
};

