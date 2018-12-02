#include <iostream>
#include <bitset>
#include <cmath>

void Display (unsigned long long msg, std::string text, std::string form)
{
    if (form == "hex")
        std::cout << text << std::hex << msg << std::endl;
    else if (form == "bit")
        std::cout << text << std::bitset<sizeof(msg) * 8>(msg) << std::endl;
}

void Permutation (unsigned long long& a, int IP[], int length, int c=0)
{
    unsigned long long _aux = 0, _selector = pow(2,length-1);
    for (int i = 0; i < length; ++i )
    {
        if (i >= IP[i])
        {
            _aux |= (a >> i - IP[i]+1)& (_selector >> i) ;
        }
        else if (i < IP[i])
        {
            _aux |= (a << IP[i] - (i+1)) & (_selector >> i);
        }
    }
    a = _aux;
}

void SplitHalf (unsigned long long& a, unsigned long& leftHalf, unsigned long& rightHalf, std::string mode)
{
    if (mode == "plaintext")
    {
        leftHalf = a & (0xFFFFFFFF00000000);
        rightHalf = a &(0x00000000FFFFFFFF);
    }
    else if (mode == "key"){
        leftHalf = a & (0xFFFFFFF000000000);
        rightHalf = a &(0x0000000FFFFFFFF0);
    }
    
}

void ExpansionPermutation (unsigned long long& _righthalf_ex)
{
    _righthalf_ex = _righthalf_ex << 32;
    
    int IP_ex[48]={32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1};
    Permutation(_righthalf_ex, IP_ex, 64,1);
}

void ParityDropBit(unsigned long long& a)
{
    unsigned long long _buffer1=0, _buffer2=0, _selector=0xFE00000000000000;
    for (int j=0; j<8; j++)
    {
        _buffer1 = (_selector >> (8*j))&a;
        if (j == 0)
            _buffer2 |= _buffer1;
        else
            _buffer2 |= _buffer1 << 1 + (j-1);
    }
    a=_buffer2;
}

void LeftRotateShift(unsigned long& a, int n, std::string half)
{
    unsigned long _aux;
    if (half == "left")
    {
        _aux = a << n;
        if (n==1)
            _aux |= (0x8000000000000000 & a)>>27;
        else if (n==2)
            _aux |=(0xC000000000000000 & a)>> 26;
    }
    else if (half == "right")
    {
        _aux = a << n;
        if (n==1)
        {
            _aux |= (0x0000000800000000 & a) >> 27;
            _aux &=  0x0000000FFFFFFFF0;
        }
        else if (n==2)
        {
            _aux |= (0x0000000C00000000 & a) >> 26;
            _aux &=  0x0000000FFFFFFFF0;
        }
    }
    a = _aux;
}

void HalfCombiantion(unsigned long long& a, unsigned long int& leftHalf_key, unsigned long int& rightHalf_key)
{
    a = leftHalf_key|rightHalf_key;
}

void ShiftAndCombine (unsigned long long& key,unsigned long int& leftHalf_key,unsigned long int& rightHalf_key, int n)
{
    
    SplitHalf (key, leftHalf_key, rightHalf_key, "key");
    LeftRotateShift(rightHalf_key, n, "right");
    LeftRotateShift(leftHalf_key, n, "left");
    HalfCombiantion(key, leftHalf_key, rightHalf_key);
}

unsigned long long KeyPermutation(unsigned long long& key)
{
    unsigned long long _key_round;
    _key_round = key >>8;
    int IP_2 [48]={14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32};
    Permutation(_key_round, IP_2, 56);
    _key_round = _key_round >>8;
    _key_round = _key_round << 16;
    return _key_round;
}


unsigned long long RoundKeyGenerator (unsigned long long& key,unsigned long int& leftHalf_key,unsigned long int& rightHalf_key, int n)
{
    unsigned long long _key_round;
    ShiftAndCombine (key,leftHalf_key,rightHalf_key, n);
    _key_round = KeyPermutation(key);
    return _key_round;
}

