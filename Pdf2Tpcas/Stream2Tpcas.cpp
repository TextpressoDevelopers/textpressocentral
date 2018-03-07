/* 
 * File:   Stream2Tpcas.cpp
 * Author: mueller
 * 
 * Created on February 1, 2013, 2:55 PM
 */

#include "Stream2Tpcas.h"

#include "uima/xmiwriter.hpp"

Stream2Tpcas::Stream2Tpcas(std::stringstream & streamin, std::string outfn, const char * cnfg) {
    m_streamin << streamin.rdbuf();
    m_outfn = outfn;
    m_cnfg = cnfg;
}

Stream2Tpcas::Stream2Tpcas(std::string & stringin, std::string outfn, const char * cnfg) {
    m_stringin = stringin;
    m_outfn = outfn;
    m_cnfg = cnfg;
}

Stream2Tpcas::Stream2Tpcas(const Stream2Tpcas & orig) {
}

Stream2Tpcas::~Stream2Tpcas() {
}

void Stream2Tpcas::writeXmi(uima::CAS & outCas, int num, std::string outfn) {
    std::string ofn;
    ofn.append(outfn);
    ofn.append("_seg_");
    std::stringstream s;
    s << num;
    ofn.append(s.str());
    //open a file stream for output xmi
    std::ofstream file;
    file.open(ofn.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Error opening output xmi: " << ofn.c_str() << std::endl;
        exit(99);
    }
    //serialize the cas
    uima::XmiWriter writer(outCas, true);
    writer.write(file);
    file.close();
}

void Stream2Tpcas::processInputStream() {
    try {
        /* Create/link up to a UIMACPP resource manager instance (singleton) */
        (void) uima::ResourceManager::createInstance("STREAM2TPCASAE");
        uima::ErrorInfo errorInfo;
        uima::AnalysisEngine * pEngine = uima::Framework::createAnalysisEngine(m_cnfg, errorInfo);
        if (errorInfo.getErrorId() != UIMA_ERR_NONE) {
            std::cerr << std::endl
                    << "  Error string  : "
                    << uima::AnalysisEngine::getErrorIdAsCString(errorInfo.getErrorId()) << std::endl
                    << "  UIMACPP Error info:" << std::endl
                    << errorInfo << std::endl;
            exit((int) errorInfo.getErrorId());
        }
        uima::TyErrorId utErrorId; // Variable to store UIMACPP return codes
        /* Get a new CAS */
        uima::CAS* cas = pEngine->newCAS();
        if (cas == NULL) {
            std::cerr << "pEngine->newCAS() failed." << std::endl;
            exit(1);
        }
        /* process input / cas */
        try {
            // Need to transfer filename to UIMA annotator. We do that by
            // putting is in front of the stream. Format:
            // e integers # filename # rest of stream
            // where integers is the number of characters up to the the second hash
            // sign
            std::string auxname = m_outfn;
            //size_t extPos = auxname.rfind('/');
            //if (extPos != std::string::npos) {
            //    // Erase everything before the slash.
            //    auxname.erase(0, extPos + 1);
            //}
            int threeintegers = auxname.length() + 5;
            threeintegers = (threeintegers < 100) ? threeintegers - 1 : threeintegers;
            threeintegers = (threeintegers < 10) ? threeintegers -1 : threeintegers;
            if (threeintegers > 999) {
                threeintegers = 999;
                auxname.erase(994, auxname.length());
            }
            std::stringstream inp;
            inp << threeintegers << "#" << auxname << "#" << m_streamin.rdbuf();
//            std::string inp = m_streamin.str();
            UnicodeString ustrInputText;
//            ustrInputText.append(UnicodeString::fromUTF8(StringPiece(inp)));
            ustrInputText.append(UnicodeString::fromUTF8(StringPiece(inp.str())));
            cas->setDocumentText(uima::UnicodeStringRef(ustrInputText));
            /* process the CAS */
            uima::CASIterator casIter = ((uima::AnalysisEngine*) pEngine)->processAndOutputNewCASes(*cas);
            int i = 0;
            while (casIter.hasNext()) {
                i++;
                uima::CAS & outCas = casIter.next();
                //write out xmi
                if (m_outfn.length() > 0) {
                    writeXmi(outCas, i, m_outfn);
                }
                //release CAS
                pEngine->getAnnotatorContext().releaseCAS(outCas);
            }
            if (m_outfn.length() > 0) {
                //open a file stream for output xmi
                std::ofstream file;
                file.open(m_outfn.c_str(), std::ios::out | std::ios::binary);
                if (!file) {
                    std::cerr << "Error opening output xmi: " << m_outfn.c_str() << std::endl;
                    exit(99);
                }
                //serialize the input cas
                uima::XmiWriter writer(*cas, true);
                writer.write(file);
                file.close();
            }
        } catch (uima::Exception e) {
            uima::ErrorInfo errInfo = e.getErrorInfo();
            std::cerr << "Error " << errInfo.getErrorId() << " " << errInfo.getMessage() << std::endl;
            std::cerr << errInfo << std::endl;
        }
        /* call collectionProcessComplete */
        utErrorId = pEngine->collectionProcessComplete();
        /* Free annotator */
        utErrorId = pEngine->destroy();
        delete cas;
        delete pEngine;
    } catch (uima::Exception e) {
        std::cerr << "Exception: " << e << std::endl;
    }
}

