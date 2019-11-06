/* 
 * File:   LexicalVariations.cpp
 * Author: mueller
 * 
 * Created on August 14, 2013, 6:01 PM
 */

#include <iostream>
#include "LexicalVariations.h"
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace {

    bool is_sibilant(std::string s) {
        return boost::algorithm::ends_with(s, "x") || boost::algorithm::ends_with(s, "z") ||
                boost::algorithm::ends_with(s, "s") || boost::algorithm::ends_with(s, "sh");
    }

    bool is_not_vowel_y(std::string s) {
        return !(boost::algorithm::ends_with(s, "ey") ||
                boost::algorithm::ends_with(s, "ay") ||
                boost::algorithm::ends_with(s, "iy") ||
                boost::algorithm::ends_with(s, "oy") ||
                boost::algorithm::ends_with(s, "uy"));
    }

    bool is_not_vowel_o(std::string s) {
        return !(boost::algorithm::ends_with(s, "eo") ||
                boost::algorithm::ends_with(s, "ao") ||
                boost::algorithm::ends_with(s, "io") ||
                boost::algorithm::ends_with(s, "oo") ||
                boost::algorithm::ends_with(s, "uo"));
    }
}

LexicalVariations::LexicalVariations(std::vector<std::string> & blfinp,
        std::string irrverbsfilename,
        std::string irrpluralfilename,
        std::string allverbsfile) {
    if (irrverbsfilename.empty()) irrverbsfilename = IRRVERBSFILE;
    if (irrpluralfilename.empty()) irrpluralfilename = IRRPLURALFILE;
    if (allverbsfile.empty()) allverbsfile = ALLVERBSFILE;
    irrverbs_ = LoadFromFile(irrverbsfilename);
    irrplurals_ = LoadFromFile(irrpluralfilename);
    allverbs_ = LoadFromFile(allverbsfile);
    std::vector<std::string>::iterator vsit;
    for (vsit = blfinp.begin(); vsit != blfinp.end(); vsit++)
        BasicLexicalForm_.push_back(*vsit);
    GenerateForms();
}

LexicalVariations::LexicalVariations(const LexicalVariations & orig) {
}

void LexicalVariations::Capitalize(std::vector<std::string> & inp) {
    std::vector<std::string> added;
    std::vector<std::string>::iterator vsit;
    for (vsit = inp.begin(); vsit != inp.end(); vsit++) {
        std::string s = (*vsit);
        s[0] = toupper(s[0]);
        added.push_back(s);
    }
    while (!added.empty()) {
        inp.push_back(added.back());
        added.pop_back();
    }
}

void LexicalVariations::Pluralize(std::vector<std::string> & inp) {
    std::vector<std::string> added;
    std::vector<std::string>::iterator vsit;
    for (vsit = inp.begin(); vsit != inp.end(); vsit++) {
        if (allverbs_.find(*vsit) == allverbs_.end())
            if (!irrplurals_[(*vsit)].empty()) {
                std::vector<std::string> splits;
                boost::split(splits, irrplurals_[(*vsit)], boost::is_any_of(","));
                while (!splits.empty()) {
                    added.push_back(splits.back());
                    splits.pop_back();
                }
            } else if (is_sibilant(*vsit))
                added.push_back(*vsit + "es");
            else if (boost::algorithm::ends_with(*vsit, "y")) {
                if (is_not_vowel_y(*vsit))
                    added.push_back((*vsit).substr(0, (*vsit).length() - 1) + "ies");
                else
                    added.push_back(*vsit + "s");
            } else
                added.push_back(*vsit + "s");
    }
    while (!added.empty()) {
        inp.push_back(added.back());
        added.pop_back();
    }
}

void LexicalVariations::Tensify(std::vector<std::string> & inp) {
    std::vector<std::string> added;
    std::vector<std::string>::iterator vsit;
    for (vsit = inp.begin(); vsit != inp.end(); vsit++) {
        if (allverbs_.find(*vsit) != allverbs_.end())
            if (!irrverbs_[(*vsit)].empty()) {
                std::vector<std::string> splits;
                boost::split(splits, irrverbs_[(*vsit)], boost::is_any_of(","));
                while (!splits.empty()) {
                    added.push_back(splits.back());
                    splits.pop_back();
                }
            } else if (is_sibilant(*vsit)) {
                added.push_back(*vsit + "es");
                added.push_back(*vsit + "ed");
                added.push_back(*vsit + "ing");
            } else if (boost::algorithm::ends_with(*vsit, "o")) {
                if (is_not_vowel_o(*vsit)) {
                    added.push_back(*vsit + "es");
                } else {
                    added.push_back(*vsit + "s");
                }
                added.push_back(*vsit + "ed");
                added.push_back(*vsit + "ing");
            } else if (boost::algorithm::ends_with(*vsit, "y")) {
                if (is_not_vowel_y(*vsit)) {
                    added.push_back((*vsit).substr(0, (*vsit).length() - 1) + "ies");
                    added.push_back((*vsit).substr(0, (*vsit).length() - 1) + "ied");
                } else {
                    added.push_back(*vsit + "s");
                    added.push_back(*vsit + "ed");
                }
                added.push_back(*vsit + "ing");
            } else {
                added.push_back(*vsit + "s");
                if (boost::algorithm::ends_with(*vsit, "e")) {
                    added.push_back(*vsit + "d");
                    added.push_back((*vsit).substr(0, (*vsit).length() - 1) + "ing");
                } else {
                    added.push_back(*vsit + "ed");
                    added.push_back(*vsit + "ing");
                }
            }

    }
    while (!added.empty()) {
        inp.push_back(added.back());
        added.pop_back();
    }
}

std::map<std::string, std::string> LexicalVariations::LoadFromFile(std::string filename) {
    std::map<std::string, std::string> ret;
    std::ifstream ifs(filename.c_str());
    if (!ifs) {
        std::cerr << "cannot open " + filename << std::endl;;
        exit(1);
    }
    std::string line;
    while (getline(ifs, line)) {
        boost::to_lower(line);
        std::vector<std::string> splits;
        boost::split(splits, line, boost::is_any_of("\t"));
        if (splits.size() > 1)
            ret.insert(std::pair<std::string, std::string > (splits[0], splits[1]));
        else
            ret.insert(std::pair<std::string, std::string > (splits[0], ""));
    }
    ifs.close();
    return ret;
}

void LexicalVariations::GenerateForms() {
    std::vector<std::string>::iterator vsit;
    Forms_.clear();
    for (vsit = BasicLexicalForm_.begin(); vsit != BasicLexicalForm_.end(); vsit++) {
        std::vector<std::string> aux;
        aux.push_back(*vsit);
        Pluralize(aux);
        Tensify(aux);
        Capitalize(aux);
        std::string final;
        std::vector<std::string>::iterator auxit;
        for (auxit = aux.begin(); auxit != aux.end(); auxit++) {
            if (*auxit != *vsit)
                if (final.empty())
                    final = *auxit;
                else
                    final += "|" + *auxit;
        }
        Forms_.push_back(final);
    }
}
