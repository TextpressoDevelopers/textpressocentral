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



// $Id$


#include "wrapper.h"
#include "vectors.h"
#include "matrices.h"
#include "gzstream.h"
#include "pstream.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cmath>

using namespace std;

#if __cplusplus >= 201103L 
#define HAS_UNORDEREDMAP
#elif defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >=1100
#define HAS_UNORDEREDMAP
#elif defined(_MSC_VER) && _MSC_VER >= 1600
#define HAS_UNORDEREDMAP
#elif defined(__GXX_EXPERIMENTAL_CXX0X__)
#define HAS_UNORDEREDMAP
#endif

#ifdef HAS_UNORDEREDMAP
#include <unordered_map>
#define hash_map unordered_map
#elif defined(__GNUC__)
#define _GLIBCXX_PERMIT_BACKWARD_HASH
#include <ext/hash_map>
using __gnu_cxx::hash_map;
namespace __gnu_cxx {

    template<>
    struct hash<string> {
        hash<char*> h;

        inline size_t operator()(const string &s) const {
            return h(s.c_str());
        };
    };
};
#else
#define hash_map map
#endif

#ifndef HUGE_VAL
#define HUGE_VAL 1e+100
#endif

typedef vector<string> strings_t;
typedef vector<int> ints_t;
bool verbose = true;

namespace crfutil {
    static int skipBlank(istream &f);
    static int skipSpace(istream &f);
    inline double expmx(double x);
    static double logSum(const VFloat *v, int n);
    static double logSum(const FVector &v);
    static void dLogSum(double g, const VFloat *v, VFloat *r, int n);
    static void dLogSum(double g, const FVector &v, FVector &r);

    class ixstream_t {
        bool z;
        ifstream fn;
        igzstream fz;

    public:

        ixstream_t(const char *name) {
            string fname = name;
            int len = fname.size();
            z = fname.substr(len - 3) == ".gz";
            if (z)
                fz.open(name);
            else
                fn.open(name);
        }

        istream& stream() {
            if (z)
                return fz;
            else
                return fn;
        }
    };

    int readDataLine(istream &f, strings_t &line, int &expected);
    int readDataSentence(istream &f, strings_t &s, int &expected);
    void checkTemplate(string tpl);
    string expandTemplate(string tpl, const strings_t &s, int columns, int pos);
    void readTemplateFile(const char *fname, strings_t &templateVector);

}

typedef hash_map<string, int> dict_t;

class Dictionary {
private:
    dict_t outputs;
    dict_t features;
    strings_t templates;
    strings_t outputnames;
    mutable dict_t internedStrings;
    int index;

public:

    Dictionary() : index(0) {
    }

    int nOutputs() const {
        return outputs.size();
    }

    int nFeatures() const {
        return features.size();
    }

    int nTemplates() const {
        return templates.size();
    }

    int nParams() const {
        return index;
    }

    int output(string s) const {
        dict_t::const_iterator it = outputs.find(s);
        return (it != outputs.end()) ? it->second : -1;
    }

    int feature(string s) const {
        dict_t::const_iterator it = features.find(s);
        return (it != features.end()) ? it->second : -1;
    }

    string outputString(int i) const {
        return outputnames.at(i);
    }

    string templateString(int i) const {
        return templates.at(i);
    }

    string internString(string s) const;

    int initFromData(const char *tFile, const char *dFile, int cutoff = 1);

    friend istream& operator>>(istream &f, Dictionary &d);
    friend ostream& operator<<(ostream &f, const Dictionary &d);
};

typedef vector<SVector> svec_t;
typedef vector<int> ivec_t;

class Sentence {
private:

    struct Rep {
        int refcount;
        int columns;
        strings_t data;
        svec_t uFeatures;
        svec_t bFeatures;
        ivec_t yLabels;

        Rep *copy() {
            return new Rep(*this);
        }
    };
    Wrapper<Rep> w;

    Rep *rep() {
        return w.rep();
    }

    const Rep *rep() const {
        return w.rep();
    }

public:

    Sentence() {
    }

