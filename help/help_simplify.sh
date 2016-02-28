#!/bin/sh

tidy -config tidy.conf help.html | \
perl -pe 's/\s*face="[^"]*"//g' | \
perl -pe 's/\s*class="[^"]*"//g' | \
perl -pe 's/\s*style="[^"]*"//g' | \
perl -pe 's/<font[^>]*>//g' | \
perl -pe 's/<\/font>//g' > help2.html

tidy -config tidy.conf help2.html > help3.html
