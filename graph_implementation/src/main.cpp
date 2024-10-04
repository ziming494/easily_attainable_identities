#include "uni_graph.hpp"

int main()
{
    string kyc_name = "agg_eai_no_dusting.csv";
    string output_filename = "address_to_hops_no_dusting_dc.txt";
    int a = calculates_eai_dist(kyc_name, output_filename);
    return 0;
}