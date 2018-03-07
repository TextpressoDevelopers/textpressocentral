/* 
 * File:   PdfMyFontInfo.h
 * Author: mueller
 *
 * Created on January 28, 2013, 11:38 AM
 */

#ifndef PDFMYFONTINFO_H
#define	PDFMYFONTINFO_H

#include <podofo/podofo.h>
#include <bitset>
#include <iostream>

using namespace std;

enum flagbits {
    FixedPitch = 1,
    Serif = 2,
    Symbolic = 3,
    Script = 4,
    Nonsymbolic = 6,
    Italic = 7,
    AllCap = 17,
    SmallCap = 18,
    ForceBold = 19
};

class PdfMyFontInfo {
public:
    PdfMyFontInfo();
    PdfMyFontInfo(PoDoFo::PdfObject * Font, PoDoFo::PdfDocument * mDoc);
    void set(PoDoFo::PdfObject * Font, PoDoFo::PdfDocument * mDoc);
    void PrintMyFontInfo(std::ostream & sOut);
    double GetStringWidth(const PoDoFo::PdfString & s);
    inline double GetCharWidth(unsigned const char c);
    inline std::string getFontName();
    inline int getItalicAngle();
    inline long getBboxXlength();
    inline long getBboxYlength();
    inline long getBboxlx();
    inline long getBboxly();
    inline long getBboxux();
    inline long getBboxuy();
    inline bool IsFlagSet(flagbits x);
    inline long getRawFlags();
    inline bool IsGreek();
    inline bool IsIncomplete();
    bool AnyRelevantFlagSet();
    bool FontNameContainsBold();
    bool FontNameContainsItalic();
    bool IsItalics();
    bool IsBold();
    PoDoFo::PdfString Convert2Unicode(const PoDoFo::PdfString & string);
    PoDoFo::PdfObject * GetRawPdfObject();
    virtual ~PdfMyFontInfo();
private:
    PoDoFo::PdfName m_fontname;
    long m_italicangle;
    long m_bboxXlength;
    long m_bboxYlength;
    long m_bboxlx;
    long m_bboxly;
    long m_bboxux;
    long m_bboxuy;
    long m_flags;
    long m_first_char;
    long m_last_char;
    bool m_is_ftinfo_italics;
    bool m_is_ftinfo_bold;
    bool m_is_ftinfo_strikeout;
    bool m_is_ftinfo_subsetting;
    bool m_is_ftinfo_underlined;
    bool m_is_greek;
    bool m_has_charwidths;
    bool m_is_ftinfo_incomplete;
    PoDoFo::PdfArray m_charwidths;
    PoDoFo::PdfObject * pm_pdfobject;
    PoDoFo::PdfFont * pm_pdffont;
    const PoDoFo::PdfFontMetrics * pm_pdffontmetrics;

};

inline std::string PdfMyFontInfo::getFontName() {
    return m_fontname.GetName();
}

inline int PdfMyFontInfo::getItalicAngle() {
    return m_italicangle;
}

inline long PdfMyFontInfo::getBboxXlength() {
    return m_bboxXlength;
}

inline long PdfMyFontInfo::getBboxYlength() {
    return m_bboxYlength;
}

inline long PdfMyFontInfo::getBboxlx() {
    return m_bboxlx;
}

inline long PdfMyFontInfo::getBboxly() {
    return m_bboxly;
}

inline long PdfMyFontInfo::getBboxux() {
    return m_bboxux;
}

inline long PdfMyFontInfo::getBboxuy() {
    return m_bboxuy;
}

inline bool PdfMyFontInfo::IsFlagSet(flagbits x) {
    bitset < 32 > flags(m_flags);
    return flags.test(x);
}

inline bool PdfMyFontInfo::IsGreek() {
    return m_is_greek;
}

inline long PdfMyFontInfo::getRawFlags() {
    return m_flags;
}

inline bool PdfMyFontInfo::IsIncomplete() {
    return m_is_ftinfo_incomplete;
}

#endif	/* PDFMYFONTINFO_H */
