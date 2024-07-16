#include <chrono>
#include <fstream>
#include <iostream>
#include <math.h>
#include <random>
#include <seal/ciphertext.h>
#include <seal/plaintext.h>
#include <seal/seal.h>
#include <string>
#include <vector>
#include "argmax.h"
#include "ckks_evaluator.h"
using namespace std;
using namespace seal;
using namespace seal::util;
using namespace std::chrono;

void MM_test();

int main()
{
    EncryptionParameters parms(scheme_type::ckks);
    long logN = 16;
    size_t poly_modulus_degree = 1 << logN;
    double scale = pow(2.0, 40);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(
        poly_modulus_degree, { 58, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 58 }));

    SEALContext context(parms, true, sec_level_type::none);

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);

    Encryptor encryptor(context, public_key);
    CKKSEncoder encoder(context);
    Evaluator evaluator(context, encoder);
    Decryptor decryptor(context, secret_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    GaloisKeys galois_keys;
    keygen.create_galois_keys(galois_keys);


    CKKSEvaluator ckks_evaluator(context, encryptor, decryptor, encoder, evaluator, scale, relin_keys, galois_keys);
    vector<double> input = { 0.4, 0.3, 0.2, 0.1, 0.7, 0.8, 0.2, 0.4 };
    Plaintext plain_input;
    Ciphertext cipher_input;
    Ciphertext cipher_output;
    vector<double> output;
    ckks_evaluator.encoder->encode(input, scale, plain_input);
    ckks_evaluator.encryptor->encrypt(plain_input, cipher_input);

    ArgmaxEvaluator argmax_evaluator(ckks_evaluator);

    std::cout << "Initilization done" << std::endl;
    cout << "Input: 0.4, 0.3, 0.2, 0.1, 0.7, 0.8, 0.2, 0.4" << endl; 

    auto start = high_resolution_clock::now();
    int size = input.size();
    argmax_evaluator.argmax(cipher_input, cipher_output, size);
    auto end = high_resolution_clock::now();
    
    
    cout << "Output: "; 
    ckks_evaluator.print_decrypted_ct(cipher_output, 8);
    cout << "argmax takes: " << duration_cast<seconds>(end - start).count() << " seconds" << endl;

}
