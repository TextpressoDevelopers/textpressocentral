// -*- C++ -*-
// CRF with stochastic gradient
// Copyright (C) 2007- Leon Bottou

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA

#include "crfasgd.h"


// ============================================================
// Utilities


static int crfutil::skipBlank(istream &f) {
    int c = f.get();
    while (f.good() && isspace(c) && c != '\n' && c != '\r')
        c = f.get();
    if (!f.eof())
        f.unget();
    return c;
}

static int crfutil::skipSpace(istream &f) {
    int c = f.get();
    while (f.good() && isspace(c))
        c = f.get();
    if (!f.eof())
        f.unget();
    return c;
}

inline double crfutil::expmx(double x) {
#define EXACT_EXPONENTIAL 0
#define IEEE754_EXPONENTIAL 0
#if EXACT_EXPONENTIAL
    return exp(-x);
#elif IEEE754_EXPONENTIAL
    // fast approximation using IEEE754 bit patterns.
    float p = x * -1.442695040f;
    float clipp = (p < -126.0f) ? -126.0f : p;

    union {
        unsigned int i;
        float f;
    } v;
    v.i = (unsigned int) ((1 << 23) * (clipp + 126.94269504f));
    return v.f;
#else
    // fast approximation of exp(-x) for x positive
#define A0   (1.0)
#define A1   (0.125)
#define A2   (0.0078125)
#define A3   (0.00032552083)
#define A4   (1.0172526e-5) 
    if (x < 13.0) {
        assert(x >= 0);
        double y;
        y = A0 + x * (A1 + x * (A2 + x * (A3 + x * A4)));
        y *= y;
        y *= y;
        y *= y;
        y = 1 / y;
        return y;
    }
    return 0;
#undef A0
#undef A1
#undef A2
#undef A3
#undef A4
#endif
}

static double crfutil::logSum(const VFloat *v, int n) {
    int i;
    VFloat m = v[0];
    for (i = 0; i < n; i++)
        m = max(m, v[i]);
    double s = 0;
    for (i = 0; i < n; i++)
        s += expmx(m - v[i]);
    return m + log(s);
}

static double crfutil::logSum(const FVector &v) {
    return logSum(v, v.size());
}

static void crfutil::dLogSum(double g, const VFloat *v, VFloat *r, int n) {
    int i;
    VFloat m = v[0];
    for (i = 0; i < n; i++)
        m = max(m, v[i]);
    double z = 0;
    for (i = 0; i < n; i++) {
        double e = expmx(m - v[i]);
        r[i] = e;
        z += e;
    }
    for (i = 0; i < n; i++)
        r[i] = g * r[i] / z;
}

static void crfutil::dLogSum(double g, const FVector &v, FVector &r) {
    assert(v.size() <= r.size());
    dLogSum(g, v, r, v.size());
}


// ============================================================
// Parsing data file


int crfutil::readDataLine(istream &f, strings_t &line, int &expected) {
    int obtained = 0;
    while (f.good()) {
        int c = skipBlank(f);
        if (c == '\n' || c == '\r')
            break;
        string s;
        f >> s;
        if (!s.empty()) {
            line.push_back(s);
            obtained += 1;
        }
    }
    int c = f.get();
    if (c == '\r' && f.get() != '\n')
        f.unget();
    if (obtained > 0) {
        if (expected <= 0)
            expected = obtained;
        else if (expected > 0 && expected != obtained) {
            cerr << "ERROR: expecting " << expected
                    << " columns in data file." << endl;
            exit(10);
        }
    } else
        skipSpace(f);
    return obtained;
}

int crfutil::readDataSentence(istream &f, strings_t &s, int &expected) {
    strings_t line;
    s.clear();
    while (f.good())
        if (readDataLine(f, s, expected))
            break;
    while (f.good())
        if (!readDataLine(f, s, expected))
            break;
    if (expected)
        return s.size() / expected;
    return 0;
}


// ============================================================
// Processing templates


void crfutil::checkTemplate(string tpl) {
    const char *p = tpl.c_str();
    if (p[0] != 'U' && p[0] != 'B') {
        cerr << "ERROR: Unrecognized template type (neither U nor B.)" << endl
                << "       Template was \"" << tpl << "\"." << endl;
        exit(10);
    }
    while (p[0]) {
        if (p[0] == '%' && p[1] == 'x') {
            bool okay = false;
            char *n = const_cast<char*> (p);
            long junk;
            (void) junk; // not used
            if (n[2] == '[') {
                junk = strtol(n + 3, &n, 10);
                while (isspace(n[0]))
                    n += 1;
                if (n[0] == ',') {
                    junk = strtol(n + 1, &n, 10);
                    while (isspace(n[0]))
                        n += 1;
                    if (n[0] == ']')
                        okay = true;
                }
            }
            if (okay)
                p = n;
            else {
                cerr << "ERROR: Syntax error in %x[.,,] expression." << endl
                        << "       Template was \"" << tpl << "\"." << endl;
                exit(10);
            }
        }
        p += 1;
    }
}

