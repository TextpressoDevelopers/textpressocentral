/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on July 26, 2019, 11:03 PM
 */

// default values
#define JSONFILE "input.json"
#define MINTERMLENGTH 2
#define MAXWORDCOUNT 5

#include <iostream>
#include <fstream>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <pqxx/pqxx>
#include "tpso.config.h"
#include "../TextpressoCentralGlobalDefinitions.h"
#include "../OboFileAnalyzer/OboFileSegmentation.h"
#include "../TpC/PgList.h"
#include "../TpOntology/tree.h"

typedef std::pair<std::string, std::string> pss;

/*
 * 
 */

namespace {

    std::string GetTmpName() {
        char tmpname[L_tmpnam];
        char * pDummy = tmpnam(tmpname);
        std::string ret(tmpname);
        while (boost::filesystem::exists(ret)) {
            char * pDummy = tmpnam(tmpname);
            ret = std::string(tmpname);
        }
        return ret;
    }

    std::string ConcatenateFiles(std::string masterobofile, std::string obofile) {
        std::string ret(GetTmpName());
        std::ofstream of_out(ret, std::ios_base::binary);
        if (boost::filesystem::exists(masterobofile)) {
            std::ifstream if_m(masterobofile, std::ios_base::binary);
            of_out << if_m.rdbuf();
            if_m.close();
        }
        if (boost::filesystem::exists(obofile)) {
            std::ifstream if_obo(obofile, std::ios_base::binary);
            of_out << if_obo.rdbuf();
            if_obo.close();
        }
        of_out.close();
        return ret;
    }

    void ReadConfigurationFile(std::string filename,
            std::map<std::string, int> & depthmap,
            std::map<std::string, std::set<std::string>> &subsetmap) {
        std::ifstream ifsfile(filename);
        if (ifsfile.is_open()) {
            std::string line;
            while (getline(ifsfile, line)) {
                if (!line.empty()) {
                    std::vector<std::string> splits;
                    boost::split(splits, line, boost::is_any_of(" "));
                    std::string word(splits[0]);
                    int d(stof(splits[1]));
                    if (!word.empty()) {
                        depthmap.insert(std::make_pair(word, d));
                        for (int i = 2; i < splits.size(); i++)
                            subsetmap[word].insert(splits[i]);
                    }
                }
            }
            ifsfile.close();
        } else {
            std::cerr << "Unable to open file ";
            std::cerr << filename << std::endl;
        }
    }

    std::string ConcatenateStrings(const std::vector<std::string> & sv, std::string separator) {
        std::string ret(sv.empty() ? "" : sv[0]);
        for (int i = 1; i < sv.size(); i++)
            ret.append(separator + sv[i]);
        return ret;
    }

    int CountWords(std::string str) {
        std::vector< std::string > result;
        boost::algorithm::split_regex(result, str, boost::regex("\\s+"));
        return result.size();
    }

    void CreateTable(pqxx::connection & cn, std::string tablename,
            const std::vector<std::string> & cols) {
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from pg_tables where tablename='";
        pc << tablename << "'";
        r = w.exec(pc.str());
        if (r.size() != 0) {
            pc.str("");
            pc << "drop table \"" << tablename << "\"";
            r = w.exec(pc.str());
        }
        pc.str("");
        pc << "create table \"" << tablename << "\" ";
        pc << "(";
        pc << "iid serial primary key";
        for (auto x : cols) pc << ", " << x;
        pc << ")";
        r = w.exec(pc.str());
        pc.str("");
        pc << "GRANT ALL PRIVILEGES ON TABLE \"";
        pc << tablename;
        pc << "\" TO \"www-data\"";
        r = w.exec(pc.str());
        w.commit();
    }

