#ifndef __PROGTEST__
#include <cassert>
#include <cstdarg>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <array>
#include <random>
#include <type_traits>

// We use std::vector as a reference to check our implementation.
// It is not available in progtest :)
#include <vector>

template < typename T >
struct Ref {
    bool empty() const { return _data.empty(); }
    size_t size() const { return _data.size(); }

    const T& operator [] (size_t index) const { return _data.at(index); }
    T& operator [] (size_t index) { return _data.at(index); }

    void insert(size_t index, T value) {
        if (index > _data.size()) throw std::out_of_range("oops");
        _data.insert(_data.begin() + index, std::move(value));
    }

    T erase(size_t index) {
        T ret = std::move(_data.at(index));
        _data.erase(_data.begin() + index);
        return ret;
    }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

private:
    std::vector<T> _data;
};

#endif

namespace config {
    inline constexpr bool PARENT_POINTERS = true;
    inline constexpr bool CHECK_DEPTH = true;
}

template < typename T>
struct Node {
    T value;
    Node<T>* parent;
    Node<T>* left;
    Node<T>* right;
    int depth;
    int size; // Размер поддерева

    Node(const T& val) : value(val), parent(nullptr), left(nullptr), right(nullptr), depth(0), size(1) {}

    static void updateDepthAndSize(Node<T>* node) {
        if (node) {
            int left_depth = node->left ? node->left->depth : -1;
            int right_depth = node->right ? node->right->depth : -1;
            node->depth = std::max(left_depth, right_depth) + 1;

            int left_size = node->left ? node->left->size : 0;
            int right_size = node->right ? node->right->size : 0;
            node->size = 1 + left_size + right_size;
        }
    }

    static Node<T>* findByIndex(Node<T>* node, size_t index) {
        if (!node) return nullptr;
        int leftSize = node->left ? node->left->size : 0;
        if (index < (size_t)leftSize) {
            return findByIndex(node->left, index);
        } else if (index == (size_t)leftSize) {
            return node;
        } else {
            return findByIndex(node->right, index - leftSize - 1);
        }
    }

    static Node<T>* insertByIndex(Node<T>*& node, size_t index, T value, Node<T>* parent = nullptr) {
        if (!node) {
            node = new Node<T>(value);
            node->parent = parent;
            return node;
        }

        int leftSize = node->left ? node->left->size : 0;

        if (index <= (size_t)leftSize) {
            node->left = insertByIndex(node->left, index, value, node);
        } else {
            node->right = insertByIndex(node->right, index - leftSize - 1, value, node);
        }

        updateDepthAndSize(node);

        // Балансировка дерева
        return balance(node);
    }

    static Node<T>* eraseByIndex(Node<T>*& node, size_t index) {
        if (!node) return nullptr;

        int leftSize = node->left ? node->left->size : 0;

        if (index < (size_t)leftSize) {
            node->left = eraseByIndex(node->left, index);
            if (node->left) node->left->parent = node;
        } else if (index == (size_t)leftSize) {
            // Удаляем текущий узел
            if (!node->left || !node->right) {
                Node<T>* temp = node->left ? node->left : node->right;
                if (temp) temp->parent = node->parent;
                delete node;
                node = temp;
            } else {
                // Узел с двумя детьми
                Node<T>* successor = minValueNode(node->right);
                node->value = successor->value;
                node->right = eraseByIndex(node->right, 0);
                if (node->right) node->right->parent = node;
            }
        } else {
            node->right = eraseByIndex(node->right, index - leftSize - 1);
            if (node->right) node->right->parent = node;
        }

        if (node) {
            updateDepthAndSize(node);

            // Балансировка дерева
            node = balance(node);
        }

        return node;
    }

    static Node<T>* minValueNode(Node<T>* node) {
        Node<T>* current = node;
        while (current && current->left != nullptr)
            current = current->left;
        return current;
    }

    static int getBalance(Node<T>* node) {
        if (!node) return 0;
        int left_depth = node->left ? node->left->depth : -1;
        int right_depth = node->right ? node->right->depth : -1;
        return right_depth - left_depth;
    }