string crfutil::expandTemplate(string tpl, const strings_t &s, int columns, int pos) {
    string e;
    int rows = s.size() / columns;
    const char *t = tpl.c_str();
    const char *p = t;

    static const char *BOS[4] = {"_B-1", "_B-2", "_B-3", "_B-4"};
    static const char *EOS[4] = {"_B+1", "_B+2", "_B+3", "_B+4"};

    while (*p) {
        if (p[0] == '%' && p[1] == 'x' && p[2] == '[') {
            if (p > t)
                e.append(t, p - t);
            // parse %x[A,B] assuming syntax has been verified
            char *n;
            int a = strtol(p + 3, &n, 10);
            while (n[0] && n[0] != ',')
                n += 1;
            int b = strtol(n + 1, &n, 10);
            while (n[0] && n[0] != ']')
                n += 1;
            p = n;
            t = n + 1;
            // catenate
            a += pos;
            if (b >= 0 && b < columns) {
                if (a >= 0 && a < rows)
                    e.append(s[a * columns + b]);
                else if (a < 0)
                    e.append(BOS[min(3, -a - 1)]);
                else if (a >= rows)
                    e.append(EOS[min(3, a - rows)]);
            }
        }
        p += 1;
    }
    if (p > t)
        e.append(t, p - t);
    return e;
}

void crfutil::readTemplateFile(const char *fname, strings_t &templateVector) {
    ifstream f(fname);
    if (!f.good()) {
        cerr << "ERROR: Cannot open " << fname << " for reading." << endl;
        exit(10);
    }
    while (f.good()) {
        int c = skipSpace(f);
        while (c == '#') {
            while (f.good() && c != '\n' && c != '\r')
                c = f.get();
            f.unget();
            c = skipSpace(f);
        }
        string s;
        getline(f, s);
        if (!s.empty()) {
            checkTemplate(s);
            templateVector.push_back(s);
        }
    }
    if (!f.eof()) {
        cerr << "ERROR: Cannot read " << fname << " for reading." << endl;
        exit(10);
    }
}

// one more crfutil function (loadsentences) is defined below.


// ============================================================
// Dictionary


string
Dictionary::internString(string s) const {
    dict_t::const_iterator it = internedStrings.find(s);
    if (it != internedStrings.end())
        return it->first;
#if defined(mutable)
    const_cast<Dictionary*> (this)->
#endif
            internedStrings[s] = 1;
    return s;
}

ostream& operator<<(ostream &f, const Dictionary &d) {
    typedef map<int, string> rev_t;
    rev_t rev;
    strings_t::const_iterator si;
    dict_t::const_iterator di;
    rev_t::const_iterator ri;
    for (di = d.outputs.begin(); di != d.outputs.end(); di++)
        rev[di->second] = di->first;
    for (ri = rev.begin(); ri != rev.end(); ri++)
        f << "Y" << ri->second << endl;
    for (si = d.templates.begin(); si != d.templates.end(); si++)
        f << "T" << *si << endl;
    rev.clear();
    for (di = d.features.begin(); di != d.features.end(); di++)
        rev[di->second] = di->first;
    for (ri = rev.begin(); ri != rev.end(); ri++)
        f << "X" << ri->second << endl;
    return f;
}

istream& operator>>(istream &f, Dictionary &d) {
    d.outputs.clear();
    d.features.clear();
    d.templates.clear();
    d.index = 0;
    int findex = 0;
    int oindex = 0;
    while (f.good()) {
        string v;
        crfutil::skipSpace(f);
        int c = f.get();
        if (c == 'Y') {
            f >> v;
            if (v.empty()) {
                cerr << "ERROR (reading dictionary): "
                        << "Invalid Y record in model file." << endl;
                exit(10);
            }
            if (findex > 0) {
                cerr << "ERROR (reading dictionary): "
                        << "Found Y record occuring after X record." << endl;
                exit(10);
            }
            d.outputs[v] = oindex++;
        } else if (c == 'T') {
            f >> v;
            if (v.empty()) {
                cerr << "ERROR (reading dictionary): "
                        << "Invalid T record." << endl;
                exit(10);
            }
            crfutil::checkTemplate(v);
            d.templates.push_back(v);
        } else if (c == 'X') {
            f >> v;
            if (v.empty()) {
                cerr << "ERROR (reading dictionary): "
                        << "Invalid X record." << endl;
                exit(10);
            }
            int nindex = findex;
            if (v[0] == 'U')
                nindex += oindex;
            else if (v[0] == 'B')
                nindex += oindex * oindex;
            else {
                cerr << "ERROR (reading dictionary): "
                        << "Invalid feature in X record: " << v << endl;
                exit(10);
            }
            d.features[v] = findex;
            findex = nindex;
        } else {
            f.unget();
            break;
        }
    }
    d.index = findex;
    if (!f.good() && !f.eof()) {
        d.outputs.clear();
        d.features.clear();
        d.templates.clear();
        d.index = 0;
    }
    d.outputnames.resize(oindex);
    for (dict_t::const_iterator it = d.outputs.begin();
            it != d.outputs.end(); it++)
        d.outputnames[it->second] = it->first;
    return f;
}


typedef pair<string, int> sipair_t;
typedef vector<sipair_t> sivector_t;

struct SIPairCompare {

    bool operator()(const sipair_t &p1, const sipair_t &p2) {
        if (p1.second > p2.second) return true;
        else if (p1.second < p2.second) return false;
        else return (p1.first < p2.first);
    }
} siPairCompare;