    void PrintHelp(const char* argv0,
            boost::program_options::options_description desc) {
        std::cout << argv0 << " ";
        std::cout << "version ";
        std::cout << TPSO_VERSION_MAJOR << ".";
        std::cout << TPSO_VERSION_MINOR << ".";
        std::cout << TPSO_VERSION_PATCH << ". ";
        std::cout << "Build date:  " << __DATE__ << std::endl;
        std::cout << desc << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Name of first level node in JSON file serves as "
                "task specification.\n"
                "Children of first level node specify input parameters.\n"
                "Several tasks can be run from the same JSON file.\n"
                "\n"
                "Tasks:\n"
                "\n"
                "\"process obofiles in directory\"\n"
                "\n"
                "\tprocess all directories in \"obo directory\",\n"
                "\tgenerating postgres tables\n"
                "\t" << PGONTOLOGYTABLENAME << "_<base of obofilename>,\n"
                "\t" << PCRELATIONSTABLENAME << "_<base of obofilename> and\n"
                "\t<base of obofilename> is the base filename of each\n"
                "\tobo file in \"obo directory\".\n"
                "\tEach <base of obofilename> will be added to table "
                << ONTOLOGYMEMBERSTABLENAME << ".\n";
    }

    void GetFilesFromDir(const std::string & dirname,
            std::set<std::string> & files) {
        files.clear();
        boost::filesystem::directory_iterator end_itr; // Default constr yields past-the-end
        if (boost::filesystem::is_directory(dirname))
            for (boost::filesystem::directory_iterator itr(dirname); itr != end_itr; ++itr)
                if (is_regular_file(itr->path()))
                    files.insert(itr->path().string());
    }

    std::string GetNodeName(tree<pss>::pre_order_iterator it,
            OboFileSegmentation * ofs = nullptr) {
        std::string ret((*it).first);
        if (ofs != nullptr) {
            OboEntry * oeptr = ofs->GetOePtr((*it).first);
            if (oeptr != nullptr)
                ret = oeptr->GetName((*it).first)
                + std::string(" (") + (*it).first + std::string(")");
        }
        return ret;
    }

    void PrintTree(const tree<pss>&tr,
            tree<pss>::pre_order_iterator it,
            tree<pss>::pre_order_iterator end,
            std::ofstream & o,
            OboFileSegmentation * ofs = nullptr) {
        if (!tr.is_valid(it)) return;
        int rootdepth = tr.depth(it);
        while (it != end) {
            for (int i = 0; i < tr.depth(it) - rootdepth; ++i)
                o << "  ";
            o << "(" << tr.depth(it) << ") ";
            o << GetNodeName(it, ofs) << std::endl << std::flush;
            ++it;
        }
    }

    std::vector<std::vector<std::string>> PopulateDataVector(
            const tree<pss>& tr,
            const tree<pss>::pre_order_iterator it,
            OboFileSegmentation * ofs, const std::string irrverbfilename,
            const std::string irrpluralfilename, const std::string allverbsfilename) {
        std::vector<std::vector < std::string>> ret;
        OboEntry * oentry(ofs->GetOePtr((*it).first));
        ret.clear();
        //
        std::string id((*it).first);
        //
        std::vector<std::string>
                xrefs((oentry == nullptr) ? std::vector<std::string>()
                : oentry->GetData("xref"));
        //
        std::string term((oentry == nullptr) ? (*it).first
                : oentry->GetName((*it).first));
        //
        OboEntry * catoentry(ofs->GetOePtr((*it).second));
        std::string category;
        if (catoentry == nullptr)
            category = (*it).second;
        else
            category = catoentry->GetName((*it).second)
            + std::string(" (") + (*it).second + std::string(")");
        //
        std::string attributes("");
        //
        std::string annotationtype("lexical");
        //
        std::string lexicalvariations("");
        //
        std::string curationstatus("final");
        //
        std::string curationuse("");
        //
        std::vector<std::string>
                namespaces((oentry == nullptr) ? std::vector<std::string>()
                : oentry->GetData("namespace"));
        //
        std::string owner("Textpresso");
        //
        std::string source("obofile");
        //
        std::string version("1.0");
        if (term.length() >= MINTERMLENGTH) // only store terms with length of two or longer
            if (CountWords(term) <= MAXWORDCOUNT) { // only store terms with less than 5 words
                std::vector<std::string> d;
                d.clear();
                // id
                d.push_back(id);
                // allxrefs
                d.push_back(ConcatenateStrings(xrefs, ", "));
                // term
                d.push_back(term);
                // category
                d.push_back(category);
                // attributes
                d.push_back(attributes);
                // annotationtype
                d.push_back(annotationtype);
                // lexical variations
                d.push_back(lexicalvariations);
                // curation status
                d.push_back(curationstatus);
                // curation use
                d.push_back(curationuse);
                // comments
                d.push_back(ConcatenateStrings(namespaces, ", "));
                // owner
                d.push_back(owner);
                // source
                d.push_back(source);
                // version
                d.push_back(version);
                std::stringstream ss;
                ss << time(0);
                d.push_back(ss.str());
                ret.push_back(d);
            }
        std::vector<std::string>
                synonymsraw((oentry == nullptr) ? std::vector<std::string>()
                : oentry->GetData("synonym"));
        std::vector<std::string> synonyms;
        for (auto s : synonymsraw)
            if (s.find("EXACT") != std::string::npos) {
                std::vector<std::string> sp2;
                boost::split(sp2, s, boost::is_any_of("\""));
                synonyms.push_back(sp2[1]);
            }
        while (!synonyms.empty()) {
            std::string synterm = synonyms.back();
            synonyms.pop_back();
            if (synterm.length() >= MINTERMLENGTH) // only store terms with length of two or longer
                if (CountWords(synterm) <= MAXWORDCOUNT) { // only store terms with less than 5 words
                    std::vector<std::string> d;
                    d.clear();
                    // id
                    d.push_back(id);
                    // allxrefs
                    d.push_back(ConcatenateStrings(xrefs, ", "));
                    // term
                    d.push_back(synterm);
                    // category
                    d.push_back(category);
                    // attributes
                    d.push_back(attributes);
                    // annotationtype
                    d.push_back(annotationtype);
                    // lexical variations
                    d.push_back(lexicalvariations);
                    // curation status
                    d.push_back(curationstatus);
                    // curation use
                    d.push_back(curationuse);
                    // comments
                    d.push_back(ConcatenateStrings(namespaces, ", "));
                    // owner
                    d.push_back(owner);
                    // source
                    d.push_back(source);
                    // version
                    d.push_back(version);
                    std::stringstream ss;
                    ss << time(0);
                    d.push_back(ss.str());
                    ret.push_back(d);
                }
        }
        return ret;
    }

