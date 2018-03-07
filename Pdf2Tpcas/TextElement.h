/* 
 * File:   TextElement.h
 * Author: mueller
 *
 * Created on January 28, 2013, 11:50 AM
 */

#ifndef TEXTELEMENT_H
#define	TEXTELEMENT_H


#include <algorithm>
#include <podofo/podofo.h>
#include <math.h>
#include "PdfMyFontInfo.h"

/*!
 * a TextElement represents the text in a pdf contained in a single text block or a link to an image saved on local
 * filesystem. Each element points to the next one, creating thus a chain of elements that represents the document
 * (usually one page)
 */
class TextElement {
public:
    TextElement();
    TextElement(const TextElement * orig);
    virtual ~TextElement();
    void SetElementContent(const double dCurPosX, const double dCurPosY,
        const double nextxoff, const double nextyoff,
        const int pageno, double fsize, PdfMyFontInfo myfi,
        const double widthscalefactor,
        const PoDoFo::PdfString & rString);
    void PrintElementContent(std::ostream & sout, bool withxmltags, bool datatabdel);
    std::string PrintElementContentAsString(bool withxmltags, bool datatabdel);
    bool ElementPriority(TextElement * s);
    list<double> CalculateTwoColumnBridges(double pxc);
    bool ElementIsInBox(PoDoFo::PdfArray box);
    bool ElementIsInRect(PoDoFo::PdfRect box);
    inline double GetX();
    inline double GetY();
    inline double GetNextXOff();
    inline double GetNextYOff();
    inline int GetP();
    inline TextElement * GetNext();
    inline TextElement * SetNext(TextElement * p);
    inline PdfMyFontInfo GetMyFontInfo();
    inline double  GetFontSize();
    inline std::string GetFontName();
    inline double GetStringWidth();
    inline double GetBoxedCharWidth();
    inline long GetNumberOfCharacters();
    inline long GetNumberOfWhiteCharacters();
    inline PoDoFo::PdfString GetString();
    void AddToChainEnd(TextElement * const p);
    TextElement * InsertToSortedChain (TextElement * const p);
    list<TextElement*> SortChainGeometrically(double pl, double pw,
        double pb, double ph, double pxc, double pyc, list<double> bridges);
    void PrintChainContent(std::ostream & sout, bool withxmltags, bool datatabdel);
    void PrintChainSpaced(std::ostream & sout, bool datatabdel);
    std::string PrintChainSpacedAsString(bool databdel);
    void DeleteChain();
private:
    double m_X;
    double m_Y;
    double m_nextXOff;
    double m_nextYOff;
    int m_pageno;
    double m_fontsize;
    PdfMyFontInfo m_myfi;
    double m_stringwidth;
    double m_boxedcharwidth;
    long m_numberofchars;
    long m_numberofwhitechars;
    PoDoFo::PdfString m_rString;
    TextElement * m_next;
    double GeometricNegLogLikelihood(TextElement *s, double pl, double pw,
        double pb, double ph, double pxc, double pyc, bool linehasbridge);
};

inline double TextElement::GetX() {
    return m_X;
}

inline double TextElement::GetY() {
    return m_Y;
}
inline double TextElement::GetNextXOff() {
    return m_nextXOff;
}

inline double TextElement::GetNextYOff() {
    return m_nextYOff;
}

inline int TextElement::GetP() {
    return m_pageno;
}

inline TextElement * TextElement::GetNext() {
    return m_next;
}

inline TextElement * TextElement::SetNext(TextElement * p) {
    m_next = p;
}

inline PdfMyFontInfo TextElement::GetMyFontInfo() {
    return m_myfi;
}

inline double TextElement::GetFontSize() {
    return m_fontsize;
}

inline std::string TextElement::GetFontName() {
    return m_myfi.getFontName();
}

inline double TextElement::GetStringWidth() {
    return m_stringwidth;
}

inline double TextElement::GetBoxedCharWidth() {
    return m_boxedcharwidth;
}

inline long TextElement::GetNumberOfCharacters() {
    return m_numberofchars;
}

inline long TextElement::GetNumberOfWhiteCharacters() {
    return m_numberofwhitechars;
}

inline PoDoFo::PdfString TextElement::GetString() {
    return m_rString;
}

#endif	/* TEXTELEMENT_H */

