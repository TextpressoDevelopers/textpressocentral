/* 
 * File:   PaperOrganizer.cpp
 * Author: mueller
 * 
 * Created on October 14, 2016, 10:44 AM
 */

#include "PaperOrganizer.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <Wt/WImage>
#include <Wt/WCssDecorationStyle>
#include <Wt/WLineEdit>
#include <Wt/WTimer>
#include <textpresso/IndexManager.h>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;

// TODO add pipeline to automatically create index if does not exist

namespace {

    std::vector<std::string> GetDirEntries(std::string dir) {
        std::vector<std::string> result;
        std::vector<boost::filesystem::path> paux;
        boost::filesystem::path p(dir);
        std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(),
                std::back_inserter(paux));
        for (std::vector<boost::filesystem::path>::const_iterator
            it(paux.begin()), it_end(paux.end()); it != it_end; ++it) {
            // setting file name
            std::string fo = it->filename().generic_string();
            boost::algorithm::erase_first(fo, dir);
            if (fo != "lit.cfg") {
                result.push_back(fo);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::string GetFileType(boost::filesystem::path p) {
        std::string ret("");
        std::string f(p.extension().string<std::string>());
        if (f.find(".pdf") != std::string::npos)
            ret = "PDF";
        else if (f.find(".nxml") != std::string::npos)
            ret = "NXML";
        else if (f.find(".bib") != std::string::npos)
            ret = "BIB";
        else if ((f.find(".gz") != std::string::npos) || (f.find(".tgz") != std::string::npos))
            ret = "NXML-TAR";
        else
            ret = "?";
        return ret;
    }
}

void PaperOrganizer::loginChanged() {
    if (session_->login().state() > 0) {
        username_ = session_->login().user().identity("loginname").toUTF8();
        uploadroot_ = USERUPLOADROOTDIR + std::string("/")
                + username_ +
                "/uploadedfiles";
        if (!boost::filesystem::exists(uploadroot_)) {
            boost::filesystem::path dir(uploadroot_);
            boost::filesystem::create_directories(dir);
        }
        load_papers_lit_map();
        UpdateLiteraturesInComboBox();
        PopulateUploadTable();
    } else {
        uploadroot_ = "/tmp/";
        username_ = "NOT_LOGGED_IN";
        if (!boost::filesystem::exists(uploadroot_)) {
            boost::filesystem::path dir(uploadroot_);
            boost::filesystem::create_directories(dir);
        }
    }
}

PaperOrganizer::PaperOrganizer(Session & session, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    session_ = &session;
    setContentAlignment(Wt::AlignCenter);
    statusline_ = new Wt::WText("");
    addWidget(statusline_);
    addWidget(new Wt::WBreak());
    filetablecontainer_ = new Wt::WContainerWidget();
    filetablecontainer_->setContentAlignment(Wt::AlignCenter);
    pushbuttoncontainer_ = new Wt::WContainerWidget();
    literaturecontainer_ = new Wt::WContainerWidget();
    literaturecontainer_->setVerticalAlignment(Wt::AlignMiddle);
    addWidget(filetablecontainer_);
    addWidget(pushbuttoncontainer_);
    addWidget(new Wt::WBreak());
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
    table_->setSelectionBehavior(Wt::SelectRows);
    table_->setSelectionMode(Wt::ExtendedSelection);
    table_->selectionChanged().connect(boost::bind(&PaperOrganizer::SelectionChanged, this));
    filetablecontainer_->addWidget(table_);
    //
    Wt::WText * combotext = new Wt::WText("Current Literature: ");
    litcombo = new Wt::WComboBox();
    litcombo->addItem(Wt::WString("Select literature..."));
    UpdateLiteraturesInComboBox();
    Wt::WImage * addliticon = new Wt::WImage("resources/plus.gif");
    addliticon->mouseWentOver().connect(boost::bind(&PaperOrganizer::SetCursorHand, this, addliticon));
    addliticon->setToolTip("Add a literature to your library.");
    Wt::WImage * removeliticon = new Wt::WImage("resources/minus.gif");
    removeliticon->mouseWentOver().connect(boost::bind(&PaperOrganizer::SetCursorHand, this, removeliticon));
    removeliticon->clicked().connect(boost::bind(&PaperOrganizer::RemoveLiterature, this));
    removeliticon->setToolTip("Remove a literature from your library.");
    Wt::WLineEdit * litname = new Wt::WLineEdit();
    litname->enterPressed().connect(boost::bind(&PaperOrganizer::LitNameEntered, this, litname));
    addliticon->clicked().connect(boost::bind(&PaperOrganizer::AddLiterature, this, litname));
    litname->hide();
    //
    Wt::WPushButton * DeletePaperButton = new Wt::WPushButton("Delete selections from library!");
    DeletePaperButton->setStyleClass("btn-danger");
    DeletePaperButton->clicked().connect(boost::bind(&PaperOrganizer::DeleteSelectedItems, this));
    pushbuttoncontainer_->addWidget(DeletePaperButton);
    //
    Wt::WPushButton * DeletePaperFromCurrentLiterature = new Wt::WPushButton("Delete selections from current literature!");
    DeletePaperFromCurrentLiterature->setStyleClass("btn-warning");
    DeletePaperFromCurrentLiterature->clicked().connect(boost::bind(&PaperOrganizer::DeleteSelectedItemsFromCurrentLiterature, this));
    pushbuttoncontainer_->addWidget(DeletePaperFromCurrentLiterature);
    //
    Wt::WPushButton * AddPaperToCurrentLiterature = new Wt::WPushButton("Add selections to current literature!");
    AddPaperToCurrentLiterature->setStyleClass("btn-success");
    AddPaperToCurrentLiterature->clicked().connect(boost::bind(&PaperOrganizer::AddSelectedItemsToCurrentLiterature, this));
    pushbuttoncontainer_->addWidget(AddPaperToCurrentLiterature);
    //
    combotext->setInline(true);
    litcombo->setInline(true);
    addliticon->setInline(true);
    removeliticon->setInline(true);
    litname->setInline(true);
    literaturecontainer_->addWidget(combotext);
    literaturecontainer_->addWidget(litcombo);
    literaturecontainer_->addWidget(addliticon);
    literaturecontainer_->addWidget(removeliticon);
    literaturecontainer_->addWidget(litname);
    load_papers_lit_map();
    session_->login().changed().connect(boost::bind(&PaperOrganizer::loginChanged, this));
    loginChanged();
}

PaperOrganizer::~PaperOrganizer() {
}

void PaperOrganizer::SelectionChanged() {
    for (int i = 0; i < model_->rowCount(); i++) model_->setData(i, 0, boost::any(false), Wt::CheckStateRole);
    Wt::WItemSelectionModel * sm(table_->selectionModel());
    Wt::WModelIndexSet mis(sm->selectedIndexes());
    for (Wt::WModelIndexSet::iterator it = mis.begin(); it != mis.end(); it++)
        model_->setData((*it).row(), (*it).column(),
            boost::any(true), Wt::CheckStateRole);
}

void PaperOrganizer::DeleteSelectedItemsFromCurrentLiterature() {
    if (!(litcombo->currentText().toUTF8() == "Select literature...")) {
        Wt::WItemSelectionModel * sm(table_->selectionModel());
        Wt::WModelIndexSet mis(sm->selectedIndexes());
        std::hash<std::string> string_hash;
        for (Wt::WModelIndexSet::iterator it = mis.begin(); it != mis.end(); it++) {
            std::string fn(Wt::asString(model_->data((*it).row(), 1)).toUTF8());
            boost::filesystem::path p(fn);
            papers_lit_map[p.stem().string()].erase(litcombo->currentText().toUTF8());
        }
        save_papers_lit_map();
        boost::filesystem::remove_all(boost::filesystem::path(uploadroot_ + "/../luceneindex"));
        PopulateUploadTable();
    } else {
        statusline_->setText("Current literature name is empty.");
        statusline_->decorationStyle().setForegroundColor(Wt::red);
        statusline_->decorationStyle().setBackgroundColor(Wt::yellow);
        Wt::WTimer * timer = new Wt::WTimer();
        timer->setInterval(5000);
        timer->setSingleShot(true);
        timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
            timer->stop();
            delete timer;
            statusline_->setText("");
            statusline_->decorationStyle().setForegroundColor(Wt::black);
            statusline_->decorationStyle().setBackgroundColor(Wt::white);
        }, timer));
        timer->start();

    }
}