    std::set<pss> FindPwoP(const mms & pcr) {
        //find parents without parents;
        std::set<pss> ret;
        ret.clear();
        std::set<pss> parents;
        for (auto mmsit = pcr.begin(); mmsit != pcr.end(); mmsit++)
            parents.insert(std::make_pair((*mmsit).first, ""));
        std::set<pss> children;
        for (auto ssit = parents.begin(); ssit != parents.end(); ssit++) {
            std::pair<mms::const_iterator, mms::const_iterator> mmsii
                    = pcr.equal_range((*ssit).first);
            for (auto mmsit2 = mmsii.first; mmsit2 != mmsii.second; mmsit2++)
                children.insert(std::make_pair((*mmsit2).second, ""));
        }
        for (auto p : parents)
            if (children.find(p) == children.end()) ret.insert(p);

        return ret;
    }

    std::set<pss> ListCofP(const mms & pcr, const pss p) {
        //list children of parent
        std::set<pss> ret;
        std::pair<mms::const_iterator, mms::const_iterator> mmsii
                = pcr.equal_range(p.first);
        for (auto mmsit2 = mmsii.first; mmsit2 != mmsii.second; mmsit2++)
            ret.insert(std::make_pair((*mmsit2).second, ""));

        return ret;
    }

    bool IsParentUpstream(tree<pss> & growntree,
            tree<pss>::pre_order_iterator testit) {
        bool ret(false);
        tree<pss>::pre_order_iterator it(testit);
        while (it != growntree.begin()) {
            it = growntree.parent(it);
            if (*it == *testit) ret = true;
        }
        return ret;
    }

    void AddAllParentsUpstream(std::set<std::string> & parents, const tree<pss> & growntree,
            tree<pss>::iterator it) {
        while (it != growntree.begin()) {
            it = growntree.parent(it);
            parents.insert((*it).first);
        }
    }

