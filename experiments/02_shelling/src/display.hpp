#pragma once
#include <vector>
#include <algorithm>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <opack/core.hpp>

#include "environment.hpp"
#include "identifiers.hpp"

template<size_t H, size_t W>
class GridDisplay
{
	using Grid_t = Grid<flecs::entity, H, W>;

public: 
	GridDisplay(Grid_t& grid) : grid{ grid }
	{}

	void clearConsole() {
		#ifdef _WIN32
		#include <iostream>
			static const char* CSI = "\33[";
			printf("%s%c%s%c", CSI, 'H', CSI, '2J');

		#else
		#include <unistd.h>
			write(1, "\E[H\E[2J", 7);
		#endif
	}

	void display()
	{
		// Upper-border
		fmt::print("{0: ^{1}}", "", padding + 1);
		for (int i{ 0 }; i < W; i++)
		{
			fmt::print(fmt::fg(border_index_fg) | fmt::bg(border_index_bg),
				" {0: ^{1}} ", i, padding);
		}
		fmt::print("\n");
		fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"{0: ^{1}}┌{0:─^{2}}┐\n", "", padding, W * (padding + 2));

		for (size_t h = 0; h < H; h++) {
			fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"{0: ^{1}}│{0: ^{2}}│\n", "", padding, W * (padding + 2));
			fmt::print(fmt::fg(border_index_fg) | fmt::bg(border_index_bg),"{0: >{1}}", h, padding);
			fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"│");
			for (size_t w = 0; w < W; w++) {
				auto entity = grid.cells[grid.linearize(w, h)];
				auto team = entity.get<Team>();
				if (team)
				{
					auto flag = fmt::fg(fmt::color::white);
					if (entity.has<Happy>())
						flag |= fmt::emphasis::bold;
					else
						flag |= fmt::emphasis::strikethrough;

					if (*team == Team::Red)
						fmt::print(fmt::bg(fmt::color::red) | flag, " {0: ^{1}} ", entity, padding);
					else
						fmt::print(fmt::bg(fmt::color::blue) | flag, " {0: ^{1}} ", entity, padding);
				}
				else
					fmt::print(fmt::fg(fmt::color::gray) | fmt::bg(fmt::color::white), " {0: ^{1}} ", entity, padding);
			}
			fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"│");
			fmt::print(fmt::fg(border_index_fg) | fmt::bg(border_index_bg),"{0: <{1}}\n", h, padding);
		}
		fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"{0: ^{1}}│{0: ^{2}}│\n", "", padding, W * (padding + 2));

		// Lower-border
		fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"{0: ^{1}}└{0:─^{2}}┘\n", "", padding, W * (padding + 2));
		fmt::print("{0: ^{1}}", "", padding + 1);
		for (int i{ 0 }; i < W; i++)
		{
			fmt::print(fmt::fg(border_index_fg) | fmt::bg(border_index_bg),
				" {0: ^{1}} ", i, padding);
		}
		fmt::print("\n");
	}

private:
	Grid_t& grid;
	uint8_t padding { 3 };

private:
	fmt::color border_bg{ fmt::color::black };
	fmt::color border_fg{ fmt::color::dim_gray };
	fmt::color border_index_fg{ fmt::color::white };
	fmt::color border_index_bg{ fmt::color::black };
	fmt::color cell_bg;
	fmt::color cell_fg;

};