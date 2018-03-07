/* 
 * File:   TextElement.cpp
 * Author: mueller
 * 
 * Created on January 28, 2013, 11:50 AM
 */

#include "TextElement.h"


namespace {

    std::string utf8AsPrintableAscii(const string & string) {
        bool boingflag = false;
        int c, i, ix;
        std::string ret("");
        for (i = 0, ix = string.length(); i < ix; i++) {
            c = (unsigned char) string[i];
            if (c == 0x09 || c == 0x0a || c == 0x0d || (0x20 <= c && c <= 0x7e)) // is_printable_ascii
                ret += string[i];
            else if (c == 2) //do special case fl
                ret += "fi";
            else if (c == 3) //do special case fl
                ret += "fl";
            else if (c == 194) {
                if (i + 1 < ix)
                    if (((unsigned char) string[i + 1]) == 176) {
                        ret += " degree ";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 181) {
                        ret += "micro ";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 177) {
                        ret += " plus/minus ";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 169) {
                        ret += " copyright ";
                        i++;
                    }
            } else if (c == 197) {
                if (i + 1 < ix)
                    if (((unsigned char) string[i + 1]) == 146) {
                        ret += "-";
                        i++;
                    }
            } else if (c == 195) {
                if (i + 1 < ix)
                    if (((unsigned char) string[i + 1]) == 158) {
                        ret += "fi";
                        i++;
                    }
            } else if (c == 203) {
                if (i + 1 < ix)
                    if (((unsigned char) string[i + 1]) == 154) {
                        ret += "fi";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 156) {
                        ret += "fl";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 157) {
                        ret += "fi";
                        i++;
                    } else if (((unsigned char) string[i + 1]) == 155) {
                        ret += "fl";
                        i++;
                    }
            } else
                boingflag = true;
        }
        if (boingflag) {
            std::cerr << "UTFERROR: ";
            std::cerr << " STRING: " << string;
            std::cerr << " NUMBERS: ";
            for (i = 0, ix = string.length(); i < ix; i++) {

                c = (unsigned char) string[i];
                std::cerr << c << " ";
            }
            std::cerr << std::endl;
        }
        return ret;
    }

    bool utf8_check_is_valid(const string& string) {
        int c, i, ix, n, j;
        for (i = 0, ix = string.length(); i < ix; i++) {
            c = (unsigned char) string[i];
            std::cerr << string[i] << " - " << c << std::endl;
            //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
            if (0x00 <= c && c <= 0x7f) n = 0; // 0bbbbbbb
            else if ((c & 0xE0) == 0xC0) n = 1; // 110bbbbb
            else if (c == 0xed && i < (ix - 1) && ((unsigned char) string[i + 1] & 0xa0) == 0xa0) return false; //U+d800 to U+dfff
            else if ((c & 0xF0) == 0xE0) n = 2; // 1110bbbb
            else if ((c & 0xF8) == 0xF0) n = 3; // 11110bbb
                //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
                //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
            else return false;
            for (j = 0; j < n && i < ix; j++) { // n bytes matching 10bbbbbb follow ?
                if ((++i == ix) || (((unsigned char) string[i] & 0xC0) != 0x80))

                    return false;
            }
        }
        return true;
    }
}

TextElement::TextElement() {

    m_next = NULL;
}

TextElement::TextElement(const TextElement * orig) {

    * this = *orig;
    m_next = NULL;
}

TextElement::~TextElement() {
}

void TextElement::SetElementContent(const double dCurPosX, const double dCurPosY,
        const double nextxoff, const double nextyoff,
        const int pageno, double fsize, PdfMyFontInfo myfi,
        const double widthscalefactor,
        const PoDoFo::PdfString & rString) {

    m_pageno = pageno;
    m_X = dCurPosX;
    m_Y = dCurPosY;
    m_nextXOff = nextxoff;
    m_nextYOff = nextyoff;
    m_fontsize = fsize;
    m_myfi = myfi;
    m_stringwidth = m_myfi.GetStringWidth(rString) * widthscalefactor;
    m_boxedcharwidth = m_myfi.getBboxXlength() * widthscalefactor / 1000.0;
    m_numberofchars = rString.GetLength();
    m_numberofwhitechars = long(std::count(rString.GetStringUtf8().begin(),
            rString.GetStringUtf8().end(), ' '));
    m_rString = rString;
}

/*!
 * print the content of the text element to an output stream
 *
 * @param sout the output stream where to print the text element
 * @param withxmltags whether to print metadata with xml tags
 * @param datatabdel whether to print metadata as plain text - note that this takes precedence over <b>withxmltags</b>
 *        parameter
 */
