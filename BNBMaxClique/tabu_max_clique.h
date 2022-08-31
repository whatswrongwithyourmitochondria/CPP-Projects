#include <vector>
#include <unordered_set>
#include <list>
#include <random>

class MaxCliqueTabuSearch
{
public:
    void Init(const std::vector<std::unordered_set<int>>& input_neighbour_sets);

    void RunSearch(int iterations);

    const std::unordered_set<int>& GetClique();

private:
    std::vector<std::unordered_set<int>> neighbour_sets;
    std::vector<std::unordered_set<int>> non_neighbours;
    std::unordered_set<int> best_clique;
    std::vector<int> qco;
    std::vector<int> index;
    std::vector<int> tightness;
    int q_border;
    int c_border;
    std::mt19937 generator;
    std::list<int> tabu_insert;
    std::list<int> tabu_remove;
    int tabu_insert_maxsize;
    int tabu_remove_maxsize;

    int GetRandom(int a, int b);
    
    void ClearClique();
    
    void SwapVertices(int vertex, int border);

    void InsertToClique(int i);
    
    void RemoveFromClique(int k);
    
    std::vector<int> FindSwapCandidates(int vertex);

    std::vector<int> RandomPermutation(int size);
    
    void RemoveFromCliqueWithTabu(int vertex);

    void InsertToCliqueWithTabu(int vertex);
    
    //a function that searches for swap candidates and looks for possibilities to make 1-2 or 1-1 swaps
    // it also takes into accout tabu lists
    int Swap();

    bool Move();

    void FindInitialClique(std::vector<int> candidates, float randomization);
};