int Dictionary::initFromData(const char *tFile, const char *dFile, int cutoff) {
    // clear all
    templates.clear();
    outputs.clear();
    features.clear();
    index = 0;

    // read templates
    if (verbose)
        cout << "Reading template file " << tFile << "." << endl;
    crfutil::readTemplateFile(tFile, templates);
    int nu = 0;
    int nb = 0;
    for (unsigned int t = 0; t < templates.size(); t++)
        if (templates[t][0] == 'U')
            nu += 1;
        else if (templates[t][0] == 'B')
            nb += 1;
    if (verbose)
        cout << "  u-templates: " << nu
            << "  b-templates: " << nb << endl;
    if (nu + nb != (int) templates.size()) {
        cerr << "ERROR (building dictionary): "
                << "Problem counting templates" << endl;
        exit(10);
    }

    // process compressed datafile
    if (verbose)
        cerr << "Scanning " << dFile << " to build dictionary." << endl;
    typedef hash_map<string, int> hash_t;
    hash_t fcount;
    int columns = 0;
    int oindex = 0;
    int sentences = 0;
    strings_t s;
    crfutil::ixstream_t fx(dFile);
    istream &f = fx.stream();
    Timer timer;
    timer.start();
    while (crfutil::readDataSentence(f, s, columns)) {
        sentences += 1;
        // intern strings to save memory
        for (strings_t::iterator it = s.begin(); it != s.end(); it++)
            *it = internString(*it);
        // expand features and count them
        int rows = s.size() / columns;
        for (int pos = 0; pos < rows; pos++) {
            // check output keyword
            string &y = s[pos * columns + columns - 1];
            dict_t::iterator di = outputs.find(y);
            if (di == outputs.end())
                outputs[y] = oindex++;
            // expand templates
            for (unsigned int t = 0; t < templates.size(); t++) {
                string x = crfutil::expandTemplate(templates[t], s, columns, pos);
                hash_t::iterator hi = fcount.find(x);
                if (hi != fcount.end())
                    hi->second += 1;
                else
                    fcount[x] = 1;
            }
        }
    }
    if (!f.eof()) {
        cerr << "ERROR (building dictionary): "
                << "Problem reading data file " << dFile << endl;
        exit(10);
    }
    outputnames.resize(oindex);
    for (dict_t::const_iterator it = outputs.begin(); it != outputs.end(); it++)
        outputnames[it->second] = it->first;
    if (verbose)
        cout << "  sentences: " << sentences
            << "  outputs: " << oindex << endl;

    // sorting in frequency order
    sivector_t keys;
    for (hash_t::iterator hi = fcount.begin(); hi != fcount.end(); hi++)
        if (hi->second >= cutoff)
            keys.push_back(*hi);
    if (keys.size() <= 0) {
        cerr << "ERROR (building dictionary): "
                << "No features satisfy the cutoff frequency" << endl;
        exit(10);
    }
    sort(keys.begin(), keys.end(), siPairCompare);

    // allocating parameters
    for (unsigned int j = 0; j < keys.size(); j++) {
        string k = keys[j].first;
        features[k] = index;
        if (k[0] == 'B')
            index += oindex * oindex;
        else
            index += oindex;
    }
    if (verbose)
        cout << "  cutoff: " << cutoff
            << "  features: " << features.size()
        << "  parameters: " << index << endl
            << "  duration: " << timer.elapsed() << " seconds." << endl;

    return sentences;
}


// ============================================================
// Preprocessing data


void Sentence::init(const Dictionary &dict, const strings_t &s, int columns) {
    w.detach();
    Rep *r = rep();
    int maxcol = columns - 1;
    int maxpos = s.size() / columns - 1;
    int ntemplat = dict.nTemplates();
    r->uFeatures.clear();
    r->bFeatures.clear();
    r->yLabels.clear();
    r->columns = columns;
    // intern strings to save memory
    for (strings_t::const_iterator it = s.begin(); it != s.end(); it++)
        r->data.push_back(dict.internString(*it));
    // expand features
    for (int pos = 0; pos <= maxpos; pos++) {
        // labels
        string y = s[pos * columns + maxcol];
        int yindex = dict.output(y);
        r->yLabels.push_back(yindex);
        // features
        SVector u;
        SVector b;
        for (int t = 0; t < ntemplat; t++) {
            string tpl = dict.templateString(t);
            int findex = dict.feature(crfutil::expandTemplate(tpl, s, columns, pos));
            if (findex >= 0) {
                if (tpl[0] == 'U')
                    u.set(findex, 1);
                else if (tpl[0] == 'B')
                    b.set(findex, 1);
            }
        }
        r->uFeatures.push_back(u);
        if (pos < maxpos)
            r->bFeatures.push_back(b);
    }
}

string Sentence::data(int pos, int col) const {
    const Rep *r = rep();
    if (pos >= 0 && pos < size())
        if (col >= 0 && col < r->columns)
            return r->data[pos * r->columns + col];
    return string();
}