unsigned long long SixBitToFourBit (unsigned long long& temp, unsigned long long& row, unsigned long long& column, int n)
{
    unsigned long long _aux;
    
    _aux = temp & (0xFC00000000000000 >> (6 * n));
    _aux = _aux << (6 * n);
    row = (0x8000000000000000 & _aux)|((0x0400000000000000 & _aux) << 4);
    row = row >> 62;
    column = (0x7800000000000000 & _aux)>>59;
    return _aux;
}

unsigned long long SBox(unsigned long long& row, unsigned long long& column, int k)
{
    unsigned long long _aux;
    int S[512]={14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
        0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
        4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
        15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13,
        15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
        3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
        0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
        13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9,
        10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
        13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
        13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
        1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12,
        7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
        13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
        10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
        3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14,
        2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
        14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
        4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
        11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3,
        12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
        10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
        9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
        4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13,
        4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
        13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
        1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
        6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12,
        13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
        1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
        7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
        2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11};
    
    _aux = S[(row * 16) + column + (k * 64)];
    return _aux;
}

void FourBitSBox (unsigned long long& a, int k)
{
    a = a << (60 - ( k * 4 ));
}

void cipher_DES(unsigned long long& plaintext, unsigned long long& key, unsigned long long& ciphertext)
{
    unsigned long long temp_right_half, temp_left_half, current_round_right_half;
    unsigned long int leftHalf, rightHalf;
    unsigned long int leftHalf_key, rightHalf_key;
    unsigned long long key_round;
    std::string mystring, tex;
    int IP [64] = {58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7};
    Permutation(plaintext, IP, 64);
    SplitHalf (plaintext, leftHalf, rightHalf, "plaintext");
    // key generation //
    // parity check bit are included in permutation and are removed afterwards
    int IP_key[64] =  {57,49,41,33,25,17,9,8,1,58,50,42,34,26,18,16,10,2,59,51,43,35,27,24,19,11,3,60,52,44,36,32,63,55,47,39,31,23,15,40,7,62,54,46,38,30,22,48,14,6,61,53,45,37,29,56,21,13,5,28,20,12,4,64};
    Permutation(key, IP_key, 64);
    ParityDropBit(key);
    temp_right_half = rightHalf;
    temp_left_half = leftHalf;
    for (int i=0; i<16;i++)
    {
        current_round_right_half = temp_right_half<< 32;
        ExpansionPermutation (temp_right_half);
        int num_left_shift [16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
        key_round = RoundKeyGenerator (key, leftHalf_key, rightHalf_key, num_left_shift[i]);
        temp_right_half ^= key_round;
        unsigned long long six_bit_temp, row, column, sub_S_box, out_S_box=0;
        for (int j=0; j<8; j++)
        {
            six_bit_temp = SixBitToFourBit (temp_right_half, row, column, j);
            sub_S_box = SBox(row, column, j);
            FourBitSBox (sub_S_box, j);
            out_S_box |=sub_S_box;
        }
        int IP_s_box_output[32] = {16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25};
        Permutation(out_S_box, IP_s_box_output, 64);
        out_S_box &=0xFFFFFFFF00000000;
        temp_right_half = temp_left_half ^ out_S_box;
        temp_right_half = temp_right_half >> 32;
        temp_left_half = current_round_right_half;
        out_S_box=0;
    }
    
    temp_right_half = temp_right_half<<32;
    temp_left_half = temp_left_half >>32;
    ciphertext = temp_right_half ^ temp_left_half;
    
    int IP_inv_final[64]={40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25};
    Permutation(ciphertext, IP_inv_final, 64);
    
}

int main()
{
    unsigned long long plaintext, key, ciphertext;
    
    plaintext = 0x123456789abcdef;
    key =0x133457799BBCDFF1;
    Display (plaintext, "The plaintext is: ", "hex");
    Display (key, "The key is: ", "hex");
    cipher_DES(plaintext, key, ciphertext);
    Display (ciphertext, "The ciphertext is: ", "hex");
    
    return 0;
}
