/**
    Project: textpressocentral
    File name: CaseSensitiveAnalyzer.cpp
    
    @author valerio
    @version 1.0 6/9/17.
*/

#include "CaseSensitiveAnalyzer.h"
#include <lucene++/LuceneHeaders.h>
#include <lucene++/WordlistLoader.h>

using namespace Lucene;

DECLARE_SHARED_PTR(CaseSensitiveAnalyzer);

/// Construct an analyzer with the given stop words.
const int32_t CaseSensitiveAnalyzer::DEFAULT_MAX_TOKEN_LENGTH = 255;

CaseSensitiveAnalyzer::CaseSensitiveAnalyzer(LuceneVersion::Version matchVersion) {
    ConstructAnalyser(matchVersion, StopAnalyzer::ENGLISH_STOP_WORDS_SET());
}

CaseSensitiveAnalyzer::CaseSensitiveAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords) {
    ConstructAnalyser(matchVersion, stopWords);
}

CaseSensitiveAnalyzer::CaseSensitiveAnalyzer(LuceneVersion::Version matchVersion, const String& stopwords) {
    ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
}

CaseSensitiveAnalyzer::CaseSensitiveAnalyzer(LuceneVersion::Version matchVersion, const ReaderPtr& stopwords) {
    ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
}

CaseSensitiveAnalyzer::~CaseSensitiveAnalyzer() {
}

void CaseSensitiveAnalyzer::ConstructAnalyser(LuceneVersion::Version matchVersion, HashSet<String> stopWords) {
    stopSet = stopWords;
    enableStopPositionIncrements = StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion);
    replaceInvalidAcronym = LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_24);
    this->matchVersion = matchVersion;
    this->maxTokenLength = DEFAULT_MAX_TOKEN_LENGTH;
}

TokenStreamPtr CaseSensitiveAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    StandardTokenizerPtr tokenStream(newLucene<StandardTokenizer>(matchVersion, reader));
    tokenStream->setMaxTokenLength(maxTokenLength);
    TokenStreamPtr result(newLucene<StandardFilter>(tokenStream));
    //result = newLucene<LowerCaseFilter>(result);
    result = newLucene<StopFilter>(enableStopPositionIncrements, result, stopSet);
    return result;
}

void CaseSensitiveAnalyzer::setMaxTokenLength(int32_t length) {
    maxTokenLength = length;
}

int32_t CaseSensitiveAnalyzer::getMaxTokenLength() {
    return maxTokenLength;
}
DECLARE_SHARED_PTR(CaseSensitiveAnalyzerSavedStreams);
TokenStreamPtr CaseSensitiveAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    CaseSensitiveAnalyzerSavedStreamsPtr streams = boost::dynamic_pointer_cast<CaseSensitiveAnalyzerSavedStreams>(getPreviousTokenStream());
    if (!streams) {
        streams = newLucene<CaseSensitiveAnalyzerSavedStreams>();
        setPreviousTokenStream(streams);
        streams->tokenStream = newLucene<StandardTokenizer>(matchVersion, reader);
        streams->filteredTokenStream = newLucene<StandardFilter>(streams->tokenStream);
        //streams->filteredTokenStream = newLucene<LowerCaseFilter>(streams->filteredTokenStream);
        streams->filteredTokenStream = newLucene<StopFilter>(enableStopPositionIncrements, streams->filteredTokenStream, stopSet);
    } else {
        streams->tokenStream->reset(reader);
    }
    streams->tokenStream->setMaxTokenLength(maxTokenLength);

    streams->tokenStream->setReplaceInvalidAcronym(replaceInvalidAcronym);

    return streams->filteredTokenStream;
}

CaseSensitiveAnalyzerSavedStreams::~CaseSensitiveAnalyzerSavedStreams() {
}