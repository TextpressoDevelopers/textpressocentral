/* 
 * File:   TpOntApi.h
 * Author: mueller
 *
 * Created on May 29, 2013, 12:13 PM
 */

#ifndef TPONTAPI_H
#define	TPONTAPI_H

#include "TpOntEntry.h"
#include "../TextpressoCentralGlobalDefinitions.h"
#include <pqxx/pqxx>
#include <vector>

#define CATEGORYCOLUMNWIDTH 1023

class TpOntApi {
public:

    enum columns {
        eid, dbxref, owner, source, version, term, category, attributes,
        annotationtype, lexicalvariations, lastupdate, status, use, comment
    };

    TpOntApi(std::string ontologytbn, std::string pcrelationstbn,
            std::string padcrelationstbn);

    std::vector<TpOntEntry*> GetResultList() {
        return result_;
    }

    long unsigned int GetResultListSize() {
        return result_.size();
    }

    TpOntEntry * GetElementFromResultList(int i) {
        return result_[i];
    }
    virtual ~TpOntApi();
    void CreateOntologyTable();
    void CreateOntologyIndex();
    void CreatePcRelationsTable();
    void CreatePadcRelationsTable();
    int DeleteInDb(); // uses result_
    int WriteDb(); // uses result_
    void UpdateDb(); // uses result_
    std::vector<std::string> GetColumnHeaders();
    std::vector<std::string> GetOneColumnLC(std::string colname, std::string match);
    void LoadAll();
    void SearchDbIid(int id);
    void SearchDbString(columns col, std::string s);
    void SearchDbPosixString(columns col, std::string s);
    void SearchDbWithWhereClause(std::string where);
    void SearchDbString(std::string colname, std::string s);
    void SearchDbTimeStamp(time_t t);
    void FilterOnIid(int id);
    void FilterOnString(columns col, std::string s);
    void FilterOnTimeStamp(time_t t);
    void PopulateFromFileWithHeaders(const char * filename);
    void PopulateTpOntologyFromFile(const char * filename);
    void PopulatePcRelationsFromFile(const char * filename);
    void PopulateTpOntologyFromOboFile(const char * filename, std::string category, const char * ccsource);
    void PopulateTpOntologyAllFromTppc(std::string dir, std::string fileprefix, std::string source, int depth);
    void ConvertObo2Tppc(std::string dir);
    void GrowTree(std::string pcfile, std::string treefile);
    void UpdateTpOntologyColumnFromFile(const char * filename, std::string condcolumn, std::string affcolumn);
    void IndexColumn(std::string tablename, std::string col);
    void DeleteAllResults();

    TpOntEntry * GetBackResult() {
        return result_.back();
    };

    void PopResult() {
        result_.pop_back();
    };

    void PushResult(TpOntEntry * t) {
        result_.push_back(t);
    };
private:
    // database connection
    pqxx::connection cn_;
    // result vector
    std::vector<TpOntEntry*> result_;
    // tablenames
    std::string ontologytablename_;
    std::string pcrelationstablename_;
    std::string padcrelationstablename_;
    void TpoeListFromPgResult(pqxx::result r);
};

#endif	/* TPONTAPI_H */
