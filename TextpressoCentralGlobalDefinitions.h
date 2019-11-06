// Global file containing all global definitions.

#ifndef TEXTPRESSOCENTRALGLOBALDEFINITIONS_H
#define TEXTPRESSOCENTRALGLOBALDEFINITIONS_H

// Are these definitions really global? Otherwise move them back to their local project.
#define PDF2TPCASDESCRIPTOR "/usr/local/uima_descriptors/TpTokenizer.xml"
#define XML2TPCASDESCRIPTOR "/usr/local/uima_descriptors/TxTokenizer.xml"
#define TPCAS2LINDEXDESCRIPTOR "/usr/local/uima_descriptors/Tpcas2Lindex.xml"

#define AVAILABLELITERATUREFILE "/usr/local/textpresso/luceneindex/subindex.config"
#define USERUPLOADROOTDIR "/usr/local/textpresso/useruploads"

#define ONTOLOGYMEMBERSTABLENAME "ontologymembers"
#define PGONTOLOGYBROWSER "dbname=www-data"
#define PGONTOLOGYBROWSWERCOLUMNS "ontologybrowsercolumnnames"
#define PGONTOLOGY "dbname=www-data"
#define PGONTOLOGYTABLENAME "tpontology"
#define PGONTOLOGYTMPTABLENAME "tmptpontology"
#define PGCURATION "dbname=www-data"
#define PGCURATIONTABLENAME "tpcuration"
#define PCRELATIONSTABLENAME "pcrelations"
#define PCRELATIONSTMPTABLENAME "tmppcrelations"
#define PADCRELATIONSTABLENAME "padcrelations"
#define PADCRELATIONSTMPTABLENAME "tmppadcrelations"
#define STOPWORDTABLENAME "stopwords"
#define PGLITERATURE "dbname=www-data"
#define PGLITPREFTABLENAME "literaturepreference"
#define PGPRELOADEDCATEGORIES "dbname=www-data"
#define PGPRELOADEDCATTABLENAME "preloadedcategories"
#define PGTIPOFDAY "dbname=www-data"
#define PGTIPOFDAYTABLENAME "tipoftheday"
#define PGCURATIONFIELDS "dbname=www-data"
#define PGCURATIONFIELDSTABLENAME "curationfields"
#define PGCURATIONFORMS "dbname=www-data"
#define PGCURATIONFORMSTABLENAME "curationforms"
#define PGCURATIONDATAFROMVIEWER "dbname=www-data"
#define PGCURATIONDATAFROMVIEWERTABLENAME "curationdatafromviewer"
#define PGLISTOFONTOLOGIES "dbname=www-data"
#define PGLISTOFONTOLOGIESTABLENAME "listofontologies"
#define PGPREPOPULATION "dbname=www-data"
#define PGPREPOPULATIONTABLENAME "prepopulation"
#define AUTHIDENTITIES "dbname=www-data"
#define AUTHIDENTITIESTABLENAME "auth_identity"
#define PGLITERATUREPERMISSION "dbname=www-data"
#define PGLITERATUREPERMISSIONTABLENAME "literaturepermissions"
#define PGCUSTOMCOLORS "dbname=www-data"
#define PGCUSTOMCOLORSTABLENAME "customcolor"
#define PGDIALOGPREFERENCES "dbname=www-data"
#define PGDIALOGPREFERENCESTABLENAME "dialogpreferencestable"
#define PGSEARCHPREFERENCES "dbname=www-data"
#define PGSEARCHPREFERENCESTABLENAME "searchpreferencestable"
#define SENTENCE_SEARCH_MAX_NUM_DISPLAY_WORDS 50

#define CATEGORYCOLUMNWIDTH 1023

#endif
