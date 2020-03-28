cmake_minimum_required(VERSION 3.5)
project(textpressocentral)

set(TPC_VERSION "v1.0.0")
set(TPC_RELEASE_NAME "White Snowberry")
add_definitions( -DTPC_VERSION=\"${TPC_VERSION}\" -DTPC_RELEASE_NAME=\"${TPC_RELEASE_NAME}\")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_FLAGS "-DBOOST_NO_CXX11_SCOPED_ENUMS -w")

set(CMAKE_STATIC_LIBRARY_PREFIX "") # avoid the prefix "lib"
set(CMAKE_SHARED_LIBRARY_PREFIX "")

SET(CMAKE_INSTALL_PREFIX /usr/local/textpresso)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:/usr/local/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(APR REQUIRED)
include_directories(${APR_INCLUDE_DIR})

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

find_package(CImg REQUIRED)
include_directories(${CImg_INCLUDE_DIRS})

find_package(Lucene++ REQUIRED)
include_directories(${Lucene++_INCLUDE_DIRS})

find_package(Wt REQUIRED)
include_directories(${Wt_INCLUDE_DIR})

find_package(Textpresso REQUIRED)
include_directories(${Textpresso_INCLUDE_DIR})

#### Main projects ####

add_executable(pdf2tpcas Pdf2Tpcas/main.cpp Pdf2Tpcas/PdfInfo.cpp
        Pdf2Tpcas/ElementCluster.cpp Pdf2Tpcas/PdfMyFontInfo.cpp
        Pdf2Tpcas/Stream2Tpcas.cpp Pdf2Tpcas/TextElement.cpp)
target_link_libraries(pdf2tpcas lucene++ boost_system boost_filesystem podofo fontconfig freetype pthread z jpeg uima icuuc boost_regex
        ${CImg_SYSTEM_LIBS})

add_executable(createlistofontologies CreateListofOntologies/main.cpp)
target_link_libraries(createlistofontologies pqxx)

add_library(libcrfsgd SHARED crfsgd/assert.h crfsgd/crfsgd.cpp crfsgd/crfsgd.h crfsgd/gzstream.h crfsgd/gzstream.cpp
        crfsgd/matrices.h crfsgd/matrices.cpp crfsgd/pstream.h crfsgd/pstream.cpp crfsgd/timer.h
        crfsgd/timer.cpp crfsgd/vectors.h crfsgd/vectors.cpp crfsgd/wrapper.h)

add_library(libcrfasgd SHARED crfasgd/assert.h crfasgd/crfasgd.cpp crfasgd/crfasgd.h crfasgd/gzstream.h crfasgd/gzstream.cpp
        crfasgd/matrices.h crfasgd/matrices.cpp crfasgd/pstream.h crfasgd/pstream.cpp crfasgd/timer.h
        crfasgd/timer.cpp crfasgd/vectors.h crfasgd/vectors.cpp crfasgd/wrapper.h)

add_executable(generatelexicalvariations GenerateLexicalVariations/main.cpp
        GenerateLexicalVariations/LexicalVariations.cpp
        GenerateLexicalVariations/TpOntApi.cpp
        GenerateLexicalVariations/TpOntEntry.cpp)
target_link_libraries(generatelexicalvariations boost_system boost_filesystem pqxx)

add_executable(lexicalvariations LexicalVariations/main.cpp
        LexicalVariations/LexicalVariations.cpp)

add_executable(obofileanalyzer OboFileAnalyzer/main.cpp 
        OboFileAnalyzer/OboEntry.cpp
        OboFileAnalyzer/OboFileSegmentation.cpp)

# main web application - tpc
file(GLOB TpC_SRC "TpC/*.h" "TpC/*.cpp" "TpC/*.C" "TpC/*.hpp" "TpC/lucene/CaseSensitiveAnalyzer.h"
        "TpC/lucene/CaseSensitiveAnalyzer.cpp")
add_executable(tpc ${TpC_SRC})
set_target_properties(tpc PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(tpc uima ${Wt_LIBRARIES} ${Wt_HTTP_LIBRARY} curl
            boost_system boost_filesystem icuuc xerces-c
            boost_thread boost_regex pthread pqxx lucene++ boost_iostreams ${CImg_SYSTEM_LIBS} ${Textpresso_LIBRARY}
            boost_serialization)
ELSE(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(tpc uima ${Wt_LIBRARIES} ${Wt_FCGI_LIBRARY} curl
            boost_system boost_filesystem icuuc xerces-c boost_thread boost_regex pthread pqxx lucene++ boost_iostreams
            ${CImg_SYSTEM_LIBS} ${Textpresso_LIBRARY} boost_serialization)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

add_executable(tpcuration TpCuration/main.cpp TpCuration/TpCurApi.cpp)
target_link_libraries(tpcuration pqxx)

add_executable(tpflat2oboconverter TpFlat2OboConverter/main.cpp)

add_executable(tpso Tpso/main.cpp OboFileAnalyzer/OboFileSegmentation.cpp
        OboFileAnalyzer/OboEntry.cpp TpC/PgList.cpp
        LexicalVariations/LexicalVariations.cpp)
target_link_libraries(tpso pthread boost_regex boost_program_options boost_filesystem boost_system pqxx)

add_executable(tpontology TpOntology/main.cpp TpOntology/TpOntApi.cpp
        TpOntology/TpOntEntry.cpp)
target_link_libraries(tpontology pqxx boost_filesystem boost_system)

add_executable(xml2tpcas Xml2Tpcas/main.cpp Xml2Tpcas/pugixml.cpp
        Xml2Tpcas/PugiXml2Tpcas.cpp Xml2Tpcas/ReadXml2Stream.cpp
        Xml2Tpcas/Stream2Tpcas.cpp)
target_link_libraries(xml2tpcas uima icuuc boost_system boost_filesystem)

#### Subprojects ####

# create textpresso directory structure locally - skip bin and lib
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/resources)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/resources/resources-tpc)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/resources/resources-web)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/cronjobs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/luceneindex)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/tpontology)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/useful)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/useruploads)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/html)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/tpcas)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/etc)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/conf)

