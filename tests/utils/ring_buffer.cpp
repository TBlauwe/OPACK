#include <doctest/doctest.h>
#include <opack/utils/ring_buffer.hpp>
#include <fmt/core.h>

template<typename T>
void print(const RingBuffer<T>& rg)
{
    fmt::print("Size : {}\n", rg.size());
    fmt::print("Capacity : {}\n", rg.capacity());
    fmt::print("Value : [");
    for (auto v : rg)
    { 
        fmt::print("{} ", v);
    } 
    fmt::print("]\n");
}

TEST_CASE("Ring Buffer validity")
{
    RingBuffer<int> rg;
    SUBCASE("No size")
    {
        rg = RingBuffer<int>();
        CHECK(rg.size() == RingBuffer<int>::default_size);
    }

    SUBCASE("Size 1")
    {
        rg = RingBuffer<int>(1);
        CHECK(rg.size() == 1);
    }

    SUBCASE("Size 2")
    {
        rg = RingBuffer<int>(2);
        CHECK(rg.size() == 2);
    }

    SUBCASE("Size 10")
    {
        rg = RingBuffer<int>(10);
        CHECK(rg.size() == 10);
    }

    MESSAGE("Const ref iteration");
    {
        int counter{ 0 };
        for (const auto& v : rg)
        {
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Ref Iteration");
    {
        int counter{ 0 };
        for (auto& v : rg)
        {
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Value iteration");
    {
        int counter{ 0 };
        for (auto v : rg)
        {
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Const value iteration");
    {
        int counter{ 0 };
        for (const auto v : rg)
        {
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Const ref reverse iteration");
    {
        int counter{ 0 };
        for (auto it = rg.rbegin(); it != rg.rend() ; it++)
        {
            counter++;
        }
        CHECK(counter == rg.size());
    }
}

TEST_CASE("Ring Buffer")
{
    auto rg = RingBuffer<int>(3);
    SUBCASE("0 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{0, 0, 0});
    }

    rg.push(1);
    SUBCASE("1 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{1, 0, 0});
    }

    rg.push(2);
    SUBCASE("2 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{2, 1, 0});
    }

    rg.push(3);
    SUBCASE("3 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{3, 2, 1});
    }

    rg.push(4);
    SUBCASE("4 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{4, 3, 2});
    }
}
