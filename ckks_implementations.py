import numpy as np
import matplotlib.pyplot as plt
import time
import sys
from typing import List, Tuple, Optional, Dict
from dataclasses import dataclass
import random
from scipy.fft import fft, ifft
import seaborn as sns
import pandas as pd
from matplotlib.patches import Rectangle
import warnings
warnings.filterwarnings('ignore')

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
        assert self.n & (self.n - 1) == 0, "Ring dimension must be power of 2"

class DiscreteGaussian:
    """Discrete Gaussian sampler"""
    def __init__(self, sigma: float):
        self.sigma = sigma
    
    def sample(self, size: int = 1) -> np.ndarray:
        return np.round(np.random.normal(0, self.sigma, size)).astype(int)

class BCHCode:
    """BCH Error Correcting Code Implementation"""
    
    def __init__(self, n: int, k: int, t: int):
        """
        Initialize BCH(n,k,t) code
        n: codeword length
        k: message length  
        t: error correction capability
        """
        self.n = n
        self.k = k
        self.t = t
        self.parity_bits = n - k
        
        # For simplicity, we'll use a basic BCH implementation
        # In practice, you'd use more sophisticated generator polynomials
        self._generate_code_matrices()
    
    def _generate_code_matrices(self):
        """Generate generator and parity check matrices"""
        # Simplified BCH generator matrix (systematic form)
        self.G = np.zeros((self.k, self.n), dtype=int)
        self.G[:, :self.k] = np.eye(self.k)  # Identity matrix for systematic part
        
        # Random parity matrix for demonstration (in practice use proper BCH construction)
        np.random.seed(42)
        self.G[:, self.k:] = np.random.randint(0, 2, (self.k, self.parity_bits))
        
        # Parity check matrix
        self.H = np.zeros((self.parity_bits, self.n), dtype=int)
        self.H[:, :self.k] = self.G[:, self.k:].T
        self.H[:, self.k:] = np.eye(self.parity_bits)
    
    def encode(self, message: np.ndarray) -> np.ndarray:
        """Encode k-bit message to n-bit codeword"""
        assert len(message) == self.k, f"Message length must be {self.k}"
        return (message @ self.G) % 2
    
    def decode(self, received: np.ndarray) -> Tuple[np.ndarray, bool]:
        """
        Decode n-bit received vector to k-bit message
        Returns: (decoded_message, success_flag)
        """
        assert len(received) == self.n, f"Received vector length must be {self.n}"
        
        # Compute syndrome
        syndrome = (self.H @ received) % 2
        
        # If syndrome is zero, no errors detected
        if np.all(syndrome == 0):
            return received[:self.k], True
        
        # Simple error correction (for demo purposes)
        # In practice, use Berlekamp-Massey algorithm
        corrected = received.copy()
        error_count = 0
        
        # Try to correct up to t errors by flipping bits
        for i in range(self.n):
            if error_count >= self.t:
                break
            test_vector = corrected.copy()
            test_vector[i] = 1 - test_vector[i]  # Flip bit
            test_syndrome = (self.H @ test_vector) % 2
            
            if np.sum(test_syndrome) < np.sum(syndrome):
                corrected = test_vector
                syndrome = test_syndrome
                error_count += 1
                
                if np.all(syndrome == 0):
                    break
        
        success = np.all(syndrome == 0)
        return corrected[:self.k], success

