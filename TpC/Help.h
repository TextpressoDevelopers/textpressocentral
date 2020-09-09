/* 
 * File:   Help.h
 * Author: mueller
 *
 * Created on July 24, 2019, 6:33 PM
 */

#ifndef HELP_H
#define HELP_H

#include <Wt/WContainerWidget>

class Help : public Wt::WContainerWidget {
public:
    Help();
    void LoadContent();
    Help(const Help& orig);
    void DisplayRandomItem(Wt::WContainerWidget * se);
    void DisplayTextAndLinksFromFile(std::string filename,
            Wt::WColor textcolor, Wt::WContainerWidget * container);
    virtual ~Help();
private:

};

#endif /* HELP_H */

