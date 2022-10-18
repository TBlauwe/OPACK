#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <effolkronium/random.hpp>

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

		using Influences = std::unordered_map<UIndex, std::unordered_set<VIndex>>;
		using Preferences = std::unordered_map<UIndex, std::unordered_set<UIndex>>;
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
				return V[*effolkronium::random_static::get(m_highest_scores)];
			return std::nullopt;
		}


		void positive_influence(const U_t u, const V_t v)
		{
			positive_influence_from_id(u_index(u), v);
		}

		void negative_influence(const U_t u, const V_t v)
		{
			negative_influence_from_id(u_index(u), v);
		}

		void influence(const U_t u, const V_t v, bool is_positive)
		{
			if (is_positive)
				positive_influence(u, v);
			else
				negative_influence(u, v);
		}

		void entry(const V_t v)
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

		U_t& u_at(const UIndex idx)
		{
			return U.at(idx);
		}

		V_t& v_at(const VIndex idx)
		{
			return V.at(idx);
		}

		int score(const V_t v)
		{
			return score_at_index(v_index(v));
		}

		int score_at_index(const VIndex idx)
		{
			return m_scores.at(idx);
		}

		Scores& scores()
		{
			return m_scores;
		}

		bool is_highest(const V_t v)
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

		void print(std::function<const char *(const U_t&)> f,  std::function<const char *(const V_t&)>g)
		{
			std::unordered_set<VIndex> displayed;
			for (auto& [u_idx, v_idxs] : positive_influences())
			{
				for (auto& v_idx : v_idxs)
				{
					displayed.insert(v_idx);
					fmt::print("{:>30} --- + ---> {:<30} [{}]\n", f(u_at(u_idx)), g(v_at(v_idx)), score_at_index(v_idx));
				}
			}
			for (auto& [u_idx, v_idxs] : negative_influences())
			{
				for (auto& v_idx : v_idxs)
				{
					displayed.insert(v_idx);
					fmt::print("{:>30} --- - ---> {:<30} [{}]\n", f(u_at(u_idx)), g(v_at(v_idx)), score_at_index(v_idx));
				}
			}
			for (auto& [v_idx, score] : scores())
			{
				if(!displayed.contains(v_idx))
					fmt::print("{:>30}            {:<30} [{}]\n", "", g(v_at(v_idx)), score);
			}
		}

	private:

		void positive_influence_from_id(const UIndex u_idx, const V_t v)
		{
			auto v_idx = v_index(v);
			auto [_, added] = m_positive_influences.try_emplace(u_idx).first->second.emplace(v_idx);
			if (added)
				++m_scores.try_emplace(v_idx).first->second;
		}

		void negative_influence_from_id(const UIndex u_idx, const V_t v)
		{
			auto v_idx = v_index(v);
			auto [_, added] = m_negative_influences.try_emplace(u_idx).first->second.emplace(v_idx);
			if (added)
				--m_scores.try_emplace(v_idx).first->second;
		}

		void influence_from_id(const UIndex u_idx, const V_t v, bool is_positive)
		{
			if (is_positive)
				positive_influence_from_id(u_idx, v);
			else
				negative_influence_from_id(u_idx, v);
		}

		UIndex u_index(const U_t u)
		{
			UIndex idx = std::distance(U.begin(), std::find(U.begin(), U.end(), u));
			if (idx >= U.size())
				U.push_back(u);
			return idx;
		}

		VIndex v_index(const V_t v)
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

			void entry(const V_t v)
			{
				m_graph.entry(v);
			}

			void influence(const V_t v, bool is_positive)
			{
				m_graph.influence_from_id(u_index, v, is_positive);
			}

			void positive_influence(const V_t v)
			{
				m_graph.positive_influence_from_id(u_index, v);
			}

			void negative_influence(const V_t v)
			{
				m_graph.negative_influence_from_id(u_index, v);
			}

			IPGraph<U_t, V_t>& global_graph() { return m_graph; };
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
