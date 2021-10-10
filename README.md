# Experimental data acquisition and processingsystem for ECG signals

```
📦DAQ_ECG
  ┣ 📂diagrams
 ┃ ┗ 📜diagrams.drawio
 ┣ 📂ESP32
 ┃ ┣ 📂ECG_daq
 ┃ ┃ ┣ 📂include
 ┃ ┃ ┃ ┗ 📜main.h
 ┃ ┃ ┣ 📂lib
 ┃ ┃ ┣ 📂src
 ┃ ┃ ┃ ┣ 📜CMakeLists.txt
 ┃ ┃ ┃ ┗ 📜main.c
 ┃ ┃ ┣ 📜.gitignore
 ┃ ┃ ┣ 📜CMakeLists.txt
 ┃ ┃ ┣ 📜lbcrypto-doxy-config
 ┃ ┃ ┣ 📜platformio.ini
 ┃ ┃ ┗ 📜sdkconfig.esp32dev
 ┃ ┗ 📂ECG_generator
 ┃ ┃ ┣ 📂include
 ┃ ┃ ┃ ┗ 📜main.h
 ┃ ┃ ┣ 📂lib
 ┃ ┃ ┣ 📂src
 ┃ ┃ ┃ ┣ 📜CMakeLists.txt
 ┃ ┃ ┃ ┗ 📜main.c
 ┃ ┃ ┣ 📜.gitignore
 ┃ ┃ ┣ 📜CMakeLists.txt
 ┃ ┃ ┣ 📜lbcrypto-doxy-config
 ┃ ┃ ┣ 📜platformio.ini
 ┃ ┃ ┗ 📜sdkconfig.esp32dev
 ┣ 📂latex
 ┃ ┣ 📂images
 ┃ ┃ ┗ 📜cardiac_signal.JPG
 ┃ ┣ 📂text
 ┃ ┃ ┣ 📂body
 ┃ ┃ ┃ ┣ 📜conclusions.tex
 ┃ ┃ ┃ ┣ 📜discussions.tex
 ┃ ┃ ┃ ┣ 📜introducion.tex
 ┃ ┃ ┃ ┣ 📜methodology.tex
 ┃ ┃ ┃ ┣ 📜results.tex
 ┃ ┃ ┃ ┗ 📜specifications.tex
 ┃ ┃ ┣ 📂pos
 ┃ ┃ ┃ ┗ 📜appendix.tex
 ┃ ┃ ┣ 📂pre
 ┃ ┃ ┃ ┣ 📜abstract.tex
 ┃ ┃ ┃ ┣ 📜cover.tex
 ┃ ┃ ┃ ┗ 📜tables.tex
 ┃ ┃ ┣ 📜body.tex
 ┃ ┃ ┣ 📜pos.tex
 ┃ ┃ ┗ 📜pre.tex
 ┃ ┣ 📜bib.bib
 ┃ ┗ 📜main.tex
 ┣ 📂LTSpice
 ┃ ┣ 📜ecg_acquisition.asc
 ┃ ┣ 📜ecg_simulation.asc
 ┣ 📂python
 ┃ ┣ 📂Functional script
 ┃ ┃ ┗ 📜usb_client.py
 ┃ ┣ 📂Test scripts
 ┃ ┃ ┣ 📜data_char.txt
 ┃ ┃ ┣ 📜data_float.txt
 ┃ ┃ ┣ 📜ecg_acquisition_frequency.txt
 ┃ ┃ ┣ 📜ecg_acquisition_time.txt
 ┃ ┃ ┣ 📜ecg_dsp.ipynb
 ┃ ┃ ┣ 📜ecg_generator.ipynb
 ┃ ┃ ┣ 📜ecg_generator.py
 ┃ ┃ ┣ 📜ecg_heart_rate.py
 ┃ ┃ ┣ 📜ecg_ltspice.ipynb
 ┃ ┃ ┗ 📜filters.ipynb
 ┣ 📜.gitattributes
 ┣ 📜.gitignore
 ┗ 📜README.md
 ```