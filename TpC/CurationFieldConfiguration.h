/* 
 * File:   CurationFieldConfiguration.h
 * Author: mueller
 *
 * Created on April 7, 2015, 10:41 AM
 */

#ifndef CURATIONFIELDCONFIGURATION_H
#define	CURATIONFIELDCONFIGURATION_H

#include <string>
#include <pqxx/pqxx>

class CurationFieldConfiguration {
public:

    enum fieldtype {
        lineedit, textarea, checkbox, combobox, listmaker
    };

    struct pginfo {
        std::string col;
        std::string whereclause;
        std::string database;
        std::string tablename;
    };

    CurationFieldConfiguration(std::string dbname, std::string tablename, std::string formname);
    void SaveField(std::string fieldname, fieldtype ft, int x, int y,
            bool b, pginfo s, pginfo v, std::string combochoices, int prepopid);
    bool HasFields();
    long unsigned int NumberOfFields();
    std::string GetFieldName(long unsigned int i);
    fieldtype GetFieldType(long unsigned int i);
    int GetX(long unsigned int i);
    int GetY(long unsigned int i);
    bool GetCheckboxDefault(long unsigned int i);
    pginfo GetSuggestionBoxInfo(long unsigned int i);
    pginfo GetValidationInfo(long unsigned int i);
    bool HasSuggestionBoxInfo(long unsigned int i);
    bool HasValidationInfo(long unsigned int i);
    std::string GetComboChoice(long unsigned int i);
    int GetPrePopId(long unsigned int i);
    std::string GetCurrentVersionFormName();
    void SaveFieldData(int record, fieldtype fieldtype, std::string fieldname,
            bool d, std::string data);
    void ReadFieldData(const int & record, const fieldtype & fieldtype,
            const std::string & fieldname, bool & d, std::string & data);
    virtual ~CurationFieldConfiguration();
private:
    // database connection
    pqxx::connection cn_;
    std::string tablename_;
    std::string formname_;
    std::string currentversionformname_;
    std::string nextversionformname_;
    std::vector<std::string> fieldname_;
    std::vector<fieldtype> fieldtype_;
    std::vector<int> x_;
    std::vector<int> y_;
    std::vector<bool> checkboxdefault_;
    std::vector<pginfo> suggestionboxinfo_;
    std::vector<pginfo> validationinfo_;
    std::vector<bool> hassuggestionboxinfo_;
    std::vector<bool> hasvalidationinfo_;
    std::vector<std::string> comboboxchoices_;
    std::vector<int> prepopids_;
    void CreateCurationFieldConfigurationTable();
    void CreateCurationFieldDataTable();
    void FindCurrentVersion();
    void LoadCurationFieldConfigurationTableFromDb();
};

#endif	/* CURATIONFIELDCONFIGURATION_H */
