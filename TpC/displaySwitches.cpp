/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   displaySwitches.cpp
 * Author: mueller
 * 
 * Created on April 17, 2023, 7:30 PM
 */

#include "displaySwitches.h"
#include <boost/filesystem.hpp>
displaySwitches::displaySwitches() {
    suppressed_.clear();
    string filename("/data/textpresso/etc/suppressed");
    if (boost::filesystem::exists(filename)) {
        ifstream f(filename);
        string in;
        while (getline(f, in)) suppressed_.insert(in);
        f.close();
    }
}

displaySwitches::displaySwitches(const displaySwitches& orig) {
}

displaySwitches::~displaySwitches() {
}

