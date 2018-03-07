/* 
 * File:   UploadManager.cpp
 * Author: mueller
 * 
 * Created on October 12, 2016, 10:53 AM
 */

#include "UploadManager.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <sys/stat.h>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WCssDecorationStyle>

UploadManager::UploadManager(Session & session, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    session_ = &session;
    statusline_ = new Wt::WText("");
    addWidget(statusline_);
    addWidget(new Wt::WBreak());
    fileuploadcontainer_ = new Wt::WContainerWidget();
    addWidget(fileuploadcontainer_);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WText("All files uploaded so far in this session are listed below:"));
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    listcontainer_ = new Wt::WContainerWidget();
    addWidget(listcontainer_);
    SetUpNewFileUpload(fileuploadcontainer_);
}

void UploadManager::SetUpNewFileUpload(Wt::WContainerWidget * parent) {
    // Provide file upload
    Wt::WFileUpload * fileupload = new Wt::WFileUpload();
    fileupload->setInline(true);
    fileupload->setMargin(10, Wt::Right);
    // Provide a button
    Wt::WPushButton * uploadButton = new Wt::WPushButton("Upload");
    uploadButton->setInline(true);
    uploadButton->setMargin(10, Wt::Left | Wt::Right);
    // Allow multiple file uploads
    fileupload->setMultiple(true);
    fileupload->uploaded().connect(boost::bind(&UploadManager::FileUploaded, this, fileupload));
    // React to a upload problem.
    fileupload->fileTooLarge().connect(boost::bind(&UploadManager::FileTooLarge, this, uploadButton));
    // Upload when the button is clicked.
    uploadButton->clicked().connect(std::bind([ = ] (){
        fileupload->upload();
        uploadButton->disable();
    }));
    parent->addWidget(fileupload);
    parent->addWidget(uploadButton);
}

void UploadManager::FileUploaded(Wt::WFileUpload * ful) {
    std::string msg("");
    if (session_->login().state() > 0) {
        bool somefileexisted = false;
        int count = 0;
        std::string root = USERUPLOADROOTDIR + std::string("/")
                + session_->login().user().identity("loginname").toUTF8() + "/uploadedfiles";
        if (!boost::filesystem::exists(root)) {
            boost::filesystem::path dir(root);
            boost::filesystem::create_directories(dir);
        }
        std::vector<Wt::Http::UploadedFile> upldfls = ful->uploadedFiles();
        std::vector<Wt::Http::UploadedFile>::iterator it;
        for (it = upldfls.begin(); it < upldfls.end(); it++) {
            boost::filesystem::path src
                    = boost::filesystem::path((*it).spoolFileName());
            boost::filesystem::path dst
                    = boost::filesystem::path(root + "/" + (*it).clientFileName());
            listcontainer_->addWidget(new Wt::WText((*it).clientFileName()));
            listcontainer_->addWidget(new Wt::WBreak());
            if (!boost::filesystem::exists(dst)) {
                boost::filesystem::copy_file(src, dst);
                count++;
            } else {
                somefileexisted = true;
            }
        }
        if (count == 0) {
            msg += "No";
        } else {
            msg += boost::lexical_cast<std::string>(count);
        }
        msg += " file";
        if (count > 1) msg += "s";
        msg += " successfully uploaded.";
        if (somefileexisted) {
            msg += " At least one filename already existed in destination ";
            msg += "directory. No files were overwritten during upload.";
        }
    }
    msg += "When you are done, proceed to the 'Organize' tab.";
    statusline_->setText(msg);
    statusline_->decorationStyle().setBackgroundColor(Wt::white);
    fileuploadcontainer_->clear();
    Wt::WPushButton * newupload = new Wt::WPushButton("New Upload");
    fileuploadcontainer_->addWidget(newupload);
    newupload->clicked().connect(std::bind([ = ] (){
        statusline_->setText("");
        fileuploadcontainer_->clear();
        SetUpNewFileUpload(fileuploadcontainer_);
    }));
}

void UploadManager::FileTooLarge(Wt::WPushButton * ub) {
    statusline_->setText("File or set of files too large!");
    statusline_->decorationStyle().setBackgroundColor(Wt::red);
    ub->enable();
}

UploadManager::~UploadManager() {
}