ostream& operator<<(ostream &f, const Sentence &s) {
    int maxpos = s.size() - 1;
    int columns = s.columns();
    for (int pos = 0; pos <= maxpos; pos++) {
        for (int col = 0; col < columns; col++)
            f << s.data(pos, col) << " ";
        f << endl << "   Y" << pos << " " << s.y(pos) << endl;
        f << "   U" << pos << s.u(pos);
        if (pos < maxpos)
            f << "   B" << pos << s.b(pos);
    }
    return f;
}


void crfutil::loadSentences(const char *fname, const Dictionary &dict, dataset_t &data) {
    if (verbose)
        cout << "Reading and preprocessing " << fname << "." << endl;
    Timer timer;
    int sentences = 0;
    int columns = 0;
    strings_t s;
    crfutil::ixstream_t fx(fname);
    istream &f = fx.stream();
    timer.start();
    while (crfutil::readDataSentence(f, s, columns)) {
        Sentence ps;
        ps.init(dict, s, columns);
        data.push_back(ps);
        sentences += 1;
    }
    if (verbose)
        cout << "  processed: " << sentences << " sentences." << endl
            << "  duration: " << timer.elapsed() << " seconds." << endl;
}


// ============================================================
// Weights


Weights::Weights()
: nOutputs(0), wDivisor(1.0), aDivisor(1.0), wFraction(0.0) {
}

Weights::Weights(int nParams, int nOutputs)
: nOutputs(0), wDivisor(1.0), aDivisor(1.0), wFraction(0.0) {
    resize(nParams, nOutputs);
}

void Weights::clear() {
    resize(0, 0);
}

void Weights::resize(int nParams, int nOutputs) {
    this->nOutputs = nOutputs;
    w.resize(nParams);
    a.resize(nParams);
    for (int i = 0; i < nParams; i++)
        w[i] = a[i] = 0.0;
    aDivisor = 1.0;
    wDivisor = 1.0;
    wFraction = 0.0;
}

void Weights::normalize() const {
    const_cast<Weights*> (this)->normalize();
}

void Weights::normalize() {
    if (!a.size())
        a.resize(w.size());
    for (int i = 0; i < w.size(); i++) {
        a[i] = (wFraction * w[i] + a[i]) / aDivisor;
        w[i] = (w[i]) / wDivisor;
    }
    wDivisor = 1.0;
    aDivisor = 1.0;
    wFraction = 0.0;
}

ostream& operator<<(ostream &f, const Weights &ww) {
    ww.normalize();
    f << ww.a;
    return f;
}

istream& operator>>(istream &f, Weights &ww) {
    FVector w;
    f >> w;
    if (w.size() != ww.w.size()) {
        cerr << "ERROR (reading weights):"
                << "Invalid weight vector size: " << w.size() << endl;
        exit(10);
    }
    for (int i = 0; i < ww.w.size(); i++)
        ww.w[i] = ww.a[i] = w[i];
    ww.wDivisor = 1.0;
    ww.aDivisor = 1.0;
    ww.wFraction = 0.0;
    return f;
}


// ============================================================
// Scorer [using non averaged weights]


Scorer::Scorer(const Sentence &s_, const Dictionary &d_, Weights &ww_)
: s(s_), d(d_), ww(ww_), scoresOk(false) {
    assert(ww.w.size() == d.nParams());
}

void Scorer::computeScores() {
    if (!scoresOk) {
        const VFloat *w = ww.w;
        int nout = d.nOutputs();
        int npos = s.size();
        int pos;
        // compute uScores
        uScores.resize(npos);
        for (pos = 0; pos < npos; pos++) {
            FVector &u = uScores[pos];
            u.resize(nout);
            SVector x = s.u(pos);
            for (const SVector::Pair *p = x; p->i >= 0; p++) {
                int k = p->i;
                for (int j = 0; j < nout; j++, k++)
                    u[j] += w[k] * p->v;
            }
            for (int j = 0; j < nout; j++)
                u[j] /= ww.wDivisor;
        }
        // compute bScores
        bScores.resize(npos - 1);
        for (pos = 0; pos < npos - 1; pos++) {
            FMatrix &b = bScores[pos];
            b.resize(nout, nout);
            SVector x = s.b(pos);
            for (const SVector::Pair *p = x; p->i >= 0; p++) {
                int k = p->i;
                for (int i = 0; i < nout; i++) {
                    FVector &bi = b[i];
                    for (int j = 0; j < nout; j++, k++)
                        bi[j] += w[k] * p->v;
                }
            }
            for (int i = 0; i < nout; i++) {
                FVector &bi = b[i];
                for (int j = 0; j < nout; j++)
                    bi[j] /= ww.wDivisor;
            }
        }
    }
    scoresOk = true;
}

double Scorer::viterbi(ints_t &path) {
    computeScores();
    int npos = s.size();
    int nout = d.nOutputs();
    int pos, i, j;

    // allocate backpointer array
    vector<ints_t> pointers(npos);
    for (int i = 0; i < npos; i++)
        pointers[i].resize(nout);

    // process scores
    FVector scores = uScores[0];
    for (pos = 1; pos < npos; pos++) {
        FVector us = uScores[pos];
        for (i = 0; i < nout; i++) {
            FVector bs = bScores[pos - 1][i];
            bs.add(scores);
            int bestj = 0;
            double bests = bs[0];
            for (j = 1; j < nout; j++)
                if (bs[j] > bests) {
                    bests = bs[j];
                    bestj = j;
                }
            pointers[pos][i] = bestj;
            us[i] += bests;
        }
        scores = us;
    }
    // find best final score
    int bestj = 0;
    double bests = scores[0];
    for (j = 1; j < nout; j++)
        if (scores[j] > bests) {
            bests = scores[j];
            bestj = j;
        }
    // backtrack
    path.resize(npos);
    for (pos = npos - 1; pos >= 0; pos--) {
        path[pos] = bestj;
        bestj = pointers[pos][bestj];
    }
    return bests;
}

