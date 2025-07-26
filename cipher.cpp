#include <iostream>

// ---------------- Global Constants and Variables ----------------
const int ROUNDS = 32;
const int BLOCK_SIZE = 64;
const int HALF_BLOCK_SIZE = BLOCK_SIZE / 2;
int sboxes[ROUNDS][256];         // Dynamic S-boxes
unsigned char roundKeys[ROUNDS]; // Scheduled round keys

// ---------------- Utility Functions ----------------

// Print a 64-bit number in binary format.
void printBinary(unsigned long long value) {
    for (int i = 63; i >= 0; --i) {
        unsigned long long bit = (value >> i) & 1; // Extract the i-th bit
        std::cout << bit;
        if (i % 8 == 0) std::cout << " "; // Add a space every 8 bits for readability
    }
    std::cout << "\n";
}

// Compute the length of a C-string.
int stringLength(const char* str) {
    int len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

// Print a 64-bit value in hexadecimal.
void printHex(unsigned long long value) {
    for (int i = 15; i >= 0; --i) {
        unsigned long long nibble = (value >> (i * 4)) & 0xF;
        std::cout << (nibble < 10 ? char('0' + nibble)
                                   : char('A' + (nibble - 10)));
    }
}

// ---------------- Complexity Reporting Function ----------------
// This function prints the exact time and space complexity for a function.
void printComplexity(const char* funcName, const char* timeComp, const char* spaceComp) {
    std::cout << "Function " << funcName << " Complexity: Time = " << timeComp 
              << ", Space = " << spaceComp << "\n";
}

// ---------------- Round-Key Generation ----------------
// Simple hash function for generating round keys.
unsigned long long simpleHash(unsigned long long base, int round) {
    unsigned long long hash = base ^ 0xA5A5A5A5A5A5A5A5ULL;
    for (int i = 0; i < 8; ++i) {
        hash = ((hash << 7) | (hash >> (64 - 7))) ^ (round * 157 + i * 73);
        hash ^= (hash << 11) ^ (hash >> 3);
    }
    return hash;
}

// Generate round keys from the user key.
void generateRoundKeys(unsigned long long key) {
    for (int i = 0; i < ROUNDS; ++i) {
        unsigned long long hashed = simpleHash(key, i);
        roundKeys[i] = (hashed >> ((i % 8) * 8)) & 0xFF;
    }
}

// Display the round keys (only once).
void displayRoundKeys() {
    std::cout << "\n========== ROUND KEYS ==========\n";
    for (int i = 0; i < ROUNDS; ++i) {
        std::cout << "Round " << (i + 1) << " Key: 0x";
        if ((int)roundKeys[i] < 16)
            std::cout << "0";
        std::cout << std::hex << (int)roundKeys[i] << std::dec << "\n";
    }
    std::cout << "================================\n\n";
}

// ---------------- Number Theory & S-box Functions ----------------
// Compute the modular inverse modulo 257.
int modInverse(int a) {
    for (int i = 1; i < 256; ++i) {
        if ((a * i) % 257 == 1)
            return i;
    }
    return 0;
}

// Generate the dynamic S-box for a given round.
void generateSBox(int round) {
    for (int i = 0; i < 256; ++i) {
        int val = (i + round * 17) % 256;
        int modInput = (val ^ 0x5F);
        if (modInput == 0)
            modInput = 1;  // Avoid zero input.
        sboxes[round][i] = modInverse(modInput);
    }
    // Print the generated S-box.
    std::cout << "S-box for Round " << (round + 1) << ":\n";
    for (int i = 0; i < 256; ++i) {
        std::cout << sboxes[round][i] << ((i % 16 == 15) ? "\n" : " ");
    }
    std::cout << "\n";
}

// ---------------- Permutation Functions ----------------
// Permutation step applied at the end of each encryption round.
void permutationStep(unsigned int &left, unsigned int &right) {
    left ^= (right >> 3);
    right ^= (left << 5);
    left = (left >> 16) | (left << 16);
    right = (right >> 8) | (right << 24);
}

// Inverse permutation step to exactly undo permutationStep.
void inversePermutationStep(unsigned int &left, unsigned int &right) {
    right = (right >> 24) | (right << 8);
    left = (left << 16) | (left >> 16);
    right ^= (left << 5);
    left ^= (right >> 3);
}

// ---------------- Feistel Network Functions ----------------
// The Feistel function uses the S-box for substitution and mixing.
unsigned int feistelFunction(unsigned int halfBlock, int round) {
    unsigned char a = (halfBlock >> 24) & 0xFF;
    unsigned char b = (halfBlock >> 16) & 0xFF;
    unsigned char c = (halfBlock >> 8) & 0xFF;
    unsigned char d = halfBlock & 0xFF;
    unsigned int res = sboxes[round][a] ^ sboxes[round][b];
    res ^= sboxes[round][c];
    res ^= sboxes[round][d];
    res = (res << 7) | (res >> (32 - 7));
    res ^= (res >> 16);
    res = (res << 3) | (res >> (32 - 3));
    return res;
}

// Encrypt the 64-bit block using the Feistel network.
void feistelEncrypt(unsigned long long &data) {
    unsigned int left = (data >> HALF_BLOCK_SIZE) & 0xFFFFFFFF;
    unsigned int right = data & 0xFFFFFFFF;

    std::cout << "\n====== ENCRYPTION PROCESS ======\n";
    for (int round = 0; round < ROUNDS; ++round) {
        unsigned int temp = right;
        right = left ^ feistelFunction(right, round);
        left = temp;
        permutationStep(left, right);
        std::cout << "Encryption Round " << (round + 1)
                  << ": Left Half = " << left << " (Binary: ";
        printBinary(left);
        std::cout << "), Right Half = " << right << " (Binary: ";
        printBinary(right);
        std::cout << ")\n";
    }
    data = ((unsigned long long)left << HALF_BLOCK_SIZE) | right;
    std::cout << "====== ENCRYPTION COMPLETE ======\n";

    std::cout << "Encrypted Hexadecimal Output: ";
    printHex(data);
    std::cout << "\nEncrypted Binary Output: ";
    printBinary(data);
    std::cout << "\n";
}



// Decrypt the 64-bit block by reversing the encryption process.
void feistelDecrypt(unsigned long long &data) {
    unsigned int left = (data >> HALF_BLOCK_SIZE) & 0xFFFFFFFF;
    unsigned int right = data & 0xFFFFFFFF;

    std::cout << "\n====== DECRYPTION PROCESS ======\n";
    for (int round = ROUNDS - 1; round >= 0; --round) {
        inversePermutationStep(left, right);
        unsigned int temp = right;
        right = left;
        left = temp ^ feistelFunction(left, round);
        std::cout << "Decryption Round " << (round + 1)
                  << ": Left Half = " << left << " (Binary: ";
        printBinary(left);
        std::cout << "), Right Half = " << right << " (Binary: ";
        printBinary(right);
        std::cout << ")\n";
    }
    data = ((unsigned long long)left << HALF_BLOCK_SIZE) | right;
    std::cout << "====== DECRYPTION COMPLETE ======\n";

    std::cout << "Decrypted Hexadecimal Output: ";
    printHex(data);
    std::cout << "\nDecrypted Binary Output: ";
    printBinary(data);
    std::cout << "\n";
}



// ---------------- Data Conversion Functions ----------------
// Convert an 8-character string into a 64-bit block (zero-padded).
unsigned long long stringTo64Bit(const char* input) {
    unsigned long long data = 0;
    int len = stringLength(input);
    for (int i = 0; i < 8; ++i) {
        data |= (i < len ? (unsigned long long)input[i] : 0)
                << (56 - i * 8);
    }
    return data;
}

// Convert a 64-bit block back into an 8-character string.
void longLongToString(unsigned long long data, char* output) {
    for (int i = 0; i < 8; ++i)
        output[i] = (data >> (56 - i * 8)) & 0xFF;
    output[8] = '\0';
}

// ---------------- Cryptanalysis Test Functions ----------------
// Avalanche Test: Flip one bit, encrypt both versions, and measure output bit differences.
void avalancheTest(const char* input) {
    unsigned long long original = stringTo64Bit(input);
    unsigned long long flipped = original ^ (1ULL << 0); // Flip LSB
    unsigned long long enc1 = original, enc2 = flipped;

    feistelEncrypt(enc1);
    feistelEncrypt(enc2);

    int diff = 0;
    for (int i = 0; i < 64; ++i) {
        if (((enc1 >> i) & 1) != ((enc2 >> i) & 1))
            diff++;
    }

    std::cout << "Avalanche Effect: " << diff << "/64 bits changed ("
              << (diff * 100 / 64) << "%)\n";

    std::cout << "Original Binary Output: ";
    printBinary(enc1);
    std::cout << "Flipped Binary Output: ";
    printBinary(enc2);
    std::cout << "\n";
}



// Differential Test: Encrypt two inputs differing by a small delta, then print the XOR difference.
void differentialTest(const char* input) {
    unsigned long long original = stringTo64Bit(input);
    unsigned long long delta = 0x0000000000000001ULL;
    unsigned long long input2 = original ^ delta;
    unsigned long long enc1 = original, enc2 = input2;
    feistelEncrypt(enc1);
    feistelEncrypt(enc2);
    unsigned long long diff = enc1 ^ enc2;
    std::cout << "Differential Output: ";
    printHex(diff);
    std::cout << "\n";
    std::cout << "Flipped Bit Positions: ";
    for (int i = 0; i < 64; ++i) {
        if (((diff >> i) & 1) == 1)
            std::cout << i << " ";
    }
    std::cout << "\n";
    printComplexity("Differential Test", "64 iterations", "Constant (O(1))");
}

// Linear Test: Compute the XOR of input and output bits.
void linearTest(const char* input) {
    unsigned long long original = stringTo64Bit(input);
    unsigned long long enc = original;
    feistelEncrypt(enc);
    int inputXOR = 0, outputXOR = 0;
    for (int i = 0; i < 64; ++i) {
        inputXOR ^= (original >> i) & 1;
        outputXOR ^= (enc >> i) & 1;
    }
    std::cout << "Linear Correlation: XOR(InputBits) = " << inputXOR
              << ", XOR(OutputBits) = " << outputXOR << "\n";
    std::cout << (inputXOR == outputXOR ? "High Correlation\n" : "Low Correlation\n");
    printComplexity("Linear Test", "64 iterations", "Constant (O(1))");
}

// ---------------- Main Function with a Switch-Case Menu ----------------
int main() {
    unsigned long long userKey = 0;
    std::cout << "Enter 16-character hex key (64-bit): ";
    char keyInput[17];
    std::cin.getline(keyInput, 17);
    // Input Validation
    if (stringLength(keyInput) != 16) {
        std::cout << "Error: Key must be exactly 16 hexadecimal characters.\n";
        return 0;
    }
    for (int i = 0; i < 16; ++i) {
        char c = keyInput[i];
        userKey <<= 4;
        if (c >= '0' && c <= '9')
            userKey |= c - '0';
        else if (c >= 'A' && c <= 'F')
            userKey |= c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            userKey |= c - 'a' + 10;
    }
    
    // Generate and display round keys once.
    generateRoundKeys(userKey);
    displayRoundKeys();
    
    while (true) {
        std::cout << "\n=== Symmetric Block Cipher Menu ===\n";
        std::cout << "1. Encrypt\n";
        std::cout << "2. Decrypt\n";
        std::cout << "3. Avalanche Effect\n";
        std::cout << "4. Differential Test\n";
        std::cout << "5. Linear Approximation\n";
        std::cout << "6. Exit\n";
        std::cout << "Select option: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore();  // Remove newline
        
        char input[17];
        unsigned long long data;
        switch (choice) {
            case 1: {
                std::cout << "Enter plaintext (max 8 chars): ";
                std::cin.getline(input, 9);
                int len = stringLength(input);
                for (int i = len; i < 8; ++i)
                    input[i] = ' ';
                input[8] = '\0';
                data = stringTo64Bit(input);
                std::cout << "Original Data: ";
                printHex(data);
                std::cout << "\n";
                feistelEncrypt(data);
                std::cout << "Encrypted Data: ";
                printHex(data);
                std::cout << "\n";
                break;
            }
            case 2: {
                std::cout << "Enter encrypted hex (16 chars): ";
                std::cin.getline(input, 17);
                data = 0;
                for (int i = 0; i < 16; ++i) {
                    char c = input[i];
                    data <<= 4;
                    if (c >= '0' && c <= '9')
                        data |= c - '0';
                    else if (c >= 'A' && c <= 'F')
                        data |= c - 'A' + 10;
                    else if (c >= 'a' && c <= 'f')
                        data |= c - 'a' + 10;
                }
                feistelDecrypt(data);
                {
                    char out[9];
                    longLongToString(data, out);
                    int dlen = 8;
                    while (dlen > 0 && out[dlen - 1] == ' ')
                        dlen--;
                    out[dlen] = '\0';
                    std::cout << "Decrypted Text: \"" << out << "\"\n";
                }
                break;
            }
            case 3: {
                std::cout << "Enter input (8 chars): ";
                std::cin.getline(input, 9);
                avalancheTest(input);
                break;
            }
            case 4: {
                std::cout << "Enter input (8 chars): ";
                std::cin.getline(input, 9);
                differentialTest(input);
                break;
            }
            case 5: {
                std::cout << "Enter input (8 chars): ";
                std::cin.getline(input, 9);
                linearTest(input);
                break;
            }
            case 6: {
                std::cout << "Exiting...\n";
                return 0;
            }
            default:
                std::cout << "Invalid option.\n";
                break;
        }
    }
    
    return 0;
}