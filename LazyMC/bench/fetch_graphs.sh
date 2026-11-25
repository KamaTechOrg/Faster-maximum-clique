#!/bin/bash
# Downloads the graphs of Table I of the LazyMC paper (those that are publicly
# available and fit a 15 GB machine) and converts them with convert_graph.
# Usage: bench/fetch_graphs.sh <work_dir> <convert_graph binary>
set -u
W=$1; CONV=$2
mkdir -p "$W/raw" "$W/conv"
SNAP=https://snap.stanford.edu/data
NR=https://nrvis.com/download/data
# name | url
LIST="
USAroad|$NR/road/road-road-usa.zip
CAroad|$SNAP/roadNet-CA.txt.gz
soflow|$SNAP/sx-stackoverflow.txt.gz
talk|$SNAP/wiki-Talk.txt.gz
patents|$SNAP/cit-Patents.txt.gz
LiveJournal|$SNAP/soc-LiveJournal1.txt.gz
hudong|$NR/web/web-hudong.zip
flickr|$NR/soc/soc-flickr-und.zip
topcats|$SNAP/wiki-topcats.txt.gz
pokec|$SNAP/soc-pokec-relationships.txt.gz
dblp|$SNAP/bigdata/communities/com-dblp.ungraph.txt.gz
orkut|$SNAP/bigdata/communities/com-orkut.ungraph.txt.gz
it|$NR/web/web-it-2004.zip
hollywood|$NR/ca/ca-hollywood-2009.zip
higgs|$SNAP/higgs-social_network.edgelist.gz
uk|$NR/web/web-uk-2005.zip
WormNet|$NR/bio/bio-WormNet-v3.zip
HS-CX|$NR/bio/bio-HS-CX.zip
mouse|$NR/bio/bio-mouse-gene.zip
human-1|$NR/bio/bio-human-gene1.zip
human-2|$NR/bio/bio-human-gene2.zip
"
for row in $LIST; do
  name=${row%%|*}; url=${row#*|}; f="$W/raw/$(basename "$url")"
  [ -s "$W/conv/$name.el" ] && { echo "skip $name"; continue; }
  [ -s "$f" ] || curl -sSfL --retry 3 -o "$f" "$url" || { echo "DOWNLOAD FAILED $name"; continue; }
  case "$f" in
    *.gz)  zcat "$f" | "$CONV" "$W/conv/$name" ;;
    *.zip) inner=$(unzip -Z1 "$f" | grep -E '\.(mtx|edges|txt)$' | head -1)
           flag=""; [[ "$inner" == *.mtx ]] && flag="--mtx"
           unzip -p "$f" "$inner" | "$CONV" "$W/conv/$name" $flag ;;
  esac
done
