#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <unordered_map>
using namespace std;


class MaxCliqueProblem
{
public:
    static int GetRandom(int a, int b)
    {
        static mt19937 generator;
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
    }

    // a function to find the best clique among all the iterations along with the best randomization
    void FindClique(int iterations)
    {
        vector<int> ldwr_order = LastDegreeWithRemoveOrder();
        float best_randomization;
        for (int iteration = 0; iteration < iterations; ++iteration)
        {
            // here we'd like to check different randomizations
            float randomization = (float)iteration / iterations;
            
            vector<int> clique;
            vector<int> candidates = ldwr_order;
           
            while (!candidates.empty())
            {
                int random_index = GetRandom(0, min<int>(randomization * candidates.size() + (iteration ? 1 : 0), candidates.size() - 1));
                int vertex = candidates[random_index];
                clique.push_back(vertex);
                candidates.erase(
                    remove_if(
                        candidates.begin(), candidates.end(),
                        [this, vertex](int c) { return !neighbour_sets[vertex].count(c); }),
                    candidates.end());
            }
            if (clique.size() > best_clique.size())
            {
                best_clique = clique;
                best_randomization = randomization;
            }
        }
        cout << "best_randomization: " << best_randomization << "\n";
    }

    string PrintClique()
    {
        vector<int> best_clique = GetClique();
        sort(best_clique.begin(), best_clique.end());
        stringstream ss;
        ss << "\"";
        bool first_vertex = true;
        ss << "{";
        for (const auto& vertex : best_clique)
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

    const vector<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        if (unique(best_clique.begin(), best_clique.end()) != best_clique.end())
        {
            cout << "Duplicated vertices in the clique\n";
            return false;
        }
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not a clique\n";
                    return false;
                }
            }
        }
        return true;
    }

private:
    vector<unordered_set<int>> neighbour_sets;
    vector<int> best_clique;

    // a function to implement "small degree last with remove" algorithm
    vector<int> LastDegreeWithRemoveOrder()
    {
        unordered_map<int, unordered_set<int>> graph_cut;
        for (int i = 0; i < neighbour_sets.size(); ++i)
        {
            graph_cut[i] = neighbour_sets[i];
        }

        vector<int> vertices;
        // everytime we remove a vertex we need to change our graph, i.e. to remove edges leading to that vertice
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
        "san1000.clq", "sanr200_0.9.clq", "sanr400_0.7.clq"};
    ofstream fout("clique.csv");
    fout << "File,Time (sec),Clique size,Clique vertices," << iterations << "\n";
    for (string file : files)
    {
        MaxCliqueProblem problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.FindClique(iterations);
        clock_t finish = clock();
        if (! problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "," << double(finish - start) / 1000 << "," << problem.GetClique().size() << "," << problem.PrintClique() << '\n';
        cout << file << "," << double(finish - start) / 1000 << "," << problem.GetClique().size() << "," << problem.PrintClique() << '\n';
    }
    fout.close();
    return 0;
}
