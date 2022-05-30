#pragma once

#include <functional>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace dynamo
{
    /**
    An influence is either a positive or negative influence between a behaviour to an object @c T.
    @tparam T Type of the object influenced.
    */
    template<typename T>
    struct Influence
    {
        const T *   object;
        bool        positive;
    };

    /**
    A score is an integer associated with a object of type @c T.
    @tparam T Type of the object associated with a score.
    */
    template<typename T>
    struct Score
    {
        size_t  index;
        int     value;
    };

    /**
    @class InfluenceGraph

    @brief InfluenceGraph
    */
    template<typename T>
    class InfluenceGraph
    {
        using Behaviour_t   = Behaviour<std::vector<Influence<T>>, std::vector<T>>;
        using Influences    = std::unordered_multimap<size_t, size_t>;
        using Scores        = std::unordered_map<size_t, int>;

    public:
        InfluenceGraph() = default;
        //InfluenceGraph(const InfluenceGraph& that)
        //    :_behaviours{that._behaviours}, _values { that._values },
        //    _positive_influences{that._positive_influences},
        //    _negative_influences{that._negative_influences},
        //    highest_scores{that.highest_scores}
        //{}

        //InfluenceGraph& operator=(const InfluenceGraph& that)
        //{
        //    _behaviours = that._behaviours ;
        //    _values = that._values ;
        //    _positive_influences = that._positive_influences;
        //    _negative_influences = that._negative_influences;
        //    highest_scores = that.highest_scores;
        //    return *this;
        //}

        InfluenceGraph(AgentHandle agent, const std::vector<const Behaviour_t *>& behaviours, std::vector<T> args)
            :_behaviours{behaviours}, _values { args }
        {
            for (size_t i = 0; i < _values.size(); i++)
            {
                scores.emplace(std::make_pair(i, 0));
            }

            size_t behaviour_index = 0;
            for (Behaviour_t const* const behaviour : _behaviours)
            {
                for (const auto& influence : (*behaviour)(agent, _values))
                {
                    size_t object_index = index(*influence.object);
                    if (influence.positive)
                    {
                        _positive_influences.emplace(behaviour_index, object_index);
                        scores[object_index] += 1;
                    }
                    else
                    {
                        _negative_influences.emplace(behaviour_index, object_index);
                        scores[object_index] -= 1;
                    }
                }
                behaviour_index++;
            }

            std::vector<Score<T>>   sorted_scores {};
            for (const auto [index, score] : scores)
            {
                sorted_scores.emplace_back(index, score);
            }
            std::sort(sorted_scores.begin(), sorted_scores.end(),
                [](const Score<T>& a, const Score<T>& b)
                {
                    return a.value > b.value;
                }
            );

            if (sorted_scores.begin() != sorted_scores.end()) {
                int max_value = sorted_scores[0].value;
                for (const auto& score : sorted_scores)
                {
                    if (score.value == max_value)
                        highest_scores.push_back(score.index);
                }
            }

            selected = highest_scores[rand() % (num_eligibles())];
        }

        Influences& positive_influences()
        {
            return _positive_influences;
        }

        Influences& negative_influences()
        {
            return _negative_influences;
        }

        const Behaviour_t * behaviour(const size_t idx)
        {
            return _behaviours.at(idx);
        }

        std::vector<const Behaviour_t *>& behaviours()
        {
            return _behaviours;
        }

        T& value(const size_t idx)
        {
            return _values.at(idx);
        }

        std::vector<T>& values()
        {
            return _values;
        }

        bool is_highest(const size_t index) const
        {
            return std::find(highest_scores.begin(), highest_scores.end(), index) != highest_scores.end();
        }

        size_t num_eligibles() const
        {
            return highest_scores.size();
        }

        std::vector<size_t>& eligibles()
        {
            return highest_scores;
        }

        size_t result_index() const
        {
            return selected;
        }

        T result()
        {
            return value(selected);
        }

        size_t index(const T& object)
        {
            auto it = std::find(_values.begin(), _values.end(), object);
            if (it != _values.end())
            {
                return it - _values.begin();
            }
            else {
                return -1;
            }
        }

    private:
        Influences      _positive_influences {};
        Influences      _negative_influences {};
        Scores          scores {};
        size_t          selected { 0 };

        std::vector<const Behaviour_t *>    _behaviours {};
        std::vector<T>                      _values {};
        std::vector<size_t>                 highest_scores {};
    };
}
