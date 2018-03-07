/* 
 * File:   SpiralPainting.h
 * Author: mueller
 *
 * Created on May 13, 2014, 1:13 PM
 */

#ifndef SPIRALPAINTING_H
#define	SPIRALPAINTING_H

#include <Wt/WPaintedWidget>

class SpiralPainting : public Wt::WPaintedWidget {
public:
    SpiralPainting(long unsigned int xsize, long unsigned int ysize, 
            std::vector<std::string> & terms, int noi, Wt::WContainerWidget * parent = NULL);
protected:
    void paintEvent(Wt::WPaintDevice * paintDevice);
private:
    long unsigned int xsize_;
    long unsigned int ysize_;
    int MaxNumberOfItems_;
    std::vector<std::string> terms_;
};

#endif	/* SPIRALPAINTING_H */