    void GrowTreeFromObo(OboFileSegmentation * ofs,
            tree<pss> &growntree, int depth, std::set<std::string> & subsets) {
        growntree.clear();
        auto root = growntree.insert(growntree.begin(), std::make_pair("root", "root"));
        mms pcr(ofs->GetElementaryPcRelationships());
        std::set<pss> pwop(FindPwoP(pcr));
        std::vector<tree<pss>::pre_order_iterator> currentparents;
        for (auto c : pwop)
            currentparents.push_back(growntree.append_child(root, c));
        std::vector<tree<pss>::pre_order_iterator> nextparents;
        while (!currentparents.empty()) {
            nextparents.clear();
            for (auto p : currentparents)
                for (auto c : ListCofP(pcr, *p))
                    nextparents.push_back(growntree.append_child(p, c));
            currentparents.clear();
            while (!nextparents.empty()) {
                if (!IsParentUpstream(growntree, nextparents.back()))
                    currentparents.push_back(nextparents.back());
                nextparents.pop_back();
            }
        }
        // set restriction on category: depth of leaf
        for (auto it = growntree.begin(); it != growntree.end(); it++) {
            if (growntree.depth(it) < depth)
                (*it).second = (*it).first;
            else
                (*it).second = (*growntree.parent(it)).second;
        }
        // set restriction on category: number of children
        for (auto it = growntree.begin(); it != growntree.end(); it++)
            if (it.number_of_children() > 200)
                for (auto cit = it.begin(); cit != it.end(); cit++)
                    (*cit).second = (*it).second;
        //        // set restriction on category: must be in subsets
        //        for (tree<pss>::post_order_iterator it = growntree.begin_post();
        //                it != growntree.end_post(); it++) {
        //            OboEntry * oey(ofs->GetOePtr((*it).first));
        //            if (oey != nullptr)
        //                if (!subsets.empty())
        //                    if (!(ofs->GetOePtr((*it).first)->IsInSubsets(subsets)))
        //                        (*it).second = (*growntree.parent(it)).second;
        //        }
        // set restriction on category: node must be in subset, also include
        // its parents
        if (!subsets.empty()) {
            std::set<std::string> included;
            included.clear();
            for (tree<pss>::post_order_iterator it = growntree.begin_post();
                    it != growntree.end_post(); it++) {
                OboEntry * oey(ofs->GetOePtr((*it).first));
                if (oey != nullptr)
                    if ((ofs->GetOePtr((*it).first)->IsInSubsets(subsets))) {
                        included.insert((*it).first);
                        AddAllParentsUpstream(included, growntree, it);
                    }
            }
            for (tree<pss>::post_order_iterator it = growntree.begin_post();
                    it != growntree.end_post(); it++) {
                if (included.find((*it).first) == included.end())
                    (*it).second = (*growntree.parent(it)).second;
            }
        }
    }

    void WriteOntologyToPg(pqxx::connection & cn, std::string tablename,
            const std::vector<std::string> & cols,
            const std::vector<std::vector<std::string>> &data) {
        pqxx::work w(cn);
        pqxx::tablewriter tt(w, "\"" + tablename + "\"", cols.begin(), cols.end());

        for (auto d : data)
            tt.push_back(d.begin(), d.end());
        tt.complete();
        w.commit();
    }

    void WriteRelationsToPg(std::multimap<std::string, std::string> & pcs, pqxx::connection & cn,
            std::string col1, std::string col2, std::string tablename) {
        std::vector<std::string> colpcs;
        colpcs.clear();
        colpcs.push_back(col1);
        colpcs.push_back(col2);
        pqxx::work wpc(cn);
        pqxx::tablewriter ttpc(wpc, "\"" + tablename + "\"", colpcs.begin(), colpcs.end());
        int ctpc = 0;
        for (std::multimap<std::string, std::string>::iterator it = pcs.begin(); it != pcs.end(); it++) {

            std::vector<std::string> d;
            d.push_back(((*it).first).substr(0, CATEGORYCOLUMNWIDTH));
            d.push_back(((*it).second).substr(0, CATEGORYCOLUMNWIDTH));
            ttpc.push_back(d);
            ctpc++;
        }
        ttpc.complete();
        wpc.commit();
        std::cerr << tablename << ": " << ctpc << " rows inserted." << std::endl;
    }

