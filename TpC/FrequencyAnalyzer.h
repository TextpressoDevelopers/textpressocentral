/* 
 * File:   FrequencyAnalyzer.h
 * Author: mueller
 *
 * Created on April 28, 2014, 2:34 PM
 */

#ifndef FREQUENCYANALYZER_H
#define	FREQUENCYANALYZER_H

#define TPCAS2TPCENTRALDESCRIPTOR "/usr/local/uima_descriptors/Tpcas2TpCentral.xml"

#include <uima/api.hpp>
#include <vector>
#include <string>
#include <Wt/WDialog>

class FrequencyAnalyzer {
public:
    FrequencyAnalyzer(std::vector<const char *> & paperlist);
    void ShowTagSpiral(Wt::WContainerWidget * parent = NULL);
private:

    enum rawsourcetype {
        unknown, nxml, pdf
    };

    uima::AnalysisEngine * pEngine_;
    std::vector<uima::CAS*> pCases_;
    std::map<std::string, int> KeywordCount_;
    std::map<std::string, int> CategoryCount_;
    std::multimap<int, std::string> KeywordRank_;
    std::multimap<int, std::string> CategoryRank_;
    std::multimap<int, std::string> NormalizedOverallRank_;
    Wt::WDialog * dialog_;
    Wt::WContainerWidget * paintingWidget_;
    rawsourcetype rawsource_;
    void CreateUimaEngine(const char * descriptor);
    uima::CAS * GetCas(const char * pszInputFile);
    void PopulateCounts();
    void Add2CategoryCount(uima::CAS * cas);
    void Add2KeywordCount(uima::CAS * cas);
    void DialogDone(Wt::WDialog::DialogCode code);
};

#endif	/* FREQUENCYANALYZER_H */

