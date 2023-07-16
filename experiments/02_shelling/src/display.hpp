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
	GridDisplay(flecs::world& world, Grid_t& grid) : world{ world }, grid { grid }
	{}

	void clear() 
	{
		#if defined _WIN32
    system("cls");
    //clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
    //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
    system("clear");
#endif
	}

	void display_stats()
	{
		fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"─{0:─^{1}}─\n", "Stats", W * (padding + 3));
		fmt::print("{0: >15} : {1}\n", "Tick", world.get_tick());
		fmt::print("{0: >15} : {1}%\n", "Percent similar", world.get<GlobalStats>()->percent_similar);
		fmt::print("{0: >15} : {1}%\n", "Percent unhappy", world.get<GlobalStats>()->percent_unhappy);
		fmt::print("{0: >15} : {1}\n", "Total agents", world.count(flecs::id(flecs::IsA, opack::entity<Agent>(world))));
		fmt::print("{0: >15} : {1}\n", "Happy agents", world.count<Happy>());
	}

	void display_grid()
	{
		fmt::print(fmt::fg(border_fg) | fmt::bg(border_bg),"─{0:─^{1}}─\n", "Grid", W * (padding + 3));
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
				auto team = entity.template get<Team>();
				if (team)
				{
					auto flag = fmt::fg(fmt::color::white);
					if (entity.template has<Happy>())
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
	flecs::world& world;
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