    void WriteRelationstables(pqxx::connection & cn,
            const tree<pss> & growntree,
            const std::string stem, OboFileSegmentation * ofs) {
        std::vector<std::string> tablecolspc{
            "parent varchar(" + std::to_string(CATEGORYCOLUMNWIDTH) + std::string(")"),
            "child varchar(" + std::to_string(CATEGORYCOLUMNWIDTH) + std::string(")")};
        std::string tablenamepc(PCRELATIONSTABLENAME + std::string("_") + stem);
        CreateTable(cn, tablenamepc, tablecolspc);
        std::multimap<std::string, std::string> pcs;
        std::set<std::string> seen;
        auto rootit = growntree.begin();
        std::string allchildren("");
        auto nexttolast = rootit.end()--;
        nexttolast--;
        std::string rootname(GetNodeName(rootit, ofs));
        for (auto it = rootit.begin(); it != rootit.end(); it++) {
            std::string teststring((*rootit).first + "-" + (*it).first);
            if (seen.find(teststring) == seen.end()) {
                seen.insert(teststring);
                std::string c(GetNodeName(it, ofs));
                pcs.insert(std::make_pair(rootname, c));
                allchildren += c;
                if (it != nexttolast) allchildren += "|";
            }
        }
        tree<pss>::pre_order_iterator it;
        for (it = rootit.begin(); it != rootit.end(); it++) {
            auto nexttolast = it.end()--;
            nexttolast--;
            std::string allchildren("");
            std::string p(GetNodeName(it, ofs));
            for (auto cit = it.begin(); cit != it.end(); cit++) {
                if ((*cit).first == (*cit).second) {
                    std::string teststring((*it).first + "-" + (*cit).first);
                    if (seen.find(teststring) == seen.end()) {
                        seen.insert(teststring);
                        std::string c(GetNodeName(cit, ofs));
                        pcs.insert(std::make_pair(p, c));
                        allchildren += c;

                        if (cit != nexttolast) allchildren += "|";
                    }
                }
            }
        }
        WriteRelationsToPg(pcs, cn, "parent", "child", tablenamepc);
    }

