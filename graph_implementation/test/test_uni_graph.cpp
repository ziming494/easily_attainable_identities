#include "gtest/gtest.h"
#include "uni_graph.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <sstream>

class BuildGraphFromChunksTest : public ::testing::Test {
protected:
    Graph graph1, graph2, graph3;
    std::unordered_map<std::string, Vertex> address_to_vertex1, address_to_vertex2, address_to_vertex3;
    const char *home_dir;
    std::string home_directory;
    std::pair<Edge, bool> edge_pair;

    void SetUp() override {
        // Initialize home_directory.
        home_dir = getenv("HOME");
        home_directory = std::string(home_dir);

        // Adjust the paths to suit your local structure.
        build_graph_from_chunks(home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_build_graph1_", 1, graph1, address_to_vertex1);
        build_graph_from_chunks(home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_build_graph2_", 2, graph2, address_to_vertex2);
        build_graph_from_chunks(home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_build_graph3_", 1, graph3, address_to_vertex3);

    }
};

class ReadKYCAddrTest : public ::testing::Test {
protected:
    Graph graph;
    std::unordered_map<std::string, Vertex> address_to_vertex;
    std::unordered_set<Vertex> kyc_nodes;
    const char *home_dir;
    std::string home_directory;
    std::string kyc_address_filename;

    void SetUp() override {
        // Initialize home_directory.
        home_dir = getenv("HOME");
        home_directory = std::string(home_dir);
        kyc_address_filename = home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_kyc.csv";

        build_graph_from_chunks(home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_build_graph3_", 1, graph, address_to_vertex);
        kyc_nodes = read_kyc_addr(kyc_address_filename, address_to_vertex);
    }
};

class BFSFromKYCNodesParallelTest : public ::testing::Test {
protected:
    Graph graph;
    std::unordered_map<std::string, Vertex> address_to_vertex;
    std::unordered_set<Vertex> kyc_nodes;
    std::unordered_map<Vertex, std::string> vertex_to_address;
    std::unordered_map<std::string, int> address_to_hops;
    const char *home_dir;
    std::string home_directory;
    std::string kyc_address_filename;

    void SetUp() override {
        // Initialize home_directory.
        home_dir = getenv("HOME");
        home_directory = std::string(home_dir);
        kyc_address_filename = home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_kyc.csv";

        build_graph_from_chunks(home_directory + "/econ_project/src/proj23_03_tracebility/cpp/uni_directional_implementation/test_data/test_build_graph3_", 1, graph, address_to_vertex);
        kyc_nodes = read_kyc_addr(kyc_address_filename, address_to_vertex);

        // Create reverse mapping
        for (const auto &entry : address_to_vertex) {
            vertex_to_address[entry.second] = entry.first;
        }

        // Perform BFS
        bfs_from_kyc_nodes_parallel(graph, kyc_nodes, 5, address_to_hops, vertex_to_address);
    }
};

/* Test build_graph_from_chunks */
TEST_F(BuildGraphFromChunksTest, BasicGraphInfo) {
    // Test graph 1
    EXPECT_EQ(num_vertices(graph1), 4);
    EXPECT_EQ(num_edges(graph1), 3);
    EXPECT_EQ(address_to_vertex1.size(), 4);
    // Test graph 2
    EXPECT_EQ(num_vertices(graph2), 5);
    EXPECT_EQ(num_edges(graph2), 4);
    EXPECT_EQ(address_to_vertex2.size(), 5);
}

TEST_F(BuildGraphFromChunksTest, MapStorage) {
    ASSERT_FALSE(address_to_vertex2.find("address1") == address_to_vertex2.end());
    ASSERT_FALSE(address_to_vertex2.find("address2") == address_to_vertex2.end());
    ASSERT_FALSE(address_to_vertex2.find("address3") == address_to_vertex2.end());
    ASSERT_FALSE(address_to_vertex2.find("address4") == address_to_vertex2.end());
    ASSERT_FALSE(address_to_vertex2.find("address5") == address_to_vertex2.end());
    ASSERT_TRUE(address_to_vertex2.find("address6") == address_to_vertex2.end());
}

TEST_F(BuildGraphFromChunksTest, EdgeConnectivity) {
    // Test graph 1 
    edge_pair = edge(address_to_vertex1["address1"], address_to_vertex1["address2"], graph1);
    ASSERT_TRUE(edge_pair.second);
    edge_pair = edge(address_to_vertex1["address2"], address_to_vertex1["address1"], graph1);
    ASSERT_TRUE(edge_pair.second);

    edge_pair = edge(address_to_vertex1["address2"], address_to_vertex1["address3"], graph1);
    ASSERT_FALSE(edge_pair.second);
    edge_pair = edge(address_to_vertex1["address3"], address_to_vertex1["address2"], graph1);
    ASSERT_FALSE(edge_pair.second);

    edge_pair = edge(address_to_vertex1["address3"], address_to_vertex1["address1"], graph1);
    ASSERT_TRUE(edge_pair.second);
    edge_pair = edge(address_to_vertex1["address1"], address_to_vertex1["address3"], graph1);
    ASSERT_FALSE(edge_pair.second);
    edge_pair = edge(address_to_vertex1["address4"], address_to_vertex1["address1"], graph1);
    ASSERT_FALSE(edge_pair.second);
    edge_pair = edge(address_to_vertex1["address4"], address_to_vertex1["address4"], graph1);
    ASSERT_FALSE(edge_pair.second);
}

// Test neighbors of a particular vertex
TEST_F(BuildGraphFromChunksTest, Neighbors) {
    // Let's assume you want to test the neighbors of "address1" in graph1
    const std::string address = "address6";

    // Find the vertex descriptor for the given address
    auto it = address_to_vertex3.find(address);
    ASSERT_TRUE(it != address_to_vertex3.end()) << "Vertex not found for address: " << address;

    Vertex v = it->second; // Get the vertex descriptor

    // Define expected neighbors for "address1" (adjust according to your test data)
    std::unordered_set<std::string> expected_neighbors = {
        "address7", // Assuming these are your expected neighbors
        "address8", 
        "address9", 
    };

    // Use an unordered_set to track observed neighbors
    std::unordered_set<std::string> actual_neighbors;

    // Use BGL's adjacency_iterator to iterate over adjacent vertices
    graph_traits<Graph>::adjacency_iterator ai, ai_end;
    for (tie(ai, ai_end) = adjacent_vertices(v, graph3); ai != ai_end; ++ai) {
        // Convert each vertex descriptor back to the address string
        auto addr_it = std::find_if(
            address_to_vertex3.begin(), address_to_vertex3.end(),
            [ai](const auto &p) { return p.second == *ai; }
        );
        ASSERT_TRUE(addr_it != address_to_vertex3.end()) << "Address not found for vertex!";
        actual_neighbors.insert(addr_it->first);
    }

    // Compare the actual neighbors with the expected ones
    ASSERT_EQ(actual_neighbors.size(), expected_neighbors.size()) << "Mismatch in number of neighbors!";
    for (const auto &neighbor : expected_neighbors) {
        ASSERT_TRUE(actual_neighbors.find(neighbor) != actual_neighbors.end()) << "Expected neighbor " << neighbor << " not found!";
    }
}
/* Test read_kyc_addr */
TEST_F(ReadKYCAddrTest, ReadInData) {
    ASSERT_EQ(kyc_nodes.size(), 3);
    ASSERT_FALSE(kyc_nodes.find(address_to_vertex["address2"]) == kyc_nodes.end());
    ASSERT_FALSE(kyc_nodes.find(address_to_vertex["address5"]) == kyc_nodes.end());
    ASSERT_FALSE(kyc_nodes.find(address_to_vertex["address6"]) == kyc_nodes.end());
    ASSERT_TRUE(kyc_nodes.find(address_to_vertex["address1"]) == kyc_nodes.end());
}

/* Test BFSFromKYCNodesParallel */
TEST_F(BFSFromKYCNodesParallelTest, BasicInfo) {
    ASSERT_EQ(address_to_hops.size(), 18);
    ASSERT_FALSE(address_to_hops.find("address1") == address_to_hops.end());
    ASSERT_TRUE(address_to_hops.find("address101") == address_to_hops.end());
}

TEST_F(BFSFromKYCNodesParallelTest, Layer0) {
    ASSERT_EQ(address_to_hops["address2"], 0);
    ASSERT_EQ(address_to_hops["address5"], 0);
    ASSERT_EQ(address_to_hops["address6"], 0);
}

TEST_F(BFSFromKYCNodesParallelTest, Layer1) {
    ASSERT_EQ(address_to_hops["address7"], 1);
    ASSERT_EQ(address_to_hops["address8"], 1);
    ASSERT_EQ(address_to_hops["address9"], 1);
    ASSERT_EQ(address_to_hops["address10"], 1);
}

TEST_F(BFSFromKYCNodesParallelTest, Layer2) {
    ASSERT_EQ(address_to_hops["address11"], 2);
    ASSERT_EQ(address_to_hops["address12"], 2);

}

TEST_F(BFSFromKYCNodesParallelTest, Layer3) {
    ASSERT_EQ(address_to_hops["address13"], 3);
}

TEST_F(BFSFromKYCNodesParallelTest, Layer4) {
    ASSERT_EQ(address_to_hops["address14"], 4);
}

TEST_F(BFSFromKYCNodesParallelTest, Layer5) {
    ASSERT_EQ(address_to_hops["address15"], 5);
}

TEST_F(BFSFromKYCNodesParallelTest, Layer6) {
    ASSERT_EQ(address_to_hops["address1"], 6);
    ASSERT_EQ(address_to_hops["address3"], 6);
    ASSERT_EQ(address_to_hops["address4"], 6);
    ASSERT_EQ(address_to_hops["address16"], 6);
    ASSERT_EQ(address_to_hops["address17"], 6);
    ASSERT_EQ(address_to_hops["address18"], 6);
}