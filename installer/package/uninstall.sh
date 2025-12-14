#!/bin/bash
# Secret Detector Uninstaller

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}╔════════════════════════════════════════╗${NC}"
echo -e "${YELLOW}║   Secret Detector Uninstaller v1.0.0  ║${NC}"
echo -e "${YELLOW}╚════════════════════════════════════════╝${NC}"
echo ""

if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}This uninstaller requires root privileges.${NC}"
    echo "Please run: sudo ./uninstall.sh"
    exit 1
fi

echo -e "${YELLOW}Removing Secret Detector...${NC}"

# удалить файлы
rm -rf /opt/secret-detector
rm -f /usr/local/bin/secret-detector
rm -f /usr/local/bin/secret-detector-gui
rm -f /usr/share/applications/secret-detector.desktop
rm -f /usr/share/pixmaps/secret-detector.svg

# обновить desktop database
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi

echo -e "${GREEN}Secret Detector has been uninstalled.${NC}"
