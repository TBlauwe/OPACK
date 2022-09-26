#include <cassert>
#include <vector>

template<typename T>
class ring_buffer
{
    template<typename, bool> struct iterator_t; // Forward declaration

public:
    using container = std::vector<T>;
    using container_iterator = typename std::vector<T>::iterator;
    using iterator = iterator_t<T, false>;
    using reverse_iterator = iterator_t<T, true>;
    using const_iterator = iterator_t<const T, false>;
    using const_reverse_iterator = iterator_t<const T, true>;
    static constexpr std::size_t default_size = 1;

    ring_buffer() : ring_buffer(default_size){}
    ring_buffer(std::size_t size) : m_container(size)
    {}

    /**
     * Return last @c n th element s. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value.
     * Assert if @c n is superior or equal to @c size().
     */
    T& peek(std::size_t n = 0)
    {
        assert(n >= 0 && n < size());
        auto it = begin();
        std::advance(it, n);
        return *it;
    }

    /**
     * Return last @c n th element s. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value.
     * Assert if @c n is superior or equal to @c size().
     */
    const T& peek(std::size_t n = 0) const
    {
        assert(n >= 0 && n < size());
        auto it = begin();
        std::advance(it, n);
        return *it;
    }

    /** True if contains @c value, false otherwise. */
    bool contains(const T& value) const
    {
        return std::find(m_container.begin(), m_container.end(), value) != m_container.end();
    }


    template<typename... Args>
    void push(Args&&... args)
    {
        *pos = T{std::forward<Args>(args)...};
        last = pos;
        std::advance(pos, 1);
        if(pos == m_container.end())
            pos = m_container.begin();
    }

    std::size_t size() const
    {
        return m_container.size();
    }

    std::size_t capacity() const
    {
        return m_container.capacity();
    }

          T& operator[](std::size_t idx)       { return peek(idx); }
    const T& operator[](std::size_t idx) const { return peek(idx); }

    iterator begin() { return iterator(*this, static_cast<T*>(& *last)); }
    const_iterator begin() const { return const_iterator(*this, static_cast<const T*>(& *last)); }
    iterator end() { return iterator(*this, nullptr); }
    const_iterator end() const { return const_iterator(*this, nullptr); }

    reverse_iterator rbegin() { return reverse_iterator(*this, static_cast<T*>(& *pos)); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(*this, static_cast<const T*>(& *pos)); }
    reverse_iterator rend() { return reverse_iterator(*this, nullptr); }
    const_reverse_iterator rend() const { return const_reverse_iterator(*this, nullptr); }


private:

    container m_container {};
    container_iterator pos {m_container.begin()};
    container_iterator last {std::prev(m_container.end())};

    template<typename TValue, bool is_reverse>
    struct iterator_t
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = TValue;
        using pointer           = value_type*; 
        using reference         = value_type&;
        using container         = const ring_buffer<std::remove_const_t<value_type>>;

        iterator_t(container& rg, pointer ptr) : m_rg{ rg }, m_ptr { ptr }{}

        reference operator*() const { return *m_ptr; }
        pointer operator->() { return m_ptr; }

        // Prefix increment
        iterator_t& operator++() 
        { 
            if constexpr (is_reverse)
                return backward();
            else
                return forward();
        }  

        iterator_t& operator--() 
        { 
            if constexpr (is_reverse)
                return forward();
            else
                return backward();
        }  

        iterator_t& forward()
        {
            if (m_ptr == &*m_rg.pos)
                m_ptr = nullptr;
            else if (m_ptr == &*m_rg.m_container.begin())
            {
                if constexpr (std::is_const_v<value_type>)
                    m_ptr = &*m_rg.m_container.rbegin();
                else
                    m_ptr = const_cast<value_type*>(&*m_rg.m_container.rbegin());
            }
            else
                m_ptr--; 
            return *this; 
        }

        iterator_t& backward()
        {
            if (m_ptr == &*m_rg.last)
                m_ptr = nullptr;
            else if (m_ptr == &*m_rg.m_container.rbegin())
            {
                if constexpr (std::is_const_v<value_type>)
                    m_ptr = &*m_rg.m_container.begin();
                else
                    m_ptr = const_cast<value_type*>(&*m_rg.m_container.begin());
            }
            else
                m_ptr++; 
            return *this; 
        }

        iterator_t operator++(int) { iterator_t tmp = *this; ++(*this); return tmp; }
        iterator_t operator--(int) { iterator_t tmp = *this; --(*this); return tmp; }

        friend bool operator== (const iterator_t& a, const iterator_t& b) { return a.m_ptr == b.m_ptr; }
        friend bool operator!= (const iterator_t& a, const iterator_t& b) { return a.m_ptr != b.m_ptr; }

    private:
        container& m_rg;
        pointer m_ptr;
    };
};
