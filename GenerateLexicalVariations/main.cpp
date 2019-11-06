/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on September 25, 2013, 4:32 PM
 */

#include <iostream>
#include <pqxx/pqxx>
#include "../TextpressoCentralGlobalDefinitions.h"
#include <boost/algorithm/string.hpp>
#include "LexicalVariations.h"

int main(int argc, char** argv) {
    std::string tbn;
    tbn = (argc > 1) ? std::string(argv[1]) : PGONTOLOGYTABLENAME;
    try {
        pqxx::connection cn(PGONTOLOGY);
        std::cerr << "Connected to " << cn.dbname() << std::endl;
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tbn;
        pc << " where owner = 'Textpresso'";
        r = w.exec(pc.str());
        std::cerr << r.size() << " results returned." << std::endl;
        std::vector<std::string> terms;
        std::vector<std::string> iids;
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string saux;
            r[i]["term"].to(saux);
            boost::to_lower(saux);
            terms.push_back(saux);
            r[i]["iid"].to(saux);
            iids.push_back(saux);
        }
        LexicalVariations * lv = new LexicalVariations(terms);
        for (int i = 0; i < lv->Size(); i++) {
            std::stringstream pc;
            pc << "update ";
            pc << tbn << " ";
            pc << "set lexicalvariations = $$" << lv->GetForms(i) << "$$ ";
            pc << "where iid = '" << iids[i] << "'";
            r = w.exec(pc.str());
        }
        w.commit();
        cn.disconnect();
    } catch (const std::exception & e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