    static Node<T>* balance(Node<T>* node) {
        int balanceFactor = getBalance(node);

        if (balanceFactor > 1) {
            if (getBalance(node->right) >= 0) {
                node = rotateLeft(node);
            } else {
                node->right = rotateRight(node->right);
                node = rotateLeft(node);
            }
        } else if (balanceFactor < -1) {
            if (getBalance(node->left) <= 0) {
                node = rotateRight(node);
            } else {
                node->left = rotateLeft(node->left);
                node = rotateRight(node);
            }
        }

        return node;
    }

    static Node<T>* rotateLeft(Node<T>* x) {
        Node<T>* y = x->right;
        Node<T>* T2 = y->left;

        // Выполняем вращение
        y->left = x;
        x->right = T2;

        // Обновляем родителя
        y->parent = x->parent;
        x->parent = y;
        if (T2) T2->parent = x;

        // Обновляем глубину и размер
        updateDepthAndSize(x);
        updateDepthAndSize(y);

        return y;
    }

    static Node<T>* rotateRight(Node<T>* y) {
        Node<T>* x = y->left;
        Node<T>* T2 = x->right;

        // Выполняем вращение
        x->right = y;
        y->left = T2;

        // Обновляем родителя
        x->parent = y->parent;
        y->parent = x;
        if (T2) T2->parent = y;

        // Обновляем глубину и размер
        updateDepthAndSize(y);
        updateDepthAndSize(x);

        return x;
    }

    static void destroySubtree(Node<T>* node) {
        if (!node) return;
        destroySubtree(node->left);
        destroySubtree(node->right);
        delete node;
    }
};

template < typename T >
struct Array {
    bool empty() const {
        return m_size == 0;
    }

    size_t size() const {
        return m_size;
    }

    const T& operator [] (size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        Node<T>* node = Node<T>::findByIndex(root, index);
        if (!node) {
            throw std::out_of_range("Element not found");
        }
        return node->value;
    }

    T& operator [] (size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        Node<T>* node = Node<T>::findByIndex(root, index);
        if (!node) {
            throw std::out_of_range("Element not found");
        }
        return node->value;
    }

    void insert(size_t index, T value) {
        if (index > m_size) {
            throw std::out_of_range("Insert out of range");
        }
        root = Node<T>::insertByIndex(root, index, value);
        if (root) root->parent = nullptr;
        m_size++;
    }

    T erase(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Erase out of range");
        }
        Node<T>* node = Node<T>::findByIndex(root, index);
        if (!node) {
            throw std::out_of_range("Element not found");
        }
        T res = node->value;
        root = Node<T>::eraseByIndex(root, index);
        if (root) root->parent = nullptr;
        m_size--;
        return res;
    }

    // Необходимо для тестирования структуры дерева.
    struct TesterInterface {
        static const Node<T>* root(const Array* t) { return t->root; }
        static const Node<T>* parent(const Node<T>* n) { return n->parent; }
        static const Node<T>* right(const Node<T>* n) { return n->right; }
        static const Node<T>* left(const Node<T>* n) { return n->left; }
        static const T& value(const Node<T>* n) { return n->value; }
    };

    ~Array() {
        Node<T>::destroySubtree(root);
        root = nullptr;
        m_size = 0;
    }

    Array() : m_size(0), root(nullptr) {}

private:
    size_t m_size;
    Node<T>* root;
};




#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
    using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
    va_list args1;
    va_list args2;
    va_start(args1, f);
    va_copy(args2, args1);

    std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
    va_end(args1);

    vsnprintf(buf.data(), buf.size() + 1, f, args2);
    va_end(args2);

    return buf;
}

template < typename T >
struct Tester {
    Tester() = default;

    size_t size() const {
        bool te = tested.empty();
        size_t r = ref.size();
        size_t t = tested.size();
        if (te != !t) throw TestFailed(fmt("Size: size %zu but empty is %s.",
                                           t, te ? "true" : "false"));
        if (r != t) throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
        return r;
    }

    const T& operator [] (size_t index) const {
        const T& r = ref[index];
        const T& t = tested[index];
        if (r != t) throw TestFailed("Op [] const mismatch.");
        return t;
    }

