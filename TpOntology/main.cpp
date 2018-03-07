/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on May 29, 2013, 11:02 AM
 */

#include "TpOntApi.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "../TextpressoCentralGlobalDefinitions.h"

void print_who() {
    std::cout << std::endl << "tpontology" << std::endl;
    std::cout << "Build Date: " << __DATE__ << std::endl;
    std::cout << "Version: 0.0.2" << std::endl;
}

void print_help() {
    std::cout << std::endl;
    std::cout << "Usage: tpontology command <more options>" << std::endl;
    std::cout << std::endl;
    std::cout << "commands:";
    std::cout << std::endl;
    std::cout << "  create_table_tpontology";
    std::cout << std::endl;
    std::cout << "     creates 'tpontology' table in postgres";
    std::cout << std::endl;
    std::cout << "     unless it already exists.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  create_index_tpontology";
    std::cout << std::endl;
    std::cout << "     creates term and category index on 'tpontology' table in postgres";
    std::cout << std::endl;
    std::cout << "     if table exists.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  create_table_pcrelations";
    std::cout << std::endl;
    std::cout << "     creates 'pcrelations' table in postgres";
    std::cout << std::endl;
    std::cout << "     unless it already exists.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  create_table_padcrelations";
    std::cout << std::endl;
    std::cout << "     creates 'padcrelations' table in postgres";
    std::cout << std::endl;
    std::cout << "     unless it already exists.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  populate_tpontology_from_file <filename>";
    std::cout << std::endl;
    std::cout << "     populates tpontology table from tab-delimited file.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  populate_pcrelations_from_file <filename>";
    std::cout << std::endl;
    std::cout << "     populates pcrelations table from tab-delimited file.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  populate_tpontology_from_obo <filename> <categoryid> <categorynamefile> <name of source>";
    std::cout << std::endl;
    std::cout << "     populates table from OBO file.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  update_tpontology_column_from_file <filename> <column>";
    std::cout << std::endl;
    std::cout << "     updates table from tab-delimited,";
    std::cout << std::endl;
    std::cout << "     2-column file; first column is condition column,";
    std::cout << std::endl;
    std::cout << "     second column is affected column.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  index_tpontology_column <column name>";
    std::cout << std::endl;
    std::cout << "     creates an index on column <column name> of tpontology.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  index_pcrelations_column <column name>";
    std::cout << std::endl;
    std::cout << "     creates an index on column <column name> of pcrelations.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  populate_all_from_tppc_files <dir> <fileprefix> <depth> <name of source>";
    std::cout << std::endl;
    std::cout << "     populates pcrelations and tpontology tables from tppc files";
    std::cout << std::endl;
    std::cout << "     in dir <dir> with prefix <fileprefix> using files up to depth <depth>.";
    std::cout << std::endl;
    std::cout << "     Populates parent-child (pcrelations table)";
    std::cout << std::endl;
    std::cout << "     with corresponding names. Populates tpontology table, with";
    std::cout << std::endl;
    std::cout << "     the lowest nodes combining all terms, synonyms, etc, of all";
    std::cout << std::endl;
    std::cout << "     descendants of respective lowest node.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  convert_obo_to_tppc <files directory>";
    std::cout << std::endl;
    std::cout << "     converts a set of obo-derived files into parent-children lines.";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "  grow_tree <parent-children file> <tree file>";
    std::cout << std::endl;
    std::cout << "     grows tree files form parent-children in <parent-children file>.";
    std::cout << std::endl;
    std::cout << "     Output will be put in <tree file> + \".direct.children.tppc\"";
    std::cout << std::endl;
    std::cout << "     as well as  <tree file> + \".all.descendants.tppc\".";
    std::cout << std::endl;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        print_who();
        print_help();
        return (-1);
    }
    const char * pszCommand = argv[1];
    std::string s(argv[1]);
    TpOntApi * x = new TpOntApi(PGONTOLOGYTMPTABLENAME, PCRELATIONSTMPTABLENAME, PADCRELATIONSTMPTABLENAME);
    if (s.compare("create_table_tpontology") == 0) {
        x->CreateOntologyTable();
    } else if (s.compare("create_index_tpontology") == 0) {
        x->CreateOntologyIndex();
    } else if (s.compare("create_table_pcrelations") == 0) {
        x->CreatePcRelationsTable();
    } else if (s.compare("create_table_padcrelations") == 0) {
        x->CreatePadcRelationsTable();
    } else if (s.compare("populate_tpontology_from_file") == 0) {
        x->PopulateTpOntologyFromFile(argv[2]);
    } else if (s.compare("populate_pcrelations_from_file") == 0) {
        x->PopulatePcRelationsFromFile(argv[2]);
    } else if (s.compare("populate_tpontology_from_obo") == 0) {
        std::string catid(argv[3]);
        std::string catname;
        std::ifstream f(argv[4]);
        std::string in;
        while (getline(f, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of("\t"));
            if (catid.compare(splits[0]) == 0)
                catname = splits[1];
        }
        f.close();
        x->PopulateTpOntologyFromOboFile(argv[2], catname, argv[5]);
    } else if (s.compare("update_tpontology_column_from_file") == 0) {
        x->UpdateTpOntologyColumnFromFile(argv[2], argv[3], argv[4]);
    } else if (s.compare("index_tpontology_column") == 0) {
        x->IndexColumn(PGONTOLOGYTMPTABLENAME, std::string(argv[2]));
    } else if (s.compare("index_pcrelations_column") == 0) {
        x->IndexColumn(PCRELATIONSTMPTABLENAME, std::string(argv[2]));
    } else if (s.compare("populate_all_from_tppc_files") == 0) {
        x->PopulateTpOntologyAllFromTppc(std::string(argv[2]),
                std::string(argv[3]), std::string(argv[5]), atoi(argv[4]));
    } else if (s.compare("convert_obo_to_tppc") == 0) {
        x->ConvertObo2Tppc(std::string(argv[2]));
    } else if (s.compare("grow_tree") == 0) {
        x->GrowTree(std::string(argv[2]), std::string(argv[3]));
    } else
        std::cerr << "No command specified." << std::endl;
    return 0;
}


