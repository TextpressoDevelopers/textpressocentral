/* 
 * File:   ElementCluster.cpp
 * Author: mueller
 * 
 * Created on January 28, 2013, 12:10 PM
 */

#include "ElementCluster.h"

ElementCluster::ElementCluster() {
    m_hierarchy = 0;
    m_stringwidth = 0.0;
    m_numberofcharacters = 0;
    m_next = NULL;
}

ElementCluster::ElementCluster(const ElementCluster& orig) {
    m_next = NULL;
}

ElementCluster::~ElementCluster() {
    if (m_list != NULL) {
        m_list->DeleteChain();
        delete m_list;
    }
}

void ElementCluster::AddToChainEnd(ElementCluster * const p) {
    ElementCluster * aux = GetNext();
    ElementCluster * previous = this;
    while (aux != NULL) {
        previous = aux;
        aux = aux->GetNext();
    }
    previous->SetNext(p);
}

void ElementCluster::FindClusters(TextElement * const p) {
    ElementCluster * CurrentCluster = this;
    TextElement * aux = p;
    int count = 0;
    if (aux != NULL) {
        TextElement * TEaddition = new TextElement(aux);
        CurrentCluster->SetList(TEaddition);
        double x = aux->GetX();
        double y = aux->GetY();
        double xlast = x + aux->GetNextXOff();
        double ylast = y + aux->GetNextYOff();
        CurrentCluster->SetHierarchy(++count);
        CurrentCluster->SetStartX(x);
        CurrentCluster->SetStartY(y);
        CurrentCluster->SetFontSize(aux->GetFontSize());
        CurrentCluster->SetFontName(aux->GetFontName());
        CurrentCluster->SetStringWidth(aux->GetStringWidth());
        CurrentCluster->SetNumberOfCharacters(aux->GetNumberOfCharacters());
        aux = aux->GetNext();
        while (aux != NULL) {
            x = aux->GetX();
            y = aux->GetY();
            TextElement * TEaddition = new TextElement(aux);
            if ((xlast == x) && (ylast == y)) {
                std::string s1 = aux->GetFontName();
                if (CurrentCluster->GetFontName().compare(s1) != 0)
                    CurrentCluster->SetFontName("Various");
                double d1 = aux->GetFontSize();
                if (CurrentCluster->GetFontSize() != d1)
                    CurrentCluster->SetFontSize(0.0);
                TextElement * teaux = CurrentCluster->GetList();
                teaux->AddToChainEnd(TEaddition);
                CurrentCluster->AddToStringWidth(TEaddition->GetStringWidth());
                CurrentCluster->AddToNumberOfCharacters(TEaddition->GetNumberOfCharacters());
            } else {
                CurrentCluster->SetEndX(xlast);
                CurrentCluster->SetEndY(ylast);
                ElementCluster * ECaddition = new ElementCluster;
                this->AddToChainEnd(ECaddition);
                CurrentCluster = ECaddition;
                CurrentCluster->SetList(TEaddition);
                CurrentCluster->SetHierarchy(++count);
                CurrentCluster->SetStartX(x);
                CurrentCluster->SetStartY(y);
                CurrentCluster->SetFontSize(aux->GetFontSize());
                CurrentCluster->SetFontName(aux->GetFontName());
                CurrentCluster->SetStringWidth(aux->GetStringWidth());
                CurrentCluster->SetNumberOfCharacters(aux->GetNumberOfCharacters());
            }
            xlast = x + aux->GetNextXOff();
            ylast = y + aux->GetNextYOff();
            aux = aux->GetNext();
        }
        CurrentCluster->SetEndX(xlast);
        CurrentCluster->SetEndY(ylast);
    }
    p->DeleteChain();
}

void ElementCluster::PrintClusterProperties(std::ostream& sout, bool datatabdel) {
    if (datatabdel) {
        sout << GetHierarchy() << "\t";
        sout << GetStartX() << "\t";
        sout << GetStartY() << "\t";
        sout << GetEndX() << "\t";
        sout << GetEndY() << "\t";
        sout << GetFontName() << "\t";
        sout << GetFontSize() << "\t";
        sout << GetNumberOfCharacters() << "\t";
        sout << GetStringWidth() << "\t";
    } else {
        sout << "Hierarchy: " << GetHierarchy() << std::endl;
        sout << "Start: (" << GetStartX() << ",";
        sout << GetStartY() << ")" << std::endl;
        sout << "End: (" << GetEndX() << ",";
        sout << GetEndY() << ")" << std::endl;
        sout << "Fontname: " << GetFontName() << std::endl;
        sout << "Fontsize: " << GetFontSize() << std::endl;
        sout << "Number of Characters: " << GetNumberOfCharacters() << std::endl;
        sout << "Stringwidth: " << GetStringWidth() << std::endl;
        sout << "Elements:" << std::endl;
    }
}

void ElementCluster::PrintChainContent(std::ostream & sout, bool withxmltags, bool datatabdel) {
    ElementCluster * aux = this;
    while (aux != NULL) {
        aux->PrintClusterProperties(sout, datatabdel);
        TextElement * list = aux->GetList();
        list->PrintChainContent(sout, withxmltags, datatabdel);
        aux = aux->GetNext();
    }
}

void ElementCluster::PrintChainSpaced(std::ostream & sout, bool datatabdel) {
    ElementCluster * aux = this;
    while (aux != NULL) {
        aux->PrintClusterProperties(sout, datatabdel);
        TextElement * list = aux->GetList();
        list->PrintChainSpaced(sout, datatabdel);
        if (datatabdel) {
            sout << "\n\n";
        } else {
            sout << std::endl;
            sout << std::endl;
        }
        aux = aux->GetNext();
    }
}

