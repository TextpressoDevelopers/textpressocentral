/* 
 * File:   LexicalVariations.h
 * Author: mueller
 *
 * Created on August 14, 2013, 6:01 PM
 */

#ifndef LEXICALVARIATIONS_H
#define LEXICALVARIATIONS_H

#define ALLVERBSFILE "resources/allverbs.txt"
#define IRRVERBSFILE "resources/irrverbs.txt"
#define IRRPLURALFILE "resources/irrplurals.txt"

#include<string>
#include<vector>
#include<map>

class LexicalVariations {
public:
    LexicalVariations(std::vector<std::string> & blfinp,
            std::string irrverbsfilename = "",
            std::string irrpluralfilename = "",
            std::string allverbsfile = "");
    LexicalVariations(const LexicalVariations & orig);

    std::string GetBasicLexicalForm(int i) {
        return BasicLexicalForm_[i];
    }

    void SetBasicLexicalForm(int i, std::string s) {
        BasicLexicalForm_[i] = s;
    }

    std::string GetForms(int i) {
        return Forms_[i];
    }

    long unsigned int Size() {
        return BasicLexicalForm_.size();
    }
    void GenerateForms();
private:
    void Capitalize(std::vector<std::string> & inp);
    void Pluralize(std::vector<std::string> & inp);
    void Tensify(std::vector<std::string> & inp);
    std::map<std::string, std::string> LoadFromFile(std::string filename);
    std::vector<std::string> BasicLexicalForm_;
    std::vector<std::string> Forms_;
    std::map<std::string, std::string> allverbs_;
    std::map<std::string, std::string> irrverbs_;
    std::map<std::string, std::string> irrplurals_;
};

#endif /* LEXICALVARIATIONS_H */

