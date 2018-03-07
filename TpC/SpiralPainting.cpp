/* 
 * File:   SpiralPainting.cpp
 * Author: mueller
 * 
 * Created on May 13, 2014, 1:13 PM
 */

#include "SpiralPainting.h"
#include <Wt/WPainter>
//#include <Wt/WFont>

SpiralPainting::SpiralPainting(long unsigned int xsize, long unsigned int ysize,
        std::vector<std::string> & terms, int noi, Wt::WContainerWidget * parent) : Wt::WPaintedWidget(parent) {
    xsize_ = xsize;
    ysize_ = ysize;
    MaxNumberOfItems_ = noi;
    std::vector<std::string>::iterator vsit;
    for (vsit = terms.begin(); vsit != terms.end(); vsit++)
        terms_.push_back(*vsit);
    resize(Wt::WLength(xsize_, Wt::WLength::Pixel), Wt::WLength(ysize_, Wt::WLength::Pixel));
}

void SpiralPainting::paintEvent(Wt::WPaintDevice * paintDevice) {
    Wt::WPainter painter(paintDevice);
    painter.setBrush(Wt::WBrush(Wt::WBrush(Wt::blue)));
    painter.translate(double(xsize_) / 2.0, double(ysize_) / 2.0);
    int numberOfItems = (MaxNumberOfItems_ > terms_.size()) ? terms_.size() : MaxNumberOfItems_;
    double radiusoffset = 48.0;
    double rotationstep = 30.0;
    double maxradius = (xsize_ > ysize_) ? ysize_ / 2.0 : xsize_ / 2.0;
    double radiusstep = abs(maxradius - radiusoffset) / double(numberOfItems);
    int colorstep = 240 / numberOfItems;
    for (int i = 0; i < numberOfItems; i++) {
        double radius = radiusstep * double(i) + radiusoffset;
        double angle = rotationstep * double(i);
        double x = radius * sin(angle);
        double y = radius * cos(angle);
        painter.setBrush(Wt::WBrush(Wt::WColor(255 - colorstep * i, colorstep * i, colorstep * i)));
        painter.drawEllipse(x, y, 10, 10);
        const Wt::WColor pcolor(255 - colorstep * i, colorstep * i, colorstep * i);
        Wt::WPen pen;
        pen.setColor(pcolor);
        painter.setPen(pen);
        Wt::WFont font;
        font.setFamily(Wt::WFont::Default, "'Times New Roman'");
        font.setSize(18);
        painter.setFont(font);
        Wt::WString text(terms_[i]);
        if (x >= 0.0)
            painter.drawText(x + 15, y, 0, 10, Wt::AlignLeft | Wt::AlignMiddle, text);
        else
            painter.drawText(x - 15, y, 0, 10, Wt::AlignRight | Wt::AlignMiddle, text);
    }
}
