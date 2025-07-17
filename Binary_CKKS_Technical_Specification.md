# Binary CKKS Technical Specification

## Abstract

This document provides a comprehensive technical specification for the Binary CKKS homomorphic encryption scheme, a variant of the CKKS scheme tailored for approximate computation over Gaussian integers with binary polynomial encoding. The scheme enables stable and deep homomorphic evaluation using binary arithmetic while maintaining semantic security.

---

## 1. Mathematical Foundation

### 1.1 Algebraic Structures

**Ring Definition:**
- Primary ring: $R = \mathbb{Z}[X]/(X^M + 1)$ where $M = 2^k$
- Binary polynomial ring: $BP$ (binary representation of $R$)
- Quotient ring: $R_q = R/qR$ for prime $q$

**Canonical Embedding:**
- Maps ring elements to complex vectors via $\sigma: R \to \mathbb{C}^{M/2}$
- Preserves algebraic operations for homomorphic computation

### 1.2 Binary Polynomial Encoding

**Transformation Function $p^{-1}$:**
- Converts standard polynomial coefficients to binary representation
- Maps $\mathbb{Z}_q \to \{0,1\}^{\lceil \log_2 q \rceil}$
- Inverse operation $p$ reconstructs original coefficients

**Key Innovation:**
The binary encoding reduces storage requirements by ~98% while maintaining homomorphic properties through specialized arithmetic operations.

---

## 2. Scheme Specification

### 2.1 Key Generation Algorithm

**Input:** Security parameter $\lambda$

**Parameter Selection:**
1. Choose power-of-two $M = M(\lambda, q_L)$
2. Select integer $h = h(\lambda, q_L)$ (Hamming weight)
3. Choose integer $P = P(\lambda, q_L)$ 
4. Select real $\sigma = \sigma(\lambda, q_L)$ (noise parameter)

**Key Generation Steps:**
1. **Secret Key:**
   - Sample $s \leftarrow \text{HWT}(h)$ (sparse ternary polynomial)
   - Apply binary encoding: $\mathsf{sk} \leftarrow (1, p^{-1}(s))$

2. **Public Key:**
   - Sample $a \leftarrow BP$ (uniformly random binary polynomial)
   - Sample $e \leftarrow \text{DG}(\sigma^2)$ (discrete Gaussian noise)
   - Apply encoding: $e \leftarrow p^{-1}(e)$
   - Compute: $b \leftarrow -a \cdot s + e$ (in binary arithmetic)
   - Set: $\mathsf{pk} \leftarrow (b, a) \in BP^2$

3. **Evaluation Key:**
   - Sample $a_0 \leftarrow BP$ and $e_0 \leftarrow \text{DG}(\sigma^2)$
   - Apply encoding: $e_0 \leftarrow p^{-1}(e_0)$
   - Compute: $b_0 \leftarrow -a_0 \cdot s + e_0 + s^2$ (binary operations)
   - Set: $\mathsf{evk} \leftarrow (b_0, a_0) \in BP^2$

### 2.2 Encoding Algorithm

**Input:** Vector $z = (z_j)_{j \in T} \in \mathbb{Z}[i]^{N/2}$ of Gaussian integers, scaling factor $\Delta$

**Encoding Process:**
1. Compute scaled canonical embedding: $\lfloor \Delta \cdot \pi^{-1}(z) \rceil_{\sigma(R)}$
2. Apply inverse canonical embedding to obtain polynomial
3. Transform to binary representation: $m(X) = p^{-1}(\text{result})$

**Output:** Binary polynomial $m(X) \in BP$

### 2.3 Decoding Algorithm

**Input:** Binary polynomial $m(X) \in BP$, scaling factor $\Delta$

**Decoding Process:**
1. Convert from binary: $\hat{m} = p(m(X))$
2. Apply canonical embedding: $\pi \circ \sigma(\hat{m})$
3. Scale and round: $z_j = \lfloor \Delta^{-1} \cdot \hat{m}(\zeta_M^j) \rceil$

**Output:** Vector $z = (z_j)_{j \in T}$ of approximated Gaussian integers

