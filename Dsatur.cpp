#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <tuple>
#include <unordered_map>
#include <algorithm>

struct V { std::string name, label; };
struct E { std::string label; };

using DG = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, V, E>;
using UG = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, V, E>;

std::vector<std::pair<std::string, int>> dsatur(
    const std::map<std::string, std::set<std::string>>& adj_map,
    const std::string& no_inicial) {
    
    // Converte strings -> índices numéricos (0..n-1)
    std::unordered_map<std::string, int> nome_id;
    std::vector<std::string> id_nome;
    int id = 0;
    for (const auto& [u, _] : adj_map) {
        nome_id[u] = id;
        id_nome.push_back(u);
        id++;
    }
    int n = id_nome.size();
    
    // Estruturas DSATUR
    std::vector<std::vector<int>> adj(n);
    std::vector<int> color(n, -1), saturation(n, 0), degree(n, 0);
    std::vector<bool> colored(n, false);
    
    // Constrói adj numérica + graus
    for (const auto& [u_str, viz] : adj_map) {
        int u = nome_id.at(u_str);
        degree[u] = viz.size();
        for (const std::string& v_str : viz) {
            int v = nome_id.at(v_str);
            adj[u].push_back(v);
        }
    }
    
    // Priority queue: (saturação, grau, -id) max-heap
    auto comp = [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b) || 
               (std::get<0>(a) == std::get<0>(b) && 
                (std::get<1>(a) < std::get<1>(b) || 
                 (std::get<1>(a) == std::get<1>(b) && std::get<2>(a) > std::get<2>(b))));
    };
    std::priority_queue<std::tuple<int,int,int>, 
                        std::vector<std::tuple<int,int,int>>, decltype(comp)> pq(comp);
    
    // Inicializa fila (prioriza nó inicial)
    for (int i = 0; i < n; ++i) {
        pq.push({0, degree[i], -i});
    }
    
    while (!pq.empty()) {
        auto [_, __, neg_u] = pq.top(); pq.pop();
        int u = -neg_u;
        if (colored[u]) continue;
        
        // Menor cor válida
        std::set<int> used;
        for (int v : adj[u]) {
            if (color[v] != -1) used.insert(color[v]);
        }
        int cor = 0;
        while (used.count(cor)) ++cor;
        color[u] = cor;
        colored[u] = true;
        
        // Atualiza saturação dos vizinhos
        for (int v : adj[u]) {
            if (!colored[v]) {
                saturation[v]++;
                pq.push({saturation[v], degree[v], -v});
            }
        }
    }
    
    // Retorna nó+cor
    std::vector<std::pair<std::string, int>> resultado;
    for (int i = 0; i < n; ++i) {
        resultado.emplace_back(id_nome[i], color[i]);
    }
    return resultado;
}



int main() {
    std::string caminho = "ex.dot";
    //std::cout << "Insira o nome do arquivo a ser lido: ";
    //std::cin >> caminho;

    std::ifstream in(caminho);
    if (!in) { std::cerr << caminho << " não existe\n"; return 1; }

    in.clear();
    in.seekg(0);

     UG g;
        boost::dynamic_properties dp(boost::ignore_other_properties);
        dp.property("node_id", get(&V::name, g));
        dp.property("label",   get(&V::label, g));
        dp.property("label",   get(&E::label, g));
        if (!boost::read_graphviz(in, g, dp)) { std::cerr << "falha DOT\n"; return 2; }

        std::map<std::string, std::set<std::string>> listaadj;

        for (auto e : boost::make_iterator_range(edges(g))) {
            const std::string& u = g[source(e, g)].name;
            const std::string& v = g[target(e, g)].name;
            listaadj[u].insert(v); 
            listaadj[v].insert(u);
        }
        auto coloracao = dsatur(listaadj, "2");  
        std::sort(coloracao.begin(), coloracao.end(),
          [](const std::pair<std::string,int>& a,
             const std::pair<std::string,int>& b) {
              return std::stoi(a.first) < std::stoi(b.first); // numérico
          });

        std::cout << "Coloração DSATUR:\n";
        int cromatico = 0;
        for (const auto& [no, cor] : coloracao) {
            std::cout << no << ": cor " << cor << '\n';
            cromatico = std::max(cromatico, cor);
        }
        std::cout << "χ(G) ≈ " << cromatico + 1 << std::endl; 
}