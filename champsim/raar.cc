int main(int argc, char** argv)
{
    // ------------------------------------------------------------------------ //
    /////////////////////// trace reading complete ///////////////////////////////
    // ------------------------------------------------------------------------ //

    // Open the profile file
    profile = fopen(argv[3], "r");
    uint64_t miss_pc;

    // Build a trace window
    while (fgets(miss_instr, sizeof(miss_instr), profile)) {
        printf("%s", miss_instr); 
        miss_pc = (int)strtol(miss_instr, NULL, 0);
        last_occur_window = build_window(miss_pc);
    }

    fclose(profile);
    
    cout << "creating the graph" << endl;
    Graph g = graph_create();
    cout << "building the graph" << endl;
    vertex_descriptor_t root = build_graph(last_occur_window, &g, miss_pc);
    print_vertices(&g);
    boost::property_map<Graph, boost::vertex_bundle_t>::type pmap = boost::get(boost::vertex_bundle, g);
    VertexProperty root_p = boost::get(pmap, root);
    print_vertex_property(root_p);

    cout << "do compaction to the graph" << endl;
    store_load_bypassing(&g, root);
    print_vertices(&g);
    
    return 0;
}
