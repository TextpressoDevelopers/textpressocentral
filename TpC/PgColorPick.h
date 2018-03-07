/* 
 * File:   PgColorPick.h
 * Author: mueller
 *
 * Created on January 27, 2016, 11:06 AM
 */

#ifndef PGCOLORPICK_H
#define	PGCOLORPICK_H

#include <pqxx/pqxx>

class PgColorPick {
public:
    PgColorPick(std::string dbname, std::string tablename, std::string username);
    void ReadTable();
    void SaveTable();
    PgColorPick(const PgColorPick& orig);
    int GetRed(int i) { return red_[i]; }
    int GetGreen(int i) { return green_[i]; }
    int GetBlue(int i) { return blue_[i]; }
    void GetRed(int i, int v) { red_[i] = v; }
    void GetGreen(int i, int v) { green_[i] = v; }
    void GetBlue(int i, int v) { blue_[i] = v; }
    virtual ~PgColorPick();
private:
    pqxx::connection cn_;
    std::string tablename_;
    void CreateColorPickTable();
    std::vector<int> red_;
    std::vector<int> green_;
    std::vector<int> blue_;

};

#endif	/* PGCOLORPICK_H */

