#include <unordered_map>

#include "utils.h"
using namespace std;

tuple<int, vector<int>> GreedyGraphColoring(const vector<unordered_set<int>>& neighbour_sets, const vector<int>& vertices_order)
{
    int maxcolor = 0;
    vector<int> colors(neighbour_sets.size(), 0);
    for (int vertex : vertices_order)
    {
        vector<bool> colors_used(maxcolor + 1, false);

        for (int neighbour : neighbour_sets[vertex])
        {
            colors_used[colors[neighbour]] = true;
        }

        int color = 1;
        for (color; color < colors_used.size(); ++color)
        {
            if (!colors_used[color])
            {
                colors[vertex] = color;
                break;
            }
        }
        if (color == maxcolor + 1)
        {
            colors[vertex] = ++maxcolor;
        }
    }
    return make_tuple(maxcolor, colors);
}

vector<int> PardalosOrder(const vector<unordered_set<int>>& neighbour_sets)
{
    unordered_map<int, unordered_set<int>> graph_cut;
    for (int i = 0; i < neighbour_sets.size(); ++i)
    {
        graph_cut[i] = neighbour_sets[i];
    }

    vector<int> vertices;
    // everytime we remove a vertex we need to change our graph, i.e. to remove edges leading to that vertex
    while (!graph_cut.empty())
    {
        int min_degree_vertex = graph_cut.begin()->first;
        for (const auto& [vertex, neighbours] : graph_cut)
        {
            if (neighbours.size() < graph_cut[min_degree_vertex].size())
            {
                min_degree_vertex = vertex;
            }
        }
        vertices.push_back(min_degree_vertex);
        for (int neighbour : graph_cut[min_degree_vertex])
        {
            graph_cut[neighbour].erase(min_degree_vertex);
        }
        graph_cut.erase(min_degree_vertex);
    }

    return vertices;
}

vector<int> SmallDegreeLastWithRemoveOrder(const vector<unordered_set<int>>& neighbour_sets)
{
    auto vertices = PardalosOrder(neighbour_sets);
    reverse(vertices.begin(), vertices.end());
    return vertices;
}
    