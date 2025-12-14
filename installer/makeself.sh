#!/bin/bash
# Create self-extracting installer

set -e

# определить директории относительно скрипта
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$SCRIPT_DIR/package"
OUTPUT_FILE="$SCRIPT_DIR/../secret-detector-1.0.0-linux-x86_64.run"

echo "Creating self-extracting installer..."
echo "Script dir: $SCRIPT_DIR"
echo "Package dir: $PACKAGE_DIR"

# проверить что все файлы на месте
echo ""
echo "Checking package contents..."
if [ ! -f "$PACKAGE_DIR/secret_detector" ]; then
    echo "ERROR: secret_detector not found!"
    exit 1
fi

if [ ! -f "$PACKAGE_DIR/secret_detector_gui" ]; then
    echo "ERROR: secret_detector_gui not found!"
    exit 1
fi

if [ ! -d "$PACKAGE_DIR/lib" ]; then
    echo "WARNING: lib/ directory not found - creating empty one"
    mkdir -p "$PACKAGE_DIR/lib"
fi

if [ ! -f "$PACKAGE_DIR/secret_detector_wrapper.sh" ]; then
    echo "ERROR: wrapper scripts not found!"
    exit 1
fi

if [ ! -f "$PACKAGE_DIR/install.sh" ]; then
    echo "ERROR: install.sh not found!"
    exit 1
fi

echo "All required files found"
echo ""
echo "Package contents:"
ls -lh "$PACKAGE_DIR"
echo ""

# создать архив из package/
echo "Creating archive..."
cd "$PACKAGE_DIR"
tar czf "$SCRIPT_DIR/payload.tar.gz" .
cd "$SCRIPT_DIR"

# создать установочный скрипт stub
cat > installer_stub.sh << 'STUB'
#!/bin/bash
# Self-extracting installer for Secret Detector

set -e

GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}Secret Detector Installer${NC}"
echo "Extracting files..."

TMPDIR=$(mktemp -d)
ARCHIVE=$(awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' "$0")

tail -n+$ARCHIVE "$0" | tar xz -C "$TMPDIR"

echo "Running installation script..."
cd "$TMPDIR"
bash install.sh

# Cleanup
cd /
rm -rf "$TMPDIR"

exit 0

__ARCHIVE_BELOW__
STUB

# объединить stub + archive
cat installer_stub.sh payload.tar.gz > "$OUTPUT_FILE"
chmod +x "$OUTPUT_FILE"

# очистить временные файлы
rm installer_stub.sh payload.tar.gz

echo ""
echo "Successfully created installer!"
echo ""
echo "Output: $OUTPUT_FILE"
ls -lh "$OUTPUT_FILE"
echo ""
echo "File size: $(du -h "$OUTPUT_FILE" | cut -f1)"
echo ""
echo "To test: sudo $OUTPUT_FILE"
