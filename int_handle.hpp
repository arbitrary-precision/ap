#ifndef DEOHAYER_AP_INT_HANDLE_HPP
#define DEOHAYER_AP_INT_HANDLE_HPP

#include "asm.hpp"

namespace ap
{
namespace library
{

#define AP_CAPACITY(expr) MAX(MIN(expr, maxwords), minwords)
#define AP_ALIGN(val, a) ((((val) + a - 1) / a) * a)

template <index_t _Maxbits, index_t _Minbits = 0>
class int_handle
{
    static_assert(_Minbits <= _Maxbits, "int_handle: _Minbits must not be greater than _Maxbits.");
    static_assert(_Maxbits > 0, "int_handle: _Maxbits must be greater than 0.");

    template <index_t _MaxbitsO, index_t _MinbitsO>
    friend class int_handle;

private:
    array<word_t> words;
    wregister reg;

public:
    enum : index_t
    {
        maxbits = AP_ALIGN(_Maxbits, word_traits::bits),
        minbits = AP_ALIGN((_Minbits != 0) ? _Minbits : _Maxbits, word_traits::bits),
        maxwords = maxbits / word_traits::bits,
        minwords = minbits / word_traits::bits,
    };

    int_handle(index_t capacity = minwords)
        : words(array_alloc<word_t>(AP_CAPACITY(capacity))),
          reg(words.get(), AP_CAPACITY(capacity), 0, false)
    {
    }

    template <index_t _MaxbitsO, index_t _MinbitsO>
    int_handle(int_handle<_MaxbitsO, _MinbitsO>&& other)
        : words(std::move(other.words)),
          reg(other.reg)
    {
        this->set_capacity(AP_CAPACITY(this->get_capacity()));
    }

    int_handle(int_handle&& other)
        : words(std::move(other.words)),
          reg(other.reg)
    {
    }

    template <index_t _MaxbitsO, index_t _MinbitsO>
    int_handle(const int_handle<_MaxbitsO, _MinbitsO>& other)
        : int_handle(other.get_capacity())
    {
        rregister other_reg = other.get_rregister();
        other_reg.size = MIN(other_reg.size, this->get_capacity());
        asm_cp(other_reg, this->reg);
        this->set_sign(other_reg.sign);
    }

    int_handle(const int_handle& other)
        : int_handle(other.get_capacity())
    {
        asm_cp(other.get_rregister(), this->reg);
        this->set_sign(other.reg.sign);
    }

    template <index_t _MaxbitsO, index_t _MinbitsO>
    int_handle& operator=(int_handle<_MaxbitsO, _MinbitsO>&& other)
    {
        this->words = std::move(other.words);
        this->reg = other.reg;
        this->set_capacity(AP_CAPACITY(this->get_capacity()));
        return *this;
    }

    int_handle& operator=(int_handle&& other)
    {
        if (this != &other)
        {
            this->words = std::move(other.words);
            this->reg = other.reg;
        }
        return *this;
    }

    template <index_t _MaxbitsO, index_t _MinbitsO>
    int_handle& operator=(const int_handle<_MaxbitsO, _MinbitsO>& other)
    {
        this->set_capacity(other.get_capacity());
        rregister other_reg = other.get_rregister();
        other_reg.size = MIN(other_reg.size, this->get_capacity());
        asm_cp(other_reg, this->reg);
        this->set_sign(other_reg.sign);
        return *this;
    }

    int_handle& operator=(const int_handle& other)
    {
        if (this != &other)
        {
            this->set_capacity(other.get_capacity());
            asm_cp(other.get_rregister(), this->reg);
            this->set_sign(other.reg.sign);
        }
        return *this;
    }

    rregister get_rregister() const
    {
        return rregister(this->reg);
    }

    wregister& get_wregister()
    {
        return this->reg;
    }

    index_t get_size() const
    {
        return this->reg.size;
    }

    void set_size(index_t size)
    {
        this->reg.size = size;
    }

    bool get_sign() const
    {
        return this->reg.sign;
    }

    void set_sign(bool sign)
    {
        this->reg.sign = sign;
    }

    index_t get_capacity() const
    {
        return this->reg.capacity;
    }

    void set_capacity(index_t capacity)
    {
        capacity = AP_CAPACITY(capacity);
        if (this->reg.capacity != capacity)
        {
            this->reg.capacity = capacity;
            this->reg.size = MIN(this->reg.capacity, this->reg.size);
            this->reg.words = array_realloc<word_t>(this->words, this->reg.capacity).get();
        }
    }
};

#undef AP_ALIGN
#undef AP_CAPACITY

} // namespace library
} // namespace ap

#endif
