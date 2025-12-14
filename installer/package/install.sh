#!/bin/bash
# Secret Detector Installer v1.0.0

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   Secret Detector Installer v1.0.0    ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo ""

# проверка прав
if [ "$EUID" -ne 0 ]; then 
    echo -e "${YELLOW}This installer requires root privileges.${NC}"
    echo "Please run with sudo"
    exit 1
fi

INSTALL_DIR="/opt/secret-detector"
BIN_DIR="/usr/local/bin"
DESKTOP_DIR="/usr/share/applications"
ICON_DIR="/usr/share/pixmaps"

echo -e "${GREEN}[1/6]${NC} Checking dependencies..."

# для Ubuntu/Debian
if command -v apt-get &> /dev/null; then
    echo "Detected: Debian/Ubuntu-based system"
    
    MISSING_DEPS=""
    
    if ! dpkg -l | grep -q libqt5widgets5; then
        MISSING_DEPS="$MISSING_DEPS libqt5widgets5 libqt5gui5 libqt5core5a"
    fi
    
    if [ -n "$MISSING_DEPS" ]; then
        echo -e "${YELLOW}Installing Qt5 dependencies...${NC}"
        apt-get update -qq
        apt-get install -y $MISSING_DEPS
    fi
fi

# для Fedora/RHEL
if command -v dnf &> /dev/null; then
    echo "Detected: Fedora/RHEL-based system"
    
    if ! rpm -q qt5-qtbase &> /dev/null; then
        echo -e "${YELLOW}Installing Qt5...${NC}"
        dnf install -y qt5-qtbase
    fi
fi

# для Arch
if command -v pacman &> /dev/null; then
    echo "Detected: Arch-based system"
    
    if ! pacman -Q qt5-base &> /dev/null 2>&1; then
        echo -e "${YELLOW}Installing Qt5...${NC}"
        pacman -S --noconfirm qt5-base
    fi
fi

echo -e "${GREEN}[2/6]${NC} Creating installation directory..."
mkdir -p "$INSTALL_DIR"
mkdir -p "$BIN_DIR"
mkdir -p "$DESKTOP_DIR"
mkdir -p "$ICON_DIR"

echo -e "${GREEN}[3/6]${NC} Copying files..."

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# копировать бинарники
cp "$SCRIPT_DIR/secret_detector" "$INSTALL_DIR/"
cp "$SCRIPT_DIR/secret_detector_gui" "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/secret_detector"
chmod +x "$INSTALL_DIR/secret_detector_gui"

# копировать wrappers (обертки)
cp "$SCRIPT_DIR/secret_detector_wrapper.sh" "$INSTALL_DIR/"
cp "$SCRIPT_DIR/secret_detector_gui_wrapper.sh" "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/secret_detector_wrapper.sh"
chmod +x "$INSTALL_DIR/secret_detector_gui_wrapper.sh"

# копировать библиотеки
if [ -d "$SCRIPT_DIR/lib" ]; then
    cp -r "$SCRIPT_DIR/lib" "$INSTALL_DIR/"
    echo "Bundled libraries copied"
fi

# копировать конфигурацию
mkdir -p "$INSTALL_DIR/config"
if [ -d "$SCRIPT_DIR/config" ]; then
    cp -r "$SCRIPT_DIR/config/"* "$INSTALL_DIR/config/" 2>/dev/null || true
fi

# копировать uninstall script
if [ -f "$SCRIPT_DIR/uninstall.sh" ]; then
    cp "$SCRIPT_DIR/uninstall.sh" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/uninstall.sh"
fi

# создать симлинки
echo "Creating symlinks..."
ln -sf "$INSTALL_DIR/secret_detector_wrapper.sh" "$BIN_DIR/secret-detector"
ln -sf "$INSTALL_DIR/secret_detector_gui_wrapper.sh" "$BIN_DIR/secret-detector-gui"
ln -sf "$INSTALL_DIR/uninstall.sh" "$BIN_DIR/secret-detector-uninstall"

echo -e "${GREEN}[4/6]${NC} Creating desktop entry..."

cat > "$DESKTOP_DIR/secret-detector.desktop" << 'DESKTOP'
[Desktop Entry]
Version=1.0
Type=Application
Name=Secret Detector
Comment=Find secrets in your code
Exec=/opt/secret-detector/secret_detector_gui_wrapper.sh
Icon=secret-detector
Terminal=false
Categories=Development;Security;
Keywords=security;secrets;scanner;
DESKTOP

# создать иконку
cat > "$ICON_DIR/secret-detector.svg" << 'SVG'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
  <rect width="64" height="64" rx="8" fill="#2c3e50"/>
  <text x="32" y="42" text-anchor="middle" font-size="40" fill="#ecf0f1">🔒</text>
</svg>
SVG

echo -e "${GREEN}[5/6]${NC} Updating desktop database..."
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
fi

echo -e "${GREEN}[6/6]${NC} Installation complete!"
echo ""
echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║         Installation Summary           ║${NC}"
echo -e "${GREEN}╠════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║${NC} Install location: $INSTALL_DIR"
echo -e "${GREEN}║${NC} CLI command:      secret-detector"
echo -e "${GREEN}║${NC} GUI command:      secret-detector-gui"
echo -e "${GREEN}║${NC} Desktop entry:    Yes"
echo -e "${GREEN}║${NC} Uninstaller:      secret-detector-uninstall"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Usage:${NC}"
echo "  CLI:        secret-detector --help"
echo "  CLI:        secret-detector /path/to/project"
echo "  GUI:        secret-detector-gui"
echo "  Uninstall:  sudo secret-detector-uninstall"
echo ""
echo -e "${GREEN}Installation successful!${NC}"
