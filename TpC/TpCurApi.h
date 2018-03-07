/* 
 * File:   TpCurApi.h
 * Author: mueller
 *
 * Created on February 24, 2014, 3:34 PM
 */

#ifndef TPCURAPI_H
#define	TPCURAPI_H

#include <pqxx/pqxx>

class TpCurApi {
public:
    TpCurApi();
    TpCurApi(const TpCurApi & orig);
    virtual ~TpCurApi();
    void CreateCurationTable();
    struct dataentry {
        std::string eid;
        std::string dbxref;
        std::string term;
        std::string ontology_terms;
        std::string biological_entities;
        std::string evidence_code;
        std::string file_location;
        std::string position_in_sofastring;
        std::string paper_id;
        std::string curation_status;
        std::string curation_use;
        std::string comment;
        std::string owner;
        std::string source;
        std::string version;
        time_t last_update;        
    };
    void SubmitRow(dataentry d);
    int DeleteInPg(std::vector<std::string> & deletionlist);
    void UpdatePg(std::string iid, dataentry d);
private:
    // database connection
    pqxx::connection cn_;
};

#endif	/* TPCURAPI_H */
