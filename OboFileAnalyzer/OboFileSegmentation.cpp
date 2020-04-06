/* 
 * File:   OboFileSegmentation.cpp
 * Author: mueller
 * 
 * Created on October 14, 2013, 12:09 PM
 */

#include "OboFileSegmentation.h"
#include <fstream>
#include <set>
#include <list>

namespace {

    std::vector<std::string> FindTargetId(std::string data, std::string key,
            mms & keyphrases, mms & ignoredphrases) {
        std::vector<std::string> ret;
        int dl = data.length();
        std::pair<mms::iterator, mms::iterator> mmsii = keyphrases.equal_range(key);
        std::pair<mms::iterator, mms::iterator> mmsii_ignored = ignoredphrases.equal_range(key);
        mms::iterator mmsit, mmsit2;
        for (mmsit = mmsii.first; mmsit != mmsii.second; mmsit++) {
            std::string targetstring = (*mmsit).second;
            if (data.find(targetstring) == 0) {
                int tl = targetstring.length();
                tl = (tl > 0) ? tl + 1 : tl;
                if (dl > 0) {
                    std::string modifieddata = data.substr(tl, dl - tl + 1);
                    int iend = modifieddata.find_first_of(' ');
                    std::string targetid = modifieddata.substr(0, iend);
                    bool notignored = true;
                    for (mmsit2 = mmsii_ignored.first; mmsit2 != mmsii_ignored.second; mmsit2++)
                        if ((*mmsit2).second == targetid) notignored = false;
                    if (notignored) {
                        ret.push_back(targetid);
                    }
                }
            }
        }
        return ret;
    }
}

void OboFileSegmentation::Print2FilePcRelations(std::string fnroot, mms & pcr) {
    std::set<std::string> keys;
    mms::iterator mmsit;
    for (mmsit = pcr.begin(); mmsit != pcr.end(); mmsit++)
        keys.insert((*mmsit).first);
    std::set<std::string>::iterator ssit;
    for (ssit = keys.begin(); ssit != keys.end(); ssit++) {
        std::pair<mms::iterator, mms::iterator> mmsii = pcr.equal_range(*ssit);
        std::set<std::string> values;
        mms::iterator mmsit;
        for (mmsit = mmsii.first; mmsit != mmsii.second; mmsit++)
            values.insert((*mmsit).second);
        std::string fn = fnroot + "." + *ssit;
        std::ofstream f(fn.c_str());
        std::set<std::string>::iterator ssit2;
        for (ssit2 = values.begin(); ssit2 != values.end(); ssit2++)
            f << *ssit2 << std::endl;
        f.close();
        fn = fnroot + ".term." + *ssit;
        std::ofstream g(fn.c_str());
        for (ssit2 = values.begin(); ssit2 != values.end(); ssit2++) {
            OboEntry * oeptr = id2oboentryptr_[*ssit2];
            if (oeptr != NULL) {
                g << "[Term]" << std::endl;
                std::string sout = oeptr->PrintEntry2String();
                g << sout << std::endl;
            }
        }
        g.close();
    }
}

OboFileSegmentation::OboFileSegmentation(const char * filename) {
    std::ifstream f(filename);
    std::string in;
    std::string collectedlines;
    bool datarecording;
    while (getline(f, in)) {
        if (in.find("[Term]") != std::string::npos) {
            collectedlines.clear();
            datarecording = true;
        } else if (in.length() == 0) {
            if (datarecording) {
                datarecording = false;
                std::string saux = collectedlines.substr(0, collectedlines.length() - 1);
                OboEntry * oe = new OboEntry(saux);
                termset_.push_back(oe);
            }
        } else if (datarecording) {
            collectedlines += in + "\n";
        }
    }
    f.close();
    PopulateId2OboEntryPtr();
    InitializeKeyphrases();
    PopulateElementaryPcRelationships();
}

OboFileSegmentation::OboFileSegmentation(const OboFileSegmentation & orig) {
}

OboFileSegmentation::~OboFileSegmentation() {
    while (!termset_.empty()) {
        delete termset_.back();
        termset_.pop_back();
    }
}

void OboFileSegmentation::PopulateId2OboEntryPtr() {
    std::vector<OboEntry*>::iterator voeit;
    for (voeit = termset_.begin(); voeit != termset_.end(); voeit++) {
        // Should only be one ID. Always take first.
        if (!(((*voeit)->GetData("id")).empty())) {
            std::string id = ((*voeit)->GetData("id"))[0];
            id2oboentryptr_.insert(std::make_pair(id, *voeit));
        }
    }
}

