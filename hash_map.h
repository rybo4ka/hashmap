#include <iostream>
#include <list>
#include <stdexcept>
#include <vector>

template<class KeyType, class ValueType> class Cell {
    private:
        std::pair<KeyType, ValueType> key_value_;
    public:
        Cell() {}

        Cell(const std::pair<KeyType, ValueType> &item) : key_value_(item) {}

        Cell& operator =(const std::pair<KeyType, ValueType> &item) {
            key_value_ = item;
            return *this;
        }

        std::pair<KeyType, ValueType>& get() noexcept {
            return key_value_;
        }

        const std::pair<KeyType, ValueType>& get() const noexcept {
            return key_value_;
        }

        KeyType& get_key() noexcept {
            return key_value_.first;
        }

        const KeyType& get_key() const noexcept {
            return key_value_.first;
        }

        ValueType& get_value() noexcept {
            return key_value_.second;
        }

        const ValueType& get_value() const noexcept {
            return key_value_.second;
        }
};

class Links {
    private:
        int32_t left_, right_;

    public:
        Links() : left_(-1), right_(-1) {}

        int32_t get_left() const noexcept {
            return left_;
        }

        int32_t get_right() const noexcept {
            return right_;
        }

        void set_left(int32_t new_left) {
            left_ = new_left;
        }

