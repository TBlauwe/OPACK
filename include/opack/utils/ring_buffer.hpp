#include <vector>
#include <ranges>

#include <fmt/core.h>

template<typename T>
class RingBuffer
{
    using container = std::vector<T>;
    using iterator = typename std::vector<T>::iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

public:
    RingBuffer() : RingBuffer(1){};
    RingBuffer(std::size_t size) : m_container(size), pos{m_container.begin()}
    {}

    T& at(std::size_t n)
    {
       return m_container.at(n);
    }

    iterator begin() { return m_container.begin(); }
    const_iterator begin() const { return m_container.cbegin(); }
    iterator end() { return m_container.end(); }
    const_iterator end() const { return m_container.cend(); }
    reverse_iterator rbegin() { return m_container.rbegin(); }
    const_reverse_iterator rbegin() const { return m_container.rbegin(); }
    reverse_iterator rend() { return m_container.rend(); }
    const_reverse_iterator rend() const { return m_container.rend(); }

    template<typename... Args>
    void push(Args&&... args)
    {
        *pos = T{std::forward<Args>(args)...};
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

private:
    container m_container {};
    iterator pos {m_container.begin()};
};

template<typename T>
void print(const RingBuffer<T>& rg)
{
    fmt::print("Size : {}\n", rg.size());
    fmt::print("Capacity : {}\n", rg.capacity());
    fmt::print("Value : [");
    for (auto i = rg.rbegin(); i != rg.rend(); ++i )
    { 
        fmt::print("{} ", *i);
    } 
    fmt::print("]\n");
}

auto main() -> int
{
    auto rg = RingBuffer<int>(3);
    rg.push(1);
    print(rg);
    rg.push(2);
    print(rg);
    rg.push(3);
    print(rg);
    rg.push(4);
    print(rg);
    rg.push(5);
    print(rg);

}