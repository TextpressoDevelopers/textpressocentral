#!/usr/bin/env bash

output_dir=$1

mkdir -p "${output_dir}"/{obo,tpobofiles}
wget -r -nH --no-parent -R "index.html*" -P $output_dir http://textpressocentral.org:5555/obo/
wget -r -nH --no-parent -R "index.html*" -P $output_dir http://textpressocentral.org:5555/tpobofiles/
