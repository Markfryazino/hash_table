#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>

/**
 * We handle collisions by chain method.
 * Table structure is the following: there is an std::list containing 'key, value' pairs, which are added
 * there in arbitrary order (that is storage_ field). Apart from that, there is a vector of lists, each element of those
 * being an iterator of storage_ (table_ field). So, in order to add a new element we push it in storage_, while
 * an iterator pointing on this new element is put in one of the lists from table_. Additionally, we perform rehashing when
 * the table size exceeds capacity_ * alpha.
 */

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    using iterator = typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using const_iterator = typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;

private:
    using iterator_vector = typename std::vector<std::list<iterator>>;
    std::list<std::pair<const KeyType, ValueType>> storage_;
    Hash hasher_;
    const int32_t kInvAlpha = 2;
    int32_t capacity_;
    std::vector<std::list<iterator>> table_;
    int32_t num_elements_ = 0;

    int32_t ApplyHash(const KeyType obj) const {
        return hasher_(obj) % capacity_;
    }

    void InitializeTable(const int32_t capacity = 16) {
        capacity_ = capacity;
        table_ = iterator_vector(capacity_);
    }

    // Method that is called when a new element, guaranteed not be in the table, is added.
    void add_to_storage(const std::pair<KeyType, ValueType> obj) {
        int32_t hashed = ApplyHash(obj.first);
        storage_.push_back(obj);
        auto it = std::prev(storage_.end());
        table_[hashed].push_back(it);
    }

    // Performing rehashing
    void try_to_rehash() {
        if (num_elements_ * kInvAlpha < capacity_)
            return;

        table_.clear();
        capacity_ *= kInvAlpha;
        InitializeTable(capacity_);
        for (auto iter = storage_.begin(); iter != storage_.end(); ++iter) {
            table_[ApplyHash(iter->first)].push_back(iter);
        }
    }


public:
    explicit HashMap(Hash hasher_obj = Hash()) : hasher_(hasher_obj) {
        InitializeTable();
    }

    // Constructors are implemented as a bunch of insertions.
    template<typename _ForwardIterator>
    HashMap(_ForwardIterator begin, _ForwardIterator end, Hash hasher_obj =
    Hash()) : HashMap(hasher_obj){
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list,
            Hash hasher_obj = Hash()) : HashMap(list.begin(), list.end(), hasher_obj) {}

    // Check if table contains the key and do nothing if it does, or add it.
    iterator insert(std::pair<KeyType, ValueType> obj) {
        iterator iter = find(obj.first);
        if (iter != end())
            return iter;

        add_to_storage(obj);
        ++num_elements_;
        try_to_rehash();
        return std::prev(end());
    }

    void erase(KeyType to_delete) {
        int32_t hashed = ApplyHash(to_delete);
        for (auto iter = table_[hashed].begin(); iter != table_[hashed].end(); ++iter) {
            if ((*iter)->first == to_delete) {
                storage_.erase(*iter);
                table_[hashed].erase(iter);
                --num_elements_;
                return;
            }
        }

    }

    // Is required for internal tests.
    HashMap& operator= (const HashMap &other) {

        //Anti-self-assignment.
        if (this == &other)
            return *this;

        clear();
        hasher_ = other.hash_function();
        InitializeTable(other.capacity_);
        for (auto &el : other) {
            insert(el);
        }
        return *this;
    }

    iterator find(KeyType key) {
        int32_t hashed = ApplyHash(key);
        for (auto &el : table_[hashed]) {
            if (el->first == key)
                return iterator(el);
        }

        return end();
    }

    const_iterator find(KeyType key) const {
        int32_t hashed = ApplyHash(key);
        for (auto &el : table_[hashed]) {
            if (el->first == key)
                return el;
        }

        return end();
    }

    ValueType& operator[](KeyType key) {
        auto iter = find(key);
        if (iter == end()) {
            // Insert default value if an element was not found.
            iter = insert({key, ValueType()});
        }
        return iter->second;
    }

    const ValueType& at(KeyType key) const {
        const_iterator iter = find(key);
        if (iter == end())
            throw std::out_of_range("no such element");
        return iter->second;
    }

    void clear() {
        storage_.clear();
        table_.clear();
        InitializeTable();
        num_elements_ = 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    int32_t size() const {
        return num_elements_;
    }

    bool empty() const {
        return num_elements_ == 0;
    }

    iterator begin() {
        return iterator(storage_.begin());
    }
    iterator end() {
        return iterator(storage_.end());
    }
    const_iterator begin() const {
        return storage_.begin();
    }
    const_iterator end() const {
        return storage_.end();
    }

    ~HashMap() {
        clear();
    }
};