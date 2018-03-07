/* 
 * File:   main.cpp
 * Author: Hans-Michael Muller
 *
 * This application takes a pdf and produces a CAS (UIMA) containing basic
 * annotations and provides all supplementary files such as figures.
 * 
 * Created on January 28, 2013, 9:10 AM
 */

#include "../TextpressoCentralGlobals.h"
#include "PdfInfo.h"
#include "Stream2Tpcas.h"

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#define PDF_2_TPCAS_VERSION "0.9.0"

#ifdef _HAVE_CONFIG
#include <config.h>
#endif // _HAVE_CONFIG

void print_who() {
    std::cout << std::endl << "PDF2TPCAS" << std::endl;
    std::cout << "Build Date: " << __DATE__ << std::endl;
    std::cout << "Version: " << PDF_2_TPCAS_VERSION << std::endl;
}

void print_help() {
    std::cout << std::endl;
    std::cout << "Usage: pdf2tpcas inputfile outputdirectory" << std::endl;
    std::cout << std::endl;
    std::cout << "       pdf2tpcas extracts information from a PDF file,";
    std::cout << std::endl;
    std::cout << "       performs tokenization and basic annotations, and saves it";
    std::cout << std::endl;
    std::cout << "       as a CAS file. Images are extracted as well.";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {

    // turn Debug/Logging off to better view the output from this app!
    PoDoFo::PdfError::EnableDebug(false);
    PoDoFo::PdfError::EnableLogging(false);

    if (argc < 3) {
        print_who();
        print_help();
        return (-1);
    }

    char * pszInput = argv[1];
    char * pszOutput = argv[2];
    boost::filesystem::path p(pszInput);
    try {
        std::string auxname = p.filename().string();
        size_t extPos = auxname.rfind('.');
        if (extPos != std::string::npos) {
            // Erase the current extension.
            auxname.erase(extPos);
            // Add the new extension.
            auxname.append(".tpcas");
        }
        std::string foutname = std::string(pszOutput);
        std::string fimageoutname = std::string(pszOutput) + "/images";
        foutname.append("/");
        foutname.append(auxname);
        fimageoutname.append("/");
        fimageoutname.append(auxname);
        stringstream sout;
        PdfInfo myInfo(pszInput, fimageoutname);
        myInfo.StreamAll(sout);
        const char * descriptor = PDF2TPCASDESCRIPTOR;
        Stream2Tpcas stp(sout, foutname, descriptor);
        stp.processInputStream();
    } catch (PoDoFo::PdfError & e) {
        fprintf(stderr,
                "Error: An error %i occurred during processing the pdf file %s.\n",
                e.GetError(), p.filename().c_str());
        e.PrintErrorMsg();
        return e.GetError();
    }
    return (EXIT_SUCCESS);
}
