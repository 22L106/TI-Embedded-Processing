# 🔧 TI-Zynq AFE SPI+USB Framework (Private Repo)

> Industry-mentored embedded project under NDA with Texas Instruments.  
> This repository contains the collaborative firmware and driver development for SPI-based communication between Zynq SoC and multiple TI AFE devices, with USB-based PC interface support.

---

## 🚀 Overview

This project is a part of an exclusive Texas Instruments – PSG College of Technology collaboration under the Embedded Systems vertical. It aims to build an end-to-end firmware framework that bridges:

- 🧠 **Zynq SoC (ARM PS)** ↔ **TI Analog Front-End Devices (DUTs)** via SPI
- 💻 **PC/Host system** ↔ **Zynq** via USB Command Interface

Key objectives include:
- ✔️ Custom SPI driver with support for **sequential** and **broadcast** communication
- ✔️ Fast wake-up and configuration of **multiple DUTs**
- ✔️ Seamless integration of **TI AFE API**
- ✔️ Zynq acting as a **USB device**, accepting PC commands to control DUTs

---

## 📘 Problem Statements

### 🧩 Milestone 1 — SPI Driver + TI AFE API Integration
> **"Develop an SPI driver (sequential/broadcast read/write) to enable fast wake-up of multiple DUTs. Interface with TI AFE API."**

You will build a modular SPI driver on Zynq PS that:
- Supports multiple chip selects
- Handles wake-up/config sequence for each DUT
- Is compatible with the TI AFE API (provided under NDA)

---

### 🧩 Milestone 2 — USB ↔ Zynq Command Bridge
> **"Develop interface between Zynq and the PC to be able to send/receive commands via USB. Integrate TI APIs in Zynq and trigger them from PC."**

The Zynq PS must act as a USB device, parsing incoming commands from the host PC to:
- Trigger AFE SPI commands
- Report status or data (if needed)
- Develop an interface with ZYNQ so as to communicate directly eith TI AFE end devices.


---

## 👨‍💻 Team

Akshay Muthu Shankar G
Darshana R
Shakti Lakshmi 
Bhavaneeshwari

---