void ElementCluster::PrintStreamSpaced(std::ostream & sout, bool tags) {
    ElementCluster * aux = this;
    if (aux != NULL) {
        double xeold = aux->GetEndX();
        double ysold = aux->GetStartY();
        double ydiffold = 0.0;
        int hold = aux->GetHierarchy();
        double fontsizeold = aux->GetFontSize();
        std::string fontnameold = aux->GetFontName();
        TextElement * list = aux->GetList();
        list->PrintChainSpaced(sout, false);
        aux = aux->GetNext();
        while (aux != NULL) {
            double xs = aux->GetStartX();
            double ys = aux->GetStartY();
            int h = aux->GetHierarchy();
            double fontsize = aux->GetFontSize();
            std::string fontname = aux->GetFontName();
            double ydiff = abs(ysold - ys);
            double gap = xs - xeold;
            double ave = aux->GetStringWidth() / aux->GetNumberOfCharacters();
//            sout << "<gap/ave = " << gap << "/"<< ave << ">";
            bool gapdecision = (gap < -7.0*ave); // x cursor moves backwards a lot
            bool pdfydiff = true;
            if (gap > ave/3.0) {
                sout << " ";
            } else if (gapdecision) {
                sout << " <_pdf _cr/> ";
                sout << std::endl;
                pdfydiff = false;
            }
            if (tags) {
                if (ydiff > 1.5*ydiffold) {
                    sout << std::endl;
                    if (gapdecision)
                    sout << " <_pdf _sbr/> " << std::endl;
                } else if ((ydiff > 0) && pdfydiff) {
                    sout << " <_pdf _ydiff=";
                    sout << ((ys - ysold > 0) ? "+" : "-");
                    sout << int(ydiff) + 1 << "/> ";
                }
                if (h < hold)
                    sout << std::endl << " <_pdf _hbr/> " << std::endl;
                if (fontsize != fontsizeold) {
                    sout << " <_pdf _fsc=";
                    if (fontsize - fontsizeold > 0) sout << "+";
                    sout << int (fontsize - fontsizeold + 0.5) << "/> ";
                }
                if (fontname.compare(fontnameold) != 0)
                    sout << " <_pdf _fnc=" + fontname + "/> " ;
            }
            TextElement * list = aux->GetList();
            list->PrintChainSpaced(sout, false);
            xeold = aux->GetEndX();
            ysold = ys;
            if ((ydiff > 0.0) && gapdecision)
                ydiffold = ydiff;
            hold = h;
            fontsizeold = fontsize;
            fontnameold = fontname;
            aux = aux->GetNext();
        }
    }
}

std::string ElementCluster::PrintStringSpaced(bool tags) {
    std::string ret("");
    ElementCluster * aux = this;
    if (aux != NULL) {
        double xeold = aux->GetEndX();
        double ysold = aux->GetStartY();
        double ydiffold = 0.0;
        int hold = aux->GetHierarchy();
        double fontsizeold = aux->GetFontSize();
        std::string fontnameold = aux->GetFontName();
        TextElement * list = aux->GetList();
        ret += list->PrintChainSpacedAsString(false);
        aux = aux->GetNext();
        while (aux != NULL) {
            double xs = aux->GetStartX();
            double ys = aux->GetStartY();
            int h = aux->GetHierarchy();
            double fontsize = aux->GetFontSize();
            std::string fontname = aux->GetFontName();
            double ydiff = abs(ysold - ys);
            double gap = xs - xeold;
            double ave = aux->GetStringWidth() / aux->GetNumberOfCharacters();
//            sout << "<gap/ave = " << gap << "/"<< ave << ">";
            bool gapdecision = (gap < -7.0*ave); // x cursor moves backwards a lot
            bool pdfydiff = true;
            if (gap > ave/3.0) {
                ret += " ";
            } else if (gapdecision) {
                ret += " <_pdf _cr/> \n";
                pdfydiff = false;
            }
            if (tags) {
                if (ydiff > 1.5*ydiffold) {
                    ret += "\n";
                    if (gapdecision)
                    ret += " <_pdf _sbr/> \n";
                } else if ((ydiff > 0) && pdfydiff) {
                    ret += " <_pdf _ydiff=";
                    ret += ((ys - ysold > 0) ? "+" : "-");
                    std::stringstream ss;
                    ss <<int(ydiff) + 1;
                    ret += ss.str() + "/> ";
                }
                if (h < hold)
                    ret += "\n <_pdf _hbr/> \n";
                if (fontsize != fontsizeold) {
                    ret += " <_pdf _fsc=";
                    if (fontsize - fontsizeold > 0) ret += "+";
                    std::stringstream ss;
                    ss << int (fontsize - fontsizeold + 0.5);
                    ret += ss.str() + "/> ";
                }
                if (fontname.compare(fontnameold) != 0)
                    ret += " <_pdf _fnc=" + fontname + "/> " ;
            }
            TextElement * list = aux->GetList();
            ret += list->PrintChainSpacedAsString(false);
            xeold = aux->GetEndX();
            ysold = ys;
            if ((ydiff > 0.0) && gapdecision)
                ydiffold = ydiff;
            hold = h;
            fontsizeold = fontsize;
            fontnameold = fontname;
            aux = aux->GetNext();
        }
    }
    return ret;
}

void ElementCluster::DeleteChain() {
    ElementCluster * aux = this;
    while (aux != NULL) {
        ElementCluster * todelete = aux;
        aux = aux->GetNext();
        delete todelete;
    }
}
