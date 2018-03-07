/* 
 * File:   PdfInfo.cpp
 * Author: mueller
 * 
 * Created on January 28, 2013, 11:30 AM
 */

#include "PdfInfo.h"
#include <stack>
#include <iostream>
#include <cstdio>
#include <boost/filesystem.hpp>
#include <CImg.h>

PdfInfo::PdfInfo(const std::string& inPathname, const std::string outFilenameRoot) {
    mDoc = new PoDoFo::PdfMemDocument(inPathname.c_str());
    auxfilenameroot = outFilenameRoot;
    auxfilenamecounter = 0;
}

PdfInfo::~PdfInfo() {
    if (mDoc != NULL) {
        delete mDoc;
        mDoc = NULL;
    }
}

/*!
 * Process the stream of a single page
 *
 * The stream is read one token at a time. A token can be a pdf \a keyword (BEGIN- or END-like tokens - such as
 * <b>BT</b> and <b>ET</b> for text - to delimit objects or other context dependent keywords to specify properties) or
 * \a variants (usually values associated to a keyword to specify some properties). In this way, objects are
 * recognized through the presence of BEGIN and END keyworkds. All the tokens within an object are first saved in
 * a temporary data structure and then processed when the END keywork for the current object or other internal keywords
 * which delimit different sub-objects are found. For each object and sub-object (usually text and images) a chain of
 * TextElement objects is created and the pointer to the first element of the chain is returned
 *
 * See <a href="http://wwwimages.adobe.com/content/dam/Adobe/en/devnet/pdf/pdfs/PDF32000_2008.pdf">Pdf reference</a>
 * for further information on Pdf structure:
 *
 *
 * @param pg the number of the page of the pdf to process
 * @return a pointer to the first element of the generated chain data structure
 */