#### INSTALL ####

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/resources DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/cronjobs DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/luceneindex DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/tpontology DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/useful DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/etc DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/useruploads DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/html DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/tpcas DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/etc DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirs/conf DESTINATION ${CMAKE_INSTALL_PREFIX})

install(TARGETS generatelexicalvariations
        obofileanalyzer pdf2tpcas tpc tpflat2oboconverter tpontology tpcuration tpso xml2tpcas
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
install(FILES tpctl.sh Tpso/CreateLexica.bash DESTINATION bin
	PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ)

SET(CMAKE_INSTALL_PREFIX /usr/local/textpresso)

install(FILES etc/template.bib etc/tpsoinput.json etc/ontology.conf DESTINATION etc)
install(FILES LexicalVariations/resources/allverbs.txt TpC/resources/authstrings.xml TpC/resources/authtemplates.xml
        TpC/resources/curation-form.xml TpC/resources/home.contactus.txt TpC/resources/home.firststeps.txt
        TpC/resources/home.introduction.txt TpC/resources/home.news.txt LexicalVariations/resources/irrplurals.txt
        LexicalVariations/resources/irrverbs.txt TpC/resources/TpC.css TpC/resources/TpC.xml
        TpC/resources/uploadtemplate.tsv TpC/resources/TpCentral.css TpC/resources/TpCentral.xml
        TpC/resources/customize-colors.xml
        DESTINATION ${CMAKE_INSTALL_PREFIX}/resources/resources-tpc)
install(DIRECTORY TpC/resources/icons DESTINATION ${CMAKE_INSTALL_PREFIX}/resources/resources-tpc)
install(DIRECTORY TpC/resources/other_images DESTINATION ${CMAKE_INSTALL_PREFIX}/resources/resources-tpc)
install(FILES ConfigurationFiles/wt_config.xml ConfigurationFiles/wt_config_debug.xml ConfigurationFiles/fastcgi.conf
        DESTINATION ${CMAKE_INSTALL_PREFIX}/conf)

SET(CMAKE_INSTALL_PREFIX /usr/local)