    void init(const Dictionary &dict, const strings_t &s, int columns);

    int size() const {
        return rep()->uFeatures.size();
    }

    SVector u(int i) const {
        return rep()->uFeatures.at(i);
    }

    SVector b(int i) const {
        return rep()->bFeatures.at(i);
    }

    int y(int i) const {
        return rep()->yLabels.at(i);
    }

    int columns() const {
        return rep()->columns;
    }
    string data(int pos, int col) const;

    friend ostream& operator<<(ostream &f, const Sentence &s);
};

typedef vector<Sentence> dataset_t;
namespace crfutil {
    void loadSentences(const char *fname, const Dictionary &dict, dataset_t &data);
}

struct Weights {
    int nOutputs;
    FVector w;
    FVector a;
    VFloat wDivisor;
    VFloat aDivisor;
    VFloat wFraction;

    Weights();
    Weights(int nParams, int nOutputs);
    void clear();
    void resize(int nParams, int nOutputs);
    void normalize();
    void normalize() const;

    FVector real_w() const {
        normalize();
        return w;
    }

    FVector real_a() const {
        normalize();
        return a;
    }
};

class Scorer {
public:
    Sentence s;
    const Dictionary &d;
    Weights &ww;
    bool scoresOk;
    vector<FVector> uScores;
    vector<FMatrix> bScores;

    Scorer(const Sentence &s_, const Dictionary &d_, Weights &ww_);

    virtual ~Scorer() {
    }
    virtual void computeScores();

    virtual void uGradients(const VFloat *g, int pos, int fy, int ny) {
    }

    virtual void bGradients(const VFloat *g, int pos, int fy, int ny, int y) {
    }

    double viterbi(ints_t &path);
    int test();
    int test(ostream &f);
    double scoreCorrect();
    double gradCorrect(double g);
    double scoreForward();
    double gradForward(double g);
};

class AScorer : public Scorer {
public:
    AScorer(const Sentence &s_, const Dictionary &d_, Weights &ww_);
    virtual void computeScores();
};

class TScorer : public Scorer {
private:
    double eta;
    // double mu; // unused
public:
    TScorer(const Sentence &s_, const Dictionary &d_, Weights &ww_, double eta_, double mu_);
    virtual void uGradients(const VFloat *g, int pos, int fy, int ny);
    virtual void bGradients(const VFloat *g, int pos, int fy, int ny, int y);
};

class CrfSgd {
    Dictionary dict;
    Weights ww;
    double lambda;
    double eta0;
    double mu0;
    double t;
    double tstart;
    int epoch;

    void load(istream &f);
    void save(ostream &f) const;
    void trainOnce(const Sentence &sentence, double eta, double mu);
    double findObjBySampling(const dataset_t &data, const ivec_t &sample, Weights &w);
    double tryEtaBySampling(const dataset_t &data, const ivec_t &sample, double eta);

public:

    CrfSgd();

    int getEpoch() const {
        return epoch;
    }

    const Dictionary& getDict() const {
        return dict;
    }

    double getLambda() const {
        return lambda;
    }

    double getEta0() const {
        return eta0;
    }

    double getT() const {
        return t;
    }

    double getTstart() const {
        return tstart;
    }

    FVector getW() const {
        return ww.real_w();
    }

    FVector getA() const {
        return ww.real_a();
    }


    void initialize(const char *templatefile,
            const char *datafile,
            double c = 4,
            int cutoff = 3);

    double adjustTstart(double t);
    double adjustEta(double eta);
    double adjustEta(const dataset_t &data,
            int sample = 5000, double eta = 1, Timer *tm = 0);

    void train(const dataset_t &data, int epochs = 1, Timer *tm = 0);
    void test(const dataset_t &data, const char *conlleval = 0, Timer *tm = 0);
    void testna(const dataset_t &data, const char *conlleval = 0, Timer *tm = 0);
    void tag(const dataset_t &data);

    friend istream& operator>>(istream &f, CrfSgd &d);
    friend ostream& operator<<(ostream &f, const CrfSgd &d);
};

