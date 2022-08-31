#include <algorithm>

#include "tabu_max_clique.h"
#include "utils.h"

using namespace std;

void MaxCliqueTabuSearch::Init(const vector<unordered_set<int>>& input_neighbour_sets)
{
    neighbour_sets = input_neighbour_sets;

    non_neighbours.resize(neighbour_sets.size());
    qco.resize(neighbour_sets.size());
    index.resize(neighbour_sets.size());
    tightness.resize(neighbour_sets.size());

    for (int i = 0; i < neighbour_sets.size(); ++i)
    {
        for (int j = 0; j < neighbour_sets.size(); ++j)
        {
            if (neighbour_sets[i].count(j) == 0 && i != j)
                non_neighbours[i].insert(j);
        }
    }
}

void MaxCliqueTabuSearch::RunSearch(int iterations)
{
    auto sdlwr_order = SmallDegreeLastWithRemoveOrder(neighbour_sets);
    for (int iter = 0; iter < iterations; ++iter)
    {
        ClearClique();
        float randomization = (float)iter / iterations;
        randomization = sqrt(randomization);
        FindInitialClique(sdlwr_order, randomization);

        tabu_insert_maxsize = 3 + (iter % 5);
        tabu_remove_maxsize = 3 + tabu_insert_maxsize;
        tabu_insert.clear();
        tabu_remove.clear();

        //let's declare a counter of swaps and destroys 
        int swaps = 0;
        int destroys = 0;
        while (destroys < 2)
        {
            if (q_border > best_clique.size())
            {
                best_clique.clear();
                for (int i = 0; i < q_border; ++i)
                    best_clique.insert(qco[i]);
            }

            if (Move())
            {
                continue;
            }
            int swap_result = Swap();
            if (swap_result == 2)
            {
                continue;
            }
            else if (swap_result == 1 && swaps < 100)
            {
                ++swaps;
            }
            else
            {
                for (int i = GetRandom(2, 5); i > 0; --i)
                {
                    if (q_border > 0)
                    {
                        RemoveFromClique(qco[GetRandom(0, q_border - 1)]);
                    }
                }
                ++destroys;
                ++iter;
                swaps = 0;
                tabu_insert.clear();
                tabu_remove.clear();
            }
        }
    }
}

const unordered_set<int>& MaxCliqueTabuSearch::GetClique()
{
    return best_clique;
}

int MaxCliqueTabuSearch::GetRandom(int a, int b)
{
    uniform_int_distribution<int> uniform(a, b);
    return uniform(generator);
}
    
void MaxCliqueTabuSearch::ClearClique()
{
    q_border = 0;
    c_border = neighbour_sets.size();
    for (size_t i = 0; i < neighbour_sets.size(); ++i)
    {
        qco[i] = i;
        index[i] = i;
        tightness[i] = 0;
    }
}

void MaxCliqueTabuSearch::SwapVertices(int vertex, int border)
{
    int vertex_at_border = qco[border];
    swap(qco[index[vertex]], qco[border]);
    swap(index[vertex], index[vertex_at_border]);
}

void MaxCliqueTabuSearch::InsertToClique(int i)
{
    for (int j : non_neighbours[i])
    {
        if (tightness[j] == 0)
        {
            --c_border;
            SwapVertices(j, c_border);
        }
        ++tightness[j];
    }
    SwapVertices(i, q_border);
    ++q_border;
}

void MaxCliqueTabuSearch::RemoveFromClique(int k)
{
    for (int j : non_neighbours[k])
    {
        if (tightness[j] == 1)
        {
            SwapVertices(j, c_border);
            c_border++;
        }
        --tightness[j];
    }
    --q_border;
    SwapVertices(k, q_border);
}

vector<int> MaxCliqueTabuSearch::FindSwapCandidates(int vertex)
{
    vector<int> candidates;
    candidates.reserve(non_neighbours[vertex].size());
    for (int i : non_neighbours[vertex])
    {
        if (tightness[i] == 1)
        {
            candidates.push_back(i);
        }
    }
    return candidates;
}

vector<int> MaxCliqueTabuSearch::RandomPermutation(int size)
{
    vector<int> permutation(size);
    for (int i = 0; i < size; ++i)
    {
        permutation[i] = i;
    }
    shuffle(permutation.begin(), permutation.end(), generator);
    return permutation;
}

void MaxCliqueTabuSearch::RemoveFromCliqueWithTabu(int vertex)
{
    RemoveFromClique(vertex);
    tabu_remove.push_back(vertex);
    if (tabu_remove.size() > tabu_remove_maxsize)
    {
        tabu_remove.pop_front();
    }
}

void MaxCliqueTabuSearch::InsertToCliqueWithTabu(int vertex)
{
    InsertToClique(vertex);
    tabu_insert.push_back(vertex);
    if (tabu_insert.size() > tabu_insert_maxsize)
    {
        tabu_insert.pop_front();
    }
}

//a function that searches for swap candidates and looks for possibilities to make 1-2 or 1-1 swaps
// it also takes into accout tabu lists
int MaxCliqueTabuSearch::Swap()
{
    auto permutation = RandomPermutation(q_border);
    int swap_1_vertex = -1;
    int swap_1_candidate;
    for (int i = 0; i < q_border; ++i)
    {
        int vertex = qco[permutation[i]];
        auto swap_candidates = FindSwapCandidates(vertex);
        if (!swap_candidates.empty())
        {
            shuffle(swap_candidates.begin(), swap_candidates.end(), generator);
            for (int c1 : swap_candidates)
            {
                for (int c2 : swap_candidates)
                {
                    if (neighbour_sets[c1].count(c2))
                    {
                        RemoveFromClique(vertex);
                        InsertToClique(c1);
                        InsertToClique(c2);
                        return 2;
                    }
                }
            }
            if (swap_1_vertex == -1 && find(tabu_insert.begin(), tabu_insert.end(), vertex) == tabu_insert.end())
            {
                for (int swap_candidate : swap_candidates)
                {
                    if (find(tabu_remove.begin(), tabu_remove.end(), swap_candidate) == tabu_remove.end())
                    {
                        swap_1_vertex = vertex;
                        swap_1_candidate = swap_candidate;
                    }
                }
            }
        }
    }
    if (swap_1_vertex >= 0)
    {
        RemoveFromCliqueWithTabu(swap_1_vertex);
        InsertToCliqueWithTabu(swap_1_candidate);
        return 1;
    }
    return 0;
}

bool MaxCliqueTabuSearch::Move()
{
    if (c_border == q_border)
        return false;
    int vertex = qco[GetRandom(q_border, c_border - 1)];
    InsertToClique(vertex);
    return true;
}

void MaxCliqueTabuSearch::FindInitialClique(vector<int> candidates, float randomization)
{
    while (!candidates.empty())
    {
        int random_index = GetRandom(0, min<int>(randomization * candidates.size() + (randomization > 0 ? 1 : 0), candidates.size() - 1));
        int vertex = candidates[random_index];
        InsertToClique(vertex);
        candidates.erase(
            remove_if(
                candidates.begin(), candidates.end(),
                [this, vertex](int c) { return !neighbour_sets[vertex].count(c); }),
            candidates.end());
    }
}