    void assign(size_t index, T x) {
        ref[index] = x;
        tested[index] = std::move(x);
        operator[](index);
    }

    void insert(size_t i, T x, bool check_tree_ = false) {
        ref.insert(i, x);
        tested.insert(i, std::move(x));
        size();
        if (check_tree_) check_tree();
    }

    T erase(size_t i, bool check_tree_ = false) {
        T r = ref.erase(i);
        T t = tested.erase(i);
        if (r != t) throw TestFailed(fmt("Erase mismatch at %zu.", i));
        size();
        if (check_tree_) check_tree();
        return t;
    }

    void check_tree() const {
        using TI = typename Array<T>::TesterInterface;
        auto ref_it = ref.begin();
        bool check_value_failed = false;
        auto check_value = [&](const T& v) {
            if (check_value_failed) return;
            check_value_failed = (ref_it == ref.end() || *ref_it != v);
            if (!check_value_failed) ++ref_it;
        };

        size();

        check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);

        if (check_value_failed) throw TestFailed(
                    "Check tree: element mismatch");
    }

    template < typename Node, typename F >
    int check_node(const Node* n, const Node* p, F& check_value) const {
        if (!n) return -1;

        using TI = typename Array<T>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p) throw TestFailed("Parent mismatch.");
        }

        auto l_depth = check_node(TI::left(n), n, check_value);
        check_value(TI::value(n));
        auto r_depth = check_node(TI::right(n), n, check_value);

        if (config::CHECK_DEPTH && abs(l_depth - r_depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l_depth, r_depth
            ));

        return std::max(l_depth, r_depth) + 1;
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    Array<T> tested;
    Ref<T> ref;
};


void test_insert() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(15, (1 + i * 7) % 17, true);
    for (int i = 0; i < 10; i++) t.assign(2*i, 3*t[2*i]);
    for (size_t i = 0; i < t.size(); i++) t[i];
}

void test_erase() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);

    for (size_t i = 3; i < t.size(); i += 2) t.erase(i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(3, (1 + i * 7) % 17, true);
    for (size_t i = 1; i < t.size(); i += 3) t.erase(i, true);

    for (int i = 0; i < 20; i++) t.insert(3, 100 + i, true);

    for (int i = 0; i < 5; i++) t.erase(t.size() - 1, true);
    for (int i = 0; i < 5; i++) t.erase(0, true);

    for (int i = 0; i < 4; i++) t.insert(i, i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];
}

enum RandomTestFlags : unsigned {
    SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4
};

void test_random(size_t size, unsigned flags = 0) {
    Tester<size_t> t;
    std::mt19937 my_rand(24707 + size);

    bool seq = flags & SEQ;
    bool erase = !(flags & NO_ERASE);
    bool check_tree = flags & CHECK_TREE;

    for (size_t i = 0; i < size; i++) {
        size_t pos = seq ? 0 : my_rand() % (i + 1);
        t.insert(pos, my_rand() % (3*size), check_tree);
    }

    t.check_tree();

    for (size_t i = 0; i < t.size(); i++) t[i];

    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 7) {
            case 1: {
                if (!erase && i % 3 == 0) break;
                size_t pos = seq ? 0 : my_rand() % (t.size() + 1);
                t.insert(pos, my_rand() % 1'000'000, check_tree);
                break;
            }
            case 2:
                if (erase) t.erase(my_rand() % t.size(), check_tree);
                break;
            case 3:
                t.assign(my_rand() % t.size(), 155 + i);
                break;
            default:
                t[my_rand() % t.size()];
        }

    t.check_tree();
}

int main() {
    try {
        std::cout << "Insert test..." << std::endl;
        test_insert();

        std::cout << "Erase test..." << std::endl;
        test_erase();

        std::cout << "Tiny random test..." << std::endl;
        test_random(20, CHECK_TREE);

        std::cout << "Small random test..." << std::endl;
        test_random(200, CHECK_TREE);

        std::cout << "Bigger random test..." << std::endl;
        test_random(5'000);

        std::cout << "Bigger sequential test..." << std::endl;
        test_random(5'000, SEQ);

        std::cout << "All tests passed." << std::endl;
    } catch (const TestFailed& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
    }
}

#endif


