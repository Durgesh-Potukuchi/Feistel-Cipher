# Symmetric Block Cipher in C++

![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

This repository contains an educational implementation of a custom 64-bit symmetric block cipher. It was developed to explore and demonstrate the core principles of modern cryptography, including key scheduling, confusion/diffusion, and basic cryptanalysis, all within a C++ environment.

The cipher is built upon a **32-round Feistel network** and features dynamically generated S-boxes for each round, making it a unique study in cipher design.

---

## ✨ Features

* **64-bit Encryption & Decryption:** Encrypts and decrypts 8-character (64-bit) data blocks.
* **64-bit Keys:** Utilizes a 16-character hexadecimal string as the master key.
* **Dynamic S-Box Generation:** Creates a unique Substitution Box (S-Box) for each round using the modular multiplicative inverse in the prime field $GF(257)$, providing strong, key-dependent non-linearity.
* **Custom Key Scheduling:** Derives 32 unique round keys from the master key using a simple, effective hashing function.
* **Interactive Menu:** A user-friendly command-line interface for easy operation.
* **Cryptanalysis Suite:** Includes basic tests to evaluate the cipher's cryptographic strength:
    * **Avalanche Effect Test:** Measures how single-bit changes in the input drastically alter the output.
    * **Differential Test:** Examines how input differences propagate through the cipher.
    * **Linear Test:** Performs a basic check for linear relationships between input and output bits.

---

## 🏗️ Project Structure

The entire implementation is contained within `cipher.cpp` and is organized into several functional blocks:

* **Global Constants & Variables:** Defines core parameters like `ROUNDS`, `BLOCK_SIZE`, and global arrays for `sboxes` and `roundKeys`.
* **Utility Functions:** Helper functions for printing data in binary and hexadecimal formats.
* **Round-Key Generation:** The `simpleHash` and `generateRoundKeys` functions responsible for creating the key schedule.
* **Number Theory & S-box Functions:** The `modInverse` and `generateSBox` functions that create the dynamic substitution boxes for each round.
* **Permutation Functions:** The `permutationStep` and its inverse, which provide diffusion by mixing the data block's halves.
* **Feistel Network Functions:** The core `feistelFunction`, `feistelEncrypt`, and `feistelDecrypt` that implement the main cipher logic.
* **Cryptanalysis Test Functions:** Standalone functions for the avalanche, differential, and linear tests.
* **Main Function:** Contains the user menu, input/output handling, and orchestrates the calls to the various cipher functions.

---

## ⚙️ How to Compile and Run

#### Prerequisites
You will need a C++ compiler, such as **g++** (part of the GCC toolchain).

#### Compilation
Navigate to the project directory in your terminal and run the following command:
```bash
g++ cipher.cpp -o cipher -std=c++11
```

#### Execution
To run the compiled program, use the following command:
```bash
./cipher
```
The program will first prompt you for a 16-character hexadecimal key and then display the main menu for you to select an operation.

---

## 🎓 Key Improvements & Learnings

This project evolved through debugging and refinement, leading to important improvements and takeaways.

1.  **Robust Input Handling:**
    * **Change:** The original code entered an infinite loop if the user provided non-numeric input for the menu selection.
    * **Improvement:** The main loop was updated to validate user input. It now checks `std::cin`'s error state, clears it, and discards the invalid input from the buffer, making the program resilient to user error.

2.  **Defensive Programming:**
    * **Change:** The custom `stringLength()` function was inherently unsafe, creating a risk of buffer overflow if passed a non-null-terminated string.
    * **Improvement:** While not triggered in this specific program, this highlighted a key security principle. Robust functions should not rely on callers for safety but should internally protect against invalid data.





