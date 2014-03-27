#! /bin/sh

set -e

echo "Generating updated POTFILES list..."
mydir=`dirname "$0"`
cd "$mydir"
if [ . = "$mydir" ]; then
  prog="$0"
else
  prog=`basename "$0"`
fi

rm -f po/POTFILES.in.new

(
 echo "# List of source files containing translatable strings."
 echo "# Please keep this file sorted alphabetically."
 echo "# Generated by $prog at `date`"
 echo "[encoding: UTF-8]"
 echo "inkscape.desktop.in"
 echo "share/filters/filters.svg.h"
 echo "share/palettes/palettes.h"
 echo "share/patterns/patterns.svg.h"
 echo "share/symbols/symbols.h"
 echo "share/templates/templates.h"


 find src \( -name '*.cpp' -o -name '*.[ch]' \) -type f -print0 | xargs -0 egrep -l '(\<[QNC]?_|gettext) *\(' | sort
 find share/extensions -name '*.py' -type f -print0 | xargs -0 egrep -l '(\<[QNC]?_|gettext) *\(' | sort
 find share/extensions -name '*.inx' -type f -print | sort | sed 's%^%[type: gettext/xml] %'
) > po/POTFILES.in.new

diff -q po/POTFILES.in po/POTFILES.in.new ||:
mv po/POTFILES.in.new po/POTFILES.in
echo "Done."
echo ""
echo "Now you need to run 'make distcheck' to find all the"
echo "places that get broken because of this script."
