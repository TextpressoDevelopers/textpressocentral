/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on February 24, 2014, 3:25 PM
 */

#include <iostream>
#include "TpCurApi.h"

/*
 *
 * Operations regarding the Textpresso manual annotation database
 *  
 */

void print_who() {
    std::cout << std::endl << "tpcuration" << std::endl;
    std::cout << "Build Date: " << __DATE__ << std::endl;
    std::cout << "Version: 0.0.1" << std::endl;
}

void print_help() {
    std::cout << std::endl;
    std::cout << "Usage: tpcuration command <more options>" << std::endl;
    std::cout << std::endl;
    std::cout << "commands:";
    std::cout << std::endl;
    std::cout << "  create_table_tpcuration";
    std::cout << std::endl;
    std::cout << "     creates 'tpcuration' table in postgres";
    std::cout << std::endl;
    std::cout << "     unless it already exists.";
    std::cout << std::endl;
    //
    // ... add more later.
    
}

int main(int argc, char** argv) {
   if (argc < 2) {
        print_who();
        print_help();
        return (-1);
    }
    const char * pszCommand = argv[1];
    std::string s(argv[1]);
    TpCurApi * x = new TpCurApi();
    if (s.compare("create_table_tpcuration") == 0) {
        x->CreateCurationTable();
    }

    return 0;
}
