/* 
 * File:   TpOntApi.cpp
 * Author: mueller
 * 
 * Created on May 29, 2013, 12:13 PM
 */

#include "TpOntApi.h"
#include "tree.h"
#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace {

    std::vector<std::string> GetAllFileList(std::string dir) {
        boost::filesystem::path po(dir);
        std::vector<std::string> ret;
        if (boost::filesystem::exists(po)) {
            boost::filesystem::directory_iterator end_itr; // Default cstrctor yields past-the-end
            for (boost::filesystem::directory_iterator i(po); i != end_itr; ++i) {
                // Skip if not a file
                if (!boost::filesystem::is_regular_file(i->status())) continue;
                // Skip if no match
                if (i->path().string().find(".term.") != std::string::npos) continue;
                if (i->path().string().find(".elem.") == std::string::npos) continue;
                ret.push_back(i->path().string());
            }
        }
        return ret;
    }

    tree<std::string> Obo2Tree(std::string dir) {
        std::vector<std::string> files = GetAllFileList(dir);
        tree<std::string> ret;
        tree<std::string>::iterator root = ret.insert(ret.begin(), "trivialroot");
        for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++) {
            boost::filesystem::path p(*it);
            std::string aux = p.extension().string();
            std::string parentstring = aux.substr(1, aux.length() - 1);
            tree<std::string>::iterator parent = ret.append_child(root, parentstring);
            std::ifstream f(p.string().c_str());
            std::string in;
            while (getline(f, in)) {
                ret.append_child(parent, in);
            }
            f.close();
        }
        return ret;
    }

    void WriteTppc(tree<std::string> & tr, std::ostream & soutstream, bool alldescendants = false) {
        // alldescendants = false: only write out direct children of root's children
        // alldescendants = true: write out direct children, their children, and so on.
        tree<std::string>::iterator root = tr.begin();
        tree<std::string>::sibling_iterator sit, cit;
        for (sit = root.begin(); sit != root.end(); sit++) {
            soutstream << *sit;
            std::set<std::string> seen;
            if (alldescendants) {
                tree<std::string>::pre_order_iterator pit(sit.begin());
                while (pit != sit.end()) {
                    if (seen.find(*pit) == seen.end()) {
                        seen.insert(*pit);
                        soutstream << "\t" << *pit;
                    }
                    pit++;
                }
            } else {
                for (cit = sit.begin(); cit != sit.end(); cit++) {
                    if (seen.find(*cit) == seen.end()) {
                        seen.insert(*cit);
                        soutstream << "\t" << *cit;
                    }
                }
            }
            soutstream << std::endl;
        }
    }

    tree<std::string> ReadTppc(std::string filename) {
        tree<std::string> ret;
        tree<std::string>::iterator root = ret.insert(ret.begin(), "trivialroot");
        std::ifstream f(filename.c_str());
        std::string in;
        while (getline(f, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of("\t"));
            tree<std::string>::iterator parent = ret.append_child(root, splits[0]);
            std::vector<std::string>::iterator it;
            for (it = ++splits.begin(); it != splits.end(); it++)
                ret.append_child(parent, *it);
        }
        f.close();
        return ret;
    }

    void print_tree(const tree<std::string>& tr, tree<std::string>::pre_order_iterator it, tree<std::string>::pre_order_iterator end) {
        if (!tr.is_valid(it)) return;
        int rootdepth = tr.depth(it);
        while (it != end) {
            for (int i = 0; i < tr.depth(it) - rootdepth; ++i)
                std::cout << "  ";
            std::cout << "(" << tr.depth(it) << ") ";
            std::cout << (*it) << std::endl << std::flush;
            ++it;
        }
    }

    std::map<std::string, std::string> MapNames(std::string dir) {
        boost::filesystem::path po(dir);
        std::vector<std::string> filelist;
        if (boost::filesystem::exists(po)) {
            boost::filesystem::directory_iterator end_itr; // Default constructor yields past-the-end
            for (boost::filesystem::directory_iterator i(po); i != end_itr; ++i) {
                // Skip if not a file
                if (!boost::filesystem::is_regular_file(i->status())) continue;
                // Skip if not an ids-names file
                if (i->path().string().find(".ids-names") == std::string::npos) continue;
                filelist.push_back(i->path().string());
            }
        }
        // map
        std::map<std::string, std::string> ret;
        for (std::vector<std::string>::iterator it = filelist.begin(); it != filelist.end(); it++) {
            std::ifstream infile((*it).c_str());
            std::string in;
            while (getline(infile, in)) {
                std::vector<std::string> splits;
                boost::split(splits, in, boost::is_any_of("\t"));
                ret[splits[0]] = splits[1];
            }
            infile.close();
        }
        return ret;
    }

    int WriteSingleOboIntoTable(std::string filename, std::string categoryname,
            std::string ontsource, pqxx::tablewriter & tt) {
        int ret = 0;
        std::ifstream f(filename.c_str());
        // currently filled fields:
        std::string id;
        std::string term;
        std::vector<std::string> synonyms;
        std::vector<std::string> xref;
        std::string category(categoryname);
        std::string comment;
        std::string curation_status("final");
        std::string annotationtype("lexical");
        std::string owner("textpresso");
        std::string version("1.0");
        std::string source(ontsource);
        // currently unused fields:
        std::string attributes;
        std::string curation_use;
        std::string lexicalvariations;
        std::string alldbxref;
        std::string in;
        std::set<std::string> seenterms;
        while (getline(f, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of(" "));
            if (splits[0].compare("id:") == 0) {
                id = splits[1];
            } else if (splits[0].compare("name:") == 0) {
                term = splits[1];
                for (size_t i = 2; i < splits.size(); i++)
                    term += " " + splits[i];
                seenterms.insert(term);
                boost::erase_all(term, "\"");
            } else if (splits[0].compare("synonym:") == 0) {
                if (in.find("EXACT") != std::string::npos) {
                    std::vector<std::string> sp2;
                    boost::split(sp2, in, boost::is_any_of("\""));
                    if (seenterms.find(sp2[1]) == seenterms.end()) {
                        synonyms.push_back(sp2[1]);
                        seenterms.insert(sp2[1]);
                    }
                }
            } else if (splits[0].compare("xref:") == 0) {
                xref.push_back(splits[1]);
            } else if (splits[0].compare("namespace:") == 0) {
                comment = in;
            }
        }
        f.close();
        if (!xref.empty()) {
            alldbxref = xref.back();
            xref.pop_back();
            while (!xref.empty()) {
                alldbxref += " " + xref.back();
                xref.pop_back();
            }
        }
        std::vector<std::string> d;
        d.clear();
        if (term.length() > 1) { // only store terms with length of two or longer
            d.push_back(id);
            d.push_back(alldbxref);
            d.push_back(term);
            d.push_back(category);
            d.push_back(attributes);
            d.push_back(annotationtype);
            d.push_back(lexicalvariations);
            d.push_back(curation_status);
            d.push_back(curation_use);
            d.push_back(comment);
            d.push_back(owner);
            d.push_back(source);
            d.push_back(version);
            std::stringstream ss;
            ss << time(0);
            d.push_back(ss.str());
            tt.push_back(d);
            ret++;
        }
        while (!synonyms.empty()) {
            std::vector<std::string> d;
            d.clear();
            term = synonyms.back();
            synonyms.pop_back();
            if (term.length() > 1) { // only store terms with length of two or longer
                d.push_back(id);
                d.push_back(alldbxref);
                d.push_back(term);
                d.push_back(category);
                d.push_back(attributes);
                d.push_back(annotationtype);
                d.push_back(lexicalvariations);
                d.push_back(curation_status);
                d.push_back(curation_use);
                d.push_back(comment);
                d.push_back(owner);
                d.push_back(source);
                d.push_back(version);
                std::stringstream ss;
                ss << time(0);
                d.push_back(ss.str());
                tt.push_back(d);
                ret++;
            }
        }
        return ret;
    }

    void WritePcsToPg(std::multimap<std::string, std::string> & pcs, pqxx::connection & cn,
            std::string col1, std::string col2, std::string tablename) {
        // pcrelationstablename_ postgres operations
        std::vector<std::string> colpcs;
        colpcs.clear();
        colpcs.push_back(col1);
        colpcs.push_back(col2);
        pqxx::work wpc(cn);
        pqxx::tablewriter ttpc(wpc, tablename, colpcs.begin(), colpcs.end());
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

    void WritePadcsToPg(std::map<std::string, std::string> & pcs, pqxx::connection & cn,
            std::string col1, std::string col2, std::string tablename) {
        // padcrelationstablename_ postgres operations
        std::vector<std::string> colpcs;
        colpcs.clear();
        colpcs.push_back(col1);
        colpcs.push_back(col2);
        pqxx::work wpc(cn);
        pqxx::tablewriter ttpc(wpc, tablename, colpcs.begin(), colpcs.end());
        int ctpc = 0;
        for (std::map<std::string, std::string>::iterator it = pcs.begin(); it != pcs.end(); it++) {
            std::vector<std::string> d;
            d.push_back(((*it).first).substr(0, CATEGORYCOLUMNWIDTH));
//            d.push_back(((*it).second).substr(0, CATEGORYCOLUMNWIDTH));
            d.push_back(((*it).second));
            ttpc.push_back(d);
            ctpc++;
        }
        ttpc.complete();
        wpc.commit();
        std::cerr << tablename << ": " << ctpc << " rows inserted." << std::endl;
    }

    void WriteTpontologyToPg(std::set<std::string> & ids, std::map<std::string,
            std::string> & id_names, pqxx::connection & cn, std::string dir,
            std::string fileprefix, std::string source, int depth,
            std::string ontologytablename) {
        // populate tpontology
        // select parents of direct children for all nodes up to depth - 1
        // already done above
        std::set<std::string> seen;
        std::vector<std::string> cols;
        cols.clear();
        cols.push_back("eid");
        cols.push_back("dbxref");
        cols.push_back("term");
        cols.push_back("category");
        cols.push_back("attributes");
        cols.push_back("annotationtype");
        cols.push_back("lexicalvariations");
        cols.push_back("curation_status");
        cols.push_back("curation_use");
        cols.push_back("comment");
        cols.push_back("owner");
        cols.push_back("source");
        cols.push_back("version");
        cols.push_back("last_update");
        pqxx::work w(cn);
        int ct = 0;
        pqxx::tablewriter tt(w, ontologytablename, cols.begin(), cols.end());
        // 0 to <depth>-1;
        for (std::set<std::string>::iterator it = ids.begin(); it != ids.end(); it++) {
            std::string catname;
            if (id_names.find(*it) != id_names.end())
                catname = id_names[*it] + " (" + *it + ")";
            else
                catname = *it;
            std::string filename = dir + "/" + fileprefix + ".single." + *it;
            std::string teststring(filename + "#" + catname);
            if (seen.find(teststring) == seen.end()) {
                seen.insert(teststring);
                ct += WriteSingleOboIntoTable(filename,
                        catname.substr(0, CATEGORYCOLUMNWIDTH), source, tt);
            }
        }
        // select parents and all descendants for nodes at depth.
        // category name for all these is the same (i.e., the parent's name)!!!
        // parents at <depth>;
        std::stringstream desc;
        desc << dir << "/" << fileprefix << "." << depth << ".all.descendants.tppc";
        std::ifstream infile(desc.str().c_str());
        std::string in;
        while (getline(infile, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of("\t"));
            std::string parent = splits[0];
            std::string catname;
            if (id_names.find(parent) != id_names.end())
                catname = id_names[parent] + " (" + parent + ")";
            else
                catname = parent;
            // don't forget parent itself:
            std::string filename = dir + "/" + fileprefix + ".single." + parent;
            // parents at <depth>
            std::string teststring(filename + "#" + catname);
            if (seen.find(teststring) == seen.end()) {
                seen.insert(teststring);
                ct += WriteSingleOboIntoTable(filename, catname, source, tt);
            }
            std::vector<std::string>::iterator it;
            for (it = ++splits.begin(); it != splits.end(); it++) {
                filename = dir + "/" + fileprefix + ".single." + *it;
                // all descendants of parents at <depth>
                std::string teststring(filename + "#" + catname);
                if (seen.find(teststring) == seen.end()) {
                    seen.insert(teststring);
                    ct += WriteSingleOboIntoTable(filename, catname, source, tt);
                }
            }
        }
        infile.close();
        tt.complete();
        w.commit();
        std::cerr << ontologytablename << ": " << ct << " rows inserted." << std::endl;
    }


}

