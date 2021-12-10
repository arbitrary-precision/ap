#ifndef DEOHAYER_AP_CORE_HPP
#define DEOHAYER_AP_CORE_HPP

#include "config.hpp"
#include <climits> // CHAR_BIT

#define AP_MIN(a, b) (((a) < (b)) ? (a) : (b))

#define AP_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define AP_LLU(a) static_cast<unsigned long long>(a)

#define AP_UNUSED(v) (void)v

namespace ap
{
namespace library
{

using word_t = AP_WORD;
using dword_t = AP_DWORD;
using index_t = AP_SIZE;

template <typename T>
struct integer_traits
{
    enum : index_t
    {
        bytes = sizeof(T),
        bits = sizeof(T) * CHAR_BIT
    };

    enum : T
    {
        zeros = 0,
        ones = T(~zeros),
        nmsb = T(ones >> 1),
        msb = T(~nmsb)
    };
};

using word_traits = integer_traits<word_t>;
using dword_traits = integer_traits<dword_t>;
using index_traits = integer_traits<index_t>;

// Data register.
template <typename _Words>
struct dregister
{
public:
    _Words words;
    index_t capacity : index_traits::bits / 2 - 1;
    index_t size : index_traits::bits / 2 - 1;
    index_t sign : 1;

private:
    index_t __unused : 1;

public:
    dregister() : dregister(nullptr, 0, 0, false) {}

    dregister(_Words _words, index_t _capacity, index_t _size, index_t _sign)
        : words(_words),
          capacity(_capacity),
          size(_size),
          sign(_sign) {}

    dregister(const dregister&) = default;
    dregister& operator=(const dregister&) = default;

    dregister(dregister&&) = default;
    dregister& operator=(dregister&&) = default;

    explicit operator dregister<const word_t*>() const
    {
        return dregister<const word_t*>{
            this->words,
            this->capacity,
            this->size,
            this->sign};
    }

    bool has_msb() const
    {
        return (this->capacity == this->size) && (this->words[this->capacity - 1] >> (word_traits::bits - 1));
    }

    void clear_msb()
    {
        this->words[this->capacity - 1] &= (word_traits::ones >> 1);
    }
};

using rregister = dregister<const word_t*>;
using wregister = dregister<word_t*>;

// Flag register.
struct fregister
{
protected:
    index_t value;

public:
    constexpr fregister() : fregister(0) {}
    constexpr fregister(index_t _value) : value(_value) {}

    constexpr fregister(const fregister& other) = default;
    fregister& operator=(const fregister& other) = default;

    constexpr fregister(fregister&& other) = default;
    fregister& operator=(fregister&& other) = default;

    constexpr explicit operator index_t() const
    {
        return this->value;
    }

    constexpr fregister operator|(fregister other) const
    {
        return fregister{this->value | other.value};
    }

    constexpr fregister operator&(fregister other) const
    {
        return fregister{this->value & other.value};
    }

    constexpr fregister operator^(fregister other) const
    {
        return fregister{this->value ^ other.value};
    }

    fregister& operator|=(fregister other)
    {
        this->value |= other.value;
        return (*this);
    }

    fregister& operator&=(fregister other)
    {
        this->value &= other.value;
        return (*this);
    }

    fregister& operator^=(fregister other)
    {
        this->value ^= other.value;
        return (*this);
    }

    constexpr fregister operator~() const
    {
        return fregister{~(this->value)};
    }

    constexpr bool operator==(const fregister other) const
    {
        return this->value == other.value;
    }

    constexpr bool operator!=(const fregister other) const
    {
        return this->value != other.value;
    }

    fregister& flip(fregister other)
    {
        return (*this) ^= other;
    }

    fregister& set(fregister other)
    {
        return (*this) |= other;
    }

    fregister& unset(fregister other)
    {
        return (*this) &= ~other;
    }

    constexpr bool has_any(fregister flags) const
    {
        return ((*this) & flags).value > 0;
    }

    constexpr bool has_all(fregister flags) const
    {
        return ((*this) & flags) == flags;
    }

    void clear()
    {
        this->value = 0;
    }

    constexpr bool empty() const
    {
        return this->value == 0;
    }

    enum : index_t
    {
        noneflag = 0,
        infinity = 1 << 0,
        overflow = 1 << 1
    };
};

} // namespace library
} // namespace ap

#endif