void Stream2Tpcas::processInputString() {
    try {
        /* Create/link up to a UIMACPP resource manager instance (singleton) */
        (void) uima::ResourceManager::createInstance("STREAM2TPCASAE");
        uima::ErrorInfo errorInfo;
        uima::AnalysisEngine * pEngine = uima::Framework::createAnalysisEngine(m_cnfg, errorInfo);
        if (errorInfo.getErrorId() != UIMA_ERR_NONE) {
            std::cerr << std::endl
                    << "  Error string  : "
                    << uima::AnalysisEngine::getErrorIdAsCString(errorInfo.getErrorId()) << std::endl
                    << "  UIMACPP Error info:" << std::endl
                    << errorInfo << std::endl;
            exit((int) errorInfo.getErrorId());
        }
        uima::TyErrorId utErrorId; // Variable to store UIMACPP return codes
        /* Get a new CAS */
        uima::CAS* cas = pEngine->newCAS();
        if (cas == NULL) {
            std::cerr << "pEngine->newCAS() failed." << std::endl;
            exit(1);
        }
        /* process input / cas */
        try {
            // Need to transfer filename to UIMA annotator. We do that by
            // putting is in front of the stream. Format:
            // e integers # filename # rest of stream
            // where integers is the number of characters up to the the second hash
            // sign
            std::string auxname = m_outfn;
            //size_t extPos = auxname.rfind('/');
            //if (extPos != std::string::npos) {
            //    // Erase everything before the slash.
            //    auxname.erase(0, extPos + 1);
            //}
            int threeintegers = auxname.length() + 5;
            threeintegers = (threeintegers < 100) ? threeintegers - 1 : threeintegers;
            threeintegers = (threeintegers < 10) ? threeintegers -1 : threeintegers;
            if (threeintegers > 999) {
                threeintegers = 999;
                auxname.erase(994, auxname.length());
            }
            std::stringstream aux;
            aux << threeintegers << "#" << auxname << "#";
            std::string inp = aux.str() + m_stringin;
            UnicodeString ustrInputText;
            ustrInputText.append(UnicodeString::fromUTF8(StringPiece(inp)));
            cas->setDocumentText(uima::UnicodeStringRef(ustrInputText));
            /* process the CAS */
            uima::CASIterator casIter = ((uima::AnalysisEngine*) pEngine)->processAndOutputNewCASes(*cas);
            int i = 0;
            while (casIter.hasNext()) {
                i++;
                uima::CAS & outCas = casIter.next();
                //write out xmi
                if (m_outfn.length() > 0) {
                    writeXmi(outCas, i, m_outfn);
                }
                //release CAS
                pEngine->getAnnotatorContext().releaseCAS(outCas);
            }
            if (m_outfn.length() > 0) {
                //open a file stream for output xmi
                std::ofstream file;
                file.open(m_outfn.c_str(), std::ios::out | std::ios::binary);
                if (!file) {
                    std::cerr << "Error opening output xmi: " << m_outfn.c_str() << std::endl;
                    exit(99);
                }
                //serialize the input cas
                uima::XmiWriter writer(*cas, true);
                writer.write(file);
                file.close();
            }
        } catch (uima::Exception e) {
            uima::ErrorInfo errInfo = e.getErrorInfo();
            std::cerr << "Error " << errInfo.getErrorId() << " " << errInfo.getMessage() << std::endl;
            std::cerr << errInfo << std::endl;
        }
        /* call collectionProcessComplete */
        utErrorId = pEngine->collectionProcessComplete();
        /* Free annotator */
        utErrorId = pEngine->destroy();
        delete cas;
        delete pEngine;
    } catch (uima::Exception e) {
        std::cerr << "Exception: " << e << std::endl;
    }
}