TpOntApi::TpOntApi(std::string tablename, std::string pcrelationstbn,
        std::string padcrelationstbn)
: cn_(PGONTOLOGY), ontologytablename_(tablename), pcrelationstablename_(pcrelationstbn),
padcrelationstablename_(padcrelationstbn) {
}

TpOntApi::~TpOntApi() {
    DeleteAllResults();
    cn_.disconnect();
}

void TpOntApi::CreatePcRelationsTable() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << pcrelationstablename_ << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << pcrelationstablename_ << " table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table ";
        pc << pcrelationstablename_ << " ";
        pc << "(";
        pc << "iid serial primary key";
        pc << ", parent varchar(" << CATEGORYCOLUMNWIDTH << ")";
        pc << ", child varchar(" << CATEGORYCOLUMNWIDTH << ")";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();
}

void TpOntApi::CreatePadcRelationsTable() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << padcrelationstablename_ << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << padcrelationstablename_ << " table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table ";
        pc << padcrelationstablename_ << " ";
        pc << "(";
        pc << "iid serial primary key";
        pc << ", parent varchar(" << CATEGORYCOLUMNWIDTH << ")";
        pc << ", children text";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();
}

void TpOntApi::CreateOntologyTable() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << ontologytablename_ << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << ontologytablename_ << " table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table ";
        pc << ontologytablename_ << " ";
        pc << "(";
        pc << "iid serial primary key";
        pc << ", eid varchar(60)";
        pc << ", dbxref text";
        pc << ", term text";
        pc << ", category varchar(" << CATEGORYCOLUMNWIDTH << ")";
        pc << ", attributes text";
        pc << ", annotationtype varchar(20)";
        pc << ", lexicalvariations text";
        pc << ", curation_status varchar(20)";
        pc << ", curation_use varchar(60)";
        pc << ", comment text";
        pc << ", owner varchar(255)";
        pc << ", source varchar(255)";
        pc << ", version varchar(255)";
        pc << ", last_update int";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();
}

