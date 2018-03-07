/* 
 * File:   PugiXml2Tpcas.h
 * Author: mueller
 *
 * Created on April 29, 2013, 7:03 PM
 */

#ifndef PUGIXML2TPCAS_H
#define	PUGIXML2TPCAS_H

#include "uima/api.hpp"

class PugiXml2Tpcas {
public:
    PugiXml2Tpcas(const char * pszSource, const char * pszOutput, const char * cnfg);
    PugiXml2Tpcas(const PugiXml2Tpcas & orig);
    virtual ~PugiXml2Tpcas();
    void processInputSource();
    void writeXmi(uima::CAS & outCas, int num, std::string outfn);
private:
    const char * pszSource_ ;
    std::string outfn_;
    const char * cnfg_;
};

#endif	/* PUGIXML2TPCAS_H */

