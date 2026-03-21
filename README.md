# Akshara-Pi HD Carta
A DIY E-Reader Framework for ESP32 S3 hardware utilising Good Display HD Carta 5.76 epaper screen

Akshara-Pi is an open-source hardware-software framework designed to create a distraction-free, high-fidelity e-reading experience. Unlike traditional e-readers that rely on heavy, generic operating systems, Akshara-Pi uses a Lean-Architecture approach. It leverages a ESP32 S3 as a dedicated rendering engine, pushing optimized bitmaps to a high-contrast E-ink display (specifically optimized for the 5.76" ).

The core philosophy is "The Human-AI Handshake": using Generative AI (Google Gemini) to architect complex C-pointer logic and SPI timing, while the human "Maker" focuses on typography, ergonomics, and system state-flow.

🏛️ **System Architecture**
The project follows a Decoupled Rendering Pipeline to maximize the limited resources of the ESP32 S3 N8R2

1. The Ingestion Layer 
Role: Handles the "heavy lifting" of document parsing.

Logic: When a  EPUB is added, a background script pre-renders pages into raw 1-bit or 4-bit bitmaps.

Benefit: Eliminates on-the-fly rendering latency during reading.

2. The Logic Layer (C / State Machine)
Role: Manages device states (Library, Reader, Admin).

Logic: A robust C-based state machine handles hardware interrupts from the 5-way navigation switch and manages the framebuffer.

3. The Hardware Abstraction Layer (SPI / LUT)
Role: Communicates with the E-ink Controller.

Logic: Uses custom Look-Up Tables (LUTs) to achieve sub-second partial refreshes, bypassing the slow "global flash" of standard drivers.