void TpOntApi::CreateOntologyIndex() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << ontologytablename_ << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << ontologytablename_ << " table exists." << std::endl;
        pc.str("");
        pc << "create index ";
        pc << ontologytablename_ << "_category_idx on ";
        pc << ontologytablename_ << "(category)";
        r = w.exec(pc.str());
        pc.str("");
        pc << "create index ";
        pc << ontologytablename_ << "_term_idx on ";
        pc << ontologytablename_ << "(term)";
        r = w.exec(pc.str());
    }
    w.commit();
}

void TpOntApi::PopulatePcRelationsFromFile(const char* filename) {
    std::vector<std::string> cols;
    cols.clear();
    cols.push_back("parent");
    cols.push_back("child");
    pqxx::work w(cn_);
    pqxx::tablewriter tt(w, pcrelationstablename_, cols.begin(), cols.end());
    std::ifstream f(filename);
    std::string in;
    int ct = 0;
    while (getline(f, in)) {
        std::vector<std::string> splits;
        boost::split(splits, in, boost::is_any_of("\t"));
        std::vector<std::string> d;
        d.push_back(splits[0].substr(0, CATEGORYCOLUMNWIDTH));
        d.push_back(splits[1].substr(0, CATEGORYCOLUMNWIDTH));
        tt.push_back(d);
        ct++;
    }
    f.close();
    tt.complete();
    w.commit();
    std::cerr << ct << " rows inserted." << std::endl;
}