std::vector<std::string> OboFileSegmentation::ReturnAllChildren(std::string parent) {
    std::vector<std::string> ret;
    std::pair<mms::iterator, mms::iterator> mmsii;
    mmsii = pcelementaryrelationships_.equal_range(parent);
    std::list<std::string> parenthood2bchecked;
    mms::iterator mmsit;
    for (mmsit = mmsii.first; mmsit != mmsii.second; mmsit++)
        parenthood2bchecked.push_front((*mmsit).second);
    while (!parenthood2bchecked.empty()) {
        std::string current = parenthood2bchecked.back();
        parenthood2bchecked.pop_back();
        ret.push_back(current);
        mmsii = pcelementaryrelationships_.equal_range(current);
        for (mmsit = mmsii.first; mmsit != mmsii.second; mmsit++)
            parenthood2bchecked.push_front((*mmsit).second);
    }
    return ret;
}

void OboFileSegmentation::CalculateAndPrintCompositePcRelationships(const char * filename) {
    std::string fnroot(filename);
    fnroot += ".all";
    mms allrelationships;
    mms::iterator mmsit;
    std::set<std::string> parents;
    for (mmsit = pcelementaryrelationships_.begin(); mmsit != pcelementaryrelationships_.end(); mmsit++)
        parents.insert((*mmsit).first);
    for (std::set<std::string>::iterator sit = parents.begin(); sit != parents.end(); sit++) {
        allrelationships.clear();
        std::vector<std::string> allchildren = ReturnAllChildren(*sit);
        while (!allchildren.empty()) {
            allrelationships.insert(std::make_pair(*sit, allchildren.back()));
            allchildren.pop_back();
        }
        Print2FilePcRelations(fnroot, allrelationships);
    }
}

void OboFileSegmentation::PopulateElementaryPcRelationships() {
    mms::iterator mmsit;
    std::set<std::string> cpkeys;
    for (mmsit = cpkeyphrases_.begin(); mmsit != cpkeyphrases_.end(); mmsit++)
        cpkeys.insert((*mmsit).first);
    std::set<std::string> pckeys;
    for (mmsit = pckeyphrases_.begin(); mmsit != pckeyphrases_.end(); mmsit++)
        pckeys.insert((*mmsit).first);
    std::vector<OboEntry*>::iterator voeit;
    for (voeit = termset_.begin(); voeit != termset_.end(); voeit++)
        if (!(((*voeit)->GetData("id")).empty())) {
            std::string id = ((*voeit)->GetData("id"))[0];
            std::set<std::string>::iterator kit;
            //
            for (kit = cpkeys.begin(); kit != cpkeys.end(); kit++) {
                std::vector<std::string> datav = (*voeit)->GetData(*kit);
                while (!datav.empty()) {
                    std::string data = datav.back();
                    std::vector<std::string> tgtid = FindTargetId(data, *kit, cpkeyphrases_, ignoredkeyphrases_);
                    while (!tgtid.empty()) {
                        pcelementaryrelationships_.insert(std::make_pair(tgtid.back(), id));
                        tgtid.pop_back();
                    }
                    datav.pop_back();
                }
            }
            for (kit = pckeys.begin(); kit != pckeys.end(); kit++) {
                std::vector<std::string> datav = (*voeit)->GetData(*kit);
                while (!datav.empty()) {
                    std::string data = datav.back();
                    std::vector<std::string> tgtid = FindTargetId(data, *kit, pckeyphrases_, ignoredkeyphrases_);
                    while (!tgtid.empty()) {
                        pcelementaryrelationships_.insert(std::make_pair(id, tgtid.back()));
                        tgtid.pop_back();
                    }
                    datav.pop_back();
                }
            }
        }
}

void OboFileSegmentation::PrintElementaryPcRelationships(const char * filename) {
    std::string fnroot(filename);
    fnroot += ".elem";
    Print2FilePcRelations(fnroot, pcelementaryrelationships_);
}

