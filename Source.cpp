#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define DEBUG_ENCODE 0
#define DEBUG_DECODE 0

using namespace std;

std::string getAlpabet(std::string input);

int* getFrequency(std::string input, std::string alphabet);

int getSymbolIdx(char symbol, std::string alphabet);

void addBit(bool bit, int bitsToFollow, std::string& output);

unsigned short int getValue(std::string s);

unsigned short int getNewBit(char s);

std::string encode(std::string input, std::string alphabet, int* frequency);

std::string decode(std::string code, std::string alphabet, int* frequency);

std::string getAlpabet(std::string input)
{
	std::string alphabet;
	alphabet.push_back('-');
	alphabet.push_back('\0');
	for (int i = 0; i < input.size(); i++)
	{
		int j;
		for (j = 0; j < alphabet.size(); j++)
		{
			if (alphabet[j] == input[i])
			{
				break;
			}
		}
		if (j >= alphabet.size())
		{
			alphabet.push_back(input[i]);
		}
	}
	return alphabet;
}

int* getFrequency(std::string input, std::string alphabet)
{
	int* frequency = new int[alphabet.size()];
	frequency[0] = 0;
	for (int i = 1; i < alphabet.size(); ++i)
	{
		frequency[i] = 1;
	}
	for (int i = 0; i < input.size(); i++)
	{
		bool flag = false;
		for (int j = 0; j < alphabet.size(); j++)
		{
			if (alphabet[j] == input[i] || flag)
			{
				frequency[j]++;
				flag = true;
			}
		}
	}
	return frequency;
}

int getSymbolIdx(char symbol, std::string alphabet)
{
	for (int i = 0; i < alphabet.size(); ++i)
	{
		if (symbol == alphabet.at(i))
		{
			return i;
		}
	}
	return 0;
}

void addBit(bool bit, int bitsToFollow, std::string& output)
{
	output.push_back(bit + 48);
	while (bitsToFollow > 0)
	{
		output.push_back(!bit + 48);
		bitsToFollow -= 1;
	}
}

unsigned short int getValue(std::string s) {
	int i;
	unsigned short int value = 0;
	for (i = 0; i < 16; i++)
	{
		if (i == s.size())
		{
			return value;
		}
		if ('1' == s[i])
		{
			value = value | (1 << (15 - i));
		}
	}
	return value;
}

unsigned short int getNewBit(char s)
{
	if ('1' == s)
	{
		return 1;
	}
	return 0;
}

std::string encode(std::string input, std::string alphabet, int* frequency)
{
	input.push_back('\0');
	const int length = input.length();

	unsigned short int* h = new unsigned short int[input.length()];
	unsigned short int* l = new unsigned short int[input.length()];
	h[0] = 65535;
	l[0] = 0;
	const unsigned int del = frequency[alphabet.size() - 1];

	const unsigned short int FIRST_QTR = h[0] / 4 + 1;
	const unsigned short int HALF = 2 * FIRST_QTR;
	const unsigned short int THIRD_QTR = 3 * FIRST_QTR;

	std::string code;
	int bitsToFollow = 0;

	if (DEBUG_ENCODE)
	{
		std::cout << "Input: " << input << std::endl;
		std::cout << "Alphabet: " << alphabet << std::endl;
		std::cout << "Del: " << del << std::endl;
		std::cout << "Frequency:" << std::endl;
		for (int i = 0; i < alphabet.size(); ++i)
		{
			std::cout << "	" << alphabet.at(i) << " : " << frequency[i] << std::endl;
		}
		std::cout << "=============" << std::endl << std::endl;
	}

	for (int i = 0; i < length;)
	{
		int symbol = getSymbolIdx(input.at(i), alphabet);
		i += 1;

		unsigned int range = h[i - 1] - l[i - 1] + 1;
		l[i] = l[i - 1] + (range * frequency[symbol - 1]) / del;
		h[i] = l[i - 1] + (range * frequency[symbol]) / del - 1;

		if (DEBUG_ENCODE)
		{
			std::cout << "Symbol: " << alphabet.at(symbol) << std::endl;

			std::cout << "Frequency: " << frequency[symbol] - frequency[symbol - 1] <<
				" [" << frequency[symbol - 1] << "; " << frequency[symbol] << "]" << std::endl;
			std::cout << "[" << l[i] << "; " << h[i] << ")" << std::endl;
		}

		for (;;)
		{
			if (h[i] < HALF)
			{
				addBit(0, bitsToFollow, code);
				bitsToFollow = 0;
			}
			else if (l[i] >= HALF)
			{
				addBit(1, bitsToFollow, code);
				bitsToFollow = 0;
				l[i] -= HALF;
				h[i] -= HALF;
			}
			else if (l[i] >= FIRST_QTR && h[i] < THIRD_QTR)
			{
				bitsToFollow += 1;
				l[i] -= FIRST_QTR;
				h[i] -= FIRST_QTR;
			}
			else
			{
				break;
			}

			l[i] = 2 * l[i];
			h[i] = 2 * h[i] + 1;

			if (DEBUG_ENCODE) std::cout << "[" << l[i] << "; " << h[i] << ")" << std::endl;
		}
		if (DEBUG_ENCODE)
		{
			std::cout << "Output: " << code << std::endl;
			std::cout << "-------------" << std::endl;
		}
	}
	if (DEBUG_ENCODE)
	{
		for (int i = 0; i <= length; ++i) {
			std::cout << "[" << l[i] << "; " << h[i] << ")" << std::endl;
		}
		std::cout << "Output: " << code << std::endl;
	}

	return code;
}