    void WriteOntologytable(pqxx::connection & cn, OboFileSegmentation * ofs,
            const tree<pss> & grownidtree,
            const std::string stem, std::string irrverbfilename,
            std::string irrpluralfilename, std::string allverbsfilename) {
        std::vector<std::string> tablecols{
            "eid varchar(60)",
            "dbxref text",
            "term text",
            "category varchar(" + std::to_string(CATEGORYCOLUMNWIDTH) + std::string(")"),
            "attributes text",
            "annotationtype varchar(20)",
            "lexicalvariations text",
            "curation_status varchar(20)",
            "curation_use varchar(60)",
            "comment text",
            "owner varchar(255)",
            "source varchar(255)",
            "version varchar(255)",
            "last_update int"};
        std::vector<std::string> cols{
            "eid", "dbxref", "term", "category", "attributes", "annotationtype",
            "lexicalvariations", "curation_status", "curation_use", "comment",
            "owner", "source", "version", "last_update"};
        std::string tablenameontology(PGONTOLOGYTABLENAME + std::string("_") + stem);
        auto rootit = grownidtree.begin();
        tree<pss>::pre_order_iterator it;
        int ct(0);
        std::vector<std::vector < std::string>> collector;
        collector.clear();
        std::string actualtablename("");
        for (it = rootit.begin(); it != rootit.end(); it++) {
            if (ct++ % 1000000 == 0) {
                actualtablename = tablenameontology + std::string("_")
                        + std::to_string(int(ct / 1000000));
                CreateTable(cn, actualtablename, tablecols);
            }
            std::vector<std::vector < std::string >>
                    d(PopulateDataVector(grownidtree, it, ofs,
                    irrverbfilename, irrpluralfilename, allverbsfilename));
            collector.insert(collector.end(), d.begin(), d.end());
            if (ct % 30000 == 0) {
                WriteOntologyToPg(cn, actualtablename, cols, collector);
                std::cout << "                                              \r";
                std::cout << stem << " " << ct + 1 << "/" << grownidtree.size() << "\r" << std::flush;
                collector.clear();
            }
        }
        if (!collector.empty()) {

            WriteOntologyToPg(cn, actualtablename, cols, collector);
            std::cout << std::endl << stem << " " << ct + 1 << "/" << grownidtree.size();
            collector.clear();
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {

    int ret(EXIT_SUCCESS);
    ////
    boost::program_options::options_description desc("options");
    boost::program_options::variables_map vm;
    boost::property_tree::ptree inputtree;
    // arguments    
    std::string jsonfile(JSONFILE);
    // should introduce a class set option here:
    desc.add_options()
            ("help,h", "produce help message")
            ("jsonfile,j",
            boost::program_options::value<std::string>(&jsonfile)
            ->default_value(JSONFILE),
            "name of input json file");
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            PrintHelp(argv[0], desc);
            return EXIT_FAILURE;
        }
        boost::program_options::notify(vm);
        std::cout << "json file: " << jsonfile << std::endl;
        std::string option = "process obofiles in directory";
        boost::property_tree::read_json(jsonfile, inputtree);
        if (inputtree.find(option) != inputtree.not_found()) {
            std::cout << "option:" << option << std::endl;
            boost::property_tree::ptree node_pod =
                    inputtree.get_child(option);
            const std::string headerfiledir
                    = node_pod.get<std::string>("headerfile directory", "oboheaderfiles");
            std::cout << "headerfile directory:" << headerfiledir << std::endl;
            const std::string obodir
                    = node_pod.get<std::string>("obo directory", "obodir");
            std::cout << "obo directory:" << obodir << std::endl;
            const std::string irrverbfilename =
                    node_pod.get<std::string>("irregular verbs filename", "irrverbs.txt");
            std::cout << "irregular verbs filename:" << irrverbfilename << std::endl;
            const std::string irrpluralfilename =
                    node_pod.get<std::string>("irregular plurals filename", "irrplurals.txt");
            std::cout << "irregular plurals filename:" << irrpluralfilename << std::endl;
            const std::string allverbsfilename =
                    node_pod.get<std::string>("all verbs filename", "allverbs.txt");
            std::cout << "all verbs filename:" << allverbsfilename << std::endl;
            const std::string conf = node_pod.get<std::string>("ontology configuration file", "ontology.conf");
            std::cout << "ontology configuration file:" << conf << std::endl;
            const int depth = node_pod.get<unsigned int>("ontology default depth", 3);
            std::cout << "ontology default depth:" << depth << std::endl;

            std::map<std::string, int> depthmap;
            depthmap.clear();
            std::map<std::string, std::set < std::string>> subsetmap;
            ReadConfigurationFile(conf, depthmap, subsetmap);
            std::set<std::string> obofiles;
            obofiles.clear();
            GetFilesFromDir(obodir, obofiles);
            PgList ontmembers(PGONTOLOGY, ONTOLOGYMEMBERSTABLENAME);
            std::vector<std::thread*> thread_vec;
            thread_vec.clear();
            for (auto x : obofiles) {
                int d(depth);
                if (depthmap.find(x) != depthmap.end()) d = depthmap[x];
                std::set<std::string> subsets = {};
                if (subsetmap.find(x) != subsetmap.end()) subsets = subsetmap[x];
                boost::filesystem::path p(x);
                std::string stem(p.stem().string());
                ontmembers.AddItem(stem);
                std::string headerfile(headerfiledir + "/" + stem + ".headerobo");
                std::string obofile(ConcatenateFiles(headerfile, x));
                thread_vec.push_back(new std::thread([ = ]
                        (std::string stem, std::string x, int d, std::set<std::string> subsets){
                    pqxx::connection cn(PGONTOLOGY);
                    OboFileSegmentation * ofs = new OboFileSegmentation(x.c_str());
                    tree<pss> growntree;
                    GrowTreeFromObo(ofs, growntree, d, subsets);
                    std::ofstream treefile(stem + ".tree");
                    if (treefile.is_open()) {
                        PrintTree(growntree, growntree.begin(), growntree.end(),
                                treefile, ofs);
                                treefile.close();
                    } else {
                        std::cerr << stem + ".tree could not be opened." << std::endl;
                    }
                    WriteRelationstables(cn, growntree, stem, ofs);
                    WriteOntologytable(cn, ofs, growntree, stem,
                    irrverbfilename, irrpluralfilename, allverbsfilename);
                    delete ofs;
                    cn.disconnect();
                }, stem, obofile, d, subsets));
            }
            while (thread_vec.size() > 0) {
                thread_vec.back()->join();
                delete thread_vec.back();
                thread_vec.pop_back();
            }
        }
        option = "dummy";
        if (inputtree.find(option) != inputtree.not_found()) {
            std::cout << "option:" << option << std::endl;
            boost::property_tree::ptree node_dummy =
                    inputtree.get_child(option);
            const std::string dfile = node_dummy.get<std::string>("dummy file", "dummy.file");
            std::cout << "dummy file:" << dfile << std::endl;
        }
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return (EXIT_FAILURE);
    }
    return ret;
}
