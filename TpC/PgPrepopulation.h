/* 
 * File:   PgPrepopulation.h
 * Author: mueller
 *
 * Created on July 17, 2015, 2:23 PM
 */

#ifndef PGPREPOPULATION_H
#define	PGPREPOPULATION_H

#include <pqxx/pqxx>

class PgPrepopulation {
public:
    PgPrepopulation(std::string dbname, std::string tablename);
    int GetNewPrePopId();
    void SaveTableData(int prepop_id, int mode, std::string data,
            std::string syndatabase, std::string syntablename,
            std::string syncolname, std::string synwhereclause);
    void ReadTableData(int prepop_id, int & mode, std::string & data,
            std::string & syndatabase, std::string & syntablename,
            std::string & syncolname, std::string & synwhereclause);
    virtual ~PgPrepopulation();
private:
    // database connection
    pqxx::connection cn_;
    std::string tablename_;
    void CreatePrepopulationTable();
};

#endif	/* PGPREPOPULATION_H */

