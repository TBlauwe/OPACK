#include <doctest/doctest.h>
#include <opack/utils/ring_buffer.hpp>
#include <fmt/core.h>

template<typename T>
void print(const ring_buffer<T>& rg)
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
    ring_buffer<int> rg;
    SUBCASE("No size")
    {
        rg = ring_buffer<int>();
        CHECK(rg.size() == ring_buffer<int>::default_size);
    }

    SUBCASE("Size 1")
    {
        rg = ring_buffer<int>(1);
        CHECK(rg.size() == 1);
    }

    SUBCASE("Size 2")
    {
        rg = ring_buffer<int>(2);
        CHECK(rg.size() == 2);
    }

    SUBCASE("Size 10")
    {
        rg = ring_buffer<int>(10);
        CHECK(rg.size() == 10);
    }

    MESSAGE("Const ref iteration");
    {
        int counter{ 0 };
        for (const auto& v : rg)
        {
            rg.peek(counter);
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Ref Iteration");
    {
        int counter{ 0 };
        for (auto& v : rg)
        {
            rg.peek(counter);
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Value iteration");
    {
        int counter{ 0 };
        for (auto v : rg)
        {
            rg.peek(counter);
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Const value iteration");
    {
        int counter{ 0 };
        for (const auto v : rg)
        {
            rg.peek(counter);
            counter++;
        }
        CHECK(counter == rg.size());
    }

    MESSAGE("Const ref reverse iteration");
    {
        int counter{ 0 };
        for (auto it = rg.rbegin(); it != rg.rend() ; it++)
        {
            rg.peek(counter);
            counter++;
        }
        CHECK(counter == rg.size());
    }
}

TEST_CASE("Ring Buffer")
{
    auto rg = ring_buffer<int>(3);
    SUBCASE("0 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{0, 0, 0});
        CHECK(rg[0] == 0);
        CHECK(rg[1] == 0);
        CHECK(rg[2] == 0);
    }

    rg.emplace(1);
    SUBCASE("1 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{1, 0, 0});
        CHECK(rg.peek(0) == 1);
        CHECK(rg.peek(1) == 0);
        CHECK(rg.peek(2) == 0);
    }

    rg.emplace(2);
    SUBCASE("2 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{2, 1, 0});
        CHECK(rg.peek(0) == 2);
        CHECK(rg.peek(1) == 1);
        CHECK(rg.peek(2) == 0);
    }

    rg.emplace(3);
    SUBCASE("3 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{3, 2, 1});
        CHECK(rg.peek(0) == 3);
        CHECK(rg.peek(1) == 2);
        CHECK(rg.peek(2) == 1);
    }

    rg.emplace(4);
    SUBCASE("4 elements")
    {
        std::vector<int> vector;
        for (auto v : rg)
        {
            vector.push_back(v);
        }
        CHECK(vector == std::vector{4, 3, 2});
        CHECK(rg[0] == 4);
        CHECK(rg[1] == 3);
        CHECK(rg[2] == 2);
    }
}
