/* 
 * File:   StopWords.cpp
 * Author: mueller
 * 
 * Created on February 14, 2014, 3:21 PM
 */

#include "StopWords.h"
#include <iostream>

StopWords::StopWords() : cn_(PGONTOLOGY) {
    std::cerr << "Stopwords: connected to " << cn_.dbname() << std::endl;
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from ";
    pc << STOPWORDTABLENAME;
    r = w.exec(pc.str());
    if (r.size() != 0) {
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string saux;
            r[i]["words"].to(saux);
            words_.insert(saux);
            //std::cerr << saux << " inserted..." << std::endl;
        }
        w.commit();
    } else {
        std::cerr << "Error reading " << STOPWORDTABLENAME << std::endl;
    }
    cn_.disconnect();
}

StopWords::StopWords(const StopWords & orig) {
}
