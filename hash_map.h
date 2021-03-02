#include <iostream>
#include <stdexcept>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
    private:
        std::vector<std::pair<std::pair<KeyType, ValueType>, bool> > table;
        std::vector<int32_t> left, right;
        std::vector<bool> is_dead;
        int32_t first, last;
        Hash hasher;
        size_t sz, used;

        void add(const std::pair<KeyType, ValueType> &item) {
            int32_t addPos = -1;
            int32_t position = hasher(item.first) % table.size();

            while (table[position].second) {
                if (table[position].first.first == item.first) {
                    if (is_dead[position]) {
                        addPos = position;
                        is_dead[position] = false;
                        break;
                    } else {
                        return;
                    }
                }
                position = (position + 1 == static_cast<int32_t>(table.size()) ? 0 : position + 1);
            }

            if (addPos == -1) {
                addPos = position;
                used++;
            }

            table[addPos].first = item;
            table[addPos].second = true;
            sz++;

            if (sz == 1) {
                first = addPos;
                left[addPos] = -1;
            } else {
                right[left[last]] = addPos;
                left[addPos] = left[last];
            }
            right[addPos] = last;
            left[last] = addPos;

            if (used * 4 >= table.size() * 3) {
                rebuild(true);
            }
        }

        void del(const KeyType &key) {
            int32_t position = hasher(key) % table.size();

            while (table[position].second) {
                if (table[position].first.first == key) {
                    if (is_dead[position]) {
                        return;
                    }

                    is_dead[position] = true;
                    sz--;

                    if (position == first) {
                        first = right[position];
                    } else {
                        right[left[position]] = right[position];
                    }
                    left[right[position]] = left[position];

                    if (used * 2 >= table.size() && used - sz >= sz) {
                        rebuild(false);
                    }

                    return;
                }
                position = (position + 1 == static_cast<int32_t>(table.size()) ? 0 : position + 1);
            }
        }

        void rebuild(bool needResize) {
            HashMap<KeyType, ValueType, Hash> copied = *this;

            for (auto &item : table) {
                item.second = false;
            }
            fill(is_dead.begin(), is_dead.end(), false);
            first = last = table.size();
            sz = used = 0;

            if (needResize) {
                left.resize(table.size() * 2 + 1);
                right.resize(table.size() * 2 + 1);
                is_dead.resize(table.size() * 2);
                table.resize(table.size() * 2);
                first *= 2;
                last *= 2;
            }

            for (const auto& item : copied) {
                add(item);
            }
        }

        int32_t find_pos(const KeyType &key) const {
            int32_t position = hasher(key) % table.size();

            while (table[position].second) {
                if (table[position].first.first == key) {
                    return (is_dead[position] ? -1 : position);
                }
                position = (position + 1 == static_cast<int32_t>(table.size()) ? 0 : position + 1);
            }

            return -1;
        }

    public:
        class iterator {
            private:
                std::vector<std::pair<std::pair<KeyType, ValueType>, bool> > *table;
                std::vector<int32_t> *left, *right;
                int32_t position;

            public:
                iterator() {
                    table = nullptr;
                    left = nullptr;
                    right = nullptr;
                    position = -1;
                }

                iterator(const iterator &other) {
                    table = other.table;
                    left = other.left;
                    right = other.right;
                    position = other.position;
                }

                iterator(std::vector<std::pair<std::pair<KeyType, ValueType>, bool> > &htable, 
                    std::vector<int32_t>& hleft, std::vector<int32_t>& hright, int32_t hposition) {
                    table = &htable;
                    left = &hleft;
                    right = &hright;
                    position = hposition;
                }

                iterator& operator ++() {
                    position = (*right)[position];
                    return *this;
                }

                iterator operator ++(int) {
                    iterator prev = *this;
                    position = (*right)[position];
                    return prev;
                }

                std::pair<const KeyType, ValueType>& operator *() const {
                    return reinterpret_cast<std::pair<const KeyType, ValueType>&>((*table)[position].first);
                }

                std::pair<const KeyType, ValueType>* operator ->() const noexcept {
                    return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&((*table)[position].first));
                }

                bool operator ==(const iterator &other) const noexcept {
                    return (table == other.table && position == other.position);
                }

                bool operator !=(const iterator &other) const noexcept {
                    return !(*this == other);
                }

                ~iterator() {

                }
        };

        class const_iterator {
            private:
                const std::vector<std::pair<std::pair<KeyType, ValueType>, bool> > *table;
                const std::vector<int32_t> *left, *right;
                int32_t position;

            public:
                const_iterator() {
                    table = nullptr;
                    left = nullptr;
                    right = nullptr;
                    position = -1;
                }

                const_iterator(const const_iterator &other) {
                    table = other.table;
                    left = other.left;
                    right = other.right;
                    position = other.position;
                }

                const_iterator(const std::vector<std::pair<std::pair<KeyType, ValueType>, bool> > &htable, 
                    const std::vector<int32_t>& hleft, const std::vector<int32_t>& hright, int32_t hposition) {
                    table = &htable;
                    left = &hleft;
                    right = &hright;
                    position = hposition;
                }

                const_iterator& operator ++() {
                    position = (*right)[position];
                    return *this;
                }

                const_iterator operator ++(int) {
                    const_iterator prev = *this;
                    position = (*right)[position];
                    return prev;
                }

                const std::pair<const KeyType, ValueType>& operator *() const {
                    return reinterpret_cast<const std::pair<const KeyType, ValueType>&>((*table)[position].first);
                }

                const std::pair<const KeyType, ValueType>* operator ->() const noexcept {
                    return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&((*table)[position].first));
                }

                bool operator ==(const const_iterator &other) const noexcept {
                    return (table == other.table && position == other.position);
                }

                bool operator !=(const const_iterator &other) const noexcept {
                    return !(*this == other);
                }

                ~const_iterator() {

                }
        };

        static const size_t START_SIZE = 128;

        HashMap(Hash neededHasher = Hash()) : hasher(neededHasher) {
            table.resize(START_SIZE);
            left.resize(START_SIZE + 1);
            right.resize(START_SIZE + 1);
            is_dead.resize(START_SIZE);
            first = last = START_SIZE;
            sz = used = 0;
        }

        HashMap(const HashMap<KeyType, ValueType, Hash> &other) : hasher(other.hasher) {
            table = other.table;
            left = other.left;
            right = other.right;
            is_dead = other.is_dead;
            first = other.first;
            last = other.last;
            sz = other.sz;
            used = other.used;
        }

        template<class Iter>
        HashMap(Iter it1, Iter it2, Hash neededHasher = Hash()) {
            *this = HashMap<KeyType, ValueType, Hash>(neededHasher);
            while (it1 != it2) {
                add(*it1);
                it1++;
            }
        }

        HashMap(std::initializer_list<std::pair<KeyType, ValueType> > l, Hash neededHasher = Hash()) {
            *this = HashMap<KeyType, ValueType, Hash>(neededHasher);
            for (auto& item : l) {
                add(item);
            }
        }

        size_t size() const noexcept {
            return sz;
        }

        bool empty() const noexcept {
            return (sz == 0);
        }

        Hash hash_function() const noexcept {
            return hasher;
        }

        void insert(const std::pair<KeyType, ValueType> &item) {
            add(item);
        }

        void erase(const KeyType &key) {
            del(key);
        }

        iterator begin() noexcept {
            return iterator(table, left, right, first);
        }

        iterator end() noexcept {
            return iterator(table, left, right, last);
        }

        const_iterator begin() const noexcept {
            return const_iterator(table, left, right, first);
        }

        const_iterator end() const noexcept {
            return const_iterator(table, left, right, last);
        }

        iterator find(const KeyType &key) {
            int32_t position = find_pos(key);

            return (position == -1 ? end() : iterator(table, left, right, position));
        }

        const_iterator find(const KeyType &key) const {
            int32_t position = find_pos(key);

            return (position == -1 ? end() : const_iterator(table, left, right, position));
        }

        ValueType& operator [](const KeyType &key) {
            int32_t position = find_pos(key);

            if (position == -1) {
                auto item = std::make_pair(key, ValueType());
                add(item);
            }

            return find(key)->second;
        }

        const ValueType& at(const KeyType &key) const {
            int32_t position = find_pos(key);

            if (position == -1) {
                throw(std::out_of_range("there is no such key!\n"));
            }

            return find(key)->second;
        }

        void clear() {
            HashMap<KeyType, ValueType, Hash> copied = *this;

            for (const auto& item : copied) {
                del(item.first);
            }
        }
};