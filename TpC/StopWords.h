/* 
 * File:   StopWords.h
 * Author: mueller
 *
 * Created on February 14, 2014, 3:21 PM
 */

#ifndef STOPWORDS_H
#define	STOPWORDS_H

#include "TextpressoCentralGlobalDefinitions.h"
#include <pqxx/pqxx>
#include <set>

class StopWords {
public:
    StopWords();
    StopWords(const StopWords & orig);
    bool isStopword(std::string s) {return (words_.find(s) != words_.end());}
private:
    pqxx::connection cn_;
    std::set<std::string> words_;
};

#endif	/* STOPWORDS_H */

