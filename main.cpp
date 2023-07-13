#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <unordered_set>

using namespace std;


// Definição da classe nó da árvore B*
class BStarNode {
public:
    vector<int> keys;
    vector<BStarNode *> children;
    bool isLeaf;

    BStarNode(bool isLeafNode) { isLeaf = isLeafNode; }
};

// Classe principal da árvore B*
class BStarTree {
private:
    BStarNode *root;
    int t; // Grau mínimo da árvore B*

public:
    BStarTree(int degree) {
        root = nullptr;
        t = degree;
    }

    void insert(int key) {
        if (root == nullptr) {
            root = new BStarNode(true);
            root->keys.push_back(key);
        } else {
            if (root->keys.size() == (2 * t) - 1) {
                BStarNode *newRoot = new BStarNode(false);
                newRoot->children.push_back(root);
                splitChild(newRoot, 0, root);
                insertNonFull(newRoot, key);
                root = newRoot;
            } else {
                insertNonFull(root, key);
            }
        }
    }

    void insertNonFull(BStarNode *node, int key) {
        int i = node->keys.size() - 1;

        if (node->isLeaf) {
            node->keys.push_back(0);

            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }

            node->keys[i + 1] = key;
        } else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }

            i++;

            if (node->children[i]->keys.size() == (2 * t) - 1) {
                splitChild(node, i, node->children[i]);

                if (key > node->keys[i]) {
                    i++;
                }
            }

            insertNonFull(node->children[i], key);
        }
    }

    void splitChild(BStarNode *parentNode, int childIndex, BStarNode *childNode) {
        BStarNode *newNode = new BStarNode(childNode->isLeaf);

        parentNode->keys.insert(parentNode->keys.begin() + childIndex,
                                childNode->keys[t - 1]);
        parentNode->children.insert(parentNode->children.begin() + childIndex + 1,
                                    newNode);

        for (int j = 0; j < t - 1; j++) {
            newNode->keys.push_back(childNode->keys[j + t]);
        }

        childNode->keys.resize(t - 1);

        if (!childNode->isLeaf) {
            for (int j = 0; j < t; j++) {
                newNode->children.push_back(childNode->children[j + t]);
            }

            childNode->children.resize(t);
        }
    }

    void remove(int key) {
        if (root == nullptr) {
            cout << "A árvore está vazia." << endl;
            return;
        }

        removeKey(root, key);

        if (root->keys.empty()) {
            if (root->isLeaf) {
                delete root;
                root = nullptr;
            } else {
                BStarNode *oldRoot = root;
                root = root->children[0];
                delete oldRoot;
            }
        }
    }

    void removeKey(BStarNode *node, int key) {
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }

        if (node->isLeaf) {
            if (i < node->keys.size() && key == node->keys[i]) {
                node->keys.erase(node->keys.begin() + i);
            } else {
                cout << "A chave " << key << " não foi encontrada na árvore." << endl;
            }
        } else {
            if (i < node->keys.size() && key == node->keys[i]) {
                removeKeyFromInternalNode(node, i);
            } else {
                bool lastChild = (i == node->keys.size());
                BStarNode *childNode = node->children[i];

                if (childNode->keys.size() == t - 1) {
                    fillChildNode(node, i);
                }

                if (lastChild && i > node->keys.size()) {
                    removeKey(node->children[i - 1], key);
                } else {
                    removeKey(node->children[i], key);
                }
            }
        }
    }

    void removeKeyFromInternalNode(BStarNode *node, int keyIndex) {
        int key = node->keys[keyIndex];
        BStarNode *predecessor = node->children[keyIndex];
        BStarNode *successor = node->children[keyIndex + 1];

        if (predecessor->keys.size() >= t) {
            int predKey = getPredecessor(predecessor);
            node->keys[keyIndex] = predKey;
            removeKey(predecessor, predKey);
        } else if (successor->keys.size() >= t) {
            int succKey = getSuccessor(successor);
            node->keys[keyIndex] = succKey;
            removeKey(successor, succKey);
        } else {
            mergeNodes(node, keyIndex);
            removeKey(predecessor, key);
        }
    }

    int getPredecessor(BStarNode *node) {
        while (!node->isLeaf) {
            node = node->children[node->children.size() - 1];
        }

        return node->keys[node->keys.size() - 1];
    }

    int getSuccessor(BStarNode *node) {
        while (!node->isLeaf) {
            node = node->children[0];
        }

        return node->keys[0];
    }

    void mergeNodes(BStarNode *parentNode, int keyIndex) {
        BStarNode *childNode = parentNode->children[keyIndex];
        BStarNode *siblingNode = parentNode->children[keyIndex + 1];

        childNode->keys.push_back(parentNode->keys[keyIndex]);

        for (int i = 0; i < siblingNode->keys.size(); i++) {
            childNode->keys.push_back(siblingNode->keys[i]);
        }

        if (!childNode->isLeaf) {
            for (int i = 0; i < siblingNode->children.size(); i++) {
                childNode->children.push_back(siblingNode->children[i]);
            }
        }

        parentNode->keys.erase(parentNode->keys.begin() + keyIndex);
        parentNode->children.erase(parentNode->children.begin() + keyIndex + 1);

        delete siblingNode;
    }

    void fillChildNode(BStarNode *parentNode, int childIndex) {
        if (childIndex != 0 &&
            parentNode->children[childIndex - 1]->keys.size() >= t) {
            borrowFromPrevious(parentNode, childIndex);
        } else if (childIndex != parentNode->keys.size() &&
                   parentNode->children[childIndex + 1]->keys.size() >= t) {
            borrowFromNext(parentNode, childIndex);
        } else {
            if (childIndex != parentNode->keys.size()) {
                mergeNodes(parentNode, childIndex);
            } else {
                mergeNodes(parentNode, childIndex - 1);
            }
        }
    }

    void borrowFromPrevious(BStarNode *parentNode, int childIndex) {
        BStarNode *childNode = parentNode->children[childIndex];
        BStarNode *siblingNode = parentNode->children[childIndex - 1];

        childNode->keys.insert(childNode->keys.begin(),
                               parentNode->keys[childIndex - 1]);

        if (!childNode->isLeaf) {
            childNode->children.insert(
                    childNode->children.begin(),
                    siblingNode->children[siblingNode->children.size() - 1]);
            siblingNode->children.pop_back();
        }

        parentNode->keys[childIndex - 1] =
                siblingNode->keys[siblingNode->keys.size() - 1];
        siblingNode->keys.pop_back();
    }

    void borrowFromNext(BStarNode *parentNode, int childIndex) {
        BStarNode *childNode = parentNode->children[childIndex];
        BStarNode *siblingNode = parentNode->children[childIndex + 1];

        childNode->keys.push_back(parentNode->keys[childIndex]);

        if (!childNode->isLeaf) {
            childNode->children.push_back(siblingNode->children[0]);
            siblingNode->children.erase(siblingNode->children.begin());
        }

        parentNode->keys[childIndex] = siblingNode->keys[0];
        siblingNode->keys.erase(siblingNode->keys.begin());
    }

    // Printar a árvore
    void traverse() {
        if (root != nullptr) {
            traverseNode(root);
        }
    }

    void traverseNode(BStarNode *node) {
        int i;
        for (i = 0; i < node->keys.size(); i++) {
            if (node->isLeaf == false) {
                traverseNode(node->children[i]);
            }
            cout << node->keys[i] << " ";
        }

        if (node->isLeaf == false) {
            traverseNode(node->children[i]);
        }
    }

    bool search(int key) {
        if (root == nullptr) {
            cout << "A árvore está vazia." << endl;
            return false;
        }

        return searchKey(root, key);
    }

    bool searchKey(BStarNode* node, int key) {
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }

        if (i < node->keys.size() && key == node->keys[i]) {
            cout << "A chave " << key << " foi encontrada na árvore." << endl;
            return true;
        } else if (node->isLeaf) {
            cout << "A chave " << key << " não foi encontrada na árvore." << endl;
            return false;
        } else {
            return searchKey(node->children[i], key);
        }
    }

};