int Scorer::test() {
    ints_t path;
    int npos = s.size();
    int errors = 0;
    viterbi(path);
    for (int pos = 0; pos < npos; pos++)
        if (path[pos] != s.y(pos))
            errors += 1;
    return errors;
}

int Scorer::test(ostream &f) {
    ints_t path;
    int npos = s.size();
    int ncol = s.columns();
    int errors = 0;
    viterbi(path);
    for (int pos = 0; pos < npos; pos++) {
        if (path[pos] != s.y(pos))
            errors += 1;
        for (int c = 0; c < ncol; c++)
            f << s.data(pos, c) << " ";
        f << d.outputString(path[pos]) << endl;
    }
    f << endl;
    return errors;
}

double Scorer::scoreCorrect() {
    computeScores();
    int npos = s.size();
    int y = s.y(0);
    double sum = uScores[0][y];
    for (int pos = 1; pos < npos; pos++) {
        int fy = y;
        y = s.y(pos);
        if (y >= 0 && fy >= 0)
            sum += bScores[pos - 1][y][fy];
        if (y >= 0)
            sum += uScores[pos][y];
    }
    return sum;
}

double Scorer::gradCorrect(double g) {
    computeScores();
    int npos = s.size();
    int y = s.y(0);
    VFloat vf = g;
    uGradients(&vf, 0, y, 1);
    double sum = uScores[0][y];
    for (int pos = 1; pos < npos; pos++) {
        int fy = y;
        y = s.y(pos);
        if (y >= 0 && fy >= 0)
            sum += bScores[pos - 1][y][fy];
        if (y >= 0)
            sum += uScores[pos][y];
        if (y >= 0 && fy >= 0)
            bGradients(&vf, pos - 1, fy, 1, y);
        if (y >= 0)
            uGradients(&vf, pos, y, 1);
    }
    return sum;
}

double Scorer::scoreForward() {
    computeScores();
    int npos = s.size();
    int nout = d.nOutputs();
    int pos, i;

    FVector scores = uScores[0];
    for (pos = 1; pos < npos; pos++) {
        FVector us = uScores[pos];
        for (i = 0; i < nout; i++) {
            FVector bs = bScores[pos - 1][i];
            bs.add(scores);
            us[i] += crfutil::logSum(bs);
        }
        scores = us;
    }
    return crfutil::logSum(scores);
}

double Scorer::gradForward(double g) {
    computeScores();
    int npos = s.size();
    int nout = d.nOutputs();
    int pos;

#define USE_FORWARD_BACKWARD 0
#if USE_FORWARD_BACKWARD

    FMatrix alpha(npos, nout);
    FMatrix beta(npos, nout);
    // forward
    alpha[0] = uScores[0];
    for (pos = 1; pos < npos; pos++) {
        FVector us = uScores[pos];
        for (int i = 0; i < nout; i++) {
            FVector bs = bScores[pos - 1][i];
            bs.add(alpha[pos - 1]);
            us[i] += logSum(bs);
        }
        alpha[pos] = us;
    }

    // backward
    beta[pos - 1] = uScores[pos - 1];
    for (pos = pos - 2; pos >= 0; pos--) {
        FVector us = uScores[pos];
        for (int i = 0; i < nout; i++) {
            FVector bs(nout);
            const FMatrix &bsc = bScores[pos];
            for (int j = 0; j < nout; j++)
                bs[j] = bsc[j][i];
            bs.add(beta[pos + 1]);
            us[i] += logSum(bs);
        }
        beta[pos] = us;
    }
    // score
    double score = logSum(beta[0]);

    // collect gradients
    for (pos = 0; pos < npos; pos++) {
        FVector b = beta[pos];
        if (pos > 0) {
            FVector a = alpha[pos - 1];
            const FMatrix &bsc = bScores[pos - 1];
            for (int j = 0; j < nout; j++) {
                FVector bs = bsc[j];
                FVector bgrad(nout);
                for (int i = 0; i < nout; i++)
                    bgrad[i] = g * expmx(max(0.0, score - bs[i] - a[i] - b[j]));
                bGradients(bgrad, pos - 1, 0, nout, j);
            }
        }
        FVector a = alpha[pos];
        FVector us = uScores[pos];
        FVector ugrad(nout);
        for (int i = 0; i < nout; i++)
            ugrad[i] = g * expmx(max(0.0, score + us[i] - a[i] - b[i]));
        uGradients(ugrad, pos, 0, nout);
    }

#else

    // forward
    FMatrix scores(npos, nout);
    scores[0] = uScores[0];
    for (pos = 1; pos < npos; pos++) {
        FVector us = uScores[pos];
        for (int i = 0; i < nout; i++) {
            FVector bs = bScores[pos - 1][i];
            bs.add(scores[pos - 1]);
            us[i] += crfutil::logSum(bs);
        }
        scores[pos] = us;
    }
    double score = crfutil::logSum(scores[npos - 1]);

    // backward with chain rule
    FVector tmp(nout);
    FVector grads(nout);
    crfutil::dLogSum(g, scores[npos - 1], grads);
    for (pos = npos - 1; pos > 0; pos--) {
        FVector ug;
        uGradients(grads, pos, 0, nout);
        for (int i = 0; i < nout; i++)
            if (grads[i]) {
                FVector bs = bScores[pos - 1][i];
                bs.add(scores[pos - 1]);
                crfutil::dLogSum(grads[i], bs, tmp);
                bGradients(tmp, pos - 1, 0, nout, i);
                ug.add(tmp);
            }
        grads = ug;
    }
    uGradients(grads, 0, 0, nout);

#endif

    return score;
}


