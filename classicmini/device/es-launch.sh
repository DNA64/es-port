#!/bin/sh
# Runs the game a generated <CODE>.desktop describes: the Exec of the entry hakchi synced, in its Path when it exists.
# ES has released the window and the controller while this runs. The RetroArch wrappers end with uistart, which brings
# the stock UI back over ES; stop it again, but not with uistop: that one also terminates ES, which keeps /dev/fb0 open.
#
#   es-launch.sh <file.desktop>
source /etc/preinit
script_init

desktop=$1
cmd=$(tr -d '\r' < "$desktop" | sed -n 's/^Exec=//p' | head -n 1)
path=$(tr -d '\r' < "$desktop" | sed -n 's/^Path=//p' | head -n 1)
[ -n "$cmd" ] || exit 1
[ -d "$path" ] && cd "$path"

sh -c "$cmd"
rc=$?

if [ -n "$(pidof clover-mcp)" ]; then
  es=$(pidof emulationstation)
  killall -s KILL clover-mcp 1>/dev/null 2>&1
  sh /etc/init.d/S99first_startup stop 1>/dev/null 2>&1
  for p in $(uilist); do
    case " $es " in *" $p "*) ;; *) kill -s TERM "$p" ;; esac
  done
fi
exit $rc
