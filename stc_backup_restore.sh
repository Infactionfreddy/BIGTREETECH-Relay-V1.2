#!/bin/bash

# Script to backup and restore firmware for STC15W201S
# Requires stcgal to be installed: pip install stcgal

# Configuration
PORT="/dev/ttyUSB0"  # Adjust to your serial port
BAUD=9600
BACKUP_FILE="firmware_backup.hex"

# Function to backup firmware
backup() {
    echo "Backing up firmware from STC15W201S..."
    stcgal -p $PORT -b $BAUD -r $BACKUP_FILE
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
    echo "Restoring firmware to STC15W201S..."
    stcgal -p $PORT -b $BAUD -w $BACKUP_FILE
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
        echo "  backup  - Read firmware from controller and save to $BACKUP_FILE"
        echo "  restore - Write firmware from $BACKUP_FILE to controller"
        ;;
esac