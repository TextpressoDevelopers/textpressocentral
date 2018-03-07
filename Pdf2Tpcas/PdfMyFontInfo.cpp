/* 
 * File:   PdfMyFontInfo.cpp
 * Author: mueller
 * 
 * Created on January 28, 2013, 11:38 AM
 */

#include "PdfMyFontInfo.h"

#include <iterator>

PdfMyFontInfo::PdfMyFontInfo() {
    m_fontname = PoDoFo::PdfName("");
    m_italicangle = 0;
    m_bboxXlength = 0;
    m_bboxYlength = 0;
    m_bboxlx = 0;
    m_bboxly = 0;
    m_bboxux = 0;
    m_bboxuy = 0;
    m_flags = 0;
    m_first_char = 0;
    m_last_char = 0;
    m_is_ftinfo_italics = false;
    m_is_ftinfo_bold = false;
    m_is_ftinfo_strikeout = false;
    m_is_ftinfo_subsetting = false;
    m_is_ftinfo_underlined = false;
    m_is_greek = false;
    m_is_ftinfo_incomplete = true;
    m_has_charwidths = false;
    m_charwidths = PoDoFo::PdfArray();
    pm_pdfobject = NULL;
    pm_pdffont = NULL;
    pm_pdffontmetrics = NULL;

}

PdfMyFontInfo::PdfMyFontInfo(PoDoFo::PdfObject * pFont,
        PoDoFo::PdfDocument * mDoc) {
    PdfMyFontInfo::set(pFont, mDoc);
}

void PdfMyFontInfo::PrintMyFontInfo(std::ostream & sOut) {

    sOut << "m_fontname: " << m_fontname.GetEscapedName() << std::endl;
    sOut << "m_italicangle: " << m_italicangle << std::endl;
    sOut << "m_bboxXlength: " << m_bboxXlength << std::endl;
    sOut << "m_bboxYlength: " << m_bboxYlength << std::endl;
    sOut << "m_bboxlx: " << m_bboxlx << std::endl;
    sOut << "m_bboxly: " << m_bboxly << std::endl;
    sOut << "m_bboxux: " << m_bboxux << std::endl;
    sOut << "m_bboxuy: " << m_bboxuy << std::endl;
    sOut << "m_flags: " << m_flags << std::endl;
    sOut << "m_first_char: " << m_first_char << std::endl;
    sOut << "m_last_char: " << m_last_char << std::endl;
    sOut << "m_is_ftinfo_italics: " << m_is_ftinfo_italics << std::endl;
    sOut << "m_is_ftinfo_bold: " << m_is_ftinfo_bold << std::endl;
    sOut << "m_is_ftinfo_strikeout: " << m_is_ftinfo_strikeout << std::endl;
    sOut << "m_is_ftinfo_subsetting: " << m_is_ftinfo_subsetting << std::endl;
    sOut << "m_is_ftinfo_underlined: " << m_is_ftinfo_underlined << std::endl;
    sOut << "m_is_greek: " << m_is_greek << std::endl;
    sOut << "m_has_charwidths: " << m_has_charwidths << std::endl;
    sOut << "m_is_ftinfo_incomplete; " << m_is_ftinfo_incomplete << std::endl << std::endl;
    sOut << "pm_pdfobject name: " << (*pm_pdfobject).GetName().GetEscapedName() << std::endl;
    sOut << "pm_pdffont name: " << (*pm_pdffont).GetIdentifier().GetEscapedName() << std::endl;
    sOut << "pm_pdffontmetrics name: " << (*pm_pdffontmetrics).GetFontname() << std::endl;

}

