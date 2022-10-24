/*****************************************************************//**
 * \file   ring_buffer.hpp
 * \brief A ring buffer is a data structure that uses a single, fixed-size buffer as
 * if it were connected end-to-end. This structure lends itself easily
 * to buffering data streams (wikipedia).
 * 
 * \author Tristan
 * \date   October 2022
 *********************************************************************/
#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

/**
 * @brief A ring buffer is a data structure that uses a single, fixed-size buffer as
 * if it were connected end-to-end. This structure lends itself easily
 * to buffering data streams (wikipedia).
 *
 * @tparam T Must be default constructible.
 k*
 * Usage :
 * @code{.cpp}
 ring_buffer<int> rg (10); // A ring buffer of size 10 initialized with 10 default T elements.
 rg.peek();                // returns last added element : 0.
 rg.emplace(1);            // Push one element. 
 rg.peek();                // returns last added element : 1.
 rg.emplace(2);            // Push one element. 
 rg.peek();                // returns last added element : 2.
 rg.peek(1);                // returns last added element : 1.
 * @endcode
 **/
template<typename T>
requires std::is_default_constructible_v<T>
class ring_buffer
{
public:
    template<typename, bool> struct iterator_t; // Forward declaration

    using              container    = std::vector<T>;
    using     container_iterator    = typename std::vector<T>::iterator;
    using               iterator    = iterator_t<T, false>;
    using       reverse_iterator    = iterator_t<T, true>;
    using         const_iterator    = iterator_t<const T, false>;
    using const_reverse_iterator    = iterator_t<const T, true>;

    explicit ring_buffer(std::size_t size = 1) : m_container(size), m_size(size)
    {
        assert(size > 0);
    }

    /**
     * Return last @c n th element s. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value.
     * Assert if @c n is superior or equal to @c size().
     */
    [[nodiscard]] T& peek(const std::size_t n = 0)
    {
        assert(n < size());
        auto it = begin();
        std::advance(it, n);
        return *it;
    }

    /**
     * Return last @c n th element s. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value.
     * Assert if @c n is superior or equal to @c size().
     */
    [[nodiscard]] const T& peek(const std::size_t n = 0) const
    {
        assert(n < size());
        auto it = cbegin();
        std::advance(it, n);
        return *it;
    }

    /** True if contains @c value, false otherwise. */
    [[nodiscard]] bool contains(const T& value) const
    {
        auto begin = m_container.begin();
        auto end = m_container.end();
        while(begin != end)
        {
	        if(*begin == value)
                return true;
            std::advance(begin, 1);
        }
        return false;
    }

    void push(T val)
    {
        m_last = m_pos;
        m_container[m_pos] = val;
        m_pos++;
        if(m_pos == m_size)
            m_pos = 0;
    }

    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_last = m_pos;
        m_container[m_pos] = T{std::forward<Args>(args)...};
        m_pos++;
        if(m_pos == m_size)
            m_pos = 0;
    }

    [[nodiscard]] std::size_t size() const
    {
        return m_size;
    }

          T& operator[](const std::size_t idx)       { return peek(idx); }
    const T& operator[](const std::size_t idx) const { return peek(idx); }

          iterator  begin()         { return iterator(*this,  m_last); }
    const_iterator  begin() const   { return const_iterator(*this, m_last); }
    const_iterator  cbegin() const  { return const_iterator(*this, m_last); }
	      iterator  end()           { return iterator(*this, m_size); }
    const_iterator  end() const     { return const_iterator(*this, m_size); }
    const_iterator  cend() const    { return const_iterator(*this, m_size); }

          reverse_iterator  rbegin()        { return reverse_iterator(*this, m_pos); }
    const_reverse_iterator  rbegin() const  { return const_reverse_iterator(*this, m_pos); }
    const_reverse_iterator  crbegin() const { return const_reverse_iterator(*this, m_pos); }
          reverse_iterator  rend()          { return reverse_iterator(*this, m_size); }
    const_reverse_iterator  rend() const    { return const_reverse_iterator(*this, m_size); }
    const_reverse_iterator  crend() const   { return const_reverse_iterator(*this, m_size); }


private:

	container m_container;
    std::size_t m_size;
    std::size_t m_pos {0};
    std::size_t m_last {m_size - 1};

public:
    template<typename TValue, bool Reverse>
    struct iterator_t
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = TValue;
        using pointer           = value_type*; 
        using reference         = value_type&;
        using container         = const ring_buffer<std::remove_const_t<value_type>>&;

        iterator_t(container rg, std::size_t idx) : m_rg{ rg }, m_idx { idx }{}

        reference operator*() const { return const_cast<reference>(m_rg.m_container[m_idx]); }
        pointer operator->() { return &m_rg.m_container[m_idx]; }

        // Prefix increment
        iterator_t& operator++() 
        { 
            if constexpr (Reverse)
                return backward();
            else
                return forward();
        }  

        iterator_t& operator--() 
        { 
            if constexpr (Reverse)
                return forward();
            else
                return backward();
        }  

        // Postfix increment
        iterator_t operator++(int) { iterator_t tmp = *this; ++(*this); return tmp; }
        iterator_t operator--(int) { iterator_t tmp = *this; --(*this); return tmp; }

        friend bool operator== (const iterator_t& a, const iterator_t& b) { return a.m_idx == b.m_idx; }
        friend bool operator!= (const iterator_t& a, const iterator_t& b) { return a.m_idx != b.m_idx; }

    private:

        iterator_t& forward()
        {
            if (m_idx == m_rg.m_pos)
                m_idx = m_rg.m_size;
            else if (m_idx == 0)
            	m_idx = m_rg.m_size - 1 ;
            else
        		--m_idx; 
            return *this; 
        }

        iterator_t& backward()
        {
            if (m_idx == m_rg.m_last)
                m_idx = m_rg.m_size;
            else if (m_idx == m_rg.m_size - 1)
            	m_idx = 0;
            else
        		++m_idx; 
            return *this; 
        }

        container m_rg;
    	std::size_t m_idx;
    };
};
