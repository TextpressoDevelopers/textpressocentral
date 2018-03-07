/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on August 14, 2013, 10:41 AM
 */

#include "LexicalVariations.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    std::vector<std::string> invec;
    std::string line;
    invec.clear();
    while (std::cin >> line) {
        boost::to_lower(line);
        invec.push_back(line);
    }
    LexicalVariations * lv = new LexicalVariations(invec);
    for (int i = 0; i < lv->Size(); i++) {
        std::cout << lv->GetBasicLexicalForm(i);
        std::cout << "\t";
        std::cout << lv->GetForms(i);
        std::cout << "\n";
    }
    return 0;
}
