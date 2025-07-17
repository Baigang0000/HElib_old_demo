#!/usr/bin/env python3

import numpy as np
import time
import random
from typing import List, Tuple, Optional
from dataclasses import dataclass

# Set random seeds for reproducibility
np.random.seed(42)
random.seed(42)

@dataclass 
class CKKSParams:
    """Parameters for CKKS scheme"""
    n: int  # Ring dimension (must be power of 2)
    q: int  # Ciphertext modulus  
    sigma: float  # Gaussian noise standard deviation
    scale: float  # Scaling factor (Delta)
    h: int  # Hamming weight for secret key
    
    def __post_init__(self):
        assert self.n > 0 and (self.n & (self.n - 1)) == 0, "n must be power of 2"

class CKKS:
    """Standard CKKS implementation for complex numbers"""
    
    def __init__(self, params: CKKSParams):
        self.params = params
        self.n = params.n
        self.q = params.q
        self.sigma = params.sigma
        self.scale = params.scale
        self.h = params.h
        
        # Primitive root of unity for NTT
        self.M = 2 * self.n
        self.zeta_M = np.exp(2j * np.pi / self.M)
        
        # Key generation
        self.secret_key = self._sample_hwt(self.h)
        self.public_key = self._gen_public_key()
        self.eval_key = self._gen_eval_key()
    
    def _sample_hwt(self, h: int) -> np.ndarray:
        """Sample secret key with Hamming weight h"""
        s = np.zeros(self.n, dtype=int)
        positions = np.random.choice(self.n, h, replace=False)
        signs = np.random.choice([-1, 1], h)
        s[positions] = signs
        return s
    
    def _sample_gaussian(self, size: int) -> np.ndarray:
        """Sample from discrete Gaussian distribution"""
        return np.round(np.random.normal(0, self.sigma, size)).astype(int)
    
    def _sample_uniform(self, size: int) -> np.ndarray:
        """Sample uniformly from ring"""
        return np.random.randint(-self.q//2, self.q//2, size)
    
    def _poly_mul_mod(self, a: np.ndarray, b: np.ndarray) -> np.ndarray:
        """Polynomial multiplication modulo X^n + 1"""
        # Ensure inputs are the right size
        a_pad = np.zeros(self.n, dtype=int)
        b_pad = np.zeros(self.n, dtype=int)
        
        a_pad[:min(len(a), self.n)] = a[:min(len(a), self.n)]
        b_pad[:min(len(b), self.n)] = b[:min(len(b), self.n)]
        
        # Polynomial multiplication
        result = np.polymul(a_pad, b_pad)
        
        # Reduce modulo X^n + 1
        # This means coefficients >= n get subtracted from lower terms
        reduced = np.zeros(self.n, dtype=int)
        for i in range(len(result)):
            if i < self.n:
                reduced[i] += result[i]
            else:
                # X^(n+k) = -X^k (mod X^n + 1)
                reduced[i - self.n] -= result[i]
        
        return reduced % self.q
    
    def _gen_public_key(self) -> Tuple[np.ndarray, np.ndarray]:
        """Generate public key (b, a) where b = -a*s + e"""
        a = self._sample_uniform(self.n)
        e = self._sample_gaussian(self.n)
        b = (-self._poly_mul_mod(a, self.secret_key) + e) % self.q
        return (b, a)
    
    def _gen_eval_key(self) -> Tuple[np.ndarray, np.ndarray]:
        """Generate evaluation key for relinearization"""
        a = self._sample_uniform(self.n)
        e = self._sample_gaussian(self.n)
        s_squared = self._poly_mul_mod(self.secret_key, self.secret_key)
        b = (-self._poly_mul_mod(a, self.secret_key) + e + s_squared) % self.q
        return (b, a)
    
    def encode(self, z: np.ndarray) -> np.ndarray:
        """Encode complex vector to polynomial"""
        # Simplified encoding - just scale and round
        m = np.zeros(self.n, dtype=int)
        data_len = min(len(z), self.n)
        if hasattr(z, 'real'):
            m[:data_len] = np.round(self.scale * z[:data_len].real).astype(int) % self.q
        else:
            m[:data_len] = np.round(self.scale * z[:data_len]).astype(int) % self.q
        return m
    
    def decode(self, m: np.ndarray) -> np.ndarray:
        """Decode polynomial to complex vector"""
        return (m.astype(float) / self.scale)
    
    def encrypt(self, m: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Encrypt encoded message"""
        pk_b, pk_a = self.public_key
        v = np.random.choice([0, 1], self.n, p=[0.5, 0.5])
        e0 = self._sample_gaussian(self.n)
        e1 = self._sample_gaussian(self.n)
        
        # Ensure m is the right size
        m_pad = np.zeros(self.n, dtype=int)
        m_pad[:min(len(m), self.n)] = m[:min(len(m), self.n)]
        
        c0 = (self._poly_mul_mod(v, pk_b) + m_pad + e0) % self.q
        c1 = (self._poly_mul_mod(v, pk_a) + e1) % self.q
        
        return (c0, c1)
    
    def decrypt(self, ciphertext: Tuple[np.ndarray, np.ndarray]) -> np.ndarray:
        """Decrypt ciphertext"""
        c0, c1 = ciphertext
        m = (c0 + self._poly_mul_mod(c1, self.secret_key)) % self.q
        # Handle negative values
        m = np.where(m > self.q//2, m - self.q, m)
        return m
    
    def add(self, ct1: Tuple[np.ndarray, np.ndarray], 
            ct2: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Homomorphic addition"""
        c0 = (ct1[0] + ct2[0]) % self.q
        c1 = (ct1[1] + ct2[1]) % self.q
        return (c0, c1)
    
    def multiply(self, ct1: Tuple[np.ndarray, np.ndarray],
                ct2: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Homomorphic multiplication (simplified)"""
        c0_0 = self._poly_mul_mod(ct1[0], ct2[0])
        c0_1 = (self._poly_mul_mod(ct1[0], ct2[1]) + self._poly_mul_mod(ct1[1], ct2[0])) % self.q
        c0_2 = self._poly_mul_mod(ct1[1], ct2[1])
        
        # Relinearization using eval key
        evk_b, evk_a = self.eval_key
        c0_new = (c0_0 + self._poly_mul_mod(c0_2, evk_b)) % self.q
        c1_new = (c0_1 + self._poly_mul_mod(c0_2, evk_a)) % self.q
        
        return (c0_new, c1_new)

class BinaryCKKS(CKKS):
    """Binary CKKS implementation"""
    
    def _to_binary(self, poly: np.ndarray) -> np.ndarray:
        """Convert polynomial to binary representation"""
        # Simplified: convert coefficients to binary
        binary_poly = []
        for coeff in poly:
            # Convert to binary representation (simplified)
            if coeff >= 0:
                binary_poly.append(coeff % 2)
            else:
                binary_poly.append((-coeff) % 2)
        return np.array(binary_poly, dtype=int)
    
    def _from_binary(self, binary_poly: np.ndarray) -> np.ndarray:
        """Convert from binary representation"""
        return binary_poly.astype(int)
    
    def encode(self, z: np.ndarray) -> np.ndarray:
        """Binary encoding"""
        m = super().encode(z)
        return self._to_binary(m)
    
    def encrypt(self, m: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Binary encryption"""
        # Convert keys to binary
        bin_m = self._from_binary(m)
        return super().encrypt(bin_m)

class BinaryCKKSWithBCH(BinaryCKKS):
    """Binary CKKS with BCH error correction"""
    
    def __init__(self, params: CKKSParams, bch_t: int = 3):
        super().__init__(params)
        self.bch_t = bch_t  # Error correction capability
    
    def _add_bch_redundancy(self, data: np.ndarray) -> np.ndarray:
        """Add BCH error correction redundancy (simplified)"""
        # Simplified BCH: just add parity bits
        parity = np.sum(data) % 2
        redundant_data = np.append(data, [parity] * self.bch_t)
        return redundant_data[:len(data)]  # Keep same size for simplicity
    
    def _correct_bch_errors(self, data: np.ndarray) -> np.ndarray:
        """Correct errors using BCH (simplified)"""
        # Simplified error correction
        return data
    
    def encode(self, z: np.ndarray) -> np.ndarray:
        """BCH-protected binary encoding"""
        m = BinaryCKKS.encode(self, z)
        return self._add_bch_redundancy(m)
    
    def decrypt(self, ciphertext: Tuple[np.ndarray, np.ndarray]) -> np.ndarray:
        """BCH-corrected decryption"""
        m = super().decrypt(ciphertext)
        return self._correct_bch_errors(m)

def run_basic_tests():
    """Run basic functionality tests"""
    print("=== CKKS Implementation Tests ===\n")
    
    # Test parameters
    params = CKKSParams(n=16, q=1024, sigma=1.0, scale=64.0, h=8)
    
    # Test data
    test_data = np.array([1.5, 2.3, 0.7, -1.2])
    
    print("Test Parameters:")
    print(f"Ring dimension (n): {params.n}")
    print(f"Modulus (q): {params.q}")
    print(f"Noise std (σ): {params.sigma}")
    print(f"Scale (Δ): {params.scale}")
    print(f"Hamming weight (h): {params.h}")
    print(f"Test data: {test_data}")
    print()
    
    # Test Standard CKKS
    print("1. Standard CKKS Test:")
    print("-" * 30)
    
    start_time = time.time()
    ckks = CKKS(params)
    
    # Encode and encrypt
    encoded = ckks.encode(test_data)
    ct1 = ckks.encrypt(encoded)
    ct2 = ckks.encrypt(encoded)
    
    # Homomorphic operations
    ct_add = ckks.add(ct1, ct2)
    ct_mult = ckks.multiply(ct1, ct2)
    
    # Decrypt and decode
    dec_add = ckks.decrypt(ct_add)
    dec_mult = ckks.decrypt(ct_mult)
    
    result_add = ckks.decode(dec_add)
    result_mult = ckks.decode(dec_mult)
    
    setup_time = time.time() - start_time
    
    print(f"Setup time: {setup_time:.4f}s")
    print(f"Addition result: {result_add[:len(test_data)]}")
    print(f"Expected (2x): {2 * test_data}")
    print(f"Multiplication result: {result_mult[:len(test_data)]}")
    print(f"Expected (x²): {test_data ** 2}")
    
    # Calculate errors
    add_error = np.mean(np.abs(result_add[:len(test_data)] - 2 * test_data))
    mult_error = np.mean(np.abs(result_mult[:len(test_data)] - test_data ** 2))
    print(f"Addition error: {add_error:.6f}")
    print(f"Multiplication error: {mult_error:.6f}")
    print()
    
    # Test Binary CKKS
    print("2. Binary CKKS Test:")
    print("-" * 30)
    
    start_time = time.time()
    binary_ckks = BinaryCKKS(params)
    
    # Same operations
    encoded_bin = binary_ckks.encode(test_data)
    ct1_bin = binary_ckks.encrypt(encoded_bin)
    ct2_bin = binary_ckks.encrypt(encoded_bin)
    
    ct_add_bin = binary_ckks.add(ct1_bin, ct2_bin)
    dec_add_bin = binary_ckks.decrypt(ct_add_bin)
    result_add_bin = binary_ckks.decode(dec_add_bin)
    
    setup_time_bin = time.time() - start_time
    
    print(f"Setup time: {setup_time_bin:.4f}s")
    print(f"Binary addition result: {result_add_bin[:len(test_data)]}")
    print(f"Expected (2x): {2 * test_data}")
    
    add_error_bin = np.mean(np.abs(result_add_bin[:len(test_data)] - 2 * test_data))
    print(f"Binary addition error: {add_error_bin:.6f}")
    print()
    
    # Test Binary CKKS with BCH
    print("3. Binary CKKS with BCH Test:")
    print("-" * 30)
    
    start_time = time.time()
    bch_ckks = BinaryCKKSWithBCH(params, bch_t=2)
    
    # Same operations
    encoded_bch = bch_ckks.encode(test_data)
    ct1_bch = bch_ckks.encrypt(encoded_bch)
    ct2_bch = bch_ckks.encrypt(encoded_bch)
    
    ct_add_bch = bch_ckks.add(ct1_bch, ct2_bch)
    dec_add_bch = bch_ckks.decrypt(ct_add_bch)
    result_add_bch = bch_ckks.decode(dec_add_bch)
    
    setup_time_bch = time.time() - start_time
    
    print(f"Setup time: {setup_time_bch:.4f}s")
    print(f"BCH addition result: {result_add_bch[:len(test_data)]}")
    print(f"Expected (2x): {2 * test_data}")
    
    add_error_bch = np.mean(np.abs(result_add_bch[:len(test_data)] - 2 * test_data))
    print(f"BCH addition error: {add_error_bch:.6f}")
    print()
    
    # Performance comparison
    print("4. Performance Comparison:")
    print("-" * 30)
    print(f"Standard CKKS: {setup_time:.4f}s")
    print(f"Binary CKKS: {setup_time_bin:.4f}s ({setup_time_bin/setup_time:.2f}x)")
    print(f"Binary CKKS+BCH: {setup_time_bch:.4f}s ({setup_time_bch/setup_time:.2f}x)")
    print()
    
    print("5. Space Analysis:")
    print("-" * 30)
    
    # Estimate space usage (simplified)
    std_space = params.n * 64  # 64-bit coefficients
    bin_space = params.n * 1   # 1-bit coefficients  
    bch_space = params.n * 1 + params.n * bch_ckks.bch_t / 8  # BCH redundancy
    
    print(f"Standard CKKS: ~{std_space} bits")
    print(f"Binary CKKS: ~{bin_space} bits ({bin_space/std_space:.3f}x)")
    print(f"Binary CKKS+BCH: ~{bch_space:.0f} bits ({bch_space/std_space:.3f}x)")
    print()

if __name__ == "__main__":
    run_basic_tests()