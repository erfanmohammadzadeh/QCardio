# AHA Dataset Conversion Guide

This document explains how to convert the AHA (American Heart Association) database into the standard WFDB (MIT-BIH) format required by QCardio and other PhysioNet-compatible tools.

---

## 1. Prerequisites

### Install WFDB Software Package

Download and install the WFDB Software Package from the official PhysioNet repository:

> **Source:** [WFDP_Software - PhysioNet](https://www.physionet.org/content/wfdb/10.7.0/)

Follow the installation instructions for your operating system (Linux, macOS, or Windows).

---

## 2. Run script

Edit shell script and specify input AHA dataset and an output for store output path.

use
```
chmod +x ./convert_AHA2MIT
```
to set execute permission.
Run it to convert dataset from AHA to MIT-BIH.