#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <optional>

namespace opack
{
	/**
	@class IPGraph

	@brief InfluenceGraph
	*/
	template<typename U_t, typename V_t>
	class IPGraph
	{
		using Index = size_t;
		using UIndex = Index;
		using VIndex = Index;

		using Score = std::pair<VIndex, int>;

		using Influences = std::unordered_multimap<UIndex, VIndex>;
		using Preferences = std::unordered_multimap<UIndex, UIndex>;
		using Scores = std::unordered_map<VIndex, int>;

	public:
		IPGraph() = default;

		std::optional<V_t> compute()
		{
			m_highest_scores.clear();
			std::vector<Score>  sorted_scores{};
			for (auto [v_idx, score] : m_scores)
			{
				sorted_scores.emplace_back(v_idx, score);
			}

			std::sort(sorted_scores.begin(), sorted_scores.end(),
				[](const Score& a, const Score& b)
				{
					return a.second > b.second;
				}
			);

			if (sorted_scores.begin() != sorted_scores.end()) {
				int max_value = sorted_scores[0].second;
				for (const auto& [v_idx, score] : sorted_scores)
				{
					if (score == max_value)
						m_highest_scores.push_back(v_idx);
				}
			}

			if (!m_highest_scores.empty())
				return V[m_highest_scores[rand() % (num_eligibles())]]; //TODO Better random or maybe determinist ?
			return std::nullopt;
		}


		void positive_influence(U_t u, V_t v)
		{
			positive_influence_from_id(u_index(u), v);
		}

		void negative_influence(U_t u, V_t v)
		{
			negative_influence_from_id(u_index(u), v);
		}

		void influence(U_t u, V_t v, bool is_positive)
		{
			if (is_positive)
				positive_influence(u, v);
			else
				negative_influence(u, v);
		}

		void entry(V_t v)
		{
			if (size_t idx = v_index(v); !m_scores.contains(idx))
				m_scores.insert({ idx, 0 });
		}

		Influences& positive_influences()
		{
			return m_positive_influences;
		}

		Influences& negative_influences()
		{
			return m_negative_influences;
		}

		U_t u_at(UIndex idx)
		{
			return U.at(idx);
		}

		V_t v_at(VIndex idx)
		{
			return V.at(idx);
		}

		int score(V_t v)
		{
			return m_scores.at(v_index(v));
		}

		Scores& scores()
		{
			return m_scores;
		}

		bool is_highest(V_t v)
		{
			return std::find(m_highest_scores.begin(), m_highest_scores.end(), v_index(v)) != m_highest_scores.end();
		}

		size_t num_eligibles() const
		{
			return m_highest_scores.size();
		}

		std::vector<V_t>& highest_scores()
		{
			return m_highest_scores;
		}

	private:

		void positive_influence_from_id(UIndex u_idx, V_t v)
		{
			auto v_idx = v_index(v);
			m_positive_influences.insert({ u_idx, v_idx });
			if (m_scores.contains(v_idx))
				m_scores.at(v_idx) += 1;
			else
				m_scores.insert_or_assign(v_idx, 1);
		}

		void negative_influence_from_id(UIndex u_idx, V_t v)
		{
			auto v_idx = v_index(v);
			m_negative_influences.insert({ u_idx, v_idx });
			if (m_scores.contains(v_idx))
				m_scores.at(v_idx) -= 1;
			else
				m_scores.insert_or_assign(v_idx, -1);
		}

		void influence_from_id(UIndex u_idx, V_t v, bool is_positive)
		{
			if (is_positive)
				positive_influence_from_id(u_idx, v);
			else
				negative_influence_from_id(u_idx, v);
		}

		UIndex u_index(U_t u)
		{
			UIndex idx = std::distance(U.begin(), std::find(U.begin(), U.end(), u));
			if (idx >= U.size())
				U.push_back(u);
			return idx;
		}

		VIndex v_index(V_t v)
		{
			VIndex idx = std::distance(V.begin(), std::find(V.begin(), V.end(), v));
			if (idx >= V.size())
				V.push_back(v);
			return idx;
		}

	public:

		class UNode
		{
		public:
			UNode(IPGraph<U_t, V_t>& graph, U_t u) : m_graph(graph), u_index(graph.u_index(u)) {}
			UNode(IPGraph<U_t, V_t>& graph, Index u_index) : m_graph(graph), u_index(u_index) {}

			void entry(V_t v)
			{
				m_graph.entry(v);
			}

			void influence(V_t v, bool is_positive)
			{
				m_graph.influence_from_id(u_index, v, is_positive);
			}

			void positive_influence(V_t v)
			{
				m_graph.positive_influence_from_id(u_index, v);
			}

			void negative_influence(V_t v)
			{
				m_graph.negative_influence_from_id(u_index, v);
			}
		private:
			IPGraph<U_t, V_t>& m_graph;
			Index u_index;
		};
		UNode scope(U_t u)
		{
			return UNode(*this, u);
		}

	private:
		std::vector<U_t> U;
		std::vector<V_t> V;

		Influences          m_positive_influences;
		Influences          m_negative_influences;
		Preferences         m_preferences;

		// Cached computation
		Scores              m_scores;
		std::vector<Index>  m_highest_scores{};
	};
}
