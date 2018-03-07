/* 
 * File:   LiteratureProcessProgress.cpp
 * Author: mueller
 * 
 * Created on October 19, 2016, 2:34 PM
 */

#include "LiteratureProcessProgress.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <boost/filesystem.hpp>
#include <Wt/WStandardItem>
#include <Wt/WCssDecorationStyle>
#include <bits/basic_string.h>
#include <regex>

namespace {

    std::vector<std::string> GetDirEntries(std::string dir) {
        std::vector<std::string> result;
        std::vector<boost::filesystem::path> paux;
        boost::filesystem::path p(dir);
        if (boost::filesystem::exists(p)) {
            std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(),
                    std::back_inserter(paux));
            for (std::vector<boost::filesystem::path>::const_iterator
                it(paux.begin()), it_end(paux.end()); it != it_end; ++it) {
                // setting file name
                std::string fo = it->filename().generic_string();
                boost::algorithm::erase_first(fo, dir);
                result.push_back(fo);
            }
            std::sort(result.begin(), result.end());
        }
        return result;
    }

    bool FileIsNewer(std::string f1, std::string f2) {
        std::time_t t1(0), t2(0);
        boost::filesystem::path p1(f1), p2(f2);
        if (boost::filesystem::exists(p1))
            t1 = boost::filesystem::last_write_time(p1);
        if (boost::filesystem::exists(p2))
            t2 = boost::filesystem::last_write_time(p2);
        return (t1 >= t2);
    }

    std::vector<std::string> ListFilesOfExt(std::string dir, std::string ext) {
        boost::filesystem::recursive_directory_iterator rdi(dir);
        boost::filesystem::recursive_directory_iterator end_rdi; // default construction yields past-the-end
        std::vector<std::string> ret;
        ret.clear();
        for (; rdi != end_rdi; rdi++)
            if (ext.compare((*rdi).path().extension().string()) == 0)
                ret.push_back((*rdi).path().stem().string());
        return ret;
    }

    std::vector<std::string> AnotB(std::vector<std::string> & a, std::vector<std::string> & b) {
        std::vector<std::string> ret;
        ret.clear();
        std::vector<std::string>::iterator it;
        for (it = a.begin(); it != a.end(); it++)
            if (std::find(b.begin(), b.end(), *it) == b.end())
                ret.push_back(*it);
        return ret;
    }
}

int LiteratureProcessProgress::get_num_processed_files() {
    int numfiles(0);
    std::ifstream file(literatureroot_ + "/tpcas/processed_files.txt");
    if (file) {
        std::string str;
        while (std::getline(file, str)) {
            if (std::regex_match(str, std::regex(".*\\.(pdf|nxml|gz)$"))) {
                ++numfiles;
            }
        }
    }
    return numfiles;
}

int LiteratureProcessProgress::get_num_tokenized_files() {
    int numfiles(0);
    std::ifstream file(literatureroot_ + "/tpcas/tokenized_files.txt");
    if (file) {
        std::string str;
        while (std::getline(file, str)) {
            if (std::regex_match(str, std::regex(".*\\.(pdf|nxml|gz)$"))) {
                ++numfiles;
            }
        }
    }
    return numfiles;
}

int LiteratureProcessProgress::get_num_uploaded_files() {
    std::vector<std::string> uploadedfiles = GetDirEntries(literatureroot_ + "/uploadedfiles");
    int numfiles(0);
    for (const auto& filename : uploadedfiles) {
        if (std::regex_match(filename, std::regex(".*\\.(pdf|nxml|gz)$"))) {
            ++numfiles;
        }
    }
    return numfiles;
}

void LiteratureProcessProgress::setDirStructure() {
    if (session_->login().state() > 0)
        literatureroot_ = USERUPLOADROOTDIR + std::string("/")
                          + session_->login().user().identity("loginname").toUTF8();
    else
        literatureroot_ = "/tmp/";
}

LiteratureProcessProgress::LiteratureProcessProgress(Session & session, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    session_ = &session;
    session_->login().changed().connect(boost::bind(&LiteratureProcessProgress::setDirStructure, this));
    setDirStructure();
    literaturecontainer_ = new Wt::WContainerWidget();
    addWidget(literaturecontainer_);
    // set table and model
    model_ = new Wt::WStandardItemModel(0, 0);
    table_ = new Wt::WTableView();
    table_->setSortingEnabled(true);
    table_->setAlternatingRowColors(true);
    table_->setSelectable(true);
    table_->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
    table_->resize(Wt::WLength(150, Wt::WLength::FontEx), Wt::WLength::Auto);
    table_->setColumnResizeEnabled(true);
    table_->setModel(model_);
    table_->setSelectionMode(Wt::NoSelection);
    literaturecontainer_->addWidget(table_);
}

LiteratureProcessProgress::~LiteratureProcessProgress() {
}

void LiteratureProcessProgress::PopulateLiteratureTable() {
    if (session_->login().state() > 0) {
        model_->clear();
        // set headers
        model_->insertColumns(0, 3);
        model_->setHeaderData(0, boost::any(std::string("files tokenized")));
        model_->setHeaderData(1, boost::any(std::string("files marked-up")));
        model_->setHeaderData(2, boost::any(std::string("indexed")));
        table_->setColumnWidth(0, 150);
        table_->setColumnWidth(1, 150);
        table_->setColumnWidth(2, 150);
        table_->setWidth(500);
        const int HEIGHT = 2;
        table_->setRowHeight(Wt::WLength(HEIGHT, Wt::WLength::FontEx));
        table_->setHeight(Wt::WLength(8 * HEIGHT, Wt::WLength::FontEx));
        model_->insertRows(0, 1);
        int num_processed = get_num_processed_files();
        int num_tokenized = get_num_tokenized_files();
        int num_uploaded = get_num_processed_files();
        if (num_uploaded > 0) {
            model_->setData(0, 0, Wt::asString(float(num_tokenized) / float(num_uploaded) * 100., "%4.1f%%"));
            model_->setData(0, 1, Wt::asString(float(num_processed) / float(num_uploaded) * 100., "%4.1f%%"));
        } else {
            model_->setData(0, 0, Wt::asString(0, "%4.1f%%"));
            model_->setData(0, 1, Wt::asString(0, "%4.1f%%"));
        }

        if (boost::filesystem::exists(literatureroot_ + "/luceneindex/cc.cfg")) {
            model_->setData(0, 2, Wt::WString("yes"));
        } else {
            model_->setData(0, 2, Wt::WString("in progress"));
        }
    }
}