### 2.4 Encryption Algorithm

**Input:** Encoded message $m(X) \in BP$, public key $\mathsf{pk} = (b, a)$

**Encryption Process:**
1. Sample randomness: $v \leftarrow \text{ZO}(0.5)$ (centered binomial)
2. Sample noise: $e_0, e_1 \leftarrow \text{DG}(\sigma^2)$
3. Apply binary encoding: $v, e_0, e_1 \leftarrow p^{-1}(v), p^{-1}(e_0), p^{-1}(e_1)$
4. Compute ciphertext components:
   - $c_0 = v \cdot b + m(X) + e_0$ (binary arithmetic)
   - $c_1 = v \cdot a + e_1$ (binary arithmetic)

**Output:** Ciphertext $\mathsf{ct} = (c_0, c_1) \in BP^2$

### 2.5 Decryption Algorithm

**Input:** Ciphertext $\mathsf{ct} = (c_0, c_1)$, secret key $\mathsf{sk} = (1, s)$

**Decryption Process:**
1. Compute inner product: $m'(X) = c_0 + c_1 \cdot s$ (binary arithmetic)
2. Handle modular reduction and noise removal

**Output:** Recovered message $m'(X) \approx m(X)$

---

## 3. Homomorphic Operations

### 3.1 Addition Operation

**Input:** Ciphertexts $\mathsf{ct}_1 = (c_{1,0}, c_{1,1})$, $\mathsf{ct}_2 = (c_{2,0}, c_{2,1})$

**Process:**
- Component-wise addition in binary polynomial ring
- $\text{Add}(\mathsf{ct}_1, \mathsf{ct}_2) = (c_{1,0} + c_{2,0}, c_{1,1} + c_{2,1})$

**Properties:**
- Minimal noise growth
- Constant-time operation
- Preserves circuit depth

### 3.2 Multiplication Operation

**Input:** Ciphertexts $\mathsf{ct}_1$, $\mathsf{ct}_2$

**Process:**
1. Tensor product multiplication in binary arithmetic
2. Relinearization using evaluation key $\mathsf{evk}$
3. Degree reduction to maintain ciphertext size

**Complexity:**
- Requires evaluation key for relinearization
- Increases noise according to binary noise growth bounds
- More complex than addition but maintains homomorphic properties

### 3.3 Refresh Operation

**Purpose:** Reset accumulated noise in ciphertexts during deep evaluation

**Process:**
1. Detect when noise approaches decryption threshold
2. Apply lightweight noise reduction technique
3. Maintain plaintext while reducing error component

**Benefits:**
- Enables deeper homomorphic evaluation chains
- Essential for practical binary arithmetic applications
- Optimized for binary representation efficiency

---

## 4. Security Analysis

### 4.1 Hardness Assumptions

**Ring Learning With Errors (RLWE):**
- Security reduces to RLWE problem in binary polynomial rings
- Binary representation maintains cryptographic hardness
- Parameters chosen to resist known attacks

**Key Security Properties:**
- **Semantic Security:** Indistinguishable encryption under chosen plaintext attacks
- **Circuit Privacy:** Homomorphic operations don't leak computation information
- **Noise Flooding:** Strategic noise addition prevents side-channel attacks

### 4.2 Parameter Selection

**Security Level λ:**
- Determines all scheme parameters
- Balances security, efficiency, and correctness
- Follows current cryptographic standards (λ = 128, 192, 256)

**Noise Parameters:**
- $\sigma$ chosen to ensure decryption correctness
- Balanced against security requirements
- Accounts for binary representation noise characteristics

### 4.3 Binary-Specific Considerations

**Coefficient Distribution:**
- Binary coefficients change noise analysis
- Require modified security parameter selection
- May affect resistance to lattice attacks

**Homomorphic Noise Growth:**
- Binary operations have different noise propagation
- Careful analysis needed for deep evaluation bounds
- Refresh operation timing critical for security

---

## 5. Performance Characteristics

### 5.1 Computational Complexity

**Key Generation:** $O(M \log M)$
- Dominated by polynomial operations
- Binary conversion adds constant factors
- FFT-based arithmetic for efficiency

