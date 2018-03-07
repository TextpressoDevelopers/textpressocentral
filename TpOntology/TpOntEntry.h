/* 
 * File:   TpOntEntry.h
 * Author: mueller
 *
 * Created on May 29, 2013, 12:15 PM
 */

#ifndef TPONTENTRY_H
#define	TPONTENTRY_H

#include <string>
#include <set>

class TpOntEntry {
public:

    //    enum annotationtypes {
    //        lexical, manual, computational
    //    };
    //
    //    enum lexicalvariations {
    //        capitalization, plural, tense
    //    };
    //
    //    enum curationstatus {
    //        preliminary, experimental, final
    //    };

    TpOntEntry();
    TpOntEntry(const TpOntEntry & orig);
    void PopulateColumn(std::string mheader, std::string data);

    int GetIid() {
        return iid_;
    }

    std::string GetEntryId() {
        return eid_;
    }

    std::string GetDbXref() {
        return dbxref_;
    }

    std::string GetOwner() {
        return owner_;
    }

    std::string GetSource() {
        return source_;
    }

    std::string GetVersion() {
        return version_;
    }

    std::string GetTerm() {
        return term_;
    }

    std::string GetCategory() {
        return category_;
    }

    std::string GetAttributes() {
        return attributes_;
    }

    //    annotationtypes GetAnnotationType() {
    //        return anntype_;
    //    }

    std::string GetAnnotationType() {
        return anntype_;
    }

    //    std::set<lexicalvariations> GetLexicalVariations() {
    //        return lexvar_;
    //    }

    std::string GetLexicalVariations() {
        return lexvar_;
    }

    time_t GetLastUpdate() {
        return last_update_;
    }

    //    curationstatus GetStatus() {
    //        return status_;
    //    }

    std::string GetStatus() {
        return status_;
    }

    std::string GetUse() {
        return use_;
    }

    std::string GetComment() {
        return comment_;
    }

    void SetIid(int i) {
        iid_ = i;
    }

    void SetEntryId(std::string s) {
        eid_ = s;
    }

    void SetDbXref(std::string s) {
        dbxref_ = s;
    }

    void SetOwner(std::string s) {
        owner_ = s;
    }

    void SetSource(std::string s) {
        source_ = s;
    }

    void SetVersion(std::string s) {
        version_ = s;
    }

    void SetTerm(std::string s) {
        term_ = s;
    }

    void SetCategory(std::string s) {
        category_ = s;
    }

    void SetAttributes(std::string s) {
        attributes_ = s;
    }

    void SetAnnotationType(std::string saux);

    void SetLexicalVariations(std::string saux) {
        lexvar_ = saux;
    }

    void InsertLexicalVariations(std::string v) {
        if (lexvar_.empty())
            lexvar_ = v;
        else
            lexvar_ += "," + v;
    }

    void UpdateLastUpdateTime() {
        last_update_ = time(0);
    }

    void SetLastUpdateTime(time_t t) {
        last_update_ = t;
    }

    void SetStatus(std::string saux);

    void SetUse(std::string s) {
        use_ = s;
    }

    void SetComment(std::string s) {
        comment_ = s;
    }

    //    std::string Atype2String();

    //    std::string Cstatus2String ();

    //    std::string Lvariations2String ();

private:
    // serial primary key
    int iid_;
    // entry id
    std::string eid_;
    // db crossreference
    std::string dbxref_;
    // ownership (login name)
    std::string owner_;
    // source (URL or other description of source)
    std::string source_;
    // version
    std::string version_;
    // term
    std::string term_;
    // category
    std::string category_;
    // attributes and values;
    std::string attributes_;
    // annotation type (entry is from list, from manual entry, or from comput. methods
    std::string anntype_;
    // allowed lexical variation (plural, capitalization, tenses);
    //    std:set<std::string> lexvar_;
    std::string lexvar_;
    // last update (date)
    time_t last_update_;
    // curation status (experimental, settled, etc?)
    std::string status_;
    // curation use (for which curation it is used).
    std::string use_;
    // comment
    std::string comment_;
};

#endif	/* TPONTENTRY_H */
