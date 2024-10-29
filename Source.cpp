#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;

unsigned char write_bit = 0;
int bit_len = 8;

int indexForSymbol(const map<char, int>& sin, char irgum) {
    int j = 0;
    for (const auto& i : sin) {
        if (irgum == i.first) {
            return j + 2;
        }
        j++;
    }
    cout << "error" << endl;
    return -1;
}

void outPutBit(int bit, ofstream& outputfile) {
    write_bit >>= 1;
    if (bit & 1) {
        write_bit |= 0x80;
    }
    bit_len--;
    if (bit_len == 0) {
        bit_len = 8;
        outputfile.write(reinterpret_cast<const char*>(&write_bit), 1);
    }
}

void bitPlusFollow(int bit, int bittofollow, ofstream& outputfile) {
    outPutBit(bit, outputfile);
    for (int i = 0; i < bittofollow; i++) {
        outPutBit(~bit, outputfile);
    }
}

int readBit(ifstream& inputfile) {
    static unsigned char buffer = 0;
    static int bit_pos = 0;

    if (bit_pos == 0) {
        inputfile.read(reinterpret_cast<char*>(&buffer), 1);
        bit_pos = 8;
    }

    int bit = buffer & 1;
    buffer >>= 1;
    bit_pos--;
    return bit;
}

void decode(const vector<pair<char, int>>& sorted_slovar, ifstream& inputfile, ofstream& outputfile) {
    int low_v = 0;
    int high_v = (1 << 16) - 1;
    int delete_val = sorted_slovar.back().second;
    int diff = high_v - low_v + 1;
    int first_q = (high_v + 1) / 4;
    int half_q = first_q * 2;
    int third_q = first_q * 3;
    int bit_to_follow = 0;

    int code = 0;
    for (int i = 0; i < 16; i++) {
        code = (code << 1) | readBit(inputfile);
    }

    while (true) {
        int j = 0;
        for (const auto& i : sorted_slovar) {
            int low_v_new = low_v + i.second * diff / delete_val;
            if (code >= low_v && code < low_v_new) {
                outputfile.put(i.first);
                high_v = low_v_new - 1;
                low_v = low_v;
                break;
            }
            low_v = low_v_new;
            j++;
        }

        if (j == sorted_slovar.size()) {
            break; // Все символы декодированы
        }

        while (true) {
            if (high_v < half_q) {
                // Ничего не делаем
            }
            else if (low_v >= half_q) {
                code -= half_q;
                low_v -= half_q;
                high_v -= half_q;
            }
            else if (low_v >= first_q && high_v < third_q) {
                code -= first_q;
                low_v -= first_q;
                high_v -= first_q;
            }
            else {
                break;
            }
            low_v += low_v;
            high_v = high_v * 2 + 1;
            code = (code << 1) | readBit(inputfile);
        }
        diff = high_v - low_v + 1;
    }
}

int main() {
    setlocale(LC_ALL, "Rus");

    map<char, int> slovar;
    int sum_slovar = 0;
    ifstream fp("input.txt");
    char chunk;
    int test_sum = 0;

    while (fp.get(chunk)) {
        test_sum++;
        if (slovar.find(chunk) == slovar.end()) {
            slovar[chunk] = 1;
        }
        else {
            slovar[chunk]++;
        }
    }
    fp.close();

    for (const auto& val : slovar) {
        sum_slovar += val.second;
    }

    if (test_sum == sum_slovar) {
        cout << "Сжатие выполнено успешно" << endl;
    }
    else {
        cout << "Сжатие не выполнено" << endl;
    }

    vector<pair<char, int>> sorted_slovar(slovar.begin(), slovar.end());
    sort(sorted_slovar.begin(), sorted_slovar.end(), [](const pair<char, int>& a, const pair<char, int>& b) {
        return a.second > b.second;
        });

    vector<int> slovar_mas = { 0, 1 };
    for (const auto& i : sorted_slovar) {
        slovar_mas.push_back(i.second + slovar_mas.back());
    }

    ofstream f("output.txt", ios::binary);
    int size = sorted_slovar.size();
    f.write(reinterpret_cast<const char*>(&size), 1);
    for (const auto& i : sorted_slovar) {
        f.write(&i.first, 1);
        f.write(reinterpret_cast<const char*>(&i.second), 4);
    }

    fp.open("input.txt");
    int low_v = 0;
    int high_v = (1 << 16) - 1;
    int delete_val = slovar_mas.back();
    int diff = high_v - low_v + 1;
    int first_q = (high_v + 1) / 4;
    int half_q = first_q * 2;
    int third_q = first_q * 3;
    int bit_to_follow = 0;

    while (fp.get(chunk)) {
        int j = indexForSymbol(map<char, int>(sorted_slovar.begin(), sorted_slovar.end()), chunk);
        high_v = low_v + slovar_mas[j] * diff / delete_val - 1;
        low_v = low_v + slovar_mas[j - 1] * diff / delete_val;

        while (true) {
            if (high_v < half_q) {
                bitPlusFollow(0, bit_to_follow, f);
                bit_to_follow = 0;
            }
            else if (low_v >= half_q) {
                bitPlusFollow(1, bit_to_follow, f);
                bit_to_follow = 0;
                low_v -= half_q;
                high_v -= half_q;
            }
            else if (low_v >= first_q && high_v < third_q) {
                bit_to_follow++;
                low_v -= first_q;
                high_v -= first_q;
            }
            else {
                break;
            }
            low_v += low_v;
            high_v = high_v * 2 + 1;
        }
        diff = high_v - low_v + 1;
    }

    high_v = low_v + slovar_mas[1] * diff / delete_val - 1;
    low_v = low_v + slovar_mas[0] * diff / delete_val;

    while (true) {
        if (high_v < half_q) {
            bitPlusFollow(0, bit_to_follow, f);
            bit_to_follow = 0;
        }
        else if (low_v >= half_q) {
            bitPlusFollow(1, bit_to_follow, f);
            bit_to_follow = 0;
            low_v -= half_q;
            high_v -= half_q;
        }
        else if (low_v >= first_q && high_v < third_q) {
            bit_to_follow++;
            low_v -= first_q;
            high_v -= first_q;
        }
        else {
            break;
        }
        low_v += low_v;
        high_v = high_v * 2 + 1;
    }
    bit_to_follow++;
    if (low_v < first_q) {
        bitPlusFollow(0, bit_to_follow, f);
        bit_to_follow = 0;
    }
    else {
        bitPlusFollow(1, bit_to_follow, f);
        bit_to_follow = 0;
    }

    write_bit >>= bit_len;
    f.write(reinterpret_cast<const char*>(&write_bit), 1);
    f.close();

    // Подсчёт сжатия
    ifstream input_file("input.txt", ios::binary | ios::ate);
    ifstream output_file("output.txt", ios::binary | ios::ate);
    int sid = input_file.tellg();
    int sib = output_file.tellg();
    input_file.close();
    output_file.close();

    cout << "Сжатие (в %): " << (static_cast<double>(sib) / sid) * 100 << "%" << endl;

    // Декодирование
    ifstream inputfile("output.txt", ios::binary);
    ofstream outputfile("decoded.txt");
    decode(sorted_slovar, inputfile, outputfile);
    inputfile.close();
    outputfile.close();

    return 0;
}
