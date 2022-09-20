#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <optional>

namespace opack 
{
    /**
    A score is an integer associated with a object of type @c T.
    @tparam T Type of the object associated with a score.
    */
    template<typename V_t>
    using Score = std::pair<V_t, int>;

    /**
    @class InfluenceGraph

    @brief InfluenceGraph
    */
    template<typename U_t, typename V_t>
    class InfluenceGraph
    {
        using Influences    = std::unordered_multimap<U_t, V_t>;
        using Scores        = std::unordered_map<V_t, int>;

    public:
        InfluenceGraph() : m_positive_influences(), m_negative_influences(), m_scores(), highest_scores()
    	{};

		std::optional<V_t> compute()
		{
            highest_scores.clear();
            std::vector<Score<V_t>>  sorted_scores {};
            for (auto[v, score] : m_scores)
            {
                sorted_scores.emplace_back(v, score );
            }

            std::sort(sorted_scores.begin(), sorted_scores.end(),
                [](const Score<V_t>& a, const Score<V_t>& b)
                {
                    return a.second > b.second;
                }
            );

            if (sorted_scores.begin() != sorted_scores.end()) {
                int max_value = sorted_scores[0].second;
                for (const auto& [v, score] : sorted_scores)
                {
                    if (score == max_value)
                        highest_scores.push_back(v);
                }
            }

            if(!highest_scores.empty())
                return highest_scores[rand() % (num_eligibles())]; //TODO Better random or maybe determinist ?
            return std::nullopt;
		}


        void positive_influence(U_t u, V_t v)
        {
            m_positive_influences.insert({ u, v });
            if(m_scores.contains(v))
                m_scores.at(v) += 1;
            else
                m_scores.insert_or_assign(v, 1);
        }

        void negative_influence(U_t u, V_t v)
        {
            m_negative_influences.insert({ u, v });
            if(m_scores.contains(v))
                m_scores.at(v) -= 1;
            else
                m_scores.insert_or_assign(v, 1);
        }

        void entry(V_t v)
        {
            if(!m_scores.contains(v))
                m_scores.insert({ v, 0 });
        }

        Influences& positive_influences()
        {
            return m_positive_influences;
        }

        Influences& negative_influences()
        {
            return m_negative_influences;
        }

        int score(V_t v) const
        {
			return m_scores.at(v);
        }

        Scores& get_scores()
        {
            return m_scores;
        }

        bool is_highest(V_t v) const
        {
            return highest_scores.contains(v);
        }

        size_t num_eligibles() const
        {
            return highest_scores.size();
        }

        std::vector<V_t>& eligibles()
        {
            return highest_scores;
        }

    private:

        Influences          m_positive_influences;
        Influences          m_negative_influences;

        // Cached computation
        Scores              m_scores;
        std::vector<V_t>    highest_scores {};
    };
}
