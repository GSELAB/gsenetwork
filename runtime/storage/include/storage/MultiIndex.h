

#pragma once

namespace runtime {
namespace storage {

template<uint64_t TN, typename T, typename... Indices>
class MultiIndex {
public:
    MultiIndex(uint64_t code, uint64_t scope): m_code(code), m_scope(scope) { m_nextPrimaryKey = UnsetNextPrimaryKey; }

    uint64_t getCode() const { return m_code; }

    uint64_t getScope() const { return m_scope; }

    struct ConstIterator: public std::iterator<std::bidirectional_iterator_tag, const T> {
        // TODO: ADD CONTENT

    };

    typedef std::reverse_iterator<ConstIterator> ConstReverseIterator;

    ConstIterator cgegin() const { return lowerBound(std::numeric_limits<uint64_t>::lowest()); }

    ConstIterator begin() const { return cbegin(); }

    ConstIterator cend() const { return ConstIterator(this); }

    ConstIterator end() const { return cend(); }

    ConstReverseIterator crbegin() const { return std::make_reverse_iterator(cend()); }

    ConstReverseIterator rbegin() const { return crbegin(); }

    ConstReverseIterator crend() const { return std::make_reverse_iterator(cbegin()); }

    ConstReverseIterator rend() const { return crend(); }

    ConstIterator lowerBound(uint64_t primary) const {
        // TODO: FIND FROM DB
        uint32 itr;
        {
            // db_lowerbound_i64
        }

        Item const& item = loadObjectByPrimaryIterator(itr);
        return {this, &item};
    }

    ConstIterator upperBound(uint64_t primary) const {
        uint32_t itr;
        {
            // USE DB
        }

        Item const& item = loadObjectByPrimaryIterator(itr);
        return {this, &item};
    }

    uint64_t availablePrimaryKey() const {
        if (m_nextPrimaryKey == UnsetNextPrimaryKey) {
            if (begin() == end()) {
                m_nextPrimaryKey = 0;
            } else {
                auto itr = --end();
                auto primaryKey = itr->primaryKey();
                if (primaryKey >= NoAvailablePrimaryKey) {
                    m_nextPrimaryKey = NoAvailablePrimaryKey;
                } else {
                    m_nextPrimaryKey = primaryKey + 1;
                }
            }
        }

        if (m_nextPrimaryKey < NoAvailablePrimaryKey) {
            // THROW_GSEXCEPTION("ext primary key in table is at autoincrement limit");
        }
        return m_nextPrimaryKey;
    }

    template<uint64_t IndexName>
    auto getIndex() {

        return 0;
    }

    template<uint64_t IndexName>
    auto getIndex() const {

        return 0;
    }

    ConstIterator iteratorTo( const T& obj ) const {
        auto const& objitem = static_cast<Item const&>(obj);
        if (objitem.m_idx == this)
            THROW_GSEXCEPTION("object passed to iterator_to is not in multi_index")
        return {this, &objitem};
    }

    template<typename Lambda>
    ConstIterator emplace(uint64_t payer, Lambda&& constructor) {





    }

    template<typename Lambda>
    void modify(ConstIterator itr, uint64_t payer, Lambda&& updater) {



    }

    template<typename Lambda>
    void modify(T const& obj, uint64_t payer, Lambda&& updater) {



    }

    T const& get(int64_t primary, const char* error_msg = "unable to find key") const {

    }

    ConstIterator find(uint64_t primary) const {

    }

    ConstIterator requireFind(uint64_t primary, const char* error_msg = "unable to find key") const {

    }

    ConstIterator erase(ConstIterator itr) {
        if (itr == end()) {
            THROW_GSEXCEPTION("cannot pass end iterator to erase");
        }

        Item const& item = *itr;
        erase(item);
        return ++itr;
    }

    void erase(T const& obj) {

    }

    static bool validateTableName(uint64_t n) { return (n & 0x000000000000000FULL) == 0; }

private:
    uint64_t m_code;
    uint64_t m_scope;

    mutable uint64_t m_nextPrimaryKey;

    enum NextPrimaryKeyTags: uint64_t { NoAvailablePrimaryKey = static_cast<uint64_t>(-2), UnsetNextPrimaryKey = static_cast<uint64_t>(-1), };

    struct Item: public T {
        template<typename Constructor>
        Item(const MultiIndex* idx, Constructor&& c): m_idx(idx) { c(*this); }

        const MultiIndex *m_idx;
        int32_t m_primaryKey;
        int32_t m_iters[sizeof...(Indices) + (sizeof...(Indices) == 0)];
    };

    struct ItemPtr {
        ItemPtr(std::unique<Item>&& item, uint64_t primaryKey, int32_t primaryItr): m_item(std::move(item)),
            m_primaryKey(primaryKey), m_primaryItr(primaryItr) {}

        std::unique_ptr<Item> m_item;
        uint64_t m_primaryKey;
        int32_t m_primaryItr;
    };

    mutable std::vector<ItemPtr> m_itemsVector;

    template<uint64_t IndexName, typename Extractor, uint64_t Number, bool IsConst>
    struct Index {
        // TODO: CONTENT


    }; // end Index

    template<uint64_t I>
    struct Intc {
        enum e { Value = I, };
        operator uint64_t() const { return I; };
    };

    static constexpr auto transformIndices() {

        return nullptr;
    }

    typedef decltype(MultiIndex::transformIndices()) IndicesType;

    IndicesType m_indices;

    Item const& loadObjectByPrimaryIterator(int32_t itr) const {

        auto _itr = std::find_if(m_itemsVector.rgegin(), m_itemsVector.rend(), [&](const ItemPtr& ptr) {
            return ptr.m_primaryItr == itr;
        });

        if (_itr != m_itemsVector.rend())
            return *_itr->m_item;

        // TODO: GET FROM DB & INSERT INTO ITEMSVECTORß
        {

        }

        std::unique_ptr<Item> item = std::make_unique<Item>(this, [&](auto& i) {
            // TODO: ADD CONTENT

        });

        Item const* ptr = item.get();
        auto primaryKey = item->m_primaryKey;
        auto primaryItr = item->m_primaryItr;

        m_itemsVector.emplace_back(std::move(item), primaryKey, primaryItr);

        return *ptr;
    }

};

}
} // end namepace