        void set_right(int32_t new_right) {
            right_ = new_right;
        }
};

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
    private:
        static const size_t kStartSize = 128;

        std::vector<std::list<Cell<KeyType, ValueType> > > table_;
        std::vector<Links> links_;
        int32_t first_, last_;
        Hash hasher_;
        size_t map_size_, map_capacity_;

        void add(const std::pair<KeyType, ValueType> &item) {
            int32_t position = hasher_(item.first) % map_capacity_;

            for (auto &cell : table_[position]) {
                if (cell.get_key() == item.first) {
                    return;
                }
            }

            table_[position].emplace_back(item);
            map_size_++;

            if (map_size_ == 1) {
                first_ = position;
                links_[position].set_left(-1);
                links_[position].set_right(last_);
                links_[last_].set_left(position);
            } else if (static_cast<int32_t>(table_[position].size()) == 1) {
                links_[links_[last_].get_left()].set_right(position);
                links_[position].set_left(links_[last_].get_left());
                links_[position].set_right(last_);
                links_[last_].set_left(position);
            }

            if (map_size_ * 4 >= map_capacity_ * 3) {
                rebuild(true);
            }
        }

        void del(const KeyType &key) {
            int32_t position = hasher_(key) % map_capacity_;

            if (table_[position].empty()) {
                return;
            }

            auto it = table_[position].begin();

            while (it != table_[position].end()) {
                if (it->get_key() == key) {
                    table_[position].erase(it);
                    map_size_--;
                    break;
                }

                it++;
            }

            if (table_[position].empty()) {
                if (position == first_) {
                    first_ = links_[position].get_right();
                } else {
                    links_[links_[position].get_left()].set_right(links_[position].get_right());
                }
                links_[links_[position].get_right()].set_left(links_[position].get_left());
            }

            if (map_size_ * 4 <= map_capacity_) {
                rebuild(false);
            }
        }

        void rebuild(bool increase_size) {
            if (increase_size) {
                map_capacity_ *= 2;
            } else if (map_capacity_ > kStartSize) {
                map_capacity_ /= 2;
            } else {
                return;
            }

            HashMap<KeyType, ValueType, Hash> copied = *this;

            for (auto &cells_list: table_) {
                cells_list.clear();
            }

            table_.resize(map_capacity_ + 1);
            links_.resize(map_capacity_ + 1);
            first_ = last_ = map_capacity_;
            map_size_ = 0;

            for (const auto& item : copied) {
                add(item);
            }
        }

        std::pair<int32_t, std::_List_iterator<Cell<KeyType, ValueType> > > find_pos(const KeyType &key) {
            int32_t position = hasher_(key) % map_capacity_;
            auto it = table_[position].begin();

            while (it != table_[position].end()) {
                if (it->get_key() == key) {
                    return std::make_pair(position, it);
                }

                it++;
            }

            return std::make_pair(-1, table_[last_].end());
        }

        std::pair<int32_t, std::_List_const_iterator<Cell<KeyType, ValueType> > > find_pos(const KeyType &key) const {
            int32_t position = hasher_(key) % map_capacity_;
            auto it = table_[position].begin();

            while (it != table_[position].end()) {
                if (it->get_key() == key) {
                    return std::make_pair(position, it);
                }

                it++;
            }

            return std::make_pair(-1, table_[last_].end());
        }

    public:
        template<class TableType, class LinksType, class IteratorType>
        class iteratorBase {
            protected:
                TableType *table_;
                LinksType *links_;
                int32_t position_;
                IteratorType it_;

            public:
                iteratorBase() {
                    table_ = nullptr;
                    links_ = nullptr;
                    position_ = -1;
                }

                iteratorBase(const iteratorBase &other) {
                    table_ = other.table_;
                    links_ = other.links_;
                    position_ = other.position_;
                    it_ = other.it_;
                }

                iteratorBase(TableType &htable, LinksType &hlinks, int32_t hposition, IteratorType hit) {
                    table_ = &htable;
                    links_ = &hlinks;
                    position_ = hposition;
                    it_ = hit;
                }

                iteratorBase& operator ++() {
                    it_++;

                    if (it_ == (*table_)[position_].end()) {
                        position_ = (*links_)[position_].get_right();
                        it_ = (*table_)[position_].begin();
                    } 

                    return *this;
                }

                iteratorBase operator ++(int) {
                    iteratorBase prev = *this;
                    ++*this;
                    return prev;
                }

                bool operator ==(const iteratorBase &other) const noexcept {
                    return (table_ == other.table_ && position_ == other.position_ && it_ == other.it_);
                }

                bool operator !=(const iteratorBase &other) const noexcept {
                    return !(*this == other);
                }
        };

        class iterator : public iteratorBase<
        std::vector<std::list<Cell<KeyType, ValueType> > >, std::vector<Links>,
            std::_List_iterator<Cell<KeyType, ValueType> > > {
                using TableType = std::vector<std::list<Cell<KeyType, ValueType> > >;
                using LinksType = std::vector<Links>;
                using IteratorType = std::_List_iterator<Cell<KeyType, ValueType> >;

                public:
                    iterator() : iteratorBase<TableType, LinksType, IteratorType>() {}

                    iterator(TableType &htable, LinksType &hlinks, int32_t hposition, IteratorType hit) : 
                        iteratorBase<TableType, LinksType, IteratorType>(htable, hlinks, hposition, hit) {}

                    std::pair<const KeyType, ValueType>& operator *() const {
                        return reinterpret_cast<std::pair<const KeyType, ValueType>&>(
                            iteratorBase<TableType, LinksType, IteratorType>::it_->get());
                    }

                    std::pair<const KeyType, ValueType>* operator ->() const noexcept {
                        return reinterpret_cast<std::pair<const KeyType, ValueType>*>(
                            &iteratorBase<TableType, LinksType, IteratorType>::it_->get());
                    }
        };
        
        class const_iterator : public iteratorBase<
        const std::vector<std::list<Cell<KeyType, ValueType> > >, const std::vector<Links>,
            std::_List_const_iterator<Cell<KeyType, ValueType> > > {
                using TableType = const std::vector<std::list<Cell<KeyType, ValueType> > >;
                using LinksType = const std::vector<Links>;
                using IteratorType = std::_List_const_iterator<Cell<KeyType, ValueType> >;

                public:
                    const_iterator() : iteratorBase<TableType, LinksType, IteratorType>() {}

                    const_iterator(TableType &htable, LinksType &hlinks, int32_t hposition, IteratorType hit) : 
                        iteratorBase<TableType, LinksType, IteratorType>(htable, hlinks, hposition, hit) {}

                    const std::pair<const KeyType, ValueType>& operator *() const {
                        return reinterpret_cast<const std::pair<const KeyType, ValueType>&>(
                            iteratorBase<TableType, LinksType, IteratorType>::it_->get());
                    }

                    const std::pair<const KeyType, ValueType>* operator ->() const noexcept {
                        return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(
                            &iteratorBase<TableType, LinksType, IteratorType>::it_->get());
                    }
        };

        HashMap(Hash neededHasher = Hash()) : hasher_(neededHasher) {
            table_.resize(kStartSize + 1);
            links_.resize(kStartSize + 1);
            first_ = last_ = kStartSize;
            map_size_ = 0;
            map_capacity_ = kStartSize;
        }

        HashMap(const HashMap<KeyType, ValueType, Hash> &other) : hasher_(other.hasher_) {
            table_ = other.table_;
            links_ = other.links_;
            first_ = other.first_;
            last_ = other.last_;
            map_size_ = other.map_size_;
            map_capacity_ = other.map_capacity_;
        }

        template<class Iter>
        HashMap(Iter it1, Iter it2, Hash neededHasher = Hash()) {
            *this = HashMap<KeyType, ValueType, Hash>(neededHasher);
            while (it1 != it2) {
                add(*it1);
                it1++;
            }
        }

        HashMap(std::initializer_list<std::pair<KeyType, ValueType> > item_list, Hash neededHasher = Hash()) {
            *this = HashMap<KeyType, ValueType, Hash>(neededHasher);
            for (auto& item : item_list) {
                add(item);
            }
        }

        size_t size() const noexcept {
            return map_size_;
        }

        bool empty() const noexcept {
            return (map_size_ == 0);
        }

        Hash hash_function() const noexcept {
            return hasher_;
        }

        void insert(const std::pair<KeyType, ValueType> &item) {
            add(item);
        }

        void erase(const KeyType &key) {
            del(key);
        }

        iterator begin() noexcept {
            return iterator(table_, links_, first_, (first_ == -1 ? table_[last_].end() : table_[first_].begin()));
        }

        iterator end() noexcept {
            return iterator(table_, links_, last_, table_[last_].end());
        }

        const_iterator begin() const noexcept {
            return const_iterator(table_, links_, first_, (first_ == -1 ? table_[last_].end() : table_[first_].begin()));
        }

        const_iterator end() const noexcept {
            return const_iterator(table_, links_, last_, table_[last_].end());
        }

        iterator find(const KeyType &key) {
            auto position = find_pos(key);

            return (position.first == -1 ? end() : iterator(table_, links_, position.first, position.second));
        }

        const_iterator find(const KeyType &key) const {
            auto position = find_pos(key);

            return (position.first == -1 ? end() : const_iterator(table_, links_, position.first, position.second));
        }

        ValueType& operator [](const KeyType &key) {
            auto position = find_pos(key);

            if (position.first == -1) {
                auto item = std::make_pair(key, ValueType());
                add(item);
            }

            return find(key)->second;
        }

        const ValueType& at(const KeyType &key) const {
            auto position = find_pos(key);

            if (position.first == -1) {
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
