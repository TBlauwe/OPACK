#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>

namespace opack 
{
    /**
    An influence is either a positive or negative influence towards an object @c T.
    @tparam T Type of the object influenced.
    */
    template<typename T>
    struct Influence
    {
        const T *   object;
        bool        is_positive;
    };

    /**
    A score is an integer associated with a object of type @c T.
    @tparam T Type of the object associated with a score.
    */
    template<typename T>
    struct Score
    {
        const T *   key;
        int         value;
    };

    /**
    @class InfluenceGraph

    @brief InfluenceGraph
    */
    template<typename U, typename V>
    class InfluenceGraph
    {
        using Influences    = std::unordered_multimap<const U *, const V *>;
        using Scores        = std::unordered_map<const V *, int>;

    public:
        InfluenceGraph() = default;

		const V * compute()
		{
            highest_scores.clear();
            std::vector<Score<V>>  sorted_scores {};
            for (const auto& pair : scores)
            {
                sorted_scores.push_back({ pair.first, pair.second });
            }

            std::sort(sorted_scores.begin(), sorted_scores.end(),
                [](const Score<V>& a, const Score<V>& b)
                {
                    return a.value > b.value;
                }
            );

            if (sorted_scores.begin() != sorted_scores.end()) {
                int max_value = sorted_scores[0].value;
                for (const auto& score : sorted_scores)
                {
                    if (score.value == max_value)
                        highest_scores.push_back(score.key);
                }
            }

            if(highest_scores.size())
                return highest_scores[rand() % (num_eligibles())];

            return nullptr;
		}


        void positive_influence(const U& u, const V& v)
        {
            _positive_influences.emplace(&u, &v);
            if(scores.find(&v) != scores.end())
                scores[&v] += 1;
            else
                scores.emplace(&v, 1);
        }

        void negative_influence(const U& u, const V& v)
        {
            _negative_influences.emplace(&u, &v);
            if(scores.find(&v) != scores.end())
                scores[&v] -= 1;
            else
                scores.emplace(&v, -1);
        }

        Influences& positive_influences()
        {
            return _positive_influences;
        }

        Influences& negative_influences()
        {
            return _negative_influences;
        }

        int score(const V& v) const
        {
            return scores.at(&v);
        }

        bool is_highest(const V* v) const
        {
            return std::find(highest_scores.begin(), highest_scores.end(), v) != highest_scores.end();
        }

        size_t num_eligibles() const
        {
            return highest_scores.size();
        }

        std::vector<const V *>& eligibles()
        {
            return highest_scores;
        }

    private:
        Influences      _positive_influences {};
        Influences      _negative_influences {};
        Scores          scores {};

        std::vector<const V *>                 highest_scores {};
    };
}