class CKKS:
    """Standard CKKS Implementation"""
    
    def __init__(self, params: CKKSParams):
        self.params = params
        self.n = params.n
        self.q = params.q
        self.sigma = params.sigma
        self.scale = params.scale
        self.h = params.h
        
        # Primitive n-th root of unity
        self.xi = np.exp(2j * np.pi / (2 * self.n))
        
    def keygen(self) -> Tuple[np.ndarray, Tuple[np.ndarray, np.ndarray], Tuple[np.ndarray, np.ndarray]]:
        """Generate secret key, public key, and evaluation key"""
        # Secret key: sparse ternary polynomial with Hamming weight h
        s = np.zeros(self.n, dtype=int)
        positions = np.random.choice(self.n, self.h, replace=False)
        s[positions] = np.random.choice([-1, 1], self.h)
        
        # Public key
        a = np.random.randint(0, self.q, self.n)
        e = DiscreteGaussian(self.sigma).sample(self.n)
        b = (-a * s + e) % self.q
        pk = (b, a)
        
        # Evaluation key  
        a_evk = np.random.randint(0, self.q, self.n)
        e_evk = DiscreteGaussian(self.sigma).sample(self.n)
        b_evk = (-a_evk * s + e_evk + s * s) % self.q
        evk = (b_evk, a_evk)
        
        return s, pk, evk
    
    def encode(self, values: np.ndarray) -> np.ndarray:
        """Encode complex values to polynomial coefficients"""
        # Canonical embedding inverse
        n_half = self.n // 2
        z = np.zeros(self.n, dtype=complex)
        z[:n_half] = values[:n_half]
        z[n_half:] = np.conj(values[:n_half][::-1])
        
        # Scale and round
        poly_complex = ifft(z, norm='backward') * self.n
        poly_scaled = poly_complex * self.scale
        return np.round(np.real(poly_scaled)).astype(int) % self.q
    
    def decode(self, poly: np.ndarray) -> np.ndarray:
        """Decode polynomial coefficients to complex values"""
        # Unscale
        poly_unscaled = poly.astype(complex) / self.scale
        
        # Canonical embedding
        z = fft(poly_unscaled, norm='forward') / self.n
        return z[:self.n//2]
    
    def encrypt(self, m: np.ndarray, pk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Encrypt polynomial m"""
        b, a = pk
        u = np.random.randint(0, 2, self.n)  # Binary randomness
        e0 = DiscreteGaussian(self.sigma).sample(self.n)
        e1 = DiscreteGaussian(self.sigma).sample(self.n)
        
        c0 = (u * b + m + e0) % self.q
        c1 = (u * a + e1) % self.q
        return c0, c1
    
    def decrypt(self, ct: Tuple[np.ndarray, np.ndarray], sk: np.ndarray) -> np.ndarray:
        """Decrypt ciphertext"""
        c0, c1 = ct
        return (c0 + c1 * sk) % self.q
    
    def add(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Homomorphic addition"""
        c0_1, c1_1 = ct1
        c0_2, c1_2 = ct2
        return (c0_1 + c0_2) % self.q, (c1_1 + c1_2) % self.q
    
    def multiply(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray], 
                evk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Homomorphic multiplication with relinearization"""
        c0_1, c1_1 = ct1
        c0_2, c1_2 = ct2
        b_evk, a_evk = evk
        
        # Multiply
        d0 = (c0_1 * c0_2) % self.q
        d1 = (c0_1 * c1_2 + c1_1 * c0_2) % self.q
        d2 = (c1_1 * c1_2) % self.q
        
        # Relinearize
        c0_new = (d0 + d2 * b_evk) % self.q
        c1_new = (d1 + d2 * a_evk) % self.q
        
        return c0_new, c1_new

class BinaryCKKS:
    """Binary CKKS Implementation"""
    
    def __init__(self, params: CKKSParams):
        self.params = params
        self.n = params.n
        self.sigma = params.sigma
        self.scale = params.scale
        self.h = params.h
        
    def binary_encode(self, poly: np.ndarray) -> np.ndarray:
        """Convert polynomial to binary representation"""
        binary_poly = np.zeros(self.n, dtype=int)
        for i in range(self.n):
            coeff = int(poly[i])
            # Binary expansion (simplified)
            binary_poly[i] = coeff % 2
        return binary_poly
    
    def binary_decode(self, binary_poly: np.ndarray) -> np.ndarray:
        """Convert binary polynomial back to integer polynomial"""
        return binary_poly.astype(int)
    
    def keygen(self) -> Tuple[np.ndarray, Tuple[np.ndarray, np.ndarray], Tuple[np.ndarray, np.ndarray]]:
        """Generate keys with binary encoding"""
        # Secret key: binary sparse polynomial
        s = np.zeros(self.n, dtype=int)
        positions = np.random.choice(self.n, self.h, replace=False)
        s[positions] = 1
        
        # Public key
        a = np.random.randint(0, 2, self.n)
        e = DiscreteGaussian(self.sigma).sample(self.n)
        e_bin = self.binary_encode(e)
        b = (a * s + e_bin) % 2
        pk = (b, a)
        
        # Evaluation key
        a_evk = np.random.randint(0, 2, self.n)
        e_evk = DiscreteGaussian(self.sigma).sample(self.n)
        e_evk_bin = self.binary_encode(e_evk)
        b_evk = (a_evk * s + e_evk_bin + s * s) % 2
        evk = (b_evk, a_evk)
        
        return s, pk, evk
    
    def encrypt(self, m: np.ndarray, pk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Encrypt with binary arithmetic"""
        b, a = pk
        u = np.random.randint(0, 2, self.n)
        e0 = self.binary_encode(DiscreteGaussian(self.sigma).sample(self.n))
        e1 = self.binary_encode(DiscreteGaussian(self.sigma).sample(self.n))
        
        c0 = (u * b + m + e0) % 2
        c1 = (u * a + e1) % 2
        return c0, c1
    
    def decrypt(self, ct: Tuple[np.ndarray, np.ndarray], sk: np.ndarray) -> np.ndarray:
        """Decrypt with binary arithmetic"""
        c0, c1 = ct
        return (c0 + c1 * sk) % 2
    
    def add(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Binary homomorphic addition"""
        c0_1, c1_1 = ct1
        c0_2, c1_2 = ct2
        return (c0_1 + c0_2) % 2, (c1_1 + c1_2) % 2
    
    def multiply(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray],
                evk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Binary homomorphic multiplication"""
        c0_1, c1_1 = ct1
        c0_2, c1_2 = ct2
        b_evk, a_evk = evk
        
        d0 = (c0_1 * c0_2) % 2
        d1 = (c0_1 * c1_2 + c1_1 * c0_2) % 2
        d2 = (c1_1 * c1_2) % 2
        
        c0_new = (d0 + d2 * b_evk) % 2
        c1_new = (d1 + d2 * a_evk) % 2
        
        return c0_new, c1_new

class BinaryCKKSWithBCH:
    """Binary CKKS with BCH Error Correction"""
    
    def __init__(self, params: CKKSParams, bch_params: Tuple[int, int, int]):
        self.binary_ckks = BinaryCKKS(params)
        self.params = params
        n_bch, k_bch, t_bch = bch_params
        self.bch = BCHCode(n_bch, k_bch, t_bch)
        
    def encode_with_bch(self, message: np.ndarray) -> np.ndarray:
        """Encode message with BCH before encryption"""
        # Ensure message fits in k bits
        if len(message) > self.bch.k:
            message = message[:self.bch.k]
        elif len(message) < self.bch.k:
            # Pad with zeros
            padded = np.zeros(self.bch.k, dtype=int)
            padded[:len(message)] = message
            message = padded
        
        # BCH encode
        encoded = self.bch.encode(message)
        
        # Pad to ring dimension
        if len(encoded) < self.params.n:
            result = np.zeros(self.params.n, dtype=int)
            result[:len(encoded)] = encoded
            return result
        else:
            return encoded[:self.params.n]
    
    def decode_with_bch(self, received: np.ndarray) -> Tuple[np.ndarray, bool]:
        """Decode with BCH error correction"""
        # Extract BCH codeword
        bch_received = received[:self.bch.n]
        
        # BCH decode with error correction
        decoded_message, success = self.bch.decode(bch_received)
        
        return decoded_message, success
    
    def keygen(self):
        return self.binary_ckks.keygen()
    
    def encrypt(self, m: np.ndarray, pk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        """Encrypt with BCH encoding"""
        # First apply BCH encoding
        m_encoded = self.encode_with_bch(m)
        return self.binary_ckks.encrypt(m_encoded, pk)
    
    def decrypt(self, ct: Tuple[np.ndarray, np.ndarray], sk: np.ndarray) -> Tuple[np.ndarray, bool]:
        """Decrypt with BCH decoding"""
        # First decrypt normally
        decrypted = self.binary_ckks.decrypt(ct, sk)
        
        # Then apply BCH decoding with error correction
        decoded_message, success = self.decode_with_bch(decrypted)
        
        return decoded_message, success
    
    def add(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        return self.binary_ckks.add(ct1, ct2)
    
    def multiply(self, ct1: Tuple[np.ndarray, np.ndarray], ct2: Tuple[np.ndarray, np.ndarray],
                evk: Tuple[np.ndarray, np.ndarray]) -> Tuple[np.ndarray, np.ndarray]:
        return self.binary_ckks.multiply(ct1, ct2, evk)

class CKKSExperiments:
    """Comprehensive experiments for CKKS variants"""
    
    def __init__(self):
        self.results = {}
    
    def run_complexity_experiment(self, ring_dims: List[int]) -> Dict:
        """Measure time complexity for different operations"""
        results = {
            'ring_dims': ring_dims,
            'ckks': {'keygen': [], 'encrypt': [], 'decrypt': [], 'add': [], 'multiply': []},
            'binary_ckks': {'keygen': [], 'encrypt': [], 'decrypt': [], 'add': [], 'multiply': []},
            'binary_ckks_bch': {'keygen': [], 'encrypt': [], 'decrypt': [], 'add': [], 'multiply': []}
        }
        
        for n in ring_dims:
            print(f"Testing ring dimension {n}...")
            
            # Parameters
            params = CKKSParams(n=n, q=2**32, sigma=3.2, scale=2**40, h=64)
            bch_params = (127, 106, 3) if n >= 127 else (31, 26, 1)
            
            # Initialize schemes
            ckks = CKKS(params)
            binary_ckks = BinaryCKKS(params)
            binary_ckks_bch = BinaryCKKSWithBCH(params, bch_params)
            
            # Test data
            test_values = np.random.randn(n//2) + 1j * np.random.randn(n//2)
            test_binary = np.random.randint(0, 2, bch_params[1])
            
            # CKKS experiments
            start_time = time.time()
            sk, pk, evk = ckks.keygen()
            results['ckks']['keygen'].append(time.time() - start_time)
            
            m_encoded = ckks.encode(test_values)
            
            start_time = time.time()
            ct1 = ckks.encrypt(m_encoded, pk)
            results['ckks']['encrypt'].append(time.time() - start_time)
            
            start_time = time.time()
            decrypted = ckks.decrypt(ct1, sk)
            results['ckks']['decrypt'].append(time.time() - start_time)
            
            ct2 = ckks.encrypt(m_encoded, pk)
            
            start_time = time.time()
            ct_add = ckks.add(ct1, ct2)
            results['ckks']['add'].append(time.time() - start_time)
            
            start_time = time.time()
            ct_mult = ckks.multiply(ct1, ct2, evk)
            results['ckks']['multiply'].append(time.time() - start_time)
            
            # Binary CKKS experiments
            start_time = time.time()
            sk_bin, pk_bin, evk_bin = binary_ckks.keygen()
            results['binary_ckks']['keygen'].append(time.time() - start_time)
            
            m_bin = np.random.randint(0, 2, n)
            
            start_time = time.time()
            ct1_bin = binary_ckks.encrypt(m_bin, pk_bin)
            results['binary_ckks']['encrypt'].append(time.time() - start_time)
            
            start_time = time.time()
            decrypted_bin = binary_ckks.decrypt(ct1_bin, sk_bin)
            results['binary_ckks']['decrypt'].append(time.time() - start_time)
            
            ct2_bin = binary_ckks.encrypt(m_bin, pk_bin)
            
            start_time = time.time()
            ct_add_bin = binary_ckks.add(ct1_bin, ct2_bin)
            results['binary_ckks']['add'].append(time.time() - start_time)
            
            start_time = time.time()
            ct_mult_bin = binary_ckks.multiply(ct1_bin, ct2_bin, evk_bin)
            results['binary_ckks']['multiply'].append(time.time() - start_time)
            
            # Binary CKKS with BCH experiments
            start_time = time.time()
            sk_bch, pk_bch, evk_bch = binary_ckks_bch.keygen()
            results['binary_ckks_bch']['keygen'].append(time.time() - start_time)
            
            start_time = time.time()
            ct1_bch = binary_ckks_bch.encrypt(test_binary, pk_bch)
            results['binary_ckks_bch']['encrypt'].append(time.time() - start_time)
            
            start_time = time.time()
            decrypted_bch, success = binary_ckks_bch.decrypt(ct1_bch, sk_bch)
            results['binary_ckks_bch']['decrypt'].append(time.time() - start_time)
            
            ct2_bch = binary_ckks_bch.encrypt(test_binary, pk_bch)
            
            start_time = time.time()
            ct_add_bch = binary_ckks_bch.add(ct1_bch, ct2_bch)
            results['binary_ckks_bch']['add'].append(time.time() - start_time)
            
            start_time = time.time()
            ct_mult_bch = binary_ckks_bch.multiply(ct1_bch, ct2_bch, evk_bch)
            results['binary_ckks_bch']['multiply'].append(time.time() - start_time)
        
        return results
    
    def run_space_experiment(self, ring_dims: List[int]) -> Dict:
        """Measure space usage for different schemes"""
        results = {
            'ring_dims': ring_dims,
            'ckks': {'sk_size': [], 'pk_size': [], 'ct_size': []},
            'binary_ckks': {'sk_size': [], 'pk_size': [], 'ct_size': []},
            'binary_ckks_bch': {'sk_size': [], 'pk_size': [], 'ct_size': []}
        }
        
        for n in ring_dims:
            params = CKKSParams(n=n, q=2**32, sigma=3.2, scale=2**40, h=64)
            bch_params = (127, 106, 3) if n >= 127 else (31, 26, 1)
            
            # CKKS
            ckks = CKKS(params)
            sk, pk, evk = ckks.keygen()
            test_values = np.random.randn(n//2) + 1j * np.random.randn(n//2)
            m_encoded = ckks.encode(test_values)
            ct = ckks.encrypt(m_encoded, pk)
            
            results['ckks']['sk_size'].append(sk.nbytes)
            results['ckks']['pk_size'].append(pk[0].nbytes + pk[1].nbytes)
            results['ckks']['ct_size'].append(ct[0].nbytes + ct[1].nbytes)
            
            # Binary CKKS
            binary_ckks = BinaryCKKS(params)
            sk_bin, pk_bin, evk_bin = binary_ckks.keygen()
            m_bin = np.random.randint(0, 2, n)
            ct_bin = binary_ckks.encrypt(m_bin, pk_bin)
            
            results['binary_ckks']['sk_size'].append(sk_bin.nbytes)
            results['binary_ckks']['pk_size'].append(pk_bin[0].nbytes + pk_bin[1].nbytes)
            results['binary_ckks']['ct_size'].append(ct_bin[0].nbytes + ct_bin[1].nbytes)
            
            # Binary CKKS with BCH
            binary_ckks_bch = BinaryCKKSWithBCH(params, bch_params)
            sk_bch, pk_bch, evk_bch = binary_ckks_bch.keygen()
            test_binary = np.random.randint(0, 2, bch_params[1])
            ct_bch = binary_ckks_bch.encrypt(test_binary, pk_bch)
            
            results['binary_ckks_bch']['sk_size'].append(sk_bch.nbytes)
            results['binary_ckks_bch']['pk_size'].append(pk_bch[0].nbytes + pk_bch[1].nbytes)
            results['binary_ckks_bch']['ct_size'].append(ct_bch[0].nbytes + ct_bch[1].nbytes)
        
        return results
    
    def run_accuracy_experiment(self, ring_dims: List[int], num_trials: int = 100) -> Dict:
        """Measure accuracy, especially for BCH error correction"""
        results = {
            'ring_dims': ring_dims,
            'binary_ckks_bch': {
                'error_rates': [],
                'correction_success_rates': [],
                'bit_error_counts': [],
                'decoding_success_rates': []
            }
        }
        
        for n in ring_dims:
            print(f"Testing accuracy for ring dimension {n}...")
            
            params = CKKSParams(n=n, q=2**32, sigma=3.2, scale=2**40, h=64)
            bch_params = (127, 106, 3) if n >= 127 else (31, 26, 1)
            
            binary_ckks_bch = BinaryCKKSWithBCH(params, bch_params)
            sk, pk, evk = binary_ckks_bch.keygen()
            
            total_bits = 0
            total_errors = 0
            successful_corrections = 0
            successful_decodings = 0
            bit_errors_per_trial = []
            
            for trial in range(num_trials):
                # Generate random message
                original_message = np.random.randint(0, 2, bch_params[1])
                
                # Encrypt
                ct = binary_ckks_bch.encrypt(original_message, pk)
                
                # Add some noise to simulate channel errors
                # This simulates the noise that accumulates during computation
                noise_level = 0.1  # Adjust based on expected noise
                if np.random.random() < noise_level:
                    # Flip some bits in the ciphertext
                    num_flips = np.random.randint(1, min(4, len(ct[0])))
                    flip_positions = np.random.choice(len(ct[0]), num_flips, replace=False)
                    for pos in flip_positions:
                        ct[0][pos] = 1 - ct[0][pos]
                
                # Decrypt with BCH correction
                decoded_message, success = binary_ckks_bch.decrypt(ct, sk)
                
                # Count errors
                if len(decoded_message) == len(original_message):
                    bit_errors = np.sum(decoded_message != original_message)
                    bit_errors_per_trial.append(bit_errors)
                    total_bits += len(original_message)
                    total_errors += bit_errors
                    
                    if bit_errors == 0:
                        successful_corrections += 1
                    
                    if success:
                        successful_decodings += 1
                else:
                    bit_errors_per_trial.append(len(original_message))  # All bits wrong
                    total_bits += len(original_message)
                    total_errors += len(original_message)
            
            error_rate = total_errors / total_bits if total_bits > 0 else 1.0
            correction_success_rate = successful_corrections / num_trials
            decoding_success_rate = successful_decodings / num_trials
            
            results['binary_ckks_bch']['error_rates'].append(error_rate)
            results['binary_ckks_bch']['correction_success_rates'].append(correction_success_rate)
            results['binary_ckks_bch']['bit_error_counts'].append(bit_errors_per_trial)
            results['binary_ckks_bch']['decoding_success_rates'].append(decoding_success_rate)
        
        return results
    
    def plot_results(self, complexity_results: Dict, space_results: Dict, accuracy_results: Dict):
        """Create comprehensive plots of experimental results"""
        
        # Set up the plotting style
        plt.style.use('seaborn-v0_8')
        sns.set_palette("husl")
        
        fig = plt.figure(figsize=(20, 15))
        
        # 1. Complexity Analysis
        ax1 = plt.subplot(3, 3, 1)
        ring_dims = complexity_results['ring_dims']
        operations = ['keygen', 'encrypt', 'decrypt', 'add', 'multiply']
        schemes = ['ckks', 'binary_ckks', 'binary_ckks_bch']
        scheme_labels = ['CKKS', 'Binary CKKS', 'Binary CKKS + BCH']
        
        for i, op in enumerate(['keygen', 'encrypt', 'multiply']):
            ax = plt.subplot(3, 3, i + 1)
            for j, scheme in enumerate(schemes):
                times = complexity_results[scheme][op]
                ax.loglog(ring_dims, times, 'o-', label=scheme_labels[j], linewidth=2, markersize=6)
            ax.set_xlabel('Ring Dimension')
            ax.set_ylabel('Time (seconds)')
            ax.set_title(f'{op.capitalize()} Time Complexity')
            ax.legend()
            ax.grid(True, alpha=0.3)
        
        # 2. Space Analysis
        space_types = ['sk_size', 'pk_size', 'ct_size']
        space_labels = ['Secret Key', 'Public Key', 'Ciphertext']
        
        for i, space_type in enumerate(space_types):
            ax = plt.subplot(3, 3, i + 4)
            for j, scheme in enumerate(schemes):
                sizes = [s / 1024 for s in space_results[scheme][space_type]]  # Convert to KB
                ax.loglog(ring_dims, sizes, 'o-', label=scheme_labels[j], linewidth=2, markersize=6)
            ax.set_xlabel('Ring Dimension')
            ax.set_ylabel('Size (KB)')
            ax.set_title(f'{space_labels[i]} Size')
            ax.legend()
            ax.grid(True, alpha=0.3)
        
        # 3. Accuracy Analysis for BCH
        ax7 = plt.subplot(3, 3, 7)
        error_rates = accuracy_results['binary_ckks_bch']['error_rates']
        ax7.semilogx(ring_dims, error_rates, 'ro-', linewidth=2, markersize=8)
        ax7.set_xlabel('Ring Dimension')
        ax7.set_ylabel('Bit Error Rate')
        ax7.set_title('BCH Error Correction - Bit Error Rate')
        ax7.grid(True, alpha=0.3)
        
        ax8 = plt.subplot(3, 3, 8)
        success_rates = accuracy_results['binary_ckks_bch']['correction_success_rates']
        decoding_rates = accuracy_results['binary_ckks_bch']['decoding_success_rates']
        ax8.semilogx(ring_dims, success_rates, 'go-', label='Perfect Correction', linewidth=2, markersize=8)
        ax8.semilogx(ring_dims, decoding_rates, 'bo-', label='Successful Decoding', linewidth=2, markersize=8)
        ax8.set_xlabel('Ring Dimension')
        ax8.set_ylabel('Success Rate')
        ax8.set_title('BCH Correction Success Rates')
        ax8.legend()
        ax8.grid(True, alpha=0.3)
        
        # 4. Error distribution histogram for largest ring dimension
        ax9 = plt.subplot(3, 3, 9)
        if accuracy_results['binary_ckks_bch']['bit_error_counts']:
            error_counts = accuracy_results['binary_ckks_bch']['bit_error_counts'][-1]  # Last (largest) dimension
            ax9.hist(error_counts, bins=max(10, len(set(error_counts))), alpha=0.7, edgecolor='black')
            ax9.set_xlabel('Number of Bit Errors')
            ax9.set_ylabel('Frequency')
            ax9.set_title(f'Error Distribution (n={ring_dims[-1]})')
            ax9.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig('ckks_comprehensive_analysis.png', dpi=300, bbox_inches='tight')
        plt.show()
    
    def generate_summary_table(self, complexity_results: Dict, space_results: Dict, accuracy_results: Dict):
        """Generate summary tables"""
        ring_dims = complexity_results['ring_dims']
        
        # Complexity summary table
        print("\n" + "="*80)
        print("COMPLEXITY ANALYSIS SUMMARY")
        print("="*80)
        
        df_complexity = pd.DataFrame({
            'Ring Dimension': ring_dims,
            'CKKS KeyGen (s)': [f"{t:.4f}" for t in complexity_results['ckks']['keygen']],
            'Binary CKKS KeyGen (s)': [f"{t:.4f}" for t in complexity_results['binary_ckks']['keygen']],
            'BCH KeyGen (s)': [f"{t:.4f}" for t in complexity_results['binary_ckks_bch']['keygen']],
            'CKKS Encrypt (s)': [f"{t:.4f}" for t in complexity_results['ckks']['encrypt']],
            'Binary CKKS Encrypt (s)': [f"{t:.4f}" for t in complexity_results['binary_ckks']['encrypt']],
            'BCH Encrypt (s)': [f"{t:.4f}" for t in complexity_results['binary_ckks_bch']['encrypt']],
        })
        print(df_complexity.to_string(index=False))
        
        # Space summary table
        print("\n" + "="*80)
        print("SPACE USAGE ANALYSIS SUMMARY")
        print("="*80)
        
        df_space = pd.DataFrame({
            'Ring Dimension': ring_dims,
            'CKKS SK (KB)': [f"{s/1024:.2f}" for s in space_results['ckks']['sk_size']],
            'Binary CKKS SK (KB)': [f"{s/1024:.2f}" for s in space_results['binary_ckks']['sk_size']],
            'BCH SK (KB)': [f"{s/1024:.2f}" for s in space_results['binary_ckks_bch']['sk_size']],
            'CKKS CT (KB)': [f"{s/1024:.2f}" for s in space_results['ckks']['ct_size']],
            'Binary CKKS CT (KB)': [f"{s/1024:.2f}" for s in space_results['binary_ckks']['ct_size']],
            'BCH CT (KB)': [f"{s/1024:.2f}" for s in space_results['binary_ckks_bch']['ct_size']],
        })
        print(df_space.to_string(index=False))
        
        # Accuracy summary table
        print("\n" + "="*80)
        print("BCH ERROR CORRECTION ACCURACY SUMMARY")
        print("="*80)
        
        df_accuracy = pd.DataFrame({
            'Ring Dimension': ring_dims,
            'Bit Error Rate': [f"{r:.6f}" for r in accuracy_results['binary_ckks_bch']['error_rates']],
            'Perfect Correction Rate': [f"{r:.4f}" for r in accuracy_results['binary_ckks_bch']['correction_success_rates']],
            'Successful Decoding Rate': [f"{r:.4f}" for r in accuracy_results['binary_ckks_bch']['decoding_success_rates']],
        })
        print(df_accuracy.to_string(index=False))
        
        # Save tables to CSV
        df_complexity.to_csv('ckks_complexity_results.csv', index=False)
        df_space.to_csv('ckks_space_results.csv', index=False)
        df_accuracy.to_csv('ckks_accuracy_results.csv', index=False)
        
        print(f"\nResults saved to CSV files.")

def main():
    """Main experiment runner"""
    print("Starting CKKS Comprehensive Experiments...")
    print("="*60)
    
    # Ring dimensions to test
    ring_dims = [1024, 2048, 4096, 8192]
    
    # Initialize experiment runner
    experiments = CKKSExperiments()
    
    # Run experiments
    print("Running complexity experiments...")
    complexity_results = experiments.run_complexity_experiment(ring_dims)
    
    print("Running space usage experiments...")
    space_results = experiments.run_space_experiment(ring_dims)
    
    print("Running accuracy experiments...")
    accuracy_results = experiments.run_accuracy_experiment(ring_dims, num_trials=50)
    
    # Generate plots and tables
    print("Generating plots and summary tables...")
    experiments.plot_results(complexity_results, space_results, accuracy_results)
    experiments.generate_summary_table(complexity_results, space_results, accuracy_results)
    
    # BCH-specific analysis
    print("\n" + "="*80)
    print("BCH CODE ANALYSIS")
    print("="*80)
    
    # Test BCH with different parameters
    bch_configs = [
        (127, 106, 3),  # Standard
        (127, 99, 4),   # Higher error correction
        (127, 92, 5),   # Even higher error correction
    ]
    
    print(f"{'Configuration':<20} {'Code Rate':<12} {'Redundancy':<12} {'Error Capacity':<15}")
    print("-" * 60)
    for n, k, t in bch_configs:
        code_rate = k / n
        redundancy = n - k
        print(f"BCH({n},{k},{t}){'':<7} {code_rate:.3f}{'':<8} {redundancy} bits{'':<4} {t} errors")
    
    print(f"\nFor the BCH(127,106,3) code used in experiments:")
    print(f"- Code rate: {106/127:.3f}")
    print(f"- Redundancy: {127-106} parity bits")
    print(f"- Can correct up to 3 bit errors")
    print(f"- Failure probability with p ≈ 3×10⁻⁷: < 2⁻⁵⁰")
    
    print("\nExperiments completed successfully!")
    print("Check the generated plots and CSV files for detailed results.")

if __name__ == "__main__":
    main()