TextElement * PdfInfo::ProcessStreamFromPage(int pg) {
    TextElement * rtp = NULL;

    // start state variables, global w.r.t. page.
    bool stateBt = false;
    PdfMyFontInfo myfi;
    double stateFontSize = 0.0;
    double statePosX = 0.0;
    double statePosY = 0.0;
    double StartofLineX = 0.0;
    double StartofLineY = 0.0;

    // remember to distinguish between user space and text space;
    // in text space is measured in unscaled text space units;
    // see also 9.4.4 in PDF manual.

    // introduce variables keeping track
    // of the beginning of current text line!

    double sCharSpace;
    double sWordSpace;
    double sHScale;
    double sVScale;

    double sLeading;
    // superscripted or subscripted
    double sRise;
    double sFontSize;

    // process only the selected page
    PoDoFo::PdfPage * curPage = mDoc->GetPage(pg);
    PoDoFo::PdfRect rect = curPage->GetMediaBox();
    double totalarea = rect.GetWidth() * rect.GetHeight();
    const char * pszToken = NULL;
    PoDoFo::PdfVariant var;
    PoDoFo::EPdfContentsType eType;
    // the tokenizer is used to get object streams from the pdf page
    PoDoFo::PdfContentsTokenizer tokenizer(curPage);
    // the stack is used to store variants associated to the current keyword, until the keyword itself is found
    std::stack<PoDoFo::PdfVariant> stack;
    while (tokenizer.ReadNext(eType, pszToken, var)) {
        if (eType == PoDoFo::ePdfContentsType_Keyword) {
            if (strcmp(pszToken, "BT") == 0) {
                // BT = Begin Text
                // This keyword is used to mark the beginning of text objects in the pdf
                stateBt = true;
                sCharSpace = 0.0;
                sWordSpace = 0.0;
                sHScale = 1.0;
                sVScale = 1.0;
                sLeading = 0.0;
                sRise = 0.0;
                sFontSize = 1.0;
            } else if (strcmp(pszToken, "ET") == 0) {
                // ET = End Text
                // This keywork is used to mark the end of text objects in the pdf
                stateBt = false;
            } else if (strcmp(pszToken, "Do") == 0) {
                // Do = paint external obj or image on the page

                // extract the image file and save it to a file on the local filesystem, then place a pointer to the
                // image in the text chain through a TextElement object
                std::string imagefilename;
                PoDoFo::PdfName ImageName = stack.top().GetName();
                PoDoFo::PdfObject * pImage
                        = curPage->
                        GetFromResources(PoDoFo::PdfName("XObject"), ImageName);
                PoDoFo::PdfObject* pObjSubType = pImage->
                        GetDictionary().GetKey(PoDoFo::PdfName::KeySubtype);
                if (pObjSubType && pObjSubType->IsName()
                        && (pObjSubType->GetName().GetName() == "Image")) {
                    PoDoFo::PdfObject * pFilter = pImage->
                            GetDictionary().GetKey(PoDoFo::PdfName::KeyFilter);
                    if (pFilter && pFilter->IsArray()
                            && pFilter->GetArray().GetSize() == 1
                            && pFilter->GetArray()[0].IsName()
                            && (pFilter->GetArray()[0].GetName().GetName()
                            == "DCTDecode"))
                        pFilter = &pFilter->GetArray()[0];
                    bool bjpeg = (pFilter && pFilter->IsName()
                            && (pFilter->GetName().GetName() == "DCTDecode"));
                    std::string ext = bjpeg ? ".jpg" : ".ppm";
                    boost::filesystem::path o;
                    do {
                        stringstream s;
                        s << ++auxfilenamecounter;
                        imagefilename = auxfilenameroot;
                        imagefilename.append(".");
                        imagefilename.append(s.str());
                        imagefilename.append(ext);
                        o = boost::filesystem::path(imagefilename);
                    } while (boost::filesystem::exists(o));
                    FILE * hFile = NULL;
                    hFile = fopen(o.c_str(), "wb");
                    if (!hFile) {
                        PODOFO_RAISE_ERROR(PoDoFo::ePdfError_InvalidHandle);
                    }
                    if (bjpeg) {
                        PoDoFo::PdfMemStream* pStream
                                = dynamic_cast<PoDoFo::PdfMemStream*>(pImage->GetStream());
                        fwrite(pStream->Get(), pStream->GetLength(), sizeof (char), hFile);
                    } else {
                        // long lBitsPerComponent = pObject->GetDictionary().GetKey( PdfName("BitsPerComponent" ) )->GetNumber();
                        // TODO: Handle colorspaces
                        // Create a ppm image
                        const char * pszPpmHeader = "P6\n# Image extracted by Textpresso\n%li %li\n%li\n";
                        fprintf(hFile, pszPpmHeader,
                                pImage->GetDictionary().GetKey(PoDoFo::PdfName("Width"))->GetNumber(),
                                pImage->GetDictionary().GetKey(PoDoFo::PdfName("Height"))->GetNumber(),
                                255);
                        char * pBuffer;
                        PoDoFo::pdf_long lLen;
                        pImage->GetStream()->GetFilteredCopy(&pBuffer, &lLen);
                        fwrite(pBuffer, lLen, sizeof (char), hFile);
                        free(pBuffer);
                    }
                    fclose(hFile);
                    // convert ppm files to jpeg and remove original ones
                    if (!bjpeg) {
                        cimg_library::CImg<unsigned char> image(o.c_str());
                        image.save((auxfilenameroot + "." + to_string(auxfilenamecounter) + ".jpg").c_str());
                        boost::filesystem::remove(o);
                        imagefilename = auxfilenameroot + "." + to_string(auxfilenamecounter) + ".jpg";
                    }
                    // add a TextElement object to the chain with a pointer to the filename of the image
                    stack.pop();
                    PoDoFo::PdfVariant objName = stack.top();
                    double y = objName.GetReal();
                    stack.pop();
                    double x = stack.top().GetReal();
                    std::string ifn = " <_pdf _image=" + imagefilename + "/> ";
                    PoDoFo::PdfString auxs(ifn);
                    TextElement * pCurTE = new TextElement;
                    pCurTE->SetElementContent(x, y, 0.0, 0.0,
                            pg, stateFontSize, myfi, sFontSize*sHScale, auxs);
                    if (rtp != NULL) {
                        rtp->AddToChainEnd(pCurTE);
                    } else {
                        rtp = pCurTE;
                    }
                }
            }
            // inside a Text block - the following code should be placed in a separate function
            if (stateBt) {
                if (strcmp(pszToken, "Tm") == 0) {
                    //Tm = Text matrix - for positioning
                    StartofLineY = stack.top().GetReal();
                    statePosY = StartofLineY;
                    stack.pop();
                    StartofLineX = stack.top().GetReal();
                    statePosX = StartofLineX;
                    stack.pop();
                    sVScale = stack.top().GetReal();
                    stack.pop();
                    stack.pop();
                    stack.pop();
                    sHScale = stack.top().GetReal();
                    stack.pop();
                    stateFontSize =
                            int(1.0e5 * sFontSize * sHScale * sVScale
                            / totalarea);
                } else if (strcmp(pszToken, "Td") == 0) {
                    // Td = position in the current user coordinate system where the text must be put
                    // 1st param = x coord
                    // 2nd param = y coord
                    StartofLineY +=
                            stack.top().GetReal() * sFontSize*sVScale;
                    statePosY = StartofLineY;
                    stack.pop();
                    StartofLineX +=
                            stack.top().GetReal() * sFontSize*sHScale;
                    statePosX = StartofLineX;
                    stack.pop();
                } else if (strcmp(pszToken, "TD") == 0) {
                    // TD = Move to the start of the next line, offset from the start of the current line by (tx, ty)
                    sLeading = -stack.top().GetReal();
                    StartofLineY -= sLeading*sFontSize*sVScale;
                    statePosY = StartofLineY;
                    stack.pop();
                    StartofLineX +=
                            stack.top().GetReal()*sFontSize*sHScale;
                    statePosX = StartofLineX;
                    stack.pop();
                } else if (strcmp(pszToken, "Tf") == 0) {
                    // Tf = Text font
                    // 1st param = font name
                    // 2nd param = font size - top of the stack
                    sFontSize = stack.top().GetReal();
                    stateFontSize =
                            int(1.0e5 * sFontSize * sHScale * sVScale
                            / totalarea);
                    stack.pop();
                    PoDoFo::PdfName fontName = stack.top().GetName();
                    PoDoFo::PdfObject * pFont = NULL;
                    // font info is stored in a separate object in the pdf file
                    pFont = curPage->GetFromResources(PoDoFo::PdfName("Font"),
                            fontName);
                    myfi.set(pFont, mDoc);
                    stack.pop();
                    if (pFont == NULL) {
                        PODOFO_RAISE_ERROR_INFO(PoDoFo::ePdfError_InvalidHandle,
                                "Cannot create font!");
                    }
                } else if (strcmp(pszToken, "Tc") == 0) {
                    // character spacing
                    sCharSpace = stack.top().GetReal();
                    stack.pop();
                } else if (strcmp(pszToken, "Tw") == 0) {
                    // word spacing
                    sWordSpace = stack.top().GetReal();
                    stack.pop();
                } else if (strcmp(pszToken, "Tz") == 0) {
                    // set the horizontal scaling
                    sHScale = stack.top().GetReal();
                    stack.pop();
                } else if (strcmp(pszToken, "TL") == 0) {
                    sLeading = stack.top().GetReal();
                    stack.pop();
                } else if (strcmp(pszToken, "Ts") == 0) {
                    sRise = stack.top().GetReal();
                    stack.pop();
                } else if (strcmp(pszToken, "T*") == 0) {
                    StartofLineY -= sLeading*sFontSize*sVScale;
                    statePosY = StartofLineY;
                    statePosX = StartofLineX;
                } else if (strcmp(pszToken, "\'") == 0) {
                    if (stack.top().IsString()) {
                        StartofLineY -= sLeading * sFontSize*sVScale;
                        statePosY = StartofLineY;
                        PoDoFo::PdfString auxs = stack.top().GetString();
                        double nX = AddToXY(myfi.GetStringWidth(auxs), 0.0,
                                sFontSize, sCharSpace, sWordSpace, sHScale);
                        double nY = AddToXY(0.0, 0.0, sFontSize,
                                sCharSpace, sWordSpace, 1.0);
                        TextElement * pCurTE = new TextElement;
                        pCurTE->SetElementContent(statePosX, statePosY, nX,
                                nY, pg, stateFontSize, myfi,
                                sFontSize*sHScale, auxs);
                        if (rtp != NULL) {
                            rtp->AddToChainEnd(pCurTE);
                        } else {
                            rtp = pCurTE;
                        }
                        statePosX += nX;
                        statePosY += nY;
                    }
                    stack.pop();
                } else if (strcmp(pszToken, "\"") == 0) {
                    if (stack.top().IsString()) {
                        const PoDoFo::PdfString s = stack.top().GetString();
                        stack.pop();
                        double c = stack.top().GetReal();
                        stack.pop();
                        double w = stack.top().GetReal();
                        sWordSpace = w;
                        sCharSpace = c;
                        StartofLineY -= sLeading * sFontSize*sVScale;
                        statePosY = StartofLineY;
                        double nX = AddToXY(myfi.GetStringWidth(s), 0.0,
                                sFontSize, sCharSpace, sWordSpace, sHScale);
                        double nY = AddToXY(0.0, 0.0, sFontSize,
                                sCharSpace, sWordSpace, 1.0);
                        TextElement * pCurTE = new TextElement;
                        pCurTE->SetElementContent(statePosX, statePosY, nX,
                                nY, pg, stateFontSize, myfi,
                                sFontSize*sHScale, s);
                        if (rtp != NULL) {
                            rtp->AddToChainEnd(pCurTE);
                        } else {
                            rtp = pCurTE;
                        }
                        statePosX += nX;
                        statePosY += nY;
                    }
                    stack.pop();
                } else if (strcmp(pszToken, "Tj") == 0) {
                    // Tj = print a string operand
                    // 1st param = string to print
                    if (stack.top().IsString()) {
                        PoDoFo::PdfString auxs = stack.top().GetString();
                        double nX = AddToXY(myfi.GetStringWidth(auxs), 0.0,
                                sFontSize, sCharSpace, sWordSpace, sHScale);
                        double nY = AddToXY(0.0, 0.0, sFontSize,
                                sCharSpace, sWordSpace, 1.0);
                        TextElement * pCurTE = new TextElement;
                        pCurTE->SetElementContent(statePosX, statePosY, nX,
                                nY, pg, stateFontSize, myfi,
                                sFontSize*sHScale, auxs);
                        if (rtp != NULL) {
                            rtp->AddToChainEnd(pCurTE);
                        } else {
                            rtp = pCurTE;
                        }
                        statePosX += nX;
                        statePosY += nY;
                    }
                    stack.pop();
                } else if (strcmp(pszToken, "TJ") == 0) {
                    if (stack.top().IsArray()) {
                        const PoDoFo::PdfArray array = stack.top().GetArray();
                        double tjsum = 0.0;
                        for (int i = 0; i < int (array.GetSize()); i++) {
                            if (array[i].IsString()) {
                                PoDoFo::PdfString auxs = array[i].GetString();
                                double w = myfi.GetStringWidth(auxs);
                                double nX = AddToXY(w, tjsum, sFontSize,
                                        sCharSpace, sWordSpace, sHScale);
                                double nY = AddToXY(0.0, 0.0, sFontSize,
                                        sCharSpace, sWordSpace, 1.0);
                                TextElement * pCurTE = new TextElement;
                                pCurTE->SetElementContent(statePosX, statePosY,
                                        nX, nY, pg,
                                        stateFontSize, myfi,
                                        sFontSize*sHScale, auxs);
                                if (rtp != NULL) {
                                    rtp->AddToChainEnd(pCurTE);
                                } else {
                                    rtp = pCurTE;
                                }
                                statePosX += nX;
                                statePosY += nY;
                                tjsum = 0.0;
                            } else if (array[i].IsReal()) {
                                tjsum += double(array[i].GetReal());
                            } else if (array[i].IsNumber()) {
                                tjsum += double(array[i].GetNumber());
                            }
                        }
                    }
                    stack.pop();
                }
            }
        } else if (eType == PoDoFo::ePdfContentsType_Variant) {
            stack.push(var);
        } else {
            // Impossible; type must be keyword or variant
            std::cerr << "INTERNAL LOGIC ERROR!" << std::endl;
            std::cerr << "Type must be keyword or variant." << std::endl;
            //PODOFO_RAISE_ERROR(PoDoFo::ePdfError_InternalLogic);
        }
    }
    return rtp;
}