// ============================================================
// AScorer [using averaged weights]


AScorer::AScorer(const Sentence &s_, const Dictionary &d_, Weights &ww_)
: Scorer(s_, d_, ww_) {
}

void AScorer::computeScores() {
    if (!scoresOk) {
        const VFloat *w = ww.w;
        const VFloat *a = ww.a;
        int nout = d.nOutputs();
        int npos = s.size();
        int pos;
        // compute uScores
        uScores.resize(npos);
        for (pos = 0; pos < npos; pos++) {
            FVector &u = uScores[pos];
            u.resize(nout);
            SVector x = s.u(pos);
            for (const SVector::Pair *p = x; p->i >= 0; p++) {
                int k = p->i;
                for (int j = 0; j < nout; j++, k++)
                    u[j] += (ww.wFraction * w[k] + a[k]) * p->v;
            }
            for (int j = 0; j < nout; j++)
                u[j] /= ww.aDivisor;
        }
        // compute bScores
        bScores.resize(npos - 1);
        for (pos = 0; pos < npos - 1; pos++) {
            FMatrix &b = bScores[pos];
            b.resize(nout, nout);
            SVector x = s.b(pos);
            for (const SVector::Pair *p = x; p->i >= 0; p++) {
                int k = p->i;
                for (int i = 0; i < nout; i++) {
                    FVector &bi = b[i];
                    for (int j = 0; j < nout; j++, k++)
                        bi[j] += p->v * (ww.wFraction * w[k] + a[k]);
                }
            }
            for (int i = 0; i < nout; i++) {
                FVector &bi = b[i];
                for (int j = 0; j < nout; j++)
                    bi[j] /= ww.aDivisor;
            }
        }
    }
    scoresOk = true;
}


// ============================================================
// TScorer - training score: update weights directly


TScorer::TScorer(const Sentence &s_, const Dictionary &d_, Weights &ww_,
        double eta_, double /* mu_ */)
: Scorer(s_, d_, ww_), eta(eta_) /*, mu(mu_)*/ {
}

void TScorer::uGradients(const VFloat *g, int pos, int fy, int ny) {
    int n = d.nOutputs();
    assert(pos >= 0 && pos < s.size());
    assert(fy >= 0 && fy < n);
    assert(fy + ny > 0 && fy + ny <= n);
    int off = fy;
    SVector x = s.u(pos);
    VFloat *w = ww.w;
    VFloat *a = ww.a;
    VFloat gain = eta * ww.wDivisor;
    VFloat wfrac = ww.wFraction;
    for (const SVector::Pair *p = x; p->i >= 0; p++)
        for (int j = 0; j < ny; j++) {
            VFloat dw = g[j] * p->v * gain;
            w[p->i + off + j] += dw;
            if (wfrac)
                a[p->i + off + j] -= dw * wfrac;
        }
}

void TScorer::bGradients(const VFloat *g, int pos, int fy, int ny, int y) {
    int n = d.nOutputs();
    assert(pos >= 0 && pos < s.size());
    assert(y >= 0 && y < n);
    assert(fy >= 0 && fy < n);
    assert(fy + ny > 0 && fy + ny <= n);
    int off = y * n + fy;
    SVector x = s.b(pos);
    VFloat *w = ww.w;
    VFloat *a = ww.a;
    VFloat gain = eta * ww.wDivisor;
    VFloat wfrac = ww.wFraction;
    for (const SVector::Pair *p = x; p->i >= 0; p++)
        for (int j = 0; j < ny; j++) {
            VFloat dw = g[j] * p->v * gain;
            w[p->i + off + j] += dw;
            if (wfrac)
                a[p->i + off + j] -= dw * wfrac;
        }
}


// ============================================================
// Main class CrfSgd


CrfSgd::CrfSgd()
: lambda(0), eta0(0), mu0(1), t(0), tstart(20000), epoch(0) {
}

