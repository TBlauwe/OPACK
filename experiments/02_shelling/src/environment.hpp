#pragma once
#include <functional>

struct Position
{
	size_t w;
	size_t h;
};

template<typename T, size_t H, size_t W>
struct Grid
{

	static constexpr size_t height() { return H; }
	static constexpr size_t weight() { return W; }
	static constexpr size_t size() { return H * W; }

	static constexpr size_t linearize(const size_t w, const size_t h) { return h * W + w; }
	static constexpr size_t linearize(const Position pos) { return pos.h * W + pos.w; }

	static constexpr Position n_neighbour(const Position pos) { return { pos.w % W , (pos.h - 1) % H }; }
	static constexpr Position s_neighbour(const Position pos) { return { pos.w % W , (pos.h + 1) % H }; }
	static constexpr Position o_neighbour(const Position pos) { return { (pos.w - 1) % W ,  pos.h }; }
	static constexpr Position e_neighbour(const Position pos) { return { (pos.w + 1) % W ,  pos.h }; }

	static constexpr Position ne_neighbour(const Position pos) { return { (pos.w + 1) % W , (pos.h - 1) % H }; }
	static constexpr Position no_neighbour(const Position pos) { return { (pos.w - 1) % W , (pos.h - 1) % H }; }
	static constexpr Position se_neighbour(const Position pos) { return { (pos.w + 1) % W , (pos.h + 1) % H }; }
	static constexpr Position so_neighbour(const Position pos) { return { (pos.w - 1) % W , (pos.h + 1) % H }; }

	Grid() = default;
	Grid(std::function<T(const size_t w, const size_t h)> builder)
	{
		for (size_t w = 0; w < W; w++) {
			for (size_t h = 0; h < H; h++) {
				cells[linearize(w, h)] = builder(w, h);
			}
		}
	}

	constexpr void set(const size_t w, const size_t h, T value) {
		cells[linearize(w, h)] = value;
	}

	constexpr const T operator()(const size_t w, const size_t h) {
		return cells[linearize(w, h)];
	}

	constexpr std::array<T, 8> neighbours(const Position pos)
	{
		return std::array<T, 8>{
			cells[linearize(n_neighbour(pos))],
				cells[linearize(s_neighbour(pos))],
				cells[linearize(e_neighbour(pos))],
				cells[linearize(o_neighbour(pos))],
				cells[linearize(ne_neighbour(pos))],
				cells[linearize(no_neighbour(pos))],
				cells[linearize(se_neighbour(pos))],
				cells[linearize(so_neighbour(pos))]
		};
	}

	std::array<T, H* W> cells;
};