**Encryption:** $O(M \log M)$ 
- Similar to standard CKKS
- Binary operations potentially faster
- Noise sampling remains bottleneck

**Homomorphic Operations:**
- **Addition:** $O(M)$ (coefficient-wise XOR)
- **Multiplication:** $O(M \log M)$ with relinearization
- **Refresh:** $O(M \log M)$ for noise reduction

### 5.2 Space Complexity

**Dramatic Storage Reduction:**
- Standard CKKS: $\sim 64M$ bits per ciphertext
- Binary CKKS: $\sim M$ bits per ciphertext
- **98.4% space savings** in theoretical optimized implementation

**Memory Access Patterns:**
- Binary representation improves cache efficiency
- Reduced memory bandwidth requirements
- Better suited for constrained environments

### 5.3 Practical Considerations

**Implementation Optimizations:**
- Bit-packing for actual storage reduction
- SIMD instructions for parallel binary operations
- Hardware acceleration opportunities

**Trade-offs:**
- Reduced precision compared to standard CKKS
- Increased computational overhead for binary conversion
- Different noise characteristics requiring analysis

---

## 6. Applications and Use Cases

### 6.1 Ideal Applications

**Resource-Constrained Environments:**
- IoT devices with limited memory
- Mobile devices requiring efficient encryption
- Embedded systems with storage constraints

**High-Throughput Scenarios:**
- Server environments processing many encryptions
- Network applications with bandwidth limitations
- Batch processing of homomorphic operations

### 6.2 Practical Implementations

**Database Operations:**
- Private database queries with space efficiency
- Encrypted data analytics with reduced storage
- Secure aggregation with minimal overhead

**Machine Learning:**
- Privacy-preserving model inference
- Secure federated learning with communication efficiency
- Encrypted neural network evaluation

---

## 7. Implementation Guidelines

### 7.1 Parameter Recommendations

**For Security Level λ = 128:**
```
M = 2^12 = 4096
h = 64
σ = 3.2  
q ≈ 2^54
```

**For Security Level λ = 192:**
```
M = 2^13 = 8192
h = 96
σ = 3.2
q ≈ 2^81  
```

### 7.2 Optimization Strategies

**Binary Arithmetic:**
- Use bitwise operations wherever possible
- Implement efficient bit-packing schemes
- Leverage hardware acceleration (SIMD, GPU)

**Memory Management:**
- Minimize memory allocations during operations
- Implement in-place operations when safe
- Consider cache-friendly data layouts

**Noise Management:**
- Monitor noise levels throughout computation
- Implement adaptive refresh strategies
- Use fresh encryptions when noise becomes critical

---

## 8. Future Research Directions

### 8.1 Theoretical Improvements

**Enhanced Binary Encoding:**
- More sophisticated binary representations
- Hybrid approaches balancing precision and efficiency
- Adaptive encoding based on computation requirements

**Advanced Error Correction:**
- Integration of more powerful error correction codes
- Syndrome-based decoding for binary coefficients
- Adaptive error correction strength

### 8.2 Implementation Enhancements

**Hardware Acceleration:**
- Custom silicon for binary homomorphic operations
- GPU acceleration for parallel binary arithmetic
- Quantum-resistant parameter selection

**Protocol Development:**
- Network protocols optimized for binary ciphertexts
- Secure multiparty computation with binary CKKS
- Threshold encryption schemes

---

## Conclusion

The Binary CKKS scheme represents a significant advancement in homomorphic encryption, offering dramatic space savings while maintaining cryptographic security and homomorphic functionality. The binary polynomial encoding enables new applications in resource-constrained environments and high-throughput scenarios.

Key contributions include:
- **98%+ space reduction** over standard CKKS
- **Maintained security** under RLWE assumptions
- **Efficient homomorphic operations** in binary arithmetic
- **Practical applicability** to real-world constrained environments

The scheme opens new research directions in efficient homomorphic encryption and provides a foundation for next-generation privacy-preserving applications.

---

*This specification is based on the concrete construction outlined in the provided binary CKKS scheme description and experimental validation.*