void PaperOrganizer::AddSelectedItemsToCurrentLiterature() {
    if (!(litcombo->currentText().toUTF8() == "Select literature...")) {
        Wt::WItemSelectionModel * sm(table_->selectionModel());
        Wt::WModelIndexSet mis(sm->selectedIndexes());
        for (Wt::WModelIndexSet::iterator it = mis.begin(); it != mis.end(); it++) {
            std::string fn(Wt::asString(model_->data((*it).row(), 1)).toUTF8());
            boost::filesystem::path p(fn);
            boost::filesystem::path source(uploadroot_ + "/" + fn);
            std::string ft = GetFileType(source);
            if (ft == "PDF" || ft == "NXML" || ft == "NXML-TAR") {
                papers_lit_map[p.stem().string()].insert(litcombo->currentText().toUTF8());
            }
        }
        save_papers_lit_map();
        boost::filesystem::remove_all(boost::filesystem::path(uploadroot_ + "/../luceneindex"));
        PopulateUploadTable();
    } else {
        statusline_->setText("Current literature name is empty.");
        statusline_->decorationStyle().setForegroundColor(Wt::red);
        statusline_->decorationStyle().setBackgroundColor(Wt::yellow);
        Wt::WTimer * timer = new Wt::WTimer();
        timer->setInterval(5000);
        timer->setSingleShot(true);
        timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
            timer->stop();
            delete timer;
            statusline_->setText("");
            statusline_->decorationStyle().setForegroundColor(Wt::black);
            statusline_->decorationStyle().setBackgroundColor(Wt::white);
        }, timer));
        timer->start();
    }
}

void PaperOrganizer::DeleteSelectedItems() {
    Wt::WItemSelectionModel * sm(table_->selectionModel());
    Wt::WModelIndexSet mis(sm->selectedIndexes());
    for (Wt::WModelIndexSet::iterator it = mis.begin(); it != mis.end(); it++) {
        std::string fn(Wt::asString(model_->data((*it).row(), 1)).toUTF8());
        boost::filesystem::path source(uploadroot_ + "/" + fn);
        boost::filesystem::path fnp(fn);
        std::string ft = GetFileType(source);
        std::string stem(boost::filesystem::path(source).stem().string());
        // remove uploaded file
        boost::filesystem::path p(source);
        boost::filesystem::remove(p);
        papers_lit_map.erase(fnp.stem().string());
    }
    save_papers_lit_map();
    boost::filesystem::remove_all(boost::filesystem::path(uploadroot_ + "/../luceneindex"));
    PopulateUploadTable();
}

void PaperOrganizer::PopulateUploadTable() {
    if (session_->login().state() > 0) {
        model_->clear();
        // set headers
        model_->insertColumns(0, 5);
        model_->setHeaderData(0, boost::any(std::string("selected")));
        model_->setHeaderData(1, boost::any(std::string("file name")));
        model_->setHeaderData(2, boost::any(std::string("file type")));
        model_->setHeaderData(3, boost::any(std::string("upload date")));
        model_->setHeaderData(4, boost::any(std::string("assigned literature(s)")));
        table_->setColumnWidth(0, 100);
        table_->setColumnWidth(1, 300);
        table_->setColumnWidth(2, 100);
        table_->setColumnWidth(3, 200);
        table_->setColumnWidth(4, 300);
        table_->setWidth(1040);
        const int HEIGHT = 4;
        table_->setRowHeight(Wt::WLength(HEIGHT, Wt::WLength::FontEx));
        table_->setHeight(Wt::WLength(16 * HEIGHT, Wt::WLength::FontEx));
        std::vector<std::string> files(GetDirEntries(uploadroot_));
        if (files.size() > 0) {
            model_->insertRows(0, files.size());
            std::vector<std::string>::iterator it;
            int row(0);
            for (it = files.begin(); it != files.end(); it++, row++) {
                model_->setData(row, 0,
                        boost::any(false), Wt::CheckStateRole);
                model_->setData(row, 1,
                        boost::any(std::string(*it)));
                boost::filesystem::path p(uploadroot_ + "/" + *it);
                model_->setData(row, 2, boost::any(GetFileType(p)));
                std::time_t t = boost::filesystem::last_write_time(p);
                char buff[20];
                strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
                model_->setData(row, 3, boost::any(std::string(buff)));
                model_->setData(row, 4, boost::any(boost::join(papers_lit_map[boost::filesystem::path(*it).stem()
                        .string()], ", ")));
            }
        }
    }
}