void TextElement::PrintElementContent(std::ostream & sout, bool withxmltags, bool datatabdel) {
    // print header info
    if (datatabdel) {
        sout << m_pageno + 1 << "\t";
        sout << int(m_X + 0.5) << "\t";
        sout << int(m_Y + 0.5) << "\t";
        sout << m_myfi.getFontName() << "\t";
        sout << m_fontsize << "\t";
    } else if (withxmltags) {
        sout << "<text";
        sout << " page='" << m_pageno + 1 << "'";
        sout << " x='" << int(m_X + 0.5) << "'";
        sout << " y='" << int(m_Y + 0.5) << "'";
        sout << " font='" << m_myfi.getFontName() << "'";
        sout << " fontsize='" << m_fontsize << "'";
    }
    std::string out = "";
    bool setcomma = false;

    // style information
    if (m_myfi.IsBold()) {
        if (setcomma) out += ",";
        out += "bold";
        setcomma = true;
    }
    if (m_myfi.IsItalics()) {
        if (setcomma) out += ",";
        out += "italics";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Symbolic)) {
        if (setcomma) out += ",";
        out += "symbolic";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Nonsymbolic)) {
        if (setcomma) out += ",";
        out += "nonsymbolic";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Serif)) {
        if (setcomma) out += ",";
        out += "serif";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Script)) {
        if (setcomma) out += ",";
        out += "script";
        setcomma = true;
    }
    if (m_myfi.IsGreek()) {
        if (setcomma) out += ",";
        out += "greek";
        setcomma = true;
    }
    if (m_myfi.AnyRelevantFlagSet()) {
        if (setcomma) out += ",";
        out += "other";
    }
    if (datatabdel) {
        sout << out << "\t";
    } else if (withxmltags) {
        if (out.compare("") != 0) {
            sout << " style='";
            sout << out;
            sout << "'";
        }
    }
    // if metadata information is needed, print the text length and then print the string content
    PoDoFo::pdf_long l = m_rString.GetLength();
    std::string convstring = utf8AsPrintableAscii(m_rString.GetStringUtf8());
    if (datatabdel) {
        sout << l << "\t";
        sout << int(m_stringwidth + 0.5) << "\t";
        sout << int(m_nextXOff + 0.5) << "\t";
        sout << convstring << "\n";
        //        sout << m_myfi.Convert2Unicode(m_rString).GetStringUtf8() << "\n";
    } else if (withxmltags) {
        if (l > 0) {
            sout << " length='" << l << "'";
        }
        sout << " stringwidth='" << int(m_stringwidth + 0.5) << "'";
        sout << " off='" << int(m_nextXOff + 0.5) << "'";
        sout << ">";
        sout << convstring;
        //        sout << m_myfi.Convert2Unicode(m_rString).GetStringUtf8();
    } else {
        sout << convstring;
        //        sout << m_myfi.Convert2Unicode(m_rString).GetStringUtf8();
    }
    if ((withxmltags) && (!datatabdel)) {

        sout << "</text>" << std::endl;
    }
}

/*!
 * get the string representing the content of the text element
 *
 * @param withxmltags whether to print metadata with xml tags
 * @param datatabdel whether to print metadata as plain text - note that this takes precedence over <b>withxmltags</b>
 *        parameter
 * @return the string representing the content of the text element
 */