void CrfSgd::load(istream &f) {
    f >> dict;
    t = 0;
    epoch = 0;
    ww.clear();
    ww.resize(dict.nParams(), dict.nOutputs());
    while (f.good()) {
        crfutil::skipSpace(f);
        int c = f.get();
        if (f.eof())
            break;
        if (c == 'T') {
            t = -1;
            f >> t;
            if (!f.good() || t <= 0) {
                cerr << "ERROR (reading model): "
                        << "Invalid iteration number: " << t << endl;
                exit(10);
            }
        } else if (c == 'E') {
            epoch = -1;
            f >> epoch;
            if (!f.good() || epoch < 0) {
                cerr << "ERROR (reading model): "
                        << "Invalid epoch number: " << epoch << endl;
                exit(10);
            }
        } else if (c == 'W') {
            f >> ww;
            if (!f.good() || ww.w.size() != dict.nParams()) {
                cerr << "ERROR (reading model): "
                        << "Invalid weight vector size: " << ww.w.size() << endl;
                exit(10);
            }
        } else if (c == 'L') {
            lambda = -1;
            f >> lambda;
            if (!f.good() || lambda <= 0) {
                cerr << "ERROR (reading model): "
                        << "Invalid lambda: " << lambda << endl;
                exit(10);
            }
        } else {
            cerr << "ERROR (reading model): "
                    << "Unrecognized line: '" << c << "'" << endl;
            exit(10);
        }
    }
}

istream& operator>>(istream &f, CrfSgd &d) {
    d.load(f);
    return f;
}

void CrfSgd::save(ostream &f) const {
    // save stuff
    f << dict;
    f << "L" << lambda << endl;
    f << "T" << t << endl;
    f << "E" << epoch << endl;
    f << "W" << ww << endl;
}

ostream& operator<<(ostream &f, const CrfSgd &d) {
    d.save(f);
    return f;
}

void CrfSgd::initialize(const char *tfname, const char *dfname, double c, int cutoff) {
    t = 0;
    epoch = 0;
    int n = dict.initFromData(tfname, dfname, cutoff);
    lambda = 1 / (c * n);
    if (verbose)
        cout << "Using c=" << c << ", i.e. lambda=" << lambda << endl;
    ww.clear();
    ww.resize(dict.nParams(), dict.nOutputs());
}

double CrfSgd::findObjBySampling(const dataset_t &data, const ivec_t &sample, Weights &weights) {
    double loss = 0;
    int n = sample.size();
    for (int i = 0; i < n; i++) {
        int j = sample[i];
        Scorer scorer(data[j], dict, weights);
        loss += scorer.scoreForward() - scorer.scoreCorrect();
    }
    double wnorm = dot(weights.w, weights.w) / (weights.wDivisor * weights.wDivisor);
    return loss / n + 0.5 * wnorm * lambda;
}

double CrfSgd::tryEtaBySampling(const dataset_t &data, const ivec_t &sample, double eta) {
    Weights weights = ww; // copy weights
    int i, n = sample.size();
    for (i = 0; i < n; i++) {
        int j = sample[i];
        TScorer scorer(data[j], dict, weights, eta, 1);
        scorer.computeScores();
        weights.wDivisor = weights.wDivisor / (1 - eta * lambda);
        scorer.gradCorrect(+1);
        scorer.gradForward(-1);
    }
    return findObjBySampling(data, sample, weights);
}

double CrfSgd::adjustTstart(double t) {
    tstart = t;
    return tstart;
}

double CrfSgd::adjustEta(double eta) {
    eta0 = eta;
    if (verbose)
        cout << " taking eta0=" << eta0;
    return eta0;
}

double CrfSgd::adjustEta(const dataset_t &data, int samples, double seta, Timer *tm) {
    ivec_t sample;
    if (verbose)
        cout << "[Calibrating] --  " << samples << " samples" << endl;
    assert(samples > 0);
    assert(dict.nOutputs() > 0);
    // choose sample
    int datasize = data.size();
    if (samples < datasize)
        for (int i = 0; i < samples; i++)
            sample.push_back((int) ((double) rand() * datasize / RAND_MAX));
    else
        for (int i = 0; i < datasize; i++)
            sample.push_back(i);
    // initial obj
    double sobj = findObjBySampling(data, sample, ww);
    cout << " initial objective=" << sobj << endl;
    // empirically find eta that works best
    double besteta = 1;
    double bestobj = sobj;
    double eta = seta;
    int totest = 10;
    double factor = 2;
    bool phase2 = false;
    while (totest > 0 || !phase2) {
        double obj = tryEtaBySampling(data, sample, eta);
        bool okay = (obj < sobj);
        if (verbose) {
            cout << " trying eta=" << eta << "  obj=" << obj;
            if (okay)
                cout << " (possible)" << endl;
            else
                cout << " (too large)" << endl;
        }
        if (okay) {
            totest -= 1;
            if (obj < bestobj) {
                bestobj = obj;
                besteta = eta;
            }
        }
        if (!phase2) {
            if (okay)
                eta = eta * factor;
            else {
                phase2 = true;
                eta = seta;
            }
        }
        if (phase2)
            eta = eta / factor;
    }
    // take it on the safe side (implicit regularization)
    besteta /= factor;
    // set initial t
    adjustEta(besteta);
    // message
    if (tm && verbose)
        cout << " time=" << tm->elapsed() << "s.";
    if (verbose)
        cout << endl;
    return eta0;
}