void PdfMyFontInfo::set(PoDoFo::PdfObject * pFont, PoDoFo::PdfDocument * mDoc) {
    m_is_ftinfo_incomplete = false;
    if (pFont->IsDictionary()) {
        PoDoFo::PdfDictionary dicft = pFont->GetDictionary();
        bool fc = dicft.HasKey(PoDoFo::PdfName("FirstChar"));
        bool lc = dicft.HasKey(PoDoFo::PdfName("LastChar"));
        m_has_charwidths = dicft.HasKey(PoDoFo::PdfName("Widths"));
        if (fc) m_first_char = dicft.GetKeyAsLong("FirstChar");
        if (lc) m_last_char = dicft.GetKeyAsLong("LastChar");

        if (m_has_charwidths) {
            PoDoFo::PdfObject * auxw = dicft.GetKey(PoDoFo::PdfName("Widths"));
            if (auxw->IsArray()) {
                m_charwidths = auxw->GetArray();
                if (m_charwidths.empty()) m_has_charwidths = false;
            } else {
                m_has_charwidths = false;
            }
        }
        if (dicft.HasKey(PoDoFo::PdfName("FontDescriptor"))) {
            PoDoFo::PdfObject * fd
                    = dicft.GetKey(PoDoFo::PdfName("FontDescriptor"));
            PoDoFo::PdfVecObjects * pDocObjects = mDoc->GetObjects();
            PoDoFo::PdfObject * FontDescriptor
                    = pDocObjects->GetObject(fd->GetReference());
            if (FontDescriptor->IsDictionary()) {
                PoDoFo::PdfDictionary dicdf = FontDescriptor->GetDictionary();
                m_fontname = dicdf.GetKeyAsName(PoDoFo::PdfName("FontName"));
                m_italicangle = dicdf.GetKeyAsLong(PoDoFo::PdfName("ItalicAngle"));
                PoDoFo::PdfObject * bboxobj
                        = dicdf.GetKey(PoDoFo::PdfName("FontBBox"));
                if (bboxobj->IsArray()) {
                    PoDoFo::PdfArray bbox = bboxobj->GetArray();
                    m_bboxlx = bbox[0].GetNumber();
                    m_bboxly = bbox[1].GetNumber();
                    m_bboxux = bbox[2].GetNumber();
                    m_bboxuy = bbox[3].GetNumber();
                    m_bboxXlength = m_bboxux - m_bboxlx;
                    m_bboxYlength = m_bboxuy - m_bboxly;
                }
                m_flags = dicdf.GetKeyAsLong(PoDoFo::PdfName("Flags"));
                m_is_greek = false;
                if (dicdf.HasKey(PoDoFo::PdfName("CharSet"))) {
                    const std::string greek [] = {
                        "Alpha", "Nu", "Beta", "Xi",
                        "Gamma", "Omicron", "Delta", "Pi",
                        "Epsilon", "Rho", "Zeta", "Sigma",
                        "Eta", "Tau", "Theta", "Ypsilon", "Upsilon",
                        "Iota", "Phi", "Kappa", "Chi",
                        "Lambda", "Psi", "Mu", "Omega",
                        "alpha", "nu", "beta", "xi",
                        "gamma", "omicron", "delta", "pi",
                        "epsilon", "rho", "zeta", "sigma",
                        "eta", "tau", "theta", "ypsilon", "upsilon",
                        "iota", "phi", "kappa", "chi",
                        "lambda", "psi", "mu", "omega",
                        "ALPHA", "NU", "BETA", "XI",
                        "GAMMA", "OMICRON", "DELTA", "PI",
                        "EPSILON", "RHO", "ZETA", "SIGMA",
                        "ETA", "TAU", "THETA", "YPSILON", "UPSILON",
                        "IOTA", "PHI", "KAPPA", "CHI",
                        "LAMBDA", "PSI", "MU", "OMEGA"
                    };
                    int sizeofgreek = sizeof (greek) / sizeof (greek[0]);
                    PoDoFo::PdfObject * cs = dicdf.GetKey(PoDoFo::PdfName("CharSet"));
                    if (cs->IsString()) {
                        std::string aux = cs->GetString().GetStringUtf8();
                        aux += '/';
                        for (int j = 0; j < sizeofgreek; j++) {
                            if (aux.find('/' + greek[j] + '/') < std::string::npos)
                                m_is_greek = true;
                            if (m_is_greek) break;
                        }
                    }
                }
            }
            if (m_has_charwidths) {
                pm_pdffont = NULL;
                pm_pdffont = static_cast<PoDoFo::PdfMemDocument*> (mDoc)->GetFont(pFont);
                if (pm_pdffont != NULL) {
                    m_is_ftinfo_bold = pm_pdffont->IsBold();
                    m_is_ftinfo_italics = pm_pdffont->IsItalic();
                    m_is_ftinfo_strikeout = pm_pdffont->IsStrikeOut();
                    m_is_ftinfo_subsetting = pm_pdffont->IsSubsetting();
                    m_is_ftinfo_underlined = pm_pdffont->IsUnderlined();
                    pm_pdffontmetrics = pm_pdffont->GetFontMetrics();
                } else {
                    m_is_ftinfo_incomplete = true;
                }
            } else {
                m_is_ftinfo_incomplete = true;
            }
        } else {
            m_is_ftinfo_incomplete = true;
        }
        pm_pdfobject = pFont;
    } else {
        m_is_ftinfo_incomplete = true;
    }
}

