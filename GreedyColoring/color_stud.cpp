#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <time.h>
#include <unordered_map>
#include <map>
#include <set>
using namespace std;


class ColoringProblemBase
{
public:
    // a function to parse files
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
                colors.resize(vertices);
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
    //a function to implement greedy algorithm of graph coloring 
    void GreedyGraphColoring()
    {
        for (int vertex : VertexColoringOrder())
        {
            unordered_set<int> pallete;
            for (int i = 1; i <= maxcolor; ++i)
            {
                pallete.insert(i);
            }

            for (int neighbour : neighbour_sets[vertex])
            {
                pallete.erase(colors[neighbour]);
            }
            
            if (pallete.empty())
            {
                colors[vertex] = ++maxcolor;
            }
            else
            {    
                colors[vertex] = *pallete.begin();
            }
        }
    }

    // a function to check if we have uncolored vertices and adjacent vertices with the same color
    bool Check()
    {
        for (size_t i = 0; i < neighbour_sets.size(); ++i)
        {
            if (colors[i] == 0)
            {
                cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }
            for (int neighbour : neighbour_sets[i])
            {
                if (colors[neighbour] == colors[i])
                {
                    cout << "Neighbour vertices " << i + 1 << ", " << neighbour + 1 <<  " have the same color\n";
                    return false;
                }
            }
        }
        return true;
    }

    int GetNumberOfColors()
    {
        return maxcolor;
    }

    const vector<int>& GetColors()
    {   
        return colors;
    }
    // a function to print a group of vertices corresponding their color
    string PrintColors()
    {
        unordered_map<int, set<int>> colors_to_vertices;
        multimap<int, int, greater<int>> popularity_to_color;
        for (int i = 0; i < colors.size(); ++i)
        {
            colors_to_vertices[colors[i]].insert(i);
        }
        for (const auto& [color, vertices] : colors_to_vertices)
        {
            popularity_to_color.insert({ vertices.size(), color });
        }
        stringstream ss;
        ss << "\"";
        bool first_color = true;
        for (const auto& [p, color] : popularity_to_color)
        {
            if (!first_color) 
            {
                ss << ", ";
            }
            ss << "{";
            bool first_vertex = true;
            for (const auto& vertice : colors_to_vertices[color])
            {
                if (!first_vertex)
                {
                    ss << ", ";
                }
                ss << vertice;
                first_vertex = false;
            }
            ss << "}";
            first_color = false;
        }
        ss << "\"";
        return ss.str();
    }
protected:
    virtual vector<int> VertexColoringOrder() = 0;

    vector<int> colors;
    int maxcolor = 1;
    vector<unordered_set<int>> neighbour_sets;
};

//a child class of ColoringProblemBase class where we implement vertex coloring in a random order
class RandomColoringProblem : public ColoringProblemBase
{
protected:
    //a function to randomize vertex coloring order 
    virtual vector<int> VertexColoringOrder() override
    {
        vector<int> vertices(neighbour_sets.size());
        for (int i = 0; i < vertices.size(); ++i)
        {
            vertices[i] = i;
        }
        static mt19937 generator(1000);
        shuffle(vertices.begin(), vertices.end(), generator);
        return vertices;
    }
};

// a child class of ColoringProblemBase class that implements "small degree last with remove" algorithm
class LastDegreeColoringProblem : public ColoringProblemBase
{
protected:
    virtual vector<int> VertexColoringOrder() override
    {
        unordered_map<int, unordered_set<int>> graph_cut;
        for (int i = 0; i < neighbour_sets.size(); ++i)
        {
            graph_cut[i] = neighbour_sets[i];
        }

        vector<int> vertices;

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

// a child class of ColoringProblem class that implements "first degree" algorithm to color vertices
class FirstDegreeColoringProblem : public ColoringProblemBase
{
protected:
    virtual vector<int> VertexColoringOrder() override
    {
        multimap<int, int, greater<int>> degree_to_vertex;
        for (int i = 0; i < neighbour_sets.size(); ++i)
        {
            degree_to_vertex.insert({neighbour_sets[i].size(), i});
        }
        vector<int> vertices;
        for (const auto& [degree, vertex] : degree_to_vertex)
        {
            vertices.push_back(vertex);
        }
        return vertices;
    }
};


int main()
{
    vector<string> files = { "myciel3.col", "myciel7.col", "latin_square_10.col", "school1.col", "school1_nsh.col",
        "mulsol.i.1.col", "inithx.i.1.col", "anna.col", "huck.col", "jean.col", "miles1000.col", "miles1500.col",
        "fpsol2.i.1.col", "le450_5a.col", "le450_15b.col", "le450_25a.col", "games120.col",
        "queen11_11.col", "queen5_5.col" };
    ofstream fout("color.csv");
    fout << "Instance, Colors, Time (sec), Color Classes\n";
    cout << "Instance, Colors, Time (sec), Color Classes\n";
    for (string file : files)
    {
        //RandomColoringProblem problem;
        //FirstDegreeColoringProblem problem;
        LastDegreeColoringProblem problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.GreedyGraphColoring();
        clock_t finish = clock();
        if (! problem.Check())
        {
            fout << "*** WARNING: incorrect coloring: ***\n";
            cout << "*** WARNING: incorrect coloring: ***\n";
        }
        fout << file << "," << problem.GetNumberOfColors() << "," << double(finish - start) / 1000 << "," << problem.PrintColors() << '\n';
        cout << file << "," << problem.GetNumberOfColors() << "," << double(finish - start) / 1000 << "," << problem.PrintColors() << '\n';
     
    }
    fout.close();
    return 0;
}