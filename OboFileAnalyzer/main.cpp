/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on October 14, 2013, 12:05 PM
 */

#include "OboFileSegmentation.h"
#include <iostream>

void print_who() {
    std::cout << std::endl << "Obo file analyzer" << std::endl;
    std::cout << "Build Date: " << __DATE__ << std::endl;
}

void print_help() {
    std::cout << std::endl;
    std::cout << "Usage: obofileanalyzer inputfile." << std::endl;
    std::cout << std::endl;
    std::cout << "       Extracts information from an OBO file";
    std::cout << std::endl;
    std::cout << "       Output files are:" << std::endl;
    std::cout << std::endl;
    std::cout << "       inputfile.all.$id" << std::endl;
    std::cout << "              contains ids of all descendants of $id." << std::endl;
    std::cout << "       inputfile.all.term.$id" << std::endl; 
    std::cout << "              contains term entries of all descendants of $id." << std::endl;
    std::cout << "       inputfile.elem.$id" << std::endl;
    std::cout << "              contains ids of all immediate children of $id." << std::endl;
    std::cout << "       inputfile.elem.term.$id" << std::endl;
    std::cout << "              contains term entries of all immediate children of $id." << std::endl;
    std::cout << "       inputfile.single.$id" << std::endl;
    std::cout << "              contains term entry of $id." << std::endl;
    std::cout << "       inputfile.ids-names" << std::endl;
    std::cout << "              contains id-name mappings.";
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_who();
        print_help();
        return (-1);
    }

    const char * pszSource = argv[1];
    
    OboFileSegmentation * ofs = new OboFileSegmentation(pszSource);
    ofs->PrintElementaryPcRelationships(pszSource);
    ofs->CalculateAndPrintCompositePcRelationships(pszSource);
    ofs->PrintIdNames(pszSource);
    ofs->PrintSingleTerms(pszSource);
    return 0;
}