void OboFileSegmentation::PrintIdNames(const char * filename) {
    std::string fnroot(filename);
    fnroot += ".ids-names";
    std::ofstream f(fnroot.c_str());
    std::map<std::string, OboEntry*>::iterator it;
    for (it = id2oboentryptr_.begin(); it != id2oboentryptr_.end(); it++) {
        std::string id = it->first;
        OboEntry * oe = it->second;
        if (oe != NULL)
            f << id << "\t" << oe->GetName(id) << std::endl;
    }
    f.close();
}

void OboFileSegmentation::PrintSingleTerms(const char * filename) {
    std::string fnroot(filename);
    fnroot += ".single";
    std::map<std::string, OboEntry*>::iterator it;
    for (it = id2oboentryptr_.begin(); it != id2oboentryptr_.end(); it++) {
        std::string id = it->first;
        OboEntry * oe = it->second;
        std::string fn = fnroot + "." + id;
        std::ofstream f(fn.c_str());
        if (oe != NULL) {
            f << "[Term]" << std::endl;
            std::string sout = oe->PrintEntry2String();
            f << sout << std::endl;
        }
        f.close();
    }
}
    bool OboFileSegmentation::IsInSubset(std::string id, std::string subsetname) {
        return GetOePtr(id)->IsInSubset(subsetname);
    }

    OboEntry* OboFileSegmentation::GetOePtr(std::string id) {
        OboEntry * ret(nullptr);
        if (id2oboentryptr_.find(id) != id2oboentryptr_.end())
            ret = id2oboentryptr_[id];
        return ret;
    }

void OboFileSegmentation::InitializeKeyphrases() {
    // this is to catch phrases of the form (..., "") that are not target ids;
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "only_in_taxon"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "located_in"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "lacks_part"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_part"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_gene_template"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_component"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "derives_from"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "reciprocal_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "increased_in_magnitude_relative_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "decreased_in_magnitude_relative_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "transcribed_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "transcribed_from"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_quality"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_origin"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "contains"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "adjacent_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "part_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "occurs_in"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "positively_regulates"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "regulates"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "negatively_regulates"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "superficial_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "subdivision_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "sexually_homologous_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "proximally_connected_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "posterior_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_superficial_part_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_right_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_proximal_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_left_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_lateral_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_distal_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_deep_part_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_central_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_potential_to_develop_into"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_muscle_origin"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_muscle_insertion"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_fused_element"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "distally_connected_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "develops_from"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "developmentally_replaces"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "connects"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "composed_primarily_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "attached_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "anterior_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "connected_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "tributary_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "supplies"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "drains"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "directly_develops_from"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "branching_part_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "anastomoses_with"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "proximalmost_part_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_ventral_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_dorsal_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "continuous_with"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_anterior_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "intersects_midsagittal_plane_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "location_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_outermost_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "in_innermost_side_of"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "ventral_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "innervates"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "extends_fibers_into"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "dorsal_to"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "happens_during"));
    ignoredkeyphrases_.insert(std::make_pair("intersection_of", "has_relative_magnitude"));

    // child-parent
    cpkeyphrases_.insert(std::make_pair("is_a", ""));
    cpkeyphrases_.insert(std::make_pair("relationship", "part_of"));
    //cpkeyphrases_.insert(std::make_pair("intersection_of", ""));

    // parent-child
    //pckeyphrases_.insert(std::make_pair("relationship", "has_part"));

    /*
    is_obsolete: true
    is_a:
    relationship: adjacent_to
    relationship: contains
    relationship: correlates_with
    relationship: decreased_in_magnitude_relative_to
    relationship: derives_from
    relationship: guided_by
    relationship: has_component
    relationship: has_cross_section
    relationship: has_functional_parent
    relationship: has_modification
    relationship: has_origin
    relationship: has_parent_hydride
    relationship: has_part
    relationship: has_quality
    relationship: has_role
    relationship: increased_in_magnitude_relative_to
    relationship: is_conjugate_acid_of
    relationship: is_conjugate_base_of
    relationship: is_enantiomer_of
    relationship: is_substituent_group_from
    relationship: is_tautomer_of
    relationship: lacks_part
    relationship: member_of
    relationship: negatively_regulates
    relationship: non_functional_homolog_of
    relationship: occurs_in
    relationship: only_in_taxon
    relationship: overlaps
    relationship: part_of
    relationship: positively_regulates
    relationship: reciprocal_of
    relationship: regulates
    relationship: results_in
    relationship: towards
    relationship: transcribed_from
    relationship: transcribed_to
    relationship: variant_of
     */
}