void TpOntApi::PopulateTpOntologyFromFile(const char* filename) {
    std::vector<std::string> cols;
    cols.clear();
    cols.push_back("eid");
    cols.push_back("dbxref");
    cols.push_back("term");
    cols.push_back("category");
    cols.push_back("attributes");
    cols.push_back("annotationtype");
    cols.push_back("lexicalvariations");
    cols.push_back("curation_status");
    cols.push_back("curation_use");
    cols.push_back("comment");
    cols.push_back("owner");
    cols.push_back("source");
    cols.push_back("version");
    cols.push_back("last_update");
    pqxx::work w(cn_);
    pqxx::tablewriter tt(w, ontologytablename_, cols.begin(), cols.end());
    std::ifstream f(filename);
    std::string in;
    int ct = 0;
    while (getline(f, in)) {
        int i = in.find_first_of('\t');
        std::string id = in.substr(0, i);
        int iold = i + 1;
        i = in.find('\t', iold);
        std::string dbxref = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string term = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string category = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string attributes = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string annotationtype = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string lexicalvariations = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string curation_status = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string curation_use = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string comment = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string owner = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string source = in.substr(iold, i - iold);
        iold = i + 1;
        i = in.find('\t', iold);
        std::string version = in.substr(iold, i - iold);
        std::vector<std::string> d;

        d.push_back(id);
        d.push_back(dbxref);
        d.push_back(term);
        d.push_back(category.substr(0, CATEGORYCOLUMNWIDTH));
        d.push_back(attributes);
        d.push_back(annotationtype);
        d.push_back(lexicalvariations);
        d.push_back(curation_status);
        d.push_back(curation_use);
        d.push_back(comment);
        d.push_back(owner);
        d.push_back(source);
        d.push_back(version);
        std::stringstream ss;
        ss << time(0);
        d.push_back(ss.str());
        tt.push_back(d);
        ct++;
    }
    f.close();
    tt.complete();
    w.commit();
    std::cerr << ct << " rows inserted." << std::endl;
}

void TpOntApi::PopulateTpOntologyFromOboFile(const char * filename,
        std::string category, const char * ccsource) {
    std::vector<std::string> cols;
    cols.clear();
    cols.push_back("eid");
    cols.push_back("dbxref");
    cols.push_back("term");
    cols.push_back("category");
    cols.push_back("attributes");
    cols.push_back("annotationtype");
    cols.push_back("lexicalvariations");
    cols.push_back("curation_status");
    cols.push_back("curation_use");
    cols.push_back("comment");
    cols.push_back("owner");
    cols.push_back("source");
    cols.push_back("version");
    cols.push_back("last_update");
    pqxx::work w(cn_);
    pqxx::tablewriter tt(w, ontologytablename_, cols.begin(), cols.end());
    std::ifstream f(filename);
    std::string in;
    int ct = 0;
    bool datarecording = false;
    std::vector<std::string> datalines;
    while (getline(f, in)) {
        if (datarecording)
            if (in.length() == 0) {
                datarecording = false;
                // currently filled fields:
                std::string id;
                std::string term;
                std::vector<std::string> synonyms;
                std::vector<std::string> xref;
                std::string comment;
                std::string curation_status("final");
                std::string annotationtype("lexical");
                std::string owner("textpresso");
                std::string version("1.0");
                std::string source(ccsource);
                // currently unused fields:
                std::string attributes;
                std::string curation_use;
                std::string lexicalvariations;
                std::set<std::string> seenterms;
                while (!datalines.empty()) {
                    std::vector<std::string> splits;
                    boost::split(splits, datalines.back(), boost::is_any_of(" "));
                    if (splits[0].compare("id:") == 0) {
                        id = splits[1];
                    } else if (splits[0].compare("name:") == 0) {
                        term = splits[1];
                        for (size_t i = 2; i < splits.size(); i++)
                            term += " " + splits[i];
                        seenterms.insert(term);
                        boost::erase_all(term, "\"");
                    } else if (splits[0].compare("synonym:") == 0) {
                        if (datalines.back().find("EXACT") != std::string::npos) {
                            std::vector<std::string> sp2;
                            boost::split(sp2, datalines.back(), boost::is_any_of("\""));
                            if (seenterms.find(sp2[1]) == seenterms.end()) {
                                synonyms.push_back(sp2[1]);
                                seenterms.insert(sp2[1]);
                            }
                        }
                    } else if (splits[0].compare("xref:") == 0) {
                        xref.push_back(splits[1]);
                    } else if (splits[0].compare("namespace:") == 0) {
                        comment = datalines.back();
                    }
                    datalines.pop_back();
                }
                std::string alldbxref;
                if (!xref.empty()) {
                    alldbxref = xref.back();
                    xref.pop_back();
                    while (!xref.empty()) {
                        alldbxref += " " + xref.back();
                        xref.pop_back();
                    }
                }
                std::vector<std::string> d;
                d.clear();
                d.push_back(id);
                d.push_back(alldbxref);
                d.push_back(term);
                d.push_back(category.substr(0, CATEGORYCOLUMNWIDTH));
                d.push_back(attributes);
                d.push_back(annotationtype);
                d.push_back(lexicalvariations);
                d.push_back(curation_status);
                d.push_back(curation_use);
                d.push_back(comment);
                d.push_back(owner);
                d.push_back(source);
                d.push_back(version);
                std::stringstream ss;
                ss << time(0);
                d.push_back(ss.str());
                tt.push_back(d);
                ct++;
                while (!synonyms.empty()) {
                    std::vector<std::string> d;
                    d.clear();
                    term = synonyms.back();
                    synonyms.pop_back();
                    d.push_back(id);
                    d.push_back(alldbxref);
                    d.push_back(term);
                    d.push_back(category.substr(0, CATEGORYCOLUMNWIDTH));
                    d.push_back(attributes);
                    d.push_back(annotationtype);
                    d.push_back(lexicalvariations);
                    d.push_back(curation_status);
                    d.push_back(curation_use);
                    d.push_back(comment);
                    d.push_back(owner);
                    d.push_back(source);
                    d.push_back(version);
                    std::stringstream ss;
                    ss << time(0);
                    d.push_back(ss.str());
                    tt.push_back(d);
                    ct++;
                }
            }
        if (datarecording)
            datalines.push_back(in);
        if (in.find("[Term]") != std::string::npos)
            datarecording = true;
    }
    f.close();
    tt.complete();
    w.commit();
    std::cerr << ct << " rows inserted." << std::endl;
}

