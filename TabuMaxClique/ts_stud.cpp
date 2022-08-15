#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
using namespace std;


class MaxCliqueTabuSearch
{
public:
    int GetRandom(int a, int b)
    {
        uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(string filename)
    {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }

            stringstream line_input(line);
            char command;
            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
                non_neighbours.resize(vertices);
                qco.resize(vertices);
                index.resize(vertices, -1);
                tightness.resize(vertices);  
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
        for (int i = 0; i < vertices; ++i)
        {
            for (int j = 0; j < vertices; ++j)
            {
                if (neighbour_sets[i].count(j) == 0 && i != j)
                    non_neighbours[i].insert(j);
            }
        }
    }

    void RunSearch(int iterations)
    {
        auto ldwr_order = LastDegreeWithRemoveOrder();
        for (int iter = 0; iter < iterations; ++iter)
        {
            ClearClique();
            float randomization = (float)iter / iterations;
            randomization = sqrt(randomization);
            FindInitialClique(ldwr_order, randomization);

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

    const unordered_set<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not clique\n";
                    return false;
                }
            }
        }
        return true;
    }

private:
    vector<unordered_set<int>> neighbour_sets;
    vector<unordered_set<int>> non_neighbours;
    unordered_set<int> best_clique;
    vector<int> qco;
    vector<int> index;
    vector<int> tightness;
    int q_border;
    int c_border;
    mt19937 generator;
    list<int> tabu_insert;
    list<int> tabu_remove;
    int tabu_insert_maxsize;
    int tabu_remove_maxsize;

    void ClearClique()
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

    void SwapVertices(int vertex, int border)
    {
        int vertex_at_border = qco[border];
        swap(qco[index[vertex]], qco[border]);
        swap(index[vertex], index[vertex_at_border]);
    }

    void InsertToClique(int i)
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

    void RemoveFromClique(int k)
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

    vector<int> FindSwapCandidates(int vertex)
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

    vector<int> RandomPermutation(int size)
    {
        vector<int> permutation(size);
        for (int i = 0; i < size; ++i)
        {
            permutation[i] = i;
        }
        shuffle(permutation.begin(), permutation.end(), generator);
        return permutation;
    }
    
    void RemoveFromCliqueWithTabu(int vertex)
    {
        RemoveFromClique(vertex);
        tabu_remove.push_back(vertex);
        if (tabu_remove.size() > tabu_remove_maxsize)
        {
            tabu_remove.pop_front();
        }
    }

    void InsertToCliqueWithTabu(int vertex)
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
    int Swap()
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
                if (find(tabu_insert.begin(), tabu_insert.end(), vertex) == tabu_insert.end())
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

    bool Move()
    {
        if (c_border == q_border)
            return false;
        int vertex = qco[GetRandom(q_border, c_border - 1)];
        InsertToClique(vertex);
        return true;
    }

    void FindInitialClique(vector<int> candidates, float randomization)
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

    // a function to implement "small degree last with remove" algorithm
    vector<int> LastDegreeWithRemoveOrder()
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

        reverse(vertices.begin(), vertices.end());
        return vertices;
    }
};

int main()
{
    int iterations;
    cout << "Number of iterations: ";
    cin >> iterations;
    vector<string> files = {
    "brock200_1.clq", "brock200_2.clq", "brock200_3.clq", "brock200_4.clq", "brock400_1.clq", "brock400_2.clq", "brock400_3.clq", "brock400_4.clq",
    "C125.9.clq",
    "gen200_p0.9_44.clq", "gen200_p0.9_55.clq",
    "hamming8-4.clq",
    "johnson8-2-4.clq", "johnson16-2-4.clq",
    "keller4.clq",
    "MANN_a27.clq", "MANN_a9.clq",
    "p_hat1000-1.clq", "p_hat1000-2.clq", "p_hat1500-1.clq", "p_hat300-3.clq", "p_hat500-3.clq",
    "san1000.clq", "sanr200_0.9.clq", "sanr400_0.7.clq" };
    ofstream fout("clique_tabu.csv");
    fout << "File,Time (sec),Clique size," << iterations << "\n";
    for (string file : files)
    {
        MaxCliqueTabuSearch problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.RunSearch(iterations);
        clock_t finish = clock();
        if (!problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "," << double(finish - start) / 1000 << "," << problem.GetClique().size() << '\n';
        cout << file << "," << double(finish - start) / 1000 << "," << problem.GetClique().size() << '\n';
    }
    fout.close();
    return 0;
}
