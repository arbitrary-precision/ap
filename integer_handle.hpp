#ifndef DEOHAYER_AP_INT_HANDLE_HPP
#define DEOHAYER_AP_INT_HANDLE_HPP

#include "asm.hpp"

namespace ap
{
namespace library
{

#define AP_ALIGN(val, a) ((((val) + a - 1) / a) * a)

template <index_t _Bitwidth>
class integer_handle
{
    template <index_t _BitwidthO>
    friend class integer_handle;

public:
    enum : index_t
    {
        bitwidth = AP_ALIGN((_Bitwidth != 0) ? _Bitwidth : (index_t{1} << (index_traits::bits / 2)) - 1, word_traits::bits),
        wordwidth = bitwidth / word_traits::bits
    };

private:
    array<word_t> words;
    wregister reg;

    void normalize()
    {
        if (this->get_capacity() != this->wordwidth)
        {
            this->reg.capacity = this->wordwidth;
            array_realloc(this->words, this->get_capacity());
            this->reg.words = this->words.get();
            this->reg.size = AP_MIN(this->reg.capacity, this->reg.size);
        }
        asm_trim(this->reg);
        if (this->reg.size == 0)
        {
            this->reg.sign = 0;
        }
    }

public:
    integer_handle()
        : words(array_alloc<word_t>(wordwidth)),
          reg(words.get(), wordwidth, 0, false)
    {
    }

    template <index_t _BitwidthO>
    integer_handle(integer_handle<_BitwidthO>&& other)
        : words(std::move(other.words)),
          reg(other.reg)
    {
        this->normalize();
    }

    integer_handle(integer_handle&& other)
        : words(std::move(other.words)),
          reg(other.reg)
    {
    }

    template <index_t _BitwidthO>
    integer_handle(const integer_handle<_BitwidthO>& other) : integer_handle()
    {
        rregister other_reg = other.get_rregister();
        other_reg.size = AP_MIN(other_reg.size, this->get_capacity());
        asm_cp(other_reg, this->reg);
        this->set_sign(other_reg.sign);
        this->normalize();
    }

    integer_handle(const integer_handle& other) : integer_handle()
    {
        asm_cp(other.get_rregister(), this->reg);
        this->set_sign(other.reg.sign);
    }

    template <index_t _BitwidthO>
    integer_handle& operator=(integer_handle<_BitwidthO>&& other)
    {
        this->words = std::move(other.words);
        this->reg = other.reg;
        this->normalize();
        return *this;
    }

    integer_handle& operator=(integer_handle&& other)
    {
        if (this != &other)
        {
            this->words = std::move(other.words);
            this->reg = other.reg;
        }
        return *this;
    }

    template <index_t _BitwidthO>
    integer_handle& operator=(const integer_handle<_BitwidthO>& other)
    {
        rregister other_reg = other.get_rregister();
        other_reg.size = AP_MIN(other_reg.size, this->get_capacity());
        asm_cp(other_reg, this->reg);
        this->set_sign(other_reg.sign);
        this->normalize();
        return *this;
    }

    integer_handle& operator=(const integer_handle& other)
    {
        if (this != &other)
        {
            asm_cp(other.get_rregister(), this->reg);
            this->set_sign(other.get_sign());
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
};

#undef AP_ALIGN

} // namespace library
} // namespace ap

#endif
