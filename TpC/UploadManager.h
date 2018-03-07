/* 
 * File:   UploadManager.h
 * Author: mueller
 *
 * Created on October 12, 2016, 10:53 AM
 */

#ifndef UPLOADMANAGER_H
#define	UPLOADMANAGER_H

#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WFileUpload>

class UploadManager : public Wt::WContainerWidget {
public:
    UploadManager(Session & session, Wt::WContainerWidget * parent = NULL);
    virtual ~UploadManager();
private:
    Session * session_;
    Wt::WText * statusline_;
    Wt::WContainerWidget * fileuploadcontainer_;
    Wt::WContainerWidget * listcontainer_;
    void SetUpNewFileUpload(Wt::WContainerWidget * parent);
    void UploadFile();
    void FileUploaded(Wt::WFileUpload * ful);
    void FileTooLarge(Wt::WPushButton * ub);

};

#endif	/* UPLOADMANAGER_H */