/*!
 * get the text from a pdf. For each page, create a PdfInfo chain content and print it to the provided output stream,
 * then deleting the chain and the PdfInfo object to free resources at each step
 *
 * @param sOutStream the output stream where to print the text
 * @param datatabdel whether to print metadata - plain text
 */
void PdfInfo::ExtractText(std::ostream & sOutStream, bool datatabdel) {
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        if (!datatabdel) {
            sOutStream << "                   " << std::endl;
            sOutStream << "** Text Elements **" << std::endl;
            sOutStream << "*******************" << std::endl;
        }
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            TextElement * tep = ProcessStreamFromPage(pg);
            tep->PrintChainContent(sOutStream, true, datatabdel);
            if (tep != NULL) {
                tep->DeleteChain();
                delete tep;
            }
        }
    }
}

void PdfInfo::ExtractGeometrically(std::ostream & sOutStream, bool datatabdel) {
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        if (!datatabdel) {
            sOutStream << "                   " << std::endl;
            sOutStream << "** Geometric Extraction **" << std::endl;
            sOutStream << "**************************" << std::endl;
        }
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            TextElement * tep = ProcessStreamFromPage(pg);
            if (tep != NULL) {
                PoDoFo::PdfPage * curPage = mDoc->GetPage(pg);
                PoDoFo::PdfRect rect = curPage->GetMediaBox();
                double pl = rect.GetLeft();
                double pw = rect.GetWidth();
                double pb = rect.GetBottom();
                double ph = rect.GetHeight();
                double pxc = pl + pw / 2.0;
                double pyc = pb + ph / 2.0;
                list<double> bridges = tep->CalculateTwoColumnBridges(pxc);
                list<TextElement*> l = tep->SortChainGeometrically(pl, pw, pb,
                        ph, pxc, pyc, bridges);
                PrintGeometricChain(sOutStream, l, true, datatabdel);
            }
        }
    }
}

