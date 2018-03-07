#!/bin/bash
#
for i in $(cat /usr/local/textpresso/tmp/allnxml.txt); do /usr/local/textpresso/bin/printbibfromnxmlorcasfile -sjtn $i; done > /usr/local/textpresso/tmp/allnxml.subject.txt
