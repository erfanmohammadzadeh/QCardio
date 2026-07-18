# ECG Processing Library Validation Framework

This project provides a comprehensive validation framework for ECG processing libraries and algorithms. The core requirement is that any ECG software must produce correct outputs when tested against physician-annotated signals from the MIT-BIH Arrhythmia Database.

## Overview

The framework transforms the MIT-BIH Arrhythmia Database into a structured, easy-to-use format suitable for automated testing and validation of ECG processing algorithms. It provides tools for:

- **Data Conversion**: Convert MIT-BIH records to structured CSV format
- **Algorithm Validation**: Compare your algorithm's output against reference annotations
- **Performance Metrics**: Calculate comprehensive metrics including TP, FP, FN, Precision, Recall, F1-Score, and Accuracy
- **Data Resampling**: Resample ECG signals to any desired sampling rate
- **Report Generation**: Export detailed validation reports in multiple formats

## Key Features

### 1. Data Management

- Automatic download and conversion of MIT-BIH database
- Customizable data structure for easy integration
- Support for multiple ECG leads (MLII, V5)
- Annotation preservation and conversion

### 2. Signal Processing

- Resample ECG signals to arbitrary sampling rates
- Filter and preprocess signals
- Extract specific record segments
- View and analyze raw signals

### 3. Validation Engine

- Compare algorithm outputs with reference annotations
- Calculate comprehensive metrics:
  - True Positives (TP)
  - False Positives (FP)
  - False Negatives (FN)
  - Precision
  - Recall/Sensitivity
  - F1-Score
  - Accuracy
  - Positive Predictive Value (PPV)
- Multi-beat type classification support

### 4. Reporting

- Export results to CSV format
- Generate detailed validation reports
- Visual comparison of results
- Statistical analysis of performance

## Installation

```bash
# Clone the repository
git clone https://github.com/erfan-mohammadzade/QCardio.git
cd QCardio
```