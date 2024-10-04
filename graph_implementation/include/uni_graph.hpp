#ifndef UNI_GRAPH_H
#define UNI_GRAPH_H

#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <unordered_map>
#include <chrono>
#include <unordered_set>
#include <vector>
#include <deque>
#include <string>
#include <tuple>
#include <limits>
#include <omp.h>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <atomic>

using namespace std;
using namespace boost;

typedef adjacency_list<vecS, vecS, directedS> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;

void bfs_from_kyc_nodes_parallel(const Graph &graph, const std::unordered_set<Vertex> &kyc_nodes, int max_hops, std::unordered_map<string, int> &address_to_hops, const std::unordered_map<Vertex, string> &vertex_to_address);

void build_graph_from_chunks(const std::string &base_filename, int num_chunks, Graph &graph, std::unordered_map<std::string, Vertex> &address_to_vertex);

std::unordered_set<Vertex> read_kyc_addr(const std::string &kyc_address_filename, const std::unordered_map<std::string, Vertex> &address_to_vertex);

int calculates_eai_dist(const std::string kyc_filename, const std::string output_filename);

#endif // UNI_GRAPH_H
