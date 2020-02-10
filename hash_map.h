#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>

/**
 * Используем разрешение коллизий методом цепочек.
 * Структура таблицы выглядит следующим образом: есть std::list, который содержит пары
 * "ключ, значение", их мы в него добавляем как попало (поле storage). Также есть вектор
 * размера capacity из списков, каждый элемент которых - это итератор списка storage (поле table).
 * Соответственно, когда мы хотим добавить элемент, мы кладем его в storage, а указывающий на него
 * (то есть на конец storage) итератор добавляем в соответствующий ключу список в table.
 * Кроме того, когда размер таблицы становится больше capacity * alpha, мы выполняем rehashing.
*/

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
private:
    std::list<std::pair<const KeyType, ValueType>> storage;
    Hash hasher;
    int inv_alpha = 2;
    int capacity;
    std::vector<std::list<typename std::list<std::pair<const KeyType, ValueType>>::iterator>> table;
    int numElements = 0;

    int applyHash(KeyType obj) const {
        return hasher(obj) % capacity;
    }

    void initializeTable(int _capacity = 16) {
        capacity = _capacity;
        table = std::vector<std::list<typename
                std::list<std::pair<const KeyType, ValueType>>::iterator>>(capacity);
    }

    // Функция, которая вызывается, когда мы хотим добавить элемент и точно знаем, что
    // его ключ в таблице раньше не встречался.
    void push_back(std::pair<KeyType, ValueType> obj, int hashed) {
        storage.push_back(obj);
        auto it = storage.end();
        --it;
        table[hashed].push_back(it);
    }

    // Выполняем rehashing
    void update() {
        if (numElements * inv_alpha < capacity)
            return;

        std::list<std::pair<KeyType, ValueType>> temp_list;
        // Просто присвоить temp_list = storage нельзя, потому что возникают проблемы с
        // const int -> int, приходится вот так
        for (auto &p : storage) {
            temp_list.push_back(std::make_pair(p.first, p.second));
        }

        storage.clear();
        table.clear();
        initializeTable(capacity * inv_alpha);
        numElements = 0;
        for (auto &el : temp_list) {
            insert(el);
        }
        temp_list.clear();
    }


public:
    using iterator = typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using const_iterator = typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;


    explicit HashMap(Hash hasherObj = Hash()) : hasher(hasherObj) {
        initializeTable();
    }

    // Конструкторы не по умолчанию выполняются просто чередой инсертов.
    template<typename _ForwardIterator>
    HashMap(_ForwardIterator begin, _ForwardIterator end, Hash hasherObj =
            Hash()) : hasher(hasherObj){
        initializeTable();
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list,
            Hash hasherObj = Hash()) : hasher(hasherObj) {
        initializeTable();
        for (auto &el : list) {
            insert(el);
        }
    }

    // Проверям, есть ли в таблице такой ключ, если да, то ничего не делаем, иначе
    // добавляем и делаем rehashing, если надо.
    void insert(std::pair<KeyType, ValueType> obj) {
        int hashed = applyHash(obj.first);
        for (auto &el : table[hashed]) {
            if (el->first == obj.first)
                return;
        }

        push_back(obj, hashed);
        ++numElements;
        update();
    }

    void erase(KeyType toDel) {
        int hashed = applyHash(toDel);
        auto iter = table[hashed].begin();
        for (; iter != table[hashed].end(); ++iter) {
            if ((*iter)->first == toDel) {
                storage.erase(*iter);
                table[hashed].erase(iter);
                --numElements;
                return;
            }
        }

    }

    // Нужно для прохождения внутренних тестов.
    HashMap& operator = (HashMap const &other) {

        //Анти-самоприсваивание
        if (this == &other)
            return *this;

        clear();
        hasher = other.hash_function();
        initializeTable(other.capacity);
        for (auto &el : other) {
            insert(el);
        }
        return *this;
    }

    iterator find(KeyType key) {
        int hashed = applyHash(key);
        for (auto &el : table[hashed]) {
            if (el->first == key)
                return iterator(el);
        }

        return end();
    }

    const_iterator find(KeyType key) const {
        int hashed = applyHash(key);
        for (auto &el : table[hashed]) {
            if (el->first == key)
                return el;
        }

        return end();
    }

    ValueType& operator[](KeyType key) {
        auto iter = find(key);
        if (iter == end()) {
            // Если не нашли элемент, то вставляем значение по умолчанию
            insert({key, ValueType()});
            iter = find(key);
        }
        return iter->second;
    }

    const ValueType& at(KeyType key) const {
        auto iter = find(key);
        if (iter == end())
            throw std::out_of_range("no such element");
        return iter->second;
    }

    void clear() {
        for (auto &p : storage) {
            int hashed = applyHash(p.first);
            table[hashed].clear();
        }
        storage.clear();
        numElements = 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    int size() const {
        return numElements;
    }

    bool empty() const {
        return !numElements;
    }

    iterator begin() {
        return iterator(storage.begin());
    }
    iterator end() {
        return iterator(storage.end());
    }
    const_iterator begin() const {
        return storage.begin();
    }
    const_iterator end() const {
        return storage.end();
    }

    ~HashMap() {
        clear();
    }
};