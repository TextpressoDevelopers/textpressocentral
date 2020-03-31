#!/bin/bash
/usr/local/textpresso/tpontology/scripts/process.obo.files.com /usr/local/textpresso/tpontology/tpobofiles 3
/usr/local/textpresso/tpontology/scripts/process.obo.files.com /usr/local/textpresso/tpontology/obofiles 3