void TpOntApi::PopulateTpOntologyAllFromTppc(std::string dir, std::string fileprefix,
        std::string source, int depth) {
    std::map<std::string, std::string> id_names(MapNames(dir));
    // select direct children;
    std::vector<std::string> dcfiles;
    for (int i = 0; i < depth; i++) {
        std::stringstream aux;
        aux << dir << "/" << fileprefix << "." << i << ".direct.children.tppc";
        dcfiles.push_back(aux.str());
    }
    // populate pcrelations and padcrelations
    std::multimap<std::string, std::string> pcs;
    std::map<std::string, std::string> padcs;
    std::set<std::string> seen;
    for (std::vector<std::string>::iterator it = dcfiles.begin(); it != dcfiles.end(); it++) {
        std::ifstream infile((*it).c_str());
        std::string in;
        while (getline(infile, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of("\t"));
            std::string parent = splits[0];
            std::string p;
            if (id_names.find(parent) != id_names.end())
                p = id_names[parent] + " (";
            p += parent;
            if (id_names.find(parent) != id_names.end())
                p += ")";
            std::vector<std::string>::iterator it;
            for (it = ++splits.begin(); it != splits.end(); it++) {
                std::string teststring(parent + "-" + *it);
                if (seen.find(teststring) == seen.end()) {
                    seen.insert(teststring);
                    std::string c;
                    if (id_names.find(*it) != id_names.end())
                        c = id_names[*it] + " (";
                    c += *it;
                    if (id_names.find(*it) != id_names.end())
                        c += ")";
                    pcs.insert(std::make_pair(p, c));
                }
            }
            it = splits.begin();
            if (++it != splits.end()) {
                std::string allchildren("");
                if (id_names.find(*it) != id_names.end())
                    allchildren = id_names[*it] + " (";
                allchildren += *it;
                if (id_names.find(*it) != id_names.end())
                    allchildren += ")";
                std::vector<std::string>::iterator it2;
                for (it2 = ++it; it2 != splits.end(); it2++) {
                    allchildren += "|";
                    if (id_names.find(*it2) != id_names.end())
                        allchildren += id_names[*it2] + " (";
                    allchildren += *it2;
                    if (id_names.find(*it2) != id_names.end())
                        allchildren += ")";
                }
                padcs.insert(std::make_pair(p, allchildren));
            }
        }
        infile.close();
    }
    // Connect 0.direct.parent with root
    std::ifstream infilep((dir + "/" + fileprefix + ".0.direct.children.tppc").c_str());
    std::string inp;
    std::string allparents("");
    while (getline(infilep, inp)) {
        std::vector<std::string> splits;
        boost::split(splits, inp, boost::is_any_of("\t"));
        std::string parent = splits[0];
        std::string p;
        if (id_names.find(parent) != id_names.end()) p = id_names[parent] + " (";
        p += parent;
        if (id_names.find(parent) != id_names.end()) p += ")";
        pcs.insert(std::make_pair("root", p));
        if (!allparents.empty()) allparents += "|";
        allparents += p;
    }
    padcs.insert(std::make_pair("root", allparents));
    infilep.close();
    WritePcsToPg(pcs, cn_, "parent", "child", pcrelationstablename_);
    // Before writing padcrelationstablename_, read from postgres what's already
    // in there and combine with results obtained from above
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select parent,children from ";
        pc << padcrelationstablename_;
        r = w.exec(pc.str());
        std::stringstream pcd;
        pcd << "delete from " << padcrelationstablename_;
        w.exec(pcd.str());
        w.commit();
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string parent;
            std::string children;
            if (r[i]["parent"].to(parent))
                if (r[i]["children"].to(children))
                    if (padcs.find(parent) != padcs.end()) {
                        // entries exist in both, pg and padcs;
                        std::vector<std::string> s1, s2;
                        boost::split(s1, padcs[parent], boost::is_any_of("|"));
                        boost::split(s2, children, boost::is_any_of("|"));
                        std::set<std::string> combined;
                        for (std::vector<std::string>::iterator it = s1.begin(); it != s1.end(); it++)
                            combined.insert(*it);
                        for (std::vector<std::string>::iterator it = s2.begin(); it != s2.end(); it++)
                            combined.insert(*it);
                        std::set<std::string>::iterator it = combined.begin();
                        padcs[parent] = *it;
                        for (std::set<std::string>::iterator it2 = ++it; it2 != combined.end(); it2++)
                            padcs[parent] += "|" + *it2;
                    } else {
                        // entry only exist in pg
                        padcs.insert(std::make_pair(parent, children));
                    }
        }
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.
        std::cerr << e.what() << std::endl;
    }
    WritePadcsToPg(padcs, cn_, "parent", "children", padcrelationstablename_);
    // store node ids for populating tpontology
    std::set<std::string> ids;
    for (std::vector<std::string>::iterator it = dcfiles.begin(); it != dcfiles.end(); it++) {
        std::ifstream infile((*it).c_str());
        std::string in;
        while (getline(infile, in)) {
            std::vector<std::string> splits;
            boost::split(splits, in, boost::is_any_of("\t"));
            std::string parent = splits[0];
            ids.insert(parent);
        }
        infile.close();
    }
    WriteTpontologyToPg(ids, id_names, cn_, dir, fileprefix, source, depth, ontologytablename_);
}