std::string TextElement::PrintElementContentAsString(bool withxmltags, bool datatabdel) {
    std::string ret("");
    if (datatabdel) {
        std::stringstream ss1;
        ss1 << m_pageno + 1;
        ret += ss1.str() + "\t";
        std::stringstream ss2;
        ss2 << int(m_X + 0.5);
        ret += ss2.str() + "\t";
        std::stringstream ss3;
        ss3 << int(m_Y + 0.5);
        ret += ss3.str() + "\t";
        ret += m_myfi.getFontName() + "\t";
        std::stringstream ss4;
        ss4 << m_fontsize;
        ret += ss4.str() + "\t";
    } else if (withxmltags) {
        ret += "<text";
        std::stringstream ss1;
        ss1 << m_pageno + 1;
        ret += " page='" + ss1.str() + "'";
        std::stringstream ss2;
        ss2 << int(m_X + 0.5);
        ret += " x='" + ss2.str() + "'";
        std::stringstream ss3;
        ss3 << int(m_Y + 0.5);
        ret += " y='" + ss3.str() + "'";
        ret += " font='" + m_myfi.getFontName() + "'";
        std::stringstream ss4;
        ss4 << m_fontsize;
        ret += " fontsize='" + ss4.str() + "'";
    }
    std::string out = "";
    bool setcomma = false;
    if (m_myfi.IsBold()) {
        if (setcomma) out += ",";
        out += "bold";
        setcomma = true;
    }
    if (m_myfi.IsItalics()) {
        if (setcomma) out += ",";
        out += "italics";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Symbolic)) {
        if (setcomma) out += ",";
        out += "symbolic";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Nonsymbolic)) {
        if (setcomma) out += ",";
        out += "nonsymbolic";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Serif)) {
        if (setcomma) out += ",";
        out += "serif";
        setcomma = true;
    }
    if (m_myfi.IsFlagSet(Script)) {
        if (setcomma) out += ",";
        out += "script";
        setcomma = true;
    }
    if (m_myfi.IsGreek()) {
        if (setcomma) out += ",";
        out += "greek";
        setcomma = true;
    }
    if (m_myfi.AnyRelevantFlagSet()) {
        if (setcomma) out += ",";
        out += "other";
    }
    if (datatabdel) {
        ret += out + "\t";
    } else if (withxmltags) {
        if (out.compare("") != 0) {
            ret += " style='";
            ret += out;
            ret += "'";
        }
    }
    PoDoFo::pdf_long l = m_rString.GetLength();
    //std::string convstring = m_rString.GetStringUtf8();
    std::string convstring = utf8AsPrintableAscii(m_rString.GetStringUtf8());
    if (datatabdel) {
        std::stringstream ss1;
        ss1 << l;
        ret += ss1.str() + "\t";
        std::stringstream ss2;
        ss2 << int(m_stringwidth + 0.5);
        ret += ss2.str() + "\t";
        std::stringstream ss3;
        ss3 << int(m_nextXOff + 0.5);
        ret += ss3.str() + "\t";
        ret += convstring + "\n";
    } else if (withxmltags) {
        if (l > 0) {
            std::stringstream ss1;
            ss1 << l;
            ret += " length='" + l + std::string("'");
        }
        std::stringstream ss2;
        ss2 << int(m_stringwidth + 0.5);
        ret += " stringwidth='" + ss2.str() + std::string("'");
        std::stringstream ss3;
        ss3 << int(m_nextXOff + 0.5);
        ret += " off='" + ss3.str() + std::string("'");
        ret += ">";
        ret += convstring;
    } else {
        ret += convstring;
    }
    if ((withxmltags) && (!datatabdel)) {

        ret += "</text>\n";
    }
    return ret;
}

bool TextElement::ElementPriority(TextElement * s) {
    int s_pageno = s->GetP();
    if (m_pageno < s_pageno)
        return true;
    if (m_pageno == s_pageno) {
        int imy = 10 * int(m_Y + 0.5);
        int isy = 10 * int(s->GetY() + 0.5);
        if (imy > isy)
            return true;
        else if (imy == isy) {
            if (m_X < s->GetX())

                return true;
        }
    }
    return false;
}

double TextElement::GeometricNegLogLikelihood(TextElement *s, double pl,
        double pw, double pb, double ph, double pxc, double pyc,
        bool linehasbridge) {
    // pl: page left
    // pw: page width
    // pb: page bottom
    // ph: page height
    // pxc: page x-center
    // pyc: page y-center

    double xn = s->GetX();
    double yn = s->GetY();

    const double xpenalty = xn;
    const double yreward = -2.1 * yn*pw;
    double twocolumnpenalty = 0.0;
    if (!linehasbridge)
        if (xn > pxc)
            if (m_X < pxc)
                twocolumnpenalty = 2.0 * yn * pw;

    return xpenalty + yreward + twocolumnpenalty;
}

bool TextElement::ElementIsInBox(PoDoFo::PdfArray box) {
    double xl = double(box[0].GetNumber());
    double yl = double(box[1].GetNumber());
    double xu = double(box[2].GetNumber());
    double yu = double(box[3].GetNumber());

    return (m_X >= xl) && (m_X < xu) && (m_Y >= yl) && (m_Y <= yu);
}

bool TextElement::ElementIsInRect(PoDoFo::PdfRect box) {
    double xl = double(box.GetLeft());
    double yl = double(box.GetBottom());
    double xu = double(box.GetLeft() + box.GetWidth());
    double yu = double(box.GetBottom() + box.GetHeight());
    double X = m_X + m_nextXOff / 2;
    double Y = m_Y + m_nextYOff / 2;

    return (X >= xl) && (X < xu) && (Y >= yl) && (Y <= yu);
}


void TextElement::AddToChainEnd(TextElement * const p) {
    TextElement * aux = GetNext();
    TextElement * previous = this;
    while (aux != NULL) {

        previous = aux;
        aux = aux->GetNext();
    }
    previous->SetNext(p);
}

