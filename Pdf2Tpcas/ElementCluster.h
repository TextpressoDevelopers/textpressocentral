/* 
 * File:   ElementCluster.h
 * Author: mueller
 *
 * Created on January 28, 2013, 12:10 PM
 */

#ifndef ELEMENTCLUSTER_H
#define	ELEMENTCLUSTER_H

#include "TextElement.h"

#include <string>

class ElementCluster {
public:
    ElementCluster();
    ElementCluster(const ElementCluster& orig);
    int GetHierarchy () { return m_hierarchy; }
    double GetStartX() { return m_startx; }
    double GetStartY() { return m_starty; }
    double GetEndX() { return m_endx; }
    double GetEndY() { return m_endy; }
    double GetFontSize() { return m_fontsize; }
    std::string GetFontName() { return m_fontname; }
    double GetStringWidth() { return m_stringwidth; }
    long GetNumberOfCharacters() { return m_numberofcharacters; }
    void SetHierarchy (int a) { m_hierarchy = a; }
    void SetStartX(double a) { m_startx = a; }
    void SetStartY(double a) { m_starty = a; }
    void SetEndX(double a) { m_endx = a; }
    void SetEndY(double a) { m_endy = a; }
    void SetFontSize(double a) { m_fontsize = a; }
    void SetFontName(std::string a) { m_fontname = a; }
    void SetStringWidth(double a) { m_stringwidth = a; }
    void AddToStringWidth(double a) { m_stringwidth += a; }
    void SetNumberOfCharacters(long a) { m_numberofcharacters = a; }
    void AddToNumberOfCharacters(long a) { m_numberofcharacters += a; }
    ElementCluster * GetNext() { return m_next; }
    void SetNext(ElementCluster * p) { m_next = p; }
    TextElement * GetList() { return m_list; }
    void SetList(TextElement * p) { m_list = p; }
    void FindClusters(TextElement * const p);
    virtual ~ElementCluster();
    void AddToChainEnd(ElementCluster * const p);
    void PrintClusterProperties(std::ostream & sout, bool datatabdel);
    void PrintChainContent(std::ostream & sout, bool withxmltags, bool datatabdel);
    void PrintChainSpaced(std::ostream & sout, bool datatabdel);
    std::string PrintChainSpacedAsString(bool datatabdel);
    void PrintStreamSpaced(std::ostream & sout, bool tags);
    std::string PrintStringSpaced(bool tags);
    void DeleteChain();
private:
    int m_hierarchy;
    double m_startx;
    double m_starty;
    double m_endx;
    double m_endy;
    double m_fontsize;
    double m_stringwidth;
    long m_numberofcharacters;
    std::string m_fontname;
    TextElement * m_list;
    ElementCluster * m_next;
};

#endif	/* ELEMENTCLUSTER_H */