void PaperOrganizer::UpdateLiteraturesInComboBox() {
    litcombo->clear();
    litcombo->addItem(Wt::WString("Select literature..."));
    for (const auto& lit : papers_lit_map["__all_literatures__"]) {
        litcombo->addItem(Wt::WString(lit));
    }
}

void PaperOrganizer::AddLiterature(Wt::WLineEdit * litname) {
    litname->show();
}

void PaperOrganizer::LitNameEntered(Wt::WLineEdit * litname) {
    litname->hide();
    if (!litname->text().empty()) {
        std::vector<std::string> takennames = tpc::index::IndexManager::get_available_corpora(tpc::index::CAS_ROOT_LOCATION.c_str());
        if (std::find(takennames.begin(), takennames.end(), litname->text().toUTF8()) == takennames.end()) {
            papers_lit_map["__all_literatures__"].insert(litname->text().toUTF8());
            litcombo->clear();
            litcombo->addItem(Wt::WString("Select literature..."));
            for (const auto& lit : papers_lit_map["__all_literatures__"]) {
                litcombo->addItem(Wt::WString(lit));
            }
            save_papers_lit_map();
            UpdateLiteraturesInComboBox();
            litcombo->setValueText(litname->text());
            litname->setText("");
        } else {
            statusline_->setText("Literature name is not available. Entry not taken.");
            statusline_->decorationStyle().setForegroundColor(Wt::red);
            statusline_->decorationStyle().setBackgroundColor(Wt::yellow);
            Wt::WTimer * timer = new Wt::WTimer();
            timer->setInterval(5000);
            timer->setSingleShot(true);
            timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
                timer->stop();
                delete timer;
                statusline_->setText("");
                statusline_->decorationStyle().setForegroundColor(Wt::black);
                statusline_->decorationStyle().setBackgroundColor(Wt::white);
            }, timer));
            timer->start();
        }
    }
}

void PaperOrganizer::RemoveLiterature() {
    if (litcombo->currentText().toUTF8() != "Select literature...") {
        for (auto it = papers_lit_map.begin(); it != papers_lit_map.end();) {
            if (it->second.find(litcombo->currentText().toUTF8()) != it->second.end()) {
                it->second.erase(litcombo->currentText().toUTF8());
                string fn = it->first;
                if (it->second.empty()) {
                    papers_lit_map.erase(it++->first);
                }
            } else {
                ++it;
            }
        }
        save_papers_lit_map();
        load_papers_lit_map();
        UpdateLiteraturesInComboBox();
        PopulateUploadTable();
        boost::filesystem::remove_all(boost::filesystem::path(uploadroot_ + "/../luceneindex"));
    }
}

void PaperOrganizer::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

void PaperOrganizer::save_papers_lit_map() {
    std::ofstream ofs(uploadroot_ + "/lit.cfg");
    {
        boost::archive::text_oarchive oa(ofs);
        oa << papers_lit_map;
    }
}

void PaperOrganizer::load_papers_lit_map() {
    std::ifstream ifs(uploadroot_ + "/lit.cfg", std::ios::binary);
    if (ifs) {
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        ia >> papers_lit_map;
    }
}