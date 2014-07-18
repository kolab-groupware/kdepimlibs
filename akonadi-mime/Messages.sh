#! /bin/sh
$EXTRACTRC `find . src -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find . src -name "*.cpp" -o -name "*.h" | grep -v "/autotests"` -o $podir/libakonadi-kmime5.pot