void PdfInfo::ExtractUris(std::ostream & sOutStream, bool datatabdel) {
    TextElement * chain[MAXPAGES];
    for (int i = 0; i < MAXPAGES; i++)
        chain[i] = NULL;
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            chain[pg] = ProcessStreamFromPage(pg);
        }
        int sumannot = 0;
        for (int pg = 0; pg < pn; pg++) {
            PoDoFo::PdfPage * curPage = mDoc->GetPage(pg);
            sumannot += curPage->GetNumAnnots();
        }
        if (sumannot > 0) {
            if (!datatabdel) {
                sOutStream << "                 " << std::endl;
                sOutStream << "** " << sumannot << " Annotations **" << std::endl;
                sOutStream << std::endl;
            }
            for (int pg = 0; pg < pn; pg++) {
                PoDoFo::PdfPage * curPage = mDoc->GetPage(pg);
                int AnnotationCount = curPage->GetNumAnnots();
                for (int i = 0; i < AnnotationCount; i++) {
                    PoDoFo::PdfAnnotation * curAnn = curPage->GetAnnotation(i);
                    if (curAnn->HasAction()) {
                        if (curAnn->GetAction()->HasURI()) {
                            PoDoFo::PdfRect rect = curAnn->GetRect();
                            if (datatabdel) {
                                sOutStream << pg + 1 << "\t";
                                sOutStream <<
                                        curAnn->GetAction()->GetURI().GetStringUtf8();
                            } else {
                                sOutStream << "Page: " << pg + 1;
                                sOutStream << "\tURI: ";
                                sOutStream <<
                                        curAnn->GetAction()->GetURI().GetStringUtf8();
                            }
                            TextElement * auxc = chain[pg];
                            TextElement * boxchain = NULL;
                            while (auxc != NULL) {
                                if (auxc->ElementIsInRect(rect)) {
                                    TextElement * addition = new TextElement(auxc);
                                    if (boxchain != NULL) {
                                        boxchain = boxchain->InsertToSortedChain(addition);
                                    } else {
                                        boxchain = addition;
                                    }
                                }
                                auxc = auxc->GetNext();
                            }
                            if (datatabdel) {
                                sOutStream << "\t";
                            } else {
                                sOutStream << "\tLinked text: ";
                            }
                            if (boxchain != NULL) {
                                boxchain->PrintChainSpaced(sOutStream, false);
                                boxchain->DeleteChain();
                                delete boxchain;
                            }
                            sOutStream << std::endl;
                        }
                    }
                }
            }
        } else if (!datatabdel) {
            sOutStream << "No annotations found." << std::endl;
        }
    }
}

