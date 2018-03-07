// Global file containing all global definitions.

#ifndef TEXTPRESSOCENTRALGLOBALS_H
#define TEXTPRESSOCENTRALGLOBALS_H


// Are these definitions really global? Otherwise move them back to their local project. 

#include "TextpressoCentralGlobalDefinitions.h"

#include <uima/api.hpp>

// If a composite delimiter exists, then there cannot be another delimiter
// that is a subset of that composite token delimiter. Decompose it accordingly.
// This applies to token and sentence delimiter
UnicodeString G_initT[] = {
    " ", "\n", "\t", "'", "\"",
    "/", "—", "(", ")", "[",
    "]", "{", "}", ":", ". ",
    "; ", ", ", "! ", "? "
};

const int G_initT_No = 19;
UnicodeString G_initS[] = {
    ".\n", "!\n", "?\n", ". ", "! ", "? ",
    ".\t", "!\t", "?\t", ".<", "!<", "?<"
};
const int G_initS_No = 12;
UnicodeString G_initP[] = {"<_pdf _image", "<_pdf _sbr", "<_pdf _hbr",
    "<_pdf _fsc", "<_pdf _fnc", "<_pdf _ydiff", "<_pdf _cr", "<_pdf _page"};
const int G_initP_No = 8;
const std::string ServerNames[] = {"http://goldturtle.caltech.edu/cgi-bin/ReceivePost.cgi",
    "http://go-genkisugi.rhcloud.com/capella", "http://localhost/cgi-bin/ReceivePost.cgi"};
const int ServerNames_No = 3;

//const std::string G_pdftagstart("<_pdf ");
//const std::string G_pdftagend("/>");
const UnicodeString usG_pdftagstart("<_pdf ");
const UnicodeString usG_pdftagend("/>");

#endif
