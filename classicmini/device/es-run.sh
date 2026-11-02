#!/bin/sh
# Stops the stock UI, builds the library from the synced games and runs EmulationStation; the stock UI comes back
# when ES exits, however it exits.
#
#   es-run.sh
here=/var/lib/hakchi/es
source /etc/preinit
script_init

trap uistart EXIT
uistop
sh "$here/es-library.sh" /tmp/es-library
HOME=$here/home LD_LIBRARY_PATH=$here "$here/emulationstation" "$@"
