/**
    Project: textpressocentral
    File name: CaseSensitiveAnalyzer.h
    
    @author valerio
    @version 1.0 6/9/17.
*/

#ifndef TEXTPRESSOCENTRAL_CASESENSITIVEANALYZER_H
#define TEXTPRESSOCENTRAL_CASESENSITIVEANALYZER_H

//#include <lucene++/Analyzer.h>
#include <lucene++/LuceneHeaders.h>

using namespace Lucene;

class CaseSensitiveAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~CaseSensitiveAnalyzerSavedStreams();
    LUCENE_CLASS(CaseSensitiveAnalyzerSavedStreams);

public:
    StandardTokenizerPtr tokenStream;
    TokenStreamPtr filteredTokenStream;
};

class CaseSensitiveAnalyzer: public Analyzer {

public:
    CaseSensitiveAnalyzer(Lucene::LuceneVersion::Version matchVersion);
    CaseSensitiveAnalyzer(Lucene::LuceneVersion::Version matchVersion, Lucene::HashSet<Lucene::String> stopWords);
    CaseSensitiveAnalyzer(Lucene::LuceneVersion::Version matchVersion, const Lucene::String &stopwords);
    CaseSensitiveAnalyzer(Lucene::LuceneVersion::Version matchVersion, const Lucene::ReaderPtr &stopwords);
    virtual ~CaseSensitiveAnalyzer();

    LUCENE_CLASS(CaseSensitiveAnalyzer);

public:
    static const int32_t DEFAULT_MAX_TOKEN_LENGTH;

protected:
    Lucene::HashSet<Lucene::String> stopSet;

    bool replaceInvalidAcronym;
    bool enableStopPositionIncrements;
    Lucene::LuceneVersion::Version matchVersion;
    int32_t maxTokenLength;

public:
    virtual Lucene::TokenStreamPtr tokenStream(const Lucene::String &fieldName, const Lucene::ReaderPtr &reader);
    void setMaxTokenLength(int32_t length);
    int32_t getMaxTokenLength();
    virtual Lucene::TokenStreamPtr reusableTokenStream(const Lucene::String &fieldName, const Lucene::ReaderPtr &reader);
    void ConstructAnalyser(Lucene::LuceneVersion::Version matchVersion, Lucene::HashSet<Lucene::String> stopWords);
};


#endif //TEXTPRESSOCENTRAL_CASESENSITIVEANALYZER_H