void TpOntApi::ConvertObo2Tppc(std::string dir) {
    tree<std::string> tr = Obo2Tree(dir);
    WriteTppc(tr, std::cout);
}

void TpOntApi::GrowTree(std::string pcfile, std::string treefile) {
    tree<std::string> pc = ReadTppc(pcfile);
    //
    tree<std::string>::iterator itp, itc;
    // find root in pc
    tree<std::string>::iterator rootit = pc.begin().begin(); // first root (pc.begin())
    // is trivial, introduced in ReadTppc.
    bool newroot = true;
    while (newroot) {
        newroot = false;
        for (itp = ++pc.begin(); itp != pc.end(); itp++) // ++pc.begin() to ignore trivial root
            for (itc = itp.begin(); itc != itp.end(); itc++)
                if (*itc == *rootit) { // current root is a child;
                    rootit = itp; // take its parent as a new root
                    newroot = true;
                    break;
                }
    }
    std::vector<tree<std::string>::iterator> current_parents;
    current_parents.push_back(rootit);
    std::vector<tree<std::string>::iterator> next_parents;
    tree<std::string> tr;
    tr.insert(tr.begin(), *rootit);
    while (!current_parents.empty()) {
        std::vector<tree<std::string>::iterator>::iterator cpit;
        for (cpit = current_parents.begin(); cpit != current_parents.end(); cpit++) {
            tree<std::string>::iterator t = std::find(pc.begin(), pc.end(), *(*cpit));
            do {
                tree<std::string>::iterator tit;
                for (tit = t.begin(); tit != t.end(); tit++) { // all children of *(*cpit)
                    if (std::find(next_parents.begin(), next_parents.end(), tit) == next_parents.end())
                        next_parents.push_back(tit);
                    //[1 copy your finding into endresult tr;
                    tree<std::string>::iterator trit = std::find(tr.begin(), tr.end(), *(*cpit));
                    do {
                        // only add child if it's not a child of the parent already
                        if (std::find(trit.begin(), trit.end(), *tit) == trit.end())
                            tr.append_child(trit, *tit);
                        trit = std::find(++trit, tr.end(), *(*cpit));
                    } while (trit != tr.end());
                    //1]
                }
                t = std::find(++t, pc.end(), *(*cpit));
            } while (t != pc.end());
        }
        current_parents.clear();
        while (!next_parents.empty()) {
            current_parents.push_back(next_parents.back());
            next_parents.pop_back();
        }
    }
    std::ofstream outtreed(std::string(treefile + ".direct.children.tppc").c_str());
    std::ofstream outtreea(std::string(treefile + ".all.descendants.tppc").c_str());
    WriteTppc(tr, outtreed, false);
    WriteTppc(tr, outtreea, true);
    outtreed.close();
    outtreea.close();
    // print tree as tree
    print_tree(tr, tr.begin(), tr.end());
    for (int i = 0; i < tr.max_depth(); i++) {
        std::stringstream auxd, auxa;
        auxd << treefile << "." << i << ".direct.children.tppc";
        auxa << treefile << "." << i << ".all.descendants.tppc";
        std::ofstream outauxd(auxd.str().c_str());
        std::ofstream outauxa(auxa.str().c_str());
        for (tree<std::string>::iterator it = tr.begin(); it != tr.end(); it++)
            if (tr.depth(it) == i) {

                tree<std::string> treeaux(it);
                WriteTppc(treeaux, outauxd, false);
                WriteTppc(treeaux, outauxa, true);
            }
        outauxd.close();
        outauxa.close();
    }
}

void TpOntApi::UpdateTpOntologyColumnFromFile(const char* filename, std::string condcolumn, std::string affcolumn) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::ifstream f(filename);
        std::string in;
        while (getline(f, in)) {
            int i = in.find_first_of('\t');
            std::string cond = in.substr(0, i);
            int iold = i + 1;
            i = in.find('\t', iold);
            std::string content = in.substr(iold, i - iold);
            std::stringstream pc;
            pc << "update ";
            pc << ontologytablename_ << " ";
            pc << "set ";
            pc << affcolumn << " ";
            pc << "= '";
            pc << content << "' ";
            pc << "where ";
            pc << condcolumn << " = '";
            pc << cond << "'";
            r = w.exec(pc.str());
            std::cerr << r.affected_rows() << " rows were updated." << std::endl;
        }
        f.close();
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