std::string decode(std::string code, std::string alphabet, int* frequency)
{
	unsigned short int* h = new unsigned short int[code.length()];
	unsigned short int* l = new unsigned short int[code.length()];
	l[0] = 0;
	h[0] = 65535;
	const unsigned int del = frequency[alphabet.size() - 1];

	const unsigned short int FIRST_QTR = h[0] / 4 + 1;
	const unsigned short int HALF = 2 * FIRST_QTR;
	const unsigned short int THIRD_QTR = 3 * FIRST_QTR;

	std::string output;
	unsigned short int newBit;
	unsigned short int value;
	value = getValue(code);
	int codeIdx = 16;
	bool isCodeEnded = 0;

	for (int i = 1;; ++i)
	{
		unsigned int range = h[i - 1] - l[i - 1] + 1;
		unsigned int cum = ((value - l[i - 1] + 1) * del - 1) / range;

		int symbol;
		for (symbol = 1; frequency[symbol] <= cum; ++symbol) {};

		l[i] = l[i - 1] + (range * frequency[symbol - 1]) / del;
		h[i] = l[i - 1] + (range * frequency[symbol]) / del - 1;

		if (DEBUG_DECODE)
		{
			std::cout << "Symbol: " << alphabet.at(symbol) << std::endl;
			std::cout << "Symbol idx: " << symbol << std::endl;
			std::cout << "Value: " << value << std::endl;
			std::cout << "----------------" << std::endl;
		}
		if (alphabet.at(symbol) == '\0')
		{
			return output;
		}
		output.push_back(alphabet.at(symbol));

		for (;;)
		{
			if (h[i] >= HALF)
			{
				if (l[i] >= HALF)
				{
					value -= HALF;
					l[i] -= HALF;
					h[i] -= HALF;
				}
				else if (l[i] >= FIRST_QTR && h[i] < THIRD_QTR)
				{
					value -= FIRST_QTR;
					l[i] -= FIRST_QTR;
					h[i] -= FIRST_QTR;
				}
				else
				{
					break;
				}
			}

			if (codeIdx < code.length())
			{
				newBit = getNewBit(code.at(codeIdx++));
			}
			else if (isCodeEnded)
			{
				newBit = 0;
			}
			else
			{
				newBit = 1;
				isCodeEnded = 1;
			}

			l[i] = 2 * l[i];
			h[i] = 2 * h[i] + 1;
			value = 2 * value + newBit;

			if (DEBUG_DECODE) std::cout << "[" << l[i] << "; " << h[i] << ")" << std::endl;
		}
	}
}

void main()
{
	ifstream inputFile("input.txt");
	if (!inputFile.is_open()) {
		cerr << "������ �������� ����� input.txt" << endl;
		return;
	}
	string input((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
	inputFile.close();

	string alphabet = getAlpabet(input);
	int* frequency = getFrequency(input, alphabet);

	string encoded = encode(input, alphabet, frequency);
	string decoded = decode(encoded, alphabet, frequency);

	ofstream encodedFile("encoded.bin", ios::binary);
	if (!encodedFile.is_open()) {
		cerr << "������ �������� ����� encoded.bin" << endl;
		return;
	}
	encodedFile.write(encoded.c_str(), encoded.size());
	encodedFile.close();

	ofstream decodedFile("decoded.txt");
	if (!decodedFile.is_open()) {
		cerr << "������ �������� ����� decoded.txt" << endl;
		return;
	}
	decodedFile << decoded;
	decodedFile.close();

	cout << "Original: " << input << endl;
	cout << "Encoded: " << encoded << endl;
	cout << "Decoded: " << decoded << endl;
}