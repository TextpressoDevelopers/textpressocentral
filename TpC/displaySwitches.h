/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   displaySwitches.h
 * Author: mueller
 *
 * Created on April 17, 2023, 7:30 PM
 */

#ifndef DISPLAYSWITCHES_H
#define DISPLAYSWITCHES_H

#include <set>
#include <string>

using namespace std;

class displaySwitches {
public:
    displaySwitches();
    displaySwitches(const displaySwitches& orig);
    bool isNotSuppressed(string s) {
        return suppressed_.find(s) == suppressed_.end();
    }
    virtual ~displaySwitches();
private:
    set<string> suppressed_;
};

#endif /* DISPLAYSWITCHES_H */