int TpOntApi::DeleteInDb() {
    int ret = 0;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        for (std::vector<TpOntEntry*>::iterator i = result_.begin(); i != result_.end(); i++) {
            std::stringstream pc;
            int ci = (*i)->GetIid();
            pc << "delete from ";
            pc << ontologytablename_ << " ";
            pc << "where iid = ";
            pc << ci;
            r = w.exec(pc.str());
            ret += r.affected_rows();
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
    return ret;
}

int TpOntApi::WriteDb() {
    int ret = 0;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        for (std::vector<TpOntEntry*>::iterator i = result_.begin(); i != result_.end(); i++) {
            std::stringstream pc;
            int ci = (*i)->GetIid();
            pc << "select * from \"";
            pc << ontologytablename_ << "\" ";
            pc << "where iid = ";
            pc << ci;
            std::cerr << pc.str() << std::endl;
            r = w.exec(pc.str());
            if (r.size() == 0) {
                pc.str("");
                pc << "insert into ";
                pc << ontologytablename_ << " ";
                pc << "values (default,$$";
                pc << (*i)->GetEntryId() << "$$,$$";
                pc << (*i)->GetDbXref() << "$$,$$";
                pc << (*i)->GetTerm() << "$$,$$";
                pc << (*i)->GetCategory() << "$$,$$";
                pc << (*i)->GetAttributes() << "$$,$$";
                pc << (*i)->GetAnnotationType() << "$$,$$";
                pc << (*i)->GetLexicalVariations() << "$$,$$";
                pc << (*i)->GetStatus() << "$$,$$";
                pc << (*i)->GetUse() << "$$,$$";
                pc << (*i)->GetComment() << "$$,$$";
                pc << (*i)->GetOwner() << "$$,$$";
                pc << (*i)->GetSource() << "$$,$$";
                pc << (*i)->GetVersion() << "$$,$$";
                pc << (*i)->GetLastUpdate() << "$$)";
                r = w.exec(pc.str());
                ret += r.affected_rows();
            } else {
                std::cerr << "id " << ci << " found in ";
                std::cerr << ontologytablename_ << ". ";
                std::cerr << "No new data inserted for this id." << std::endl;
            }
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
    return ret;
}

void TpOntApi::UpdateDb() {

    std::cerr << DeleteInDb() << " rows were deleted." << std::endl;
    std::cerr << WriteDb() << " rows were written." << std::endl;
}

void TpOntApi::TpoeListFromPgResult(pqxx::result r) {
    DeleteAllResults();
    result_.clear();
    for (pqxx::result::size_type i = 0; i != r.size(); i++) {
        TpOntEntry * tpoe = new TpOntEntry();
        int iaux;
        if (r[i]["iid"].to(iaux))
            tpoe->SetIid(iaux);
        std::string saux;
        if (r[i]["eid"].to(saux))
            tpoe->SetEntryId(saux);
        if (r[i]["dbxref"].to(saux))
            tpoe->SetDbXref(saux);
        if (r[i]["owner"].to(saux))
            tpoe->SetOwner(saux);
        if (r[i]["source"].to(saux))
            tpoe->SetSource(saux);
        if (r[i]["version"].to(saux))
            tpoe->SetVersion(saux);
        if (r[i]["term"].to(saux))
            tpoe->SetTerm(saux);
        if (r[i]["category"].to(saux))
            tpoe->SetCategory(saux);
        if (r[i]["attributes"].to(saux))
            tpoe->SetAttributes(saux);
        if (r[i]["annotationtype"].to(saux))
            tpoe->SetAnnotationType(saux);
        if (r[i]["lexicalvariations"].to(saux))
            tpoe->SetLexicalVariations(saux);
        time_t taux;
        if (r[i]["last_update"].to(taux))
            tpoe->SetLastUpdateTime(taux);
        if (r[i]["curation_status"].to(saux))
            tpoe->SetStatus(saux);
        if (r[i]["curation_use"].to(saux))
            tpoe->SetUse(saux);
        if (r[i]["comment"].to(saux))
            tpoe->SetComment(saux);
        result_.push_back(tpoe);
    }
}

std::vector<std::string> TpOntApi::GetColumnHeaders() {
    std::vector<std::string> ret;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select column_name from information_schema.columns where table_name ='";
        pc << ontologytablename_ << "'";
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string saux;
            r[i]["column_name"].to(saux);
            ret.push_back(saux);
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
    return ret;
}

std::vector<std::string> TpOntApi::GetOneColumnLC(std::string colname, std::string match) {
    std::vector<std::string> ret;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select ";
        pc << colname;
        pc << " from ";
        pc << ontologytablename_;
        if (!match.empty())
            pc << " where lower(" << colname << ") ~ '" << match << "'";
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string saux;
            r[i][colname].to(saux);
            ret.push_back(saux);
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
    return ret;
}

void TpOntApi::LoadAll() {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\"";
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbIid(int id) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << "where iid = ";
        pc << id;
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbString(columns col, std::string s) {
    std::string field;
    switch (col) {
        case TpOntApi::eid:
            field = "eid";
            break;
        case TpOntApi::dbxref:
            field = "dbxref";
            break;
        case TpOntApi::owner:
            field = "owner";
            break;
        case TpOntApi::source:
            field = "source";
            break;
        case TpOntApi::version:
            field = "version";
            break;
        case TpOntApi::term:
            field = "term";
            break;
        case TpOntApi::category:
            field = "category";
            break;
        case TpOntApi::attributes:
            field = "attribute";
            break;
        case TpOntApi::annotationtype:
            field = "annotationtype";
            break;
        case TpOntApi::lexicalvariations:
            field = "lexicalvariations";
            break;
        case TpOntApi::status:
            field = "status";
            break;
        case TpOntApi::use:
            field = "use";
            break;
        case TpOntApi::comment:
            field = "comment";
            break;
    }
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << "where ";
        pc << field << " = '";
        pc << s << "'";
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbPosixString(columns col, std::string s) {
    std::string field;
    switch (col) {
        case TpOntApi::eid:
            field = "eid";
            break;
        case TpOntApi::dbxref:
            field = "dbxref";
            break;
        case TpOntApi::owner:
            field = "owner";
            break;
        case TpOntApi::source:
            field = "source";
            break;
        case TpOntApi::version:
            field = "version";
            break;
        case TpOntApi::term:
            field = "term";
            break;
        case TpOntApi::category:
            field = "category";
            break;
        case TpOntApi::attributes:
            field = "attribute";
            break;
        case TpOntApi::annotationtype:
            field = "annotationtype";
            break;
        case TpOntApi::lexicalvariations:
            field = "lexicalvariations";
            break;
        case TpOntApi::status:
            field = "status";
            break;
        case TpOntApi::use:
            field = "use";
            break;
        case TpOntApi::comment:
            field = "comment";
            break;
    }
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << "where ";
        pc << field << " ~ '";
        pc << s << "'";
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbString(std::string colname, std::string s) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << "where ";
        pc << colname << " = '";
        pc << s << "'";
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbWithWhereClause(std::string where) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << where;
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::SearchDbTimeStamp(time_t t) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from \"";
        pc << ontologytablename_ << "\" ";
        pc << "where last_update = ";
        pc << t;
        r = w.exec(pc.str());
        w.commit();
        TpoeListFromPgResult(r);
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.

        std::cerr << e.what() << std::endl;
    }
}

void TpOntApi::FilterOnIid(int id) {
    std::vector<TpOntEntry*> ret;
    std::vector<TpOntEntry*> tbdel;
    std::vector<TpOntEntry*>::iterator it;
    for (it = result_.begin(); it != result_.end(); it++)
        if ((*it)->GetIid() == id)
            ret.push_back(*it);
        else
            tbdel.push_back(*it);
    result_.clear();
    for (it = ret.begin(); it != ret.end(); it++)
        result_.push_back(*it);
    while (!tbdel.empty()) {

        TpOntEntry * d = tbdel.back();
        delete d;
        tbdel.pop_back();
    }
}

void TpOntApi::FilterOnString(columns col, std::string s) {
    std::vector<TpOntEntry*> ret;
    std::vector<TpOntEntry*> tbdel;
    std::vector<TpOntEntry*>::iterator it;
    for (it = result_.begin(); it != result_.end(); it++) {
        switch (col) {
            case TpOntApi::eid:
                if ((*it)->GetEntryId() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::dbxref:
                if ((*it)->GetDbXref() == s)
                    ret.push_back((*it));
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::owner:
                if ((*it)->GetOwner() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::source:
                if ((*it)->GetSource() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::version:
                if ((*it)->GetVersion() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::term:
                if ((*it)->GetTerm() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::category:
                if ((*it)->GetCategory() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::attributes:
                if ((*it)->GetAttributes() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::annotationtype:
                if ((*it)->GetAnnotationType() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::lexicalvariations:
                if ((*it)->GetLexicalVariations() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::status:
                if ((*it)->GetStatus() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::use:
                if ((*it)->GetUse() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
            case TpOntApi::comment:
                if ((*it)->GetComment() == s)
                    ret.push_back(*it);
                else
                    tbdel.push_back(*it);
                break;
        }
    }
    result_.clear();
    for (it = ret.begin(); it != ret.end(); it++)
        result_.push_back(*it);
    while (!tbdel.empty()) {

        TpOntEntry * d = tbdel.back();
        delete d;
        tbdel.pop_back();
    }
}

void TpOntApi::FilterOnTimeStamp(time_t t) {
    std::vector<TpOntEntry*> ret;
    std::vector<TpOntEntry*> tbdel;
    std::vector<TpOntEntry*>::iterator it;
    for (it = result_.begin(); it != result_.end(); it++)
        if ((*it)->GetLastUpdate() == t)
            ret.push_back(*it);
        else
            tbdel.push_back(*it);
    result_.clear();
    for (it = ret.begin(); it != ret.end(); it++)
        result_.push_back(*it);
    while (!tbdel.empty()) {

        TpOntEntry * d = tbdel.back();
        delete d;
        tbdel.pop_back();
    }
}

void TpOntApi::DeleteAllResults() {
    while (!result_.empty()) {

        TpOntEntry * d = result_.back();
        delete d;
        result_.pop_back();
    }
}

void TpOntApi::IndexColumn(std::string tablename, std::string col) {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << tablename << "'";
    r = w.exec(pc.str());
    if (r.size() == 0) {
        std::cerr << tablename << " does not exist." << std::endl;
    } else {
        pc.str("");
        pc << "select * from pg_indexes where indexname='";
        pc << tablename + "_" + col + "_idx" << "'";
        r = w.exec(pc.str());
        if (r.size() != 0) {
            std::cerr << tablename + "_" + col + "_idx exists." << std::endl;
        } else {
            pc.str("");
            pc << "create index ";
            pc << tablename + "_" + col + "_idx";
            pc << " on ";
            pc << tablename;
            pc << "(";
            pc << col;
            pc << ")";
            r = w.exec(pc.str());
        }
    }
    w.commit();
}
