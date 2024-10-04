#include "uni_graph.hpp"

using namespace std;
using namespace boost;

void bfs_from_kyc_nodes_parallel(const Graph &graph, const std::unordered_set<Vertex> &kyc_nodes, int max_hops, std::unordered_map<string, int> &address_to_hops, const std::unordered_map<Vertex, string> &vertex_to_address)
{

    // Declare a hash set visited to store the nodes that have already been visited during the BFS.
    std::unordered_set<Vertex> visited;
    visited.reserve(8000000);
    // Declare a double-ended queue queue to store pairs of nodes and their associated levels (number of hops from the nearest KYC node).
    deque<pair<Vertex, int>> queue; // (node, level)
    // Declare a mutex visited_mutex to handle concurrent access to the visited hash set.
    std::mutex visited_mutex;
    std::atomic<int> visited_counter(0);
    std::cout << "Initializing KYC nodes' distance" << std::endl;

    // Initialize the BFS starting points by marking all KYC nodes as visited, and setting their distance to 0.
    for (const Vertex &kyc_node : kyc_nodes)
    {
        // Mark as visited
        visited.insert(kyc_node);
        // Prepare for the first level iteration
        queue.push_back(make_pair(kyc_node, 0));
        // Set the distance to 0 if this address is in the graph
        auto it = vertex_to_address.find(kyc_node);
        if (it != vertex_to_address.end())
        {
            address_to_hops[it->second] = 0;
        }
    }

    /* Start iterating */
    for (int current_level = 0; current_level < max_hops; ++current_level)
    {
        std::cout << "Handling level " << current_level << std::endl;
        // Declare a double-ended queue next_level_queue to store nodes and their associated levels for the next BFS level.
        deque<pair<Vertex, int>> next_level_queue;
        // Declare a hash set next_level_nodes_set to track unique nodes in the next BFS level.
        std::unordered_set<Vertex> next_level_nodes_set;

#pragma omp parallel
        {
            // Declare a double-ended queue local_queue in each parallel thread to store pairs of nodes and their associated levels.
            deque<pair<Vertex, int>> local_queue;

#pragma omp for schedule(static) nowait
            for (size_t i = 0; i < queue.size(); ++i)
            {
                Vertex node;
                int level;
                // Get the node and level from the queue
                tie(node, level) = queue[i];

                // Check all the neighbors of the node
                graph_traits<Graph>::adjacency_iterator ai, ai_end;
                for (tie(ai, ai_end) = adjacent_vertices(node, graph); ai != ai_end; ++ai)
                {
                    Vertex neighbor = *ai;
                    // Using a mutex to avoid race condition
                    {
                        std::lock_guard<std::mutex> lock(visited_mutex);
                        /*If this neighbor is not found in visited, update the visited section and local_queue */
                        if (visited.find(neighbor) == visited.end())
                        {
                            visited.insert(neighbor);
                            local_queue.push_back(make_pair(neighbor, level + 1));
                            // Increment the visited_counter and print the size of visited if it's a multiple of 10,000
                            if (++visited_counter % 1000000 == 0)
                            {
                                std::cout << "Visited size: " << visited.size() << std::endl;
                            }
                        }
                    }
                }
            }

#pragma omp critical
            {
                for (const auto &node_level_pair : local_queue) // Addtional checks for duplicates
                {
                    if (next_level_nodes_set.find(node_level_pair.first) == next_level_nodes_set.end())
                    {
                        next_level_nodes_set.insert(node_level_pair.first);
                        next_level_queue.push_back(node_level_pair);
                    }
                }
            }
        }

        for (const auto &[node, level] : next_level_queue)
        {
            auto it = vertex_to_address.find(node);
            if (it != vertex_to_address.end())
            {
                address_to_hops[it->second] = level;
            }
        }

        queue = std::move(next_level_queue);
    }
    // Iterate through all the vertices in the graph
    for (const auto &vertex_address_pair : vertex_to_address)
    {
        // If the address is not in the address_to_hops map, set its distance to 6
        if (address_to_hops.find(vertex_address_pair.second) == address_to_hops.end())
        {
            address_to_hops[vertex_address_pair.second] = 6;
        }
    }
}

