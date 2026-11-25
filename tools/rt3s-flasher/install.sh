#!/usr/bin/env bash
# Instalacja rt3s-flasher na Linuksie.
#   ./install.sh            program dla bieżącego użytkownika (pipx albo venv w ~/.local/share/rt3s-flasher/venv)
#   sudo ./install.sh --udev   tylko reguła udev (dostęp do radia bez sudo)
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)

if [ "${1:-}" = "--udev" ]; then
  [ "$(id -u)" -eq 0 ] || { echo "Reguła udev wymaga sudo: sudo $0 --udev"; exit 1; }
  install -m 644 "$here/udev/60-rt3s-flasher.rules" /etc/udev/rules.d/60-rt3s-flasher.rules
  udevadm control --reload-rules
  udevadm trigger --subsystem-match=usb --attr-match=idVendor=0483 || true
  echo "Reguła udev zainstalowana. Odłącz i podłącz kabel radia."
  exit 0
fi

[ "$(id -u)" -ne 0 ] || { echo "Program instaluj jako zwykły użytkownik (bez sudo); sudo tylko dla --udev."; exit 1; }

python3 -c 'import sys; sys.exit(sys.version_info < (3, 9))' || { echo "Potrzebny Python 3.9 lub nowszy."; exit 1; }
python3 -c 'import tkinter' 2>/dev/null || echo "UWAGA: brak tkinter, okno nie zadziała (Debian/Ubuntu: sudo apt install python3-tk). Wiersz poleceń działa."
ldconfig -p 2>/dev/null | grep -q 'libusb-1.0' || echo "UWAGA: nie widzę libusb-1.0 (Debian/Ubuntu: sudo apt install libusb-1.0-0)."

bin="$HOME/.local/bin"
mkdir -p "$bin"
if command -v pipx >/dev/null; then
  pipx install --force "$here"
else
  venv="$HOME/.local/share/rt3s-flasher/venv"
  python3 -m venv "$venv"
  "$venv/bin/pip" install --quiet --upgrade "$here"
  ln -sf "$venv/bin/rt3s-flasher" "$bin/rt3s-flasher"
fi

apps="$HOME/.local/share/applications"
mkdir -p "$apps"
sed "s|^Exec=.*|Exec=$bin/rt3s-flasher gui|" "$here/desktop/rt3s-flasher.desktop" > "$apps/rt3s-flasher.desktop"

echo "Zainstalowano: $bin/rt3s-flasher (i skrót w menu: RT3S flasher)."
case ":$PATH:" in *":$bin:"*) ;; *) echo "Dodaj $bin do PATH." ;; esac
[ -f /etc/udev/rules.d/60-rt3s-flasher.rules ] || echo "Następny krok: sudo $0 --udev"
