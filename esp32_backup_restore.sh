#!/bin/bash

# Script to backup and restore firmware for ESP32
# Requires esptool.py to be installed: pip install esptool

# Configuration - Adjust as needed
PORT="/dev/ttyUSB0"  # Serial port
BAUD=115200         # Baud rate
FLASH_SIZE="4MB"    # Flash size (2MB, 4MB, 8MB, etc.)
BACKUP_FILE="esp32_backup.bin"

# Function to backup firmware
backup() {
    echo "Backing up firmware from ESP32..."
    esptool.py --port $PORT --baud $BAUD read_flash 0x00000 0x$(printf "%x" $(($FLASH_SIZE * 1024 * 1024))) $BACKUP_FILE
    if [ $? -eq 0 ]; then
        echo "Backup successful: $BACKUP_FILE"
    else
        echo "Backup failed!"
    fi
}

# Function to restore firmware
restore() {
    if [ ! -f "$BACKUP_FILE" ]; then
        echo "Backup file $BACKUP_FILE not found!"
        exit 1
    fi
    echo "Restoring firmware to ESP32..."
    esptool.py --port $PORT --baud $BAUD write_flash 0x00000 $BACKUP_FILE
    if [ $? -eq 0 ]; then
        echo "Restore successful!"
    else
        echo "Restore failed!"
    fi
}

# Main script
case "$1" in
    backup)
        backup
        ;;
    restore)
        restore
        ;;
    *)
        echo "Usage: $0 {backup|restore}"
        echo "  backup  - Read firmware from ESP32 flash and save to $BACKUP_FILE"
        echo "  restore - Write firmware from $BACKUP_FILE to ESP32 flash"
        echo ""
        echo "Note: Adjust PORT, BAUD, FLASH_SIZE in the script as needed."
        echo "FLASH_SIZE examples: 2MB, 4MB, 8MB, 16MB"
        ;;
esac