/*

void PdfInfo::PrintPotential(std::ostream & sOutStream) {
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        sOutStream << "                   " << std::endl;
        sOutStream << "** Potentials **" << std::endl;
        sOutStream << "****************" << std::endl;
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            sOutStream << "Page " << pg + 1 << std::endl;
            TextElement * tep = ProcessStreamFromPage(pg);
            PoDoFo::PdfPage * curPage = mDoc->GetPage(pg);
            PoDoFo::PdfRect rect = curPage->GetMediaBox();
            double x = rect.GetLeft() + rect.GetWidth();
            double y = rect.GetBottom() + rect.GetHeight();
            PdfGrid * pgrid = new PdfGrid(x, y);
            pgrid->SetPotentialFromChain(tep);
            pgrid->PrintPotential(sOutStream);
        }
    }
}

 */

void PdfInfo::PrintClusters(std::ostream & sOutStream, bool datatabdel) {
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        if (!datatabdel) {
            sOutStream << "                   " << std::endl;
            sOutStream << "** Clusters **" << std::endl;
            sOutStream << "**************" << std::endl;
        }
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            if (!datatabdel)
                sOutStream << "Page " << pg + 1 << std::endl;
            TextElement * tep = ProcessStreamFromPage(pg);
            if (tep != NULL) {
                ElementCluster * ecp = new ElementCluster;
                ecp->FindClusters(tep);
                ecp->PrintChainSpaced(sOutStream, datatabdel);
                if (!datatabdel)
                    sOutStream << std::endl;
            }
        }
    }
}