void CrfSgd::trainOnce(const Sentence &sentence, double eta, double mu) {
    TScorer scorer(sentence, dict, ww, eta, mu);
    scorer.computeScores();
    ww.wDivisor /= (1 - eta * lambda);
    if (mu >= 1.0) {
        ww.wFraction = 0;
        scorer.gradCorrect(+1);
        scorer.gradForward(-1);
        ww.a.clear();
        ww.aDivisor = ww.wDivisor;
        ww.wFraction = 1;
    } else {
        if (!ww.a.size())
            ww.normalize();
        scorer.gradCorrect(+1);
        scorer.gradForward(-1);
        ww.aDivisor /= (1 - mu);
        ww.wFraction += mu * ww.aDivisor / ww.wDivisor;
    }
    if (ww.wDivisor > 1e5 || ww.aDivisor > 1e5)
        ww.normalize();
    t += 1;
}

void CrfSgd::train(const dataset_t &data, int epochs, Timer *tm) {
    if (eta0 <= 0) {
        cerr << "ERROR (train): "
                << "Must call adjustEta() before train()." << endl;
        exit(10);
    }
    ivec_t shuffle;
    shuffle.resize(data.size());
    for (int j = 0; j < epochs; j++) {
        epoch += 1;
        // shuffle examples
        for (unsigned int i = 0; i < data.size(); i++)
            shuffle[i] = i;
        random_shuffle(shuffle.begin(), shuffle.end());
        if (verbose)
            cout << "[Epoch " << epoch << "] --";
        if (verbose)
            cout.flush();
        // perform epoch
        for (unsigned int i = 0; i < data.size(); i++) {
            double eta = eta0 / pow(1 + eta0 * lambda * t, 0.75);
            double mu = (t <= tstart) ? mu0 : mu0 / (1 + mu0 * (t - tstart));
            trainOnce(data[shuffle[i]], eta, mu);
        }
        // epoch done
        ww.normalize();
        double wnorm = dot(ww.w, ww.w);
        double anorm = dot(ww.a, ww.a);
        cout << " wnorm=" << wnorm << " anorm=" << anorm;
        if (tm && verbose)
            cout << " time=" << tm->elapsed() << "s.";
        if (verbose)
            cout << endl;
    }
}

void CrfSgd::test(const dataset_t &data, const char *conlleval, Timer *tm) {
    if (dict.nOutputs() <= 0) {
        cerr << "ERROR (test): "
                << "Must call load() or initialize() before test()." << endl;
        exit(10);
    }
    opstream f;
    if (conlleval && verbose)
        f.open(conlleval);
    if (verbose)
        cout << " sentences=" << data.size();
    double obj = 0;
    int errors = 0;
    int total = 0;
    for (unsigned int i = 0; i < data.size(); i++) {
        AScorer scorer(data[i], dict, ww);
        obj += scorer.scoreForward() - scorer.scoreCorrect();
        if (conlleval && verbose)
            errors += scorer.test(f);
        else
            errors += scorer.test();
        total += data[i].size();
    }
    obj = obj / data.size();
    if (verbose)
        cout << " loss=" << obj;
    double anorm = dot(ww.a, ww.a);
    obj += 0.5 * anorm * lambda;
    double misrate = (double) (errors * 100) / (total ? total : 1);
    misrate = ((int) (misrate * 100)) / 100.0;
    if (verbose)
        cout << " obj=" << obj
            << " err=" << errors << " (" << misrate << "%)";
    if (tm && verbose)
        cout << " time=" << tm->elapsed() << "s.";
    if (verbose)
        cout << endl;
}

void CrfSgd::testna(const dataset_t &data, const char *conlleval, Timer *tm) {
    if (dict.nOutputs() <= 0) {
        cerr << "ERROR (test): "
                << "Must call load() or initialize() before test()." << endl;
        exit(10);
    }
    opstream f;
    string evalcommand;
    if (conlleval && verbose)
        f.open(conlleval);
    if (verbose)
        cout << " sentences=" << data.size();
    double obj = 0;
    int errors = 0;
    int total = 0;
    for (unsigned int i = 0; i < data.size(); i++) {
        Scorer scorer(data[i], dict, ww);
        obj += scorer.scoreForward() - scorer.scoreCorrect();
        if (conlleval && verbose)
            errors += scorer.test(f);
        else
            errors += scorer.test();
        total += data[i].size();
    }
    obj = obj / data.size();
    if (verbose)
        cout << " loss=" << obj;
    double wnorm = dot(ww.w, ww.w);
    obj += 0.5 * wnorm * lambda;
    double misrate = (double) (errors * 100) / (total ? total : 1);
    misrate = ((int) (misrate * 100)) / 100.0;
    if (verbose)
        cout << " obj=" << obj
            << " miss=" << errors << " (" << misrate << "%)";
    if (tm && verbose)
        cout << " time=" << tm->elapsed() << "s.";
    if (verbose)
        cout << endl;
}

void CrfSgd::tag(const dataset_t &data) {
    if (dict.nOutputs() <= 0) {
        cerr << "ERROR (test): "
                << "Must call load() or initialize() before tag()." << endl;
        exit(10);
    }
    for (unsigned int i = 0; i < data.size(); i++) {
        Scorer scorer(data[i], dict, ww);
        scorer.test(cout);
    }
}


///* -------------------------------------------------------------
//   Local Variables:
//   c++-font-lock-extra-types: ("\\sw+_t" "[A-Z]\\sw*[a-z]\\sw*")
//   End:
//   ------------------------------------------------------------- */