std::vector<int> gerarNumerosAleatoriosNaoRepetidos(int quantidadeNumeros, int limiteInferior, int limiteSuperior) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(limiteInferior, limiteSuperior);

    std::unordered_set<int> numeros;

    while (numeros.size() < quantidadeNumeros) {
        int numero = distribution(generator);
        numeros.insert(numero);
    }

    return std::vector<int>(numeros.begin(), numeros.end());
}

int encontrarMaiorNumero(const std::vector<int>& numeros) {
    int maior = std::numeric_limits<int>::min(); // Define um valor inicialmente como o menor possível

    for (int numero : numeros) {
        if (numero > maior) {
            maior = numero;
        }
    }

    return maior;
}


int main() {

    BStarTree tree(3);

    vector<int> numbers = gerarNumerosAleatoriosNaoRepetidos(1000000, 0, 2000000);

    for(int i = 0; i <= 1000000; i++){
        tree.insert(numbers[i]);
    }
    int num = encontrarMaiorNumero(numbers);

    auto start = std::chrono::high_resolution_clock::now();

    tree.search(num);

    auto end = std::chrono::high_resolution_clock::now();

    auto duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    auto duration_mili = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Tempo de execucao: " << duration_nano.count() << " nanosegundo" << std::endl;


    std::cout << "Tempo de execucao: " << duration_mili.count() << " milisegundos" << std::endl;

    return 0;
}