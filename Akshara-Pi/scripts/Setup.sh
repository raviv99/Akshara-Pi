#!/bin/bash

# Akshara-Pi Dependency Setup Script
# This script installs the necessary libraries for MuPDF rendering and GPIO control.

echo "🚀 Starting Akshara-Pi environment setup..."

# Update system package list
sudo apt-get update

echo "📦 Installing build essentials and SPI tools..."
sudo apt-get install -y build-essential git cmake pkg-config raspberrypi-kernel-headers

echo "📚 Installing MuPDF and rendering libraries..."
# These are the core engines for PDF/EPUB processing
sudo apt-get install -y libmupdf-dev libmupdf-third-dev libfreetype6-dev libjbig2dec0-dev libopenjp2-7-dev libjpeg-dev

echo "🕹️ Installing hardware interface libraries..."
# lgpio is the modern standard for Pi Zero 2W and Bookworm OS
sudo apt-get install -y liblgpio-dev

echo "✅ Dependencies installed successfully!"
echo "Next step: Enable SPI via 'sudo raspi-config' before running the framework."