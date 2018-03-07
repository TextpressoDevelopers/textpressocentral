/* 
 * File:   PgCurationDataFromViewer.h
 * Author: mueller
 *
 * Created on May 19, 2015, 11:09 AM
 */

#ifndef PGCURATIONDATAFROMVIEWER_H
#define	PGCURATIONDATAFROMVIEWER_H

#include <string>
#include <pqxx/pqxx>

class PgCurationDataFromViewer {
public:
    PgCurationDataFromViewer(std::string dbname, std::string tablename);
    void SaveData(int record_id, std::string term, std::string bestring, std::string additionalannotations,
    std::string filename, std::string owner, std::string paper_id, 
    time_t last_update, std::string version);
    void ReadData(const int & record_id, std::string  & term, std::string & bestring, std::string & additionalannotations,
    std::string & filename, std::string & owner, std::string & paper_id,
    time_t last_update, std::string & version);
    int NewRecordId();
    virtual ~PgCurationDataFromViewer();
private:
    // database connection
    pqxx::connection cn_;
    std::string tablename_;
    int record_id_;
    std::string term_;
    std::string bestring_;
    std::string filename_;
    std::string owner_;
    std::string paperid_;
    time_t lastupdate_;
    std::string version_;
    void CreateCurationDataFromViewerTable();
};

#endif	/* PGCURATIONDATAFROMVIEWER_H */