/*!
 * process all pages in the pdf file and print the content to an output stream
 *
 * @param sOutStream the output stream where to print the processed content of the pdf
 */
void PdfInfo::StreamAll(std::ostream& sOutStream) {
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            sOutStream << " <_pdf _page=" << pg + 1 << "/> ";
            TextElement * tep = ProcessStreamFromPage(pg);
            if (tep != NULL) {
                ElementCluster * ecp = new ElementCluster;
                ecp->FindClusters(tep);
                // at this point chain attached to tep is deleted.
                ecp->PrintStreamSpaced(sOutStream, 1);
                sOutStream << std::endl;
                ecp->DeleteChain();
                delete tep;
            }
        }
    }
}

std::string PdfInfo::StringAll() {
    std::string ret("");
    PoDoFo::PdfPagesTree * pagesObj = mDoc->GetPagesTree();
    if (pagesObj) {
        int pn = pagesObj->GetTotalNumberOfPages();
        for (int pg = 0; pg < pn; pg++) {
            std::stringstream ss;
            ss << pg + 1;
            ret  += " <_pdf _page=" + ss.str() + "/> ";
            TextElement * tep = ProcessStreamFromPage(pg);
            if (tep != NULL) {
                ElementCluster * ecp = new ElementCluster;
                ecp->FindClusters(tep);
                // at this point chain attached to tep is deleted.
                ret += ecp->PrintStringSpaced(1);
                ret += "\n";
                ecp->DeleteChain();
                delete tep;
            }
        }
    }
    return ret;
}

void PdfInfo::PrintGeometricChain(std::ostream & sOutStream, list<TextElement*> l, bool full, bool datatabdel) {
    for (list<TextElement*>::iterator i = l.begin(); i != l.end(); i++) {
        (*i)->PrintElementContent(sOutStream, full, datatabdel);
    }
}
