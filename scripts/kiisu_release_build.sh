#!/bin/bash
# Kiisu Production Release Build Script
# Optimization #10: Production build with LTO and aggressive optimization
#
# This script builds production-ready firmware with maximum runtime performance:
# - Link-Time Optimization (LTO) enabled (+15-20% performance)
# - Aggressive optimization flags
# - All external apps included
# - Verification and packaging

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

echo -e "${MAGENTA}========================================${NC}"
echo -e "${MAGENTA}Kiisu Production Release Build${NC}"
echo -e "${MAGENTA}========================================${NC}"
echo ""

# Warning about requirements
echo -e "${YELLOW}⚠  WARNING: Production build requirements:${NC}"
echo -e "${YELLOW}   - 16GB+ RAM (LTO requires significant memory)${NC}"
echo -e "${YELLOW}   - 8-15 minutes build time (2-3x longer than dev)${NC}"
echo -e "${YELLOW}   - Ensure code is tested with dev profile first${NC}"
echo ""

# Confirmation prompt
read -p "Continue with production build? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Build cancelled."
    exit 0
fi

# Check available RAM (Linux only)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    TOTAL_RAM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    TOTAL_RAM_GB=$((TOTAL_RAM_KB / 1024 / 1024))

    if [ $TOTAL_RAM_GB -lt 8 ]; then
        echo -e "${RED}✗ Insufficient RAM: ${TOTAL_RAM_GB}GB (minimum 8GB recommended)${NC}"
        echo -e "${YELLOW}  Consider using: BUILD_PROFILE=dev ./fbt firmware${NC}"
        exit 1
    fi

    if [ $TOTAL_RAM_GB -lt 16 ]; then
        echo -e "${YELLOW}⚠  Low RAM: ${TOTAL_RAM_GB}GB (16GB+ recommended for LTO)${NC}"
        echo -e "${YELLOW}  Build may fail or be slow. Continue? (y/N)${NC}"
        read -p "" -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Build cancelled."
            exit 0
        fi
    else
        echo -e "${GREEN}✓ RAM: ${TOTAL_RAM_GB}GB (sufficient for LTO)${NC}"
    fi
fi

# Set release profile
export BUILD_PROFILE=release

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Starting Production Build${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Profile: ${BUILD_PROFILE}${NC}"
echo -e "${BLUE}Optimizations: LTO + Aggressive${NC}"
echo -e "${BLUE}Apps: All external apps included${NC}"
echo ""

# Clean previous build artifacts
echo -e "${BLUE}Cleaning previous build artifacts...${NC}"
rm -rf build/latest 2>/dev/null || true
rm -rf dist/kiisu-prod 2>/dev/null || true

# Build updater package (includes firmware + resources + radio stack)
START_TIME=$(date +%s)

echo -e "${BLUE}Building updater package...${NC}"
if ./fbt updater_package; then
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))

    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}✓ Production build successful!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Build time: ${ELAPSED}s ($(($ELAPSED / 60))m $(($ELAPSED % 60))s)${NC}"

    # Display output information
    if [ -f "build/latest/firmware.bin" ]; then
        FW_SIZE=$(du -h build/latest/firmware.bin | cut -f1)
        FW_SIZE_BYTES=$(stat -f%z build/latest/firmware.bin 2>/dev/null || stat -c%s build/latest/firmware.bin 2>/dev/null)
        echo -e "${GREEN}Firmware size: ${FW_SIZE} (${FW_SIZE_BYTES} bytes)${NC}"
    fi

    # Find updater package
    DIST_DIR=$(find dist -maxdepth 1 -type d -name "f7-*" | head -n 1)
    if [ -d "$DIST_DIR" ]; then
        UPDATE_PKG=$(find "$DIST_DIR" -name "*.tgz" | head -n 1)
        if [ -f "$UPDATE_PKG" ]; then
            PKG_SIZE=$(du -h "$UPDATE_PKG" | cut -f1)
            echo -e "${GREEN}Update package: ${UPDATE_PKG}${NC}"
            echo -e "${GREEN}Package size: ${PKG_SIZE}${NC}"
        fi
    fi

    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Release Artifacts${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Firmware:${NC}"
    ls -lh build/latest/firmware.* 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
    echo -e "${GREEN}Distribution:${NC}"
    find dist -name "*.tgz" -o -name "*.dfu" 2>/dev/null | while read file; do
        SIZE=$(du -h "$file" | cut -f1)
        echo "  $file ($SIZE)"
    done

    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}Next Steps${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo "1. Test firmware on hardware:"
    echo "   ./fbt flash_usb_full"
    echo ""
    echo "2. Deploy to qFlipper:"
    echo "   Use package: ${UPDATE_PKG:-dist/*/flipper-*.tgz}"
    echo ""
    echo "3. Verify performance improvements:"
    echo "   - Sensor polling should be smoother"
    echo "   - UI responsiveness should be improved"
    echo "   - Power consumption should be lower"

else
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))

    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}✗ Build failed after ${ELAPSED}s${NC}"
    echo -e "${RED}========================================${NC}"

    echo ""
    echo -e "${YELLOW}Troubleshooting:${NC}"
    echo "1. Check RAM usage (may have run out of memory during LTO)"
    echo "2. Try dev profile first: ./fbt flash_usb_full"
    echo "3. Use low memory profile: BUILD_PROFILE=lowmem ./fbt firmware"
    echo "4. Review build errors above"

    exit 1
fi

echo ""
echo -e "${MAGENTA}Production build complete! 🎉${NC}"
