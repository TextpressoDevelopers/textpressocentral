# Textpressocentral
## Description
This is the Textpresso Central web interface project.

Textpressocentral is a platform to perform full text literature searches, view and curate research papers, 
train and apply machine learning (ML) and text mining (TM) algorithm for semantic analysis and curation purposes. 
The user is supported in this task by giving him capabilities to select, edit and store lists of papers, sentences, 
term and categories in order to perform training and mining. The system is designed with the intent to empower the user 
to perform as many operations on a literature corpus or a particular paper as possible. It uses state-of-the-art 
software packages and frameworks such as the Unstructured Information Management Architecture (UIMA), Lucene and Wt. 
The corpus of papers can be build from fulltext articles that are available in PDF or NXML format.

## Installation
### Dependencies

libtpc must be installed in the system.
 
---
**NOTE**

cmake version >= 3.5 is required.

---

### Compile and Install Textpressocentral
To compile and install Textpressocentral, run the following commands from the root directory of the repository:
```{r, engine='bash', count_lines}
$ mkdir cmake-build-release && cd cmake-build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make && make install
```

This will install Textpressocentral in its default location (/usr/local/textpresso).

To activate Textpressocentral and to control further options, use **tpctl.sh** script.

### Debugging
Textpressocentral can be also compiled and installed in debug mode, with the following commands:
```{r, engine='bash', count_lines}
$ mkdir cmake-build-debug && cd cmake-build-debug
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make && make install
```
Note that this option creates a tpc executable that is linked to the wt builtin httpd server, and does not work with 
external web servers like lighttpd or apache2. To launch the tpc executable with the builtin wt httpd, run the 
following command:

```{r, engine='bash', count_lines}
$ ./tpc --docroot /usr/lib/cgi-bin/tc --approot /usr/lib/cgi-bin/tc --http-address=0.0.0.0 --http-port=<port> --gdb -c /etc/wt/wt_config_debug.xml
```

To debug Textpressocentral using an external web server, additional modifications are required. Please refer to 
[this wt guide](http://redmine.emweb.be/projects/wt/wiki/Wt_+_Apache_+_mod_fcgid_+_Valgrind_+_gdb_+_SELinux_-_Deploy_and_Debug/diff/2)
for additional information. This method may require additional modifications to the CMakeList.txt file to compile
Textpressocentral in debug mode and link it to wtfcgi library.

---
**NOTE**

To avoid possible conflicts when running the wt builtin http server, specify a port different than the one used by 
apache2 (e.g., 8080).

---

### Install Textpressocentral on Textpresso Docker image
Texpresso provides a Docker image with all the required dependencies pre-installed. To build it and run it, follow the
README file in libtpc project. 


## Further steps for manual installation
### Load literature data
After installing Textpressocentral, the literature data must be populated. To do so, copy the LuceneIndex folder from an
existing Textpressocentral installation. For example, to copy the C. Elegans literature, run the following commands:
```{r, engine='bash', count_lines}
$ sudo scp -r <username>@<host>:/usr/local/textpresso/luceneindex/C.\\\ elegans_0/ /usr/local/textpresso/luceneindex/
$ scp -r <username>@<host>:/usr/local/textpresso/tpcas/C.\\\ elegans /usr/local/textpresso/tpcas/
$ sudo echo "C. elegans" > /usr/local/textpresso/luceneindex/subindex.config
$ sudo ln -s /usr/lib/cgi-bin/tc/images /usr/local/textpresso/tpcas/
```

---

**NOTE**
To avoid deleting the literature with the textpresso data, place it on a local folder and create symlinks. The script
**tpctl.sh** can be used to automate this process.

---

See http://tpc.textpresso.org:5445/textpresso/textpressocentral/wikis/lighttpd to configure lighttpd for running 
Textpressocentral.