TextElement * TextElement::InsertToSortedChain(TextElement * const p) {
    TextElement * aux = this;
    TextElement * prev = this;
    TextElement * first = this;
    while (aux != NULL) {
        if (!aux->ElementPriority(p)) break;
        prev = aux;
        aux = aux->GetNext();
    }
    p->SetNext(aux);
    if (prev != aux) {
        prev->SetNext(p);
    } else {

        first = p;
    }
    return first;
}

list<double> TextElement::CalculateTwoColumnBridges(double pxc) {
    list<double> final;
    TextElement * aux = this;
    while (aux != NULL) {
        double x = aux->GetX();
        double y = aux->GetY();
        double sw = aux->GetStringWidth();

        if ((x + sw > pxc) && (x < pxc))
            final.push_back(y);
        aux = aux->GetNext();
    }

    return final;
}

list<TextElement*> TextElement::SortChainGeometrically(double pl, double pw,
        double pb, double ph, double pxc, double pyc, list<double> bridges) {
    list<TextElement*> final;
    final.push_back(this);
    list<TextElement*> todo;
    TextElement * aux = GetNext();
    while (aux != NULL) {
        todo.push_back(aux);
        aux = aux->GetNext();
    }
    while (!todo.empty()) {
        double min = 9.9e250;
        bool linehasbridge = false;
        double y = final.back()->GetY();
        for (list<double>::iterator i = bridges.begin(); i != bridges.end(); i++) {
            if (abs(y - *i) < 5) linehasbridge = true;
            if (linehasbridge) break;
        }
        list<TextElement*>::iterator imin;
        for (list<TextElement*>::iterator i = todo.begin(); i != todo.end(); i++) {
            double x = final.back()->GeometricNegLogLikelihood(*i, pl, pw, pb, ph, pxc, pyc, linehasbridge);
            if (x < min) {

                min = x;
                imin = i;
            }
        }
        final.push_back(*imin);
        todo.erase(imin);
    }
    return final;
}

/*!
 * print the content of all the text blocks in the chain
 *
 * @param sout the output stream where the text must be written
 * @param withxmltags whether to print metadata with xml tags
 * @param datatabdel whether to print metadata as plain text - note that this takes precedence over <b>withxmltags</b>
 *        parameter
 */
void TextElement::PrintChainContent(std::ostream & sout, bool withxmltags, bool datatabdel) {
    TextElement * aux = this;
    while (aux != NULL) {

        aux->PrintElementContent(sout, withxmltags, datatabdel);
        aux = aux->GetNext();
    }
}

void TextElement::PrintChainSpaced(std::ostream & sout, bool datatabdel) {
    TextElement * aux = this;
    long sumnoc = 0;
    long sumnowc = 0;
    double sumstringwidth = 0.0;
    while (aux != NULL) {
        sumnoc += aux->GetNumberOfCharacters();
        sumnowc += aux->GetNumberOfWhiteCharacters();
        sumstringwidth += aux->GetStringWidth();
        aux = aux->GetNext();
    }
    double ave = sumstringwidth / double(sumnoc - sumnowc);
    // gap must be bigger than a fraction of the average width of a character
    double threshold = ave / 3.0;
    aux = this;
    while (aux != NULL) {
        double elementslength = aux->GetNextXOff();
        double stringslength = aux->GetStringWidth();
        double gap = elementslength - stringslength;

        if ((gap > threshold) && !(sumnowc)) sout << " ";
        aux->PrintElementContent(sout, false, datatabdel);
        aux = aux->GetNext();
    }
}

std::string TextElement::PrintChainSpacedAsString(bool datatabdel) {
    std::string ret("");
    TextElement * aux = this;
    long sumnoc = 0;
    long sumnowc = 0;
    double sumstringwidth = 0.0;
    while (aux != NULL) {
        sumnoc += aux->GetNumberOfCharacters();
        sumnowc += aux->GetNumberOfWhiteCharacters();
        sumstringwidth += aux->GetStringWidth();
        aux = aux->GetNext();
    }
    double ave = sumstringwidth / double(sumnoc - sumnowc);
    // gap must be bigger than a fraction of the average width of a character
    double threshold = ave / 3.0;
    aux = this;
    while (aux != NULL) {
        double elementslength = aux->GetNextXOff();
        double stringslength = aux->GetStringWidth();
        double gap = elementslength - stringslength;

        if ((gap > threshold) && !(sumnowc)) ret += " ";
        ret += aux->PrintElementContentAsString(false, datatabdel);
        aux = aux->GetNext();
    }
    return ret;
}

void TextElement::DeleteChain() {
    TextElement * aux = this;
    aux = aux->GetNext();
    while (aux != NULL) {
        TextElement * todelete = aux;
        aux = aux->GetNext();
        delete todelete;
    }
}