void build_graph_from_chunks(const std::string &base_filename, int num_chunks, Graph &graph, std::unordered_map<std::string, Vertex> &address_to_vertex)
{
    std::ostringstream chunk_filename_stream;
    std::string line;

    for (int i = 0; i < num_chunks; ++i)
    {
        chunk_filename_stream.str(""); // Clear the contents
        chunk_filename_stream.clear(); // Reset error flags
        chunk_filename_stream << base_filename << std::setw(12) << std::setfill('0') << i;
        std::string chunk_filename = chunk_filename_stream.str();
        std::ifstream input_file(chunk_filename);
        if (!input_file)
        {
            std::cerr << "Error: Unable to open file " << chunk_filename << std::endl;
            continue;
        }
        std::cout << "Processing file " << chunk_filename << std::endl;

        // Skip the header line
        std::getline(input_file, line);

        while (std::getline(input_file, line))
        {
            try
            {
                std::istringstream line_stream(line);
                std::string address1, address2, total_transfer_usd_str;
                std::getline(line_stream, address1, ',');
                std::getline(line_stream, address2, ',');
                // Total transfer amount is not needed at the moment
                std::getline(line_stream, total_transfer_usd_str, ',');
                float total_transfer_usd = std::stof(total_transfer_usd_str);

                Vertex v1, v2;
                auto it1 = address_to_vertex.find(address1);
                if (it1 == address_to_vertex.end())
                {
                    v1 = add_vertex(graph);
                    address_to_vertex.try_emplace(address1, v1);
                }
                else
                {
                    v1 = it1->second;
                }

                auto it2 = address_to_vertex.find(address2);
                if (it2 == address_to_vertex.end())
                {
                    v2 = add_vertex(graph);
                    address_to_vertex.try_emplace(address2, v2);
                }
                else
                {
                    v2 = it2->second;
                }

                if (address1 != address2 && total_transfer_usd >= 10.0)
                {
                    add_edge(v1, v2, graph);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: Failed to parse CSV line in file " << chunk_filename << ": " << e.what() << std::endl;
            }
        }
    }
}

std::unordered_set<Vertex> read_kyc_addr(const std::string &kyc_address_filename, const std::unordered_map<std::string, Vertex> &address_to_vertex)
{
    // Read KYC addresses
    ifstream kyc_address_file(kyc_address_filename);
    std::unordered_set<Vertex> kyc_nodes;
    kyc_nodes.reserve(50000000);
    string kyc_address;
    size_t counter = 0;
    // Skip 1 line
    getline(kyc_address_file, kyc_address);
    // Iterate
    while (getline(kyc_address_file, kyc_address))
    {
        auto it = address_to_vertex.find(kyc_address);
        if (it != address_to_vertex.end())
        {
            kyc_nodes.insert(it->second);
        }
        
        // Increment counter
        counter++;
        // Print progress every 1 million records
        if (counter % 1000000 == 0)
        {
            std::cout << "Processed " << counter << " records." << std::endl;
        }
    }
    kyc_address_file.close();

    return kyc_nodes;
}

int calculates_eai_dist(const std::string kyc_filename, const std::string output_filename)
{
    // Get the home directory
    const char *home_dir = getenv("HOME");
    if (!home_dir)
    {
        cerr << "Error: HOME environment variable not set" << endl;
        return 1;
    }
    string home_directory(home_dir);
    // std::cout << home_directory << std::endl;
    try
    {
        auto start = chrono::high_resolution_clock::now();

        // Initializations
        Graph graph;
        std::unordered_map<string, Vertex> address_to_vertex;
        // Reserve spaces for 13 million vertices
        address_to_vertex.reserve(13000000);
        // Vector to store the hop counts
        std::unordered_map<string, int> address_to_hops;
        address_to_hops.reserve(10000000);

// Set different file paths based on the operating system
#ifdef __APPLE__
        string kyc_address_filename = home_directory + "/circle_files/econ_project/src/proj23_03_tracebility/data/" + kyc_filename;

#elif __linux__
        // Set the paths for the Linux operating system
        string kyc_address_filename = home_directory + "/econ_project/src/proj23_03_tracebility/data/" + kyc_filename;
#endif
        /*--------------------------------------------
        Build graph using 298 chunks of transfer_history files
        --------------------------------------------*/
        int num_chunks = 298;
        std::string base_filename = home_directory + "/econ_project/src/proj23_03_tracebility/data/uni_transfer_history_";
        build_graph_from_chunks(base_filename, num_chunks, graph, address_to_vertex);

        // Create the reverse vertex_to_address mapping from address_to_vertex
        std::unordered_map<Vertex, std::string> vertex_to_address;
        vertex_to_address.reserve(address_to_vertex.size());

        for (const auto &entry : address_to_vertex)
        {
            vertex_to_address[entry.second] = entry.first;
        }

        auto end = chrono::high_resolution_clock::now();

        // Print relevant information for the graph built
        chrono::duration<double> elapsed = end - start;
        std::cout << "Build graph time: " << elapsed.count() << " seconds" << std::endl;
        std::cout << "Graph has " << num_vertices(graph) << " vertices and " << num_edges(graph) << " edges." << std::endl;

        /*--------------------------------------------
        Read KYC addresses
        --------------------------------------------*/
        std::cout << "Start reading EAI file" << std::endl;
        std::unordered_set<Vertex> kyc_nodes = read_kyc_addr(kyc_address_filename, address_to_vertex);

        /*--------------------------------------------
        Run efficient BFS and calculates KYC distance
        --------------------------------------------*/
        std::cout << "Start BFS" << std::endl;
        int max_hops = 5;
        auto start_bfs = chrono::high_resolution_clock::now();
        bfs_from_kyc_nodes_parallel(graph, kyc_nodes, max_hops, address_to_hops, vertex_to_address);

        auto end_bfs = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed_bfs = end_bfs - start_bfs;
        cout << "Iteration time: " << elapsed_bfs.count() << " seconds" << endl;
        cout << "Writing results..." << endl;
        /*--------------------------------------------
        Write results
        --------------------------------------------*/
        ofstream address_to_hops_file(home_directory + "/econ_project/src/proj23_03_tracebility/output/" + output_filename);
        for (const auto &entry : address_to_hops)
        {
            address_to_hops_file << entry.first << "," << entry.second << endl;
        }
        address_to_hops_file.close();

        return 0;
    }
    catch (std::exception const &e)
    {
        cerr << "Exception thrown: " << e.what() << "\n";
        return 1;
    }
}