double PdfMyFontInfo::GetStringWidth(const PoDoFo::PdfString & s) {
    double sum = 0.0;
    const char * a = s.GetString();
    for (int i = 0; i < s.GetLength(); i++)
        sum += GetCharWidth(a[i]);
    return sum;
}

double PdfMyFontInfo::GetCharWidth(unsigned const char c) {
    long i = long (c);
    bool out_of_range = (i - m_first_char < 0) || (i > m_last_char);
    if (!out_of_range)
        if (m_has_charwidths)
            if (m_charwidths[i - m_first_char] != PoDoFo::PdfVariant::NullValue) {
                if (m_charwidths[i - m_first_char].IsNumber())
                    return m_charwidths[i - m_first_char].GetReal() / 1000.0;
            }
    return double(m_bboxXlength) / 1000.0;
}

bool PdfMyFontInfo::AnyRelevantFlagSet() {
    bitset < 32 > flags(m_flags);
    return flags.test(FixedPitch) || flags.test(Serif)
            || flags.test(Symbolic) || flags.test(Script)
            || flags.test(Nonsymbolic) || flags.test(Italic)
            || flags.test(AllCap) || flags.test(SmallCap)
            || flags.test(ForceBold);
}

bool PdfMyFontInfo::FontNameContainsBold() {
    return (m_fontname.GetName().find("Bold") != string::npos)
            || (m_fontname.GetName().find("bold") != string::npos)
            || (m_fontname.GetName().find("BOLD") != string::npos);
}

bool PdfMyFontInfo::FontNameContainsItalic() {
    return (m_fontname.GetName().find("Italic") != string::npos)
            || (m_fontname.GetName().find("italic") != string::npos)
            || (m_fontname.GetName().find("ITALIC") != string::npos);
}

bool PdfMyFontInfo::IsItalics() {
    return IsFlagSet(Italic) || (getItalicAngle() != 0)
            || FontNameContainsItalic() || m_is_ftinfo_italics;
}

bool PdfMyFontInfo::IsBold() {
    return IsFlagSet(ForceBold) || FontNameContainsBold() || m_is_ftinfo_bold;
}

PoDoFo::PdfString PdfMyFontInfo::Convert2Unicode(const PoDoFo::PdfString & string) {
    if (pm_pdffont != NULL) {
        return pm_pdffont->GetEncoding()->ConvertToUnicode(string, pm_pdffont);
    } else {
        return string;
    }
}

PoDoFo::PdfObject * PdfMyFontInfo::GetRawPdfObject() {
    return pm_pdfobject;
}

PdfMyFontInfo::~PdfMyFontInfo() {
}
