/* 
 * File:   Stream2Tpcas.h
 * Author: mueller
 *
 * Created on February 1, 2013, 2:55 PM
 */

#ifndef STREAM2TPCAS_H
#define	STREAM2TPCAS_H

#include "uima/api.hpp"

class Stream2Tpcas {
public:
    Stream2Tpcas(std::stringstream & streamin, std::string outfn, const char * cnfg);
    Stream2Tpcas(std::string & stringin, std::string outfn, const char * cnfg);
    Stream2Tpcas(const Stream2Tpcas & orig);
    virtual ~Stream2Tpcas();
    void processInputStream();
    void processInputString();
    void writeXmi(uima::CAS & outCas, int num, std::string outfn);
private:
    std::stringstream m_streamin;
    std::string m_stringin;
    std::string m_outfn;
    const char * m_cnfg;

};

#endif	/* STREAM2TPCAS_H */
