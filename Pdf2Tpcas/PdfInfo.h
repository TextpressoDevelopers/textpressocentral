/* 
 * File:   PdfInfo.h
 * Author: mueller
 *
 * Created on January 28, 2013, 11:30 AM
 */

#ifndef PDFINFO_H
#define	PDFINFO_H

#include <podofo/podofo.h>
#include "PdfMyFontInfo.h"
#include "TextElement.h"
#include "ElementCluster.h"

#define MAXPAGES 1000

class PdfInfo {
public:
    PdfInfo(const std::string & inPathname, const std::string outFilenameRoot);
    virtual ~PdfInfo();
    TextElement * ProcessStreamFromPage(int pg);
    void ExtractText(std::ostream & sOutStream, bool datatabdel);
    void ExtractGeometrically(std::ostream & sOutStream, bool datatabdel);
    void ExtractUris(std::ostream & sOutStream, bool datatabdel);
    void PrintPotential(std::ostream & sOutStream);
    void PrintClusters(std::ostream & sOutStream, bool datatabdel);
    void StreamAll(std::ostream & sOutStream);
    std::string StringAll();
    void PrintGeometricChain(std::ostream & sOutStream, list<TextElement*> l, bool full, bool datatabdel);
private:
    PoDoFo::PdfDocument* mDoc;
    std::string auxfilenameroot;
    int auxfilenamecounter;

    inline double AddToXY(double w, double tj, double tfs,
            double tc, double tw, double th);

};

inline double PdfInfo::AddToXY(double w, double tj, double tfs,
        double tc, double tw, double th) {
    return ((w - tj / 1000.0) * tfs + tc + tw)*th;
}

#endif	/* PDFINFO_H */
