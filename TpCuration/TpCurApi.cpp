/* 
 * File:   TpCurApi.cpp
 * Author: mueller
 * 
 * Created on February 24, 2014, 3:34 PM
 */

#include "TextpressoCentralGlobalDefinitions.h"
#include "TpCurApi.h"
#include <iostream>

TpCurApi::TpCurApi() : cn_(PGCURATION) {
    std::cerr << "Connected to " << cn_.dbname() << std::endl;
}

TpCurApi::TpCurApi(const TpCurApi & orig) {
}

TpCurApi::~TpCurApi() {
    cn_.disconnect();
}

void TpCurApi::CreateCurationTable() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << PGCURATIONTABLENAME << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << PGCURATIONTABLENAME << " table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table ";
        pc << PGCURATIONTABLENAME << " ";
        pc << "(";
        pc << "iid serial primary key";
        pc << ", eid varchar(60)";
        pc << ", dbxref text";
        pc << ", term text";
        pc << ", ontology_terms varchar(255)";
        pc << ", biological_entities varchar(255)";
        pc << ", evidence_code varchar(60)";
        pc << ", file_location varchar(255)";
        pc << ", position_in_sofastring varchar(255)";
        pc << ", paper_id varchar(60)";
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

void TpCurApi::SubmitRow(dataentry incomingdata) {
    std::vector<std::string> cols;
    cols.clear();
    cols.push_back("eid");
    cols.push_back("dbxref");
    cols.push_back("term");
    cols.push_back("ontology_terms");
    cols.push_back("biological_entities");
    cols.push_back("evidence_code");
    cols.push_back("file_location");
    cols.push_back("position_in_sofastring");
    cols.push_back("paper_id");
    cols.push_back("curation_status");
    cols.push_back("curation_use");
    cols.push_back("comment");
    cols.push_back("owner");
    cols.push_back("source");
    cols.push_back("version");
    cols.push_back("last_update");
    pqxx::work w(cn_);
    pqxx::tablewriter tt(w, PGCURATIONTABLENAME, cols.begin(), cols.end());
    std::vector<std::string> d;
    d.push_back(incomingdata.eid);
    d.push_back(incomingdata.dbxref);
    d.push_back(incomingdata.term);
    d.push_back(incomingdata.ontology_terms);
    d.push_back(incomingdata.biological_entities);
    d.push_back(incomingdata.evidence_code);
    d.push_back(incomingdata.file_location);
    d.push_back(incomingdata.position_in_sofastring);
    d.push_back(incomingdata.paper_id);
    d.push_back(incomingdata.curation_status);
    d.push_back(incomingdata.curation_use);
    d.push_back(incomingdata.comment);    
    d.push_back(incomingdata.owner);
    d.push_back(incomingdata.source);
    d.push_back(incomingdata.version);
    std::stringstream ss;
    ss << incomingdata.last_update;
    d.push_back(ss.str());
    tt.push_back(d);
    tt.complete();
    w.commit();
}

int TpCurApi::DeleteInPg(std::vector<std::string> & deletionlist) {
    int ret = 0;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::vector<std::string>::iterator it;
        for (it = deletionlist.begin(); it != deletionlist.end(); it++) {
            std::stringstream pc;
            pc << "delete from ";
            pc << PGCURATIONTABLENAME << " ";
            pc << "where iid = ";
            pc << *it;
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

void TpCurApi::UpdatePg(std::string iid, dataentry d) {
    std::vector<std::string> dellist;
    dellist.push_back(iid);
    DeleteInPg(dellist);
    SubmitRow(d);
}