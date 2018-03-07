/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on April 24, 2013, 6:27 PM
 */

#define XML_2_TPCAS_VERSION "0.1.0"

#include "../TextpressoCentralGlobals.h"
#include "ReadXml2Stream.h"
#include "../Pdf2Tpcas/Stream2Tpcas.h"
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

void print_who() {
    std::cout << std::endl << "XML2TPCAS" << std::endl;
    std::cout << "Build Date: " << __DATE__ << std::endl;
    std::cout << "Version: " << XML_2_TPCAS_VERSION << std::endl;
}

void print_help() {
    std::cout << std::endl;
    std::cout << "Usage: xml2tpcas inputfile outputdirectory" << std::endl;
    std::cout << std::endl;
    std::cout << "       xml2tpcas extracts information from an XML file,";
    std::cout << std::endl;
    std::cout << "       performs tokenization and basic annotations, and saves";
    std::cout << std::endl;
    std::cout << "       a corresponding CAS file in outputdirectory.";
    std::cout << std::endl;
}

int main(int argc, char** argv) {

    if (argc < 3) {
        print_who();
        print_help();
        return (-1);
    }

    const char * pszSource = argv[1];
    char * pszOutput = argv[2];
    boost::filesystem::path p(pszSource);
    std::string auxname = p.filename().string();
    size_t extPos = auxname.rfind('.');
    if (extPos != std::string::npos) {
        // Erase the current extension.
        auxname.erase(extPos);
        // Add the new extension.
        auxname.append(".tpcas");
    }
    std::string foutname = std::string(pszOutput);
    foutname.append("/");
    foutname.append(auxname);
    ReadXml2Stream rs(pszSource);
    std::stringstream sout;
    rs.GetStream(sout);
    const char * descriptor = XML2TPCASDESCRIPTOR;
    Stream2Tpcas stp(sout, foutname, descriptor);
    stp.processInputStream();
  
    return 0;
}
