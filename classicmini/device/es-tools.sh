#!/bin/sh
#  Copyright (C) 2026 DefKorns (https://defkorns.github.io/LICENSE)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
source /etc/preinit
script_init

home_disabled="0x7FFF"
home_default="0x0024"
home_prev_file=/var/lib/hakchi/es/home_combination.prev

isValidName() {
  case "$1" in
  "" | *[!A-Za-z0-9_]*) return 1 ;;
  esac
}

isValidValue() {
  case "$1" in
  "" | *[!A-Za-z0-9_.x-]*) return 1 ;;
  esac
}

getCfg() {
  eval "echo \"\$cfg_clovercon_$1\""
}

applyCfg() {
  isValidName "$1" && isValidValue "$2" || return 1
  eval "cfg_clovercon_$1='$2'"
  save_config
  param_file="/sys/module/clovercon/parameters/$1"
  [ -w "$param_file" ] && echo "$2" > "$param_file"
}

printState() {
  if "$@"; then echo "on"; else echo "off"; fi
}

cfgEquals() {
  [ "$(getCfg "$1")" = "$2" ]
}

isJapaneseFamicom() {
  [ "$sftype" = "nes" ] && [ "$sfregion" = "jpn" ]
}

homeEnabled() {
  current="$(getCfg home_combination)"
  [ -n "$current" ] && [ "$(printf '%d' "$current")" != "$(printf '%d' "$home_disabled")" ]
}

esHasInternet() {
  ip route 2>/dev/null | grep -q '^default' || return 1
  for host in connectivitycheck.gstatic.com cloudflare.com; do
    nc -z -w 3 "$host" 443 >/dev/null 2>&1 && return 0
  done
  return 1
}

diskUsage() {
  df -h "$1" 2>/dev/null | tail -1 | awk '{ print $3 " / " $2 " (" $4 " free)" }'
}

case "$1" in
  ip)
    ifconfig | grep "inet " | grep -v 127.0.0.1 | awk 'match($0, /([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)/) { print "Local IP: " substr($0, RSTART, RLENGTH) }'
    public_ip=$(wget -q -T 4 -t 1 -O - http://ipecho.net/plain 2>/dev/null)
    if [ -n "$public_ip" ]; then echo "Public IP: $public_ip"; else echo "Public IP: offline"; fi
    ;;
  reconnect)
    if esHasInternet; then
      echo "Already connected to the internet."
    else
      /etc/init.d/S92networking restart >/dev/null 2>&1
      sleep 5
      if esHasInternet; then echo "Connected."; else echo "Connection failed. Check the Wi-Fi adapter."; fi
    fi
    ;;
  scan)
    wpa_cli scan >/dev/null 2>&1
    sleep 3
    networks=$(wpa_cli scan_results 2>/dev/null | awk -F'\t' 'NR > 2 && $5 != "" { print $5 }' | sort -u | head -n 12)
    if [ -n "$networks" ]; then echo "$networks"; else echo "No networks found."; fi
    ;;
  networks)
    wpa_cli scan >/dev/null 2>&1
    sleep 3
    wpa_cli scan_results 2>/dev/null | awk -F'	' 'NR > 2 && $5 != "" && !seen[$5]++ { print $5 "	" ($4 ~ /WPA|WEP/ ? "1" : "0") }' | head -n 20
    ;;
  wificonnect)
    network_id=$(wpa_cli list_networks | awk -F'	' -v ssid="$ES_SSID" '$2 == ssid { print $1; exit }')
    added=0
    if [ -z "$network_id" ]; then
      network_id=$(wpa_cli add_network | tail -n 1)
      wpa_cli set_network "$network_id" ssid "\"$ES_SSID\"" >/dev/null
      wpa_cli set_network "$network_id" scan_ssid 1 >/dev/null
      added=1
    fi
    if [ -n "$ES_PSK" ]; then
      wpa_cli set_network "$network_id" psk "\"$ES_PSK\"" >/dev/null
    else
      wpa_cli set_network "$network_id" key_mgmt NONE >/dev/null
    fi
    wpa_cli select_network "$network_id" >/dev/null
    connected=0
    for _ in $(seq 1 20); do
      if wpa_cli status 2>/dev/null | grep -q '^wpa_state=COMPLETED'; then connected=1; break; fi
      sleep 1
    done
    if [ "$connected" = 1 ]; then
      for _ in $(seq 1 10); do
        ip -4 addr show wlan0 2>/dev/null | grep -q 'inet ' && break
        sleep 1
      done
      ip -4 addr show wlan0 2>/dev/null | grep -q 'inet ' || /etc/init.d/S92networking restart >/dev/null 2>&1
      wpa_cli enable_network all >/dev/null
      wpa_cli save_config >/dev/null
      sleep 2
      if esHasInternet; then echo "Connected to $ES_SSID."; else echo "Connected to $ES_SSID, but there is no internet."; fi
    else
      [ "$added" = 1 ] && wpa_cli remove_network "$network_id" >/dev/null
      wpa_cli enable_network all >/dev/null
      wpa_cli save_config >/dev/null
      echo "Could not connect to $ES_SSID. Check the password."
    fi
    ;;
  sysinfo)
    [ -f /var/version ] && source /var/version
    echo "Console: $sftype-$sfregion"
    echo "hakchi: $hakchiVersion"
    echo "Kernel: $kernelVersion"
    echo "Boot: $bootVersion"
    echo "NAND: $(diskUsage "$mountpoint/var/lib")"
    echo "SD: $(diskUsage /media)"
    saves="$(readlink -f /var/saves)"
    [ -d "$saves" ] && echo "Saves: $(du -sh "$saves" 2>/dev/null | cut -f1)"
    echo "Temperature: $(cat /sys/class/thermal/thermal_zone0/temp) C"
    ;;
  savelog)
    log_dir=/media/data/log
    mkdir -p "$log_dir" && dmesg > "$log_dir/dmesg.log" && echo "Saved to $log_dir/dmesg.log" || echo "Could not save the log."
    ;;
  cache)
    sync
    echo 1 > /proc/sys/vm/drop_caches
    echo "Cache cleared."
    free -m | awk 'NR == 2 { print "Free memory: " $4 " MB" }'
    ;;
  epilepsy)
    case "$2" in
      state) if [ "$cfg_disable_armet" = "y" ]; then echo "off"; else echo "on"; fi ;;
      toggle)
        if [ "$cfg_disable_armet" = "y" ]; then cfg_disable_armet='n'; else cfg_disable_armet='y'; fi
        save_config
        ;;
    esac
    ;;
  clovercon)
    case "$2" in
      available)
        case "$3" in
          fc_start) printState isJapaneseFamicom ;;
          *) echo "off" ;;
        esac
        ;;
      home)
        case "$3" in
          state) printState homeEnabled ;;
          toggle)
            if homeEnabled; then
              getCfg home_combination > "$home_prev_file"
              applyCfg home_combination "$home_disabled"
            else
              applyCfg home_combination "$(cat "$home_prev_file" 2>/dev/null || echo "$home_default")"
            fi
            ;;
        esac
        ;;
      state)
        isValidName "$3" && printState cfgEquals "$3" 1
        ;;
      toggle)
        isValidName "$3" && { if [ "$(getCfg "$3")" = "1" ]; then applyCfg "$3" 0; else applyCfg "$3" 1; fi; }
        ;;
      is)
        isValidName "$3" && printState cfgEquals "$3" "$4"
        ;;
      set)
        applyCfg "$3" "$4"
        ;;
    esac
    ;;
esac
