#pragma once
#include <vector>
#include <unordered_set>
#include <tuple>

std::tuple<int, std::vector<int>> GreedyGraphColoring(const std::vector<std::unordered_set<int>>& neighbour_sets, const std::vector<int>& vertices_order);

std::vector<int> PardalosOrder(const std::vector<std::unordered_set<int>>& neighbour_sets);

std::vector<int> SmallDegreeLastWithRemoveOrder(const std::vector<std::unordered_set<int>>& neighbour_sets);
