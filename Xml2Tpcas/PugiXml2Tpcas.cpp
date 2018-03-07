/* 
 * File:   PugiXml2Tpcas.cpp
 * Author: mueller
 * 
 * Created on April 29, 2013, 7:03 PM
 */

#include "PugiXml2Tpcas.h"
#include "uima/xmiwriter.hpp"
#include <boost/filesystem.hpp>

PugiXml2Tpcas::PugiXml2Tpcas(const char * pszSource, const char * pszOutput, const char * cnfg) {
    pszSource_ = pszSource;
    cnfg_ = cnfg;
    boost::filesystem::path p(pszSource);
    std::string auxname = p.filename().string();
    size_t extPos = auxname.rfind('.');
    if (extPos != std::string::npos) {
        // Erase the current extension.
        auxname.erase(extPos);
        // Add the new extension.
        auxname.append(".tpcas");
    }
    outfn_ = std::string(pszOutput);
    outfn_.append("/");
    outfn_.append(auxname);
}

PugiXml2Tpcas::PugiXml2Tpcas(const PugiXml2Tpcas & orig) {
}

PugiXml2Tpcas::~PugiXml2Tpcas() {
}

void PugiXml2Tpcas::writeXmi(uima::CAS & outCas, int num, std::string outfn) {
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

void PugiXml2Tpcas::processInputSource() {
    try {
        /* Create/link up to a UIMACPP resource manager instance (singleton) */
        (void) uima::ResourceManager::createInstance("STREAM2TPCASAE");
        uima::ErrorInfo errorInfo;
        uima::AnalysisEngine * pEngine = uima::Framework::createAnalysisEngine(cnfg_, errorInfo);
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
            std::ifstream ifs(pszSource_);
            std::string inp((std::istreambuf_iterator<char>(ifs)),
                    (std::istreambuf_iterator<char>()));
            ifs.close();
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
                if (outfn_.length() > 0) {
                    writeXmi(outCas, i, outfn_);
                }
                //release CAS
                pEngine->getAnnotatorContext().releaseCAS(outCas);
            }
            if (outfn_.length() > 0) {
                //open a file stream for output xmi
                std::ofstream file;
                file.open(outfn_.c_str(), std::ios::out | std::ios::binary);
                if (!file) {
                    std::cerr << "Error opening output xmi: " << outfn_.c_str() << std::endl;
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