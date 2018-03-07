/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on May 28, 2015, 3:15 PM
 */

#include <pqxx/pqxx>
#include <iostream>

void CreatePgTable(pqxx::connection & cn) {
    pqxx::work w(cn);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='listofontologies'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << "listofontologies table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table listofontologies (";
        pc << "idprefix varchar(64)";
        pc << ", name text";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();
}

void PopulatePgTable(pqxx::connection & cn) {
    pqxx::work w(cn);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from listofontologies";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << "listofontologies table already populated." << std::endl;
    } else {
        pc.str("");
        pc << "insert into listofontologies values ('GO', 'Gene Ontology');";
        pc << "insert into listofontologies values ('tppce', 'C. elegans Protein');";
        pc << "insert into listofontologies values ('SO', 'Sequence Ontology');";
        pc << "insert into listofontologies values ('PATO', 'Quality Ontology (PATO)');";
        pc << "insert into listofontologies values ('CHEBI', 'Chemical Entities of Biological Interest');";
        pc << "insert into listofontologies values ('WBbt', 'C. elegans Cell and Anatomy Ontology');";
        pc << "insert into listofontologies values ('WBPhenotype', 'Variant');";
        pc << "insert into listofontologies values ('WBls', 'Nematode Life Stage');";
        pc << "insert into listofontologies values ('tpcca', 'CCC Localization Assay');";
        pc << "insert into listofontologies values ('tpccc', 'CCC Localization Components');";
        pc << "insert into listofontologies values ('tpccv', 'CCC Localization Verbs');";
        pc << "insert into listofontologies values ('tpgce', 'Gene C. elegans');";
        pc << "insert into listofontologies values ('tpdi', 'Human Disease');";
        pc << "insert into listofontologies values ('tpmol', 'Molecule (uncategorized)');";
        pc << "insert into listofontologies values ('tppce', 'Protein (C. elegans)');";
        pc << "insert into listofontologies values ('tpvce', 'Variation (C. elegans)');";
        r = w.exec(pc.str());
    }
    w.commit();
}

int main(int argc, char** argv) {
    pqxx::connection cn("dbname=www-data");
    CreatePgTable(cn);
    PopulatePgTable(cn);
    cn.disconnect();
    return 0;
}

