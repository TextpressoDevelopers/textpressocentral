/* 
 * File:   ReadXml2Stream.cpp
 * Author: mueller
 * 
 * Created on April 29, 2013, 10:28 AM
 */

#include "ReadXml2Stream.h"
#include <iostream>

const char * node_types[] = {
    "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
};

void ReadXml2Stream::TraverseTree(pugi::xml_node pnode) {
    for (pugi::xml_node_iterator it = pnode.begin(); it != pnode.end(); ++it) {
        if (it->type() == pugi::node_element) {
            sout_ << "<" << it->name();
            if (it->attributes_begin() != it->attributes_end()) {
                for (pugi::xml_attribute_iterator ait = it->attributes_begin();
                        ait != it->attributes_end(); ++ait) {
                    sout_ << " " << ait->name() << "=\"" << ait->value() << "\"";
                }
            }
            sout_ << ">";
        } else if (it->type() != pugi::node_pcdata) {
            std::cerr << "There was a node of type ";
            std::cerr << node_types[it->type()];
            std::cerr << " in the XML file." << std::endl;
        }
        sout_ << it->value();
        TraverseTree(*it);
        if (it->type() == pugi::node_element)
            sout_ << "</" << it->name() << ">";
    }
}

ReadXml2Stream::ReadXml2Stream(const pugi::char_t * pszSource) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(pszSource);
    if (result) {
        //std::cerr << "XML [" << pszSource << "] parsed without errors.\n\n";
        doc.print(sout_);
//        doc.print(std::cerr);
    } else {
        std::cerr << "XML [" << pszSource << "] parsed with errors.\n";
        std::cerr << "Error description: " << result.description() << "\n";
        std::cerr << "Error offset: " << result.offset << ".\n\n";
    }
}

ReadXml2Stream::ReadXml2Stream() {
}

ReadXml2Stream::ReadXml2Stream(const ReadXml2Stream & orig) {
}

ReadXml2Stream::~ReadXml2Stream() {
}

