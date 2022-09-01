#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <algorithm>

#include "tabu_max_clique.h"
#include "utils.h"

using namespace std;

class BnBSolver
{
public:
    void ReadGraphFile(string filename)
    {
        ifstream fin(filename);
        string line;
        int vert = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }
            if (line[0] == 'p')
            {
                stringstream s(line);
                char c;
                string in;
                s >> c >> in >> vert >> edges;
                neighbours.resize(vert);
            }
            else
            {
                stringstream s(line);
                char c;
                int st, fn;
                s >> c >> st >> fn;
                neighbours[st - 1].insert(fn - 1);
                neighbours[fn - 1].insert(st - 1);
            }
        }
    }

    tuple<double, double> RunBnB(long time_limit)
    {
        clock_t start = clock();
        clock_t deadline = start + time_limit * 1000;
        MaxCliqueTabuSearch st;
        st.Init(neighbours);
        st.RunSearch(1000);
        best_clique = st.GetClique();
        clock_t heuristic_finish = clock();
        
        clique.clear();
        auto pardalos = PardalosOrder(neighbours);
        for (int i = 0; i < pardalos.size(); ++i)
        {
            if (clock() > deadline)
            {
                break;
            }
            int vertex = pardalos[i];

            vector<int> new_candidates;
            new_candidates.reserve(pardalos.size());
            for (int j = pardalos.size() - 1; j > i; --j)
            {
                if (neighbours[vertex].count(pardalos[j]))
                {
                    new_candidates.push_back(pardalos[j]);
                }
            }
            clique.insert(vertex);
            BnBRecursion(new_candidates);
            clique.erase(vertex);
        }
        clock_t finish = clock();
        return make_tuple(double(heuristic_finish - start) / 1000, double(finish - heuristic_finish) / 1000);
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
                if (i != j && neighbours[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not clique\n";
                    return false;
                }
            }
        }
        return true;
    }

    string PrintClique()
    {
        vector<int> clique;
        clique.insert(clique.end(), best_clique.begin(), best_clique.end());
        sort(clique.begin(), clique.end());
        stringstream ss;
        ss << "\"";
        bool first_vertex = true;
        ss << "{";
        for (const auto& vertex : clique)
        {
            if (!first_vertex)
            {
                ss << ",";
            }
            ss << vertex;
            first_vertex = false;
        }
        ss << "}";
        ss << "\"";
        return ss.str();
    }

private:
    //candidates should be passed in antipardalos order for coloring
    void BnBRecursion(const vector<int>& candidates)
    {
        if (candidates.empty())
        {
            if (clique.size() > best_clique.size())
            {
                best_clique = clique;
            }
            return;
        }

        auto [maxcolor, colors] = GreedyGraphColoring(neighbours, candidates);

        vector<vector<int>> colors_to_vertices(maxcolor + 1);
        //traverse in pardalos order
        for (int i = candidates.size() - 1; i >= 0; --i)
        {
            auto vertex = candidates[i];
            colors_to_vertices[colors[vertex]].push_back(vertex);
        }

        vector<bool> visited_candidates(neighbours.size(), false);
        for (int color = maxcolor; color > 0; --color)
        {
            if (clique.size() + color <= best_clique.size()) 
            {
                return;
            }
            
            for (int vertex_to_add : colors_to_vertices[color])
            {
                visited_candidates[vertex_to_add] = true;
                    
                vector<int> new_candidates;
                new_candidates.reserve(candidates.size());
                for (int candidate : candidates)
                {
                    if (!visited_candidates[candidate] && neighbours[vertex_to_add].count(candidate))
                    {
                        new_candidates.push_back(candidate);
                    }
                }
                clique.insert(vertex_to_add);
                BnBRecursion(new_candidates);
                clique.erase(vertex_to_add);
            }
        }
    }

private:
    vector<unordered_set<int>> neighbours;
    unordered_set<int> best_clique;
    unordered_set<int> clique;
};

int main()
{
    cout << "Time limit (sec): ";
    long time_limit;
    cin >> time_limit;
    vector<string> files = {
    "brock200_1.clq", "brock200_2.clq", "brock200_3.clq", "brock200_4.clq", /*"brock400_1.clq", "brock400_2.clq", "brock400_3.clq", "brock400_4.clq",*/
    "C125.9.clq",
    "gen200_p0.9_44.clq", "gen200_p0.9_55.clq",
    "hamming8-4.clq",
    "johnson8-2-4.clq", "johnson16-2-4.clq",
    "keller4.clq",
    "MANN_a27.clq", "MANN_a9.clq",
    "p_hat1000-1.clq", /*"p_hat1000-2.clq",*/ "p_hat1500-1.clq", "p_hat300-3.clq", /*"p_hat500-3.clq",*/
    "san1000.clq", "sanr200_0.9.clq"/*, "sanr400_0.7.clq"*/};

    ofstream fout("clique_bnb.csv");
    fout << "File,Heuristic time (sec),BnB time (sec),Clique size,Clique vertices," << time_limit << "\n";
    for (string file : files)
    {
        BnBSolver problem;
        problem.ReadGraphFile(file);
        auto [heuristic_time, bnb_time] = problem.RunBnB(time_limit);
        if (! problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "," << heuristic_time << "," << bnb_time << "," << problem.GetClique().size() << "," << problem.PrintClique() << ",\n";
        cout << file << "," << heuristic_time << "," << bnb_time << "," << problem.GetClique().size() << "," << problem.PrintClique() << ",\n";
    }
    return 0;
}
