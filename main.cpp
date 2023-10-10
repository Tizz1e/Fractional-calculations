#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

class FixedPoint {
private:
    unsigned value;
    int shift;
    int size;
    unsigned int reverseValue(unsigned int givenValue) const {
        return (~givenValue + 1) & ((1LL << size) - 1);
    }
public:
    FixedPoint(const string& _format, const string& _value) {
        shift = stoi(_format.substr(_format.find('.') + 1));
        size = stoi(_format.substr(0, _format.find('.'))) + shift;
        if (size > 32 || size - shift < 1) {
            cerr << "Incorrect format\n";
            throw string("incorrect format");
        }
        value = stoll(_value, 0, 16);
    }
    FixedPoint(unsigned value, int size, int shift) {
        this->value = value;
        this->size = size;
        this->shift = shift;
    }
    FixedPoint() {
        value = 0;
        shift = 0;
        size = 0;
    }
    int getSign() const {
        return (int)value >> (size - 1);
    }
    FixedPoint operator + (const FixedPoint& other) const {
        unsigned newValue = (value + other.value) & ((1LL << size) - 1);
        return {newValue, size, shift};
    }
    FixedPoint operator - (const FixedPoint& other) const {
        unsigned newValue = value + reverseValue(other.value);
        return {newValue, size, shift};
    }
    FixedPoint operator * (const FixedPoint& other) const {
        int firstSign = getSign();
        int secondSign = other.getSign();
        int newSign = firstSign ^ secondSign;
        unsigned value1 = value;
        if (firstSign) {
            value1 = reverseValue(value1);
        }
        unsigned int value2 = other.value;
        if (secondSign) {
            value2 = reverseValue(value2);
        }
        unsigned int newValue = (((unsigned long long)value1 * value2) >> shift) & ((1LL << size) - 1);
        if (newSign) {
            newValue = reverseValue(newValue);
        }
        return {newValue, size, shift};
    }
    FixedPoint operator / (const FixedPoint& other) const {
        if (other.value == 0) {
            return {};
        }
        int firstSign = getSign();
        int secondSign = other.getSign();
        int newSign = firstSign ^ secondSign;
        unsigned value1 = value;
        if (firstSign) {
            value1 = reverseValue(value1);
        }
        unsigned int value2 = other.value;
        if (secondSign) {
            value2 = reverseValue(value2);
        }
        unsigned int newValue = (((unsigned long long)value1 << shift) / value2) & ((1LL << size) - 1);
        if (newSign) {
            newValue = reverseValue(newValue);
        }
        return {newValue, size, shift};
    }
    void out() {
        if (size == 0) {
            cout << "error\n";
            cerr << "Division by zero error\n";
        }
        int sign = getSign();
        unsigned int tmpValue = value;
        if (sign) {
            cout << "-";
            tmpValue = reverseValue(tmpValue);
        }
        int integerPart = (int)tmpValue >> shift;
        cout << integerPart << ".";
        int fractionPart = (int)tmpValue & ((1 << shift) - 1);
        fractionPart = (fractionPart * 1000) >> shift;
        if (fractionPart == 0) {
            cout << "000\n";
        } else if (fractionPart < 10) {
            cout << "00";
            cout << fractionPart << '\n';
        } else if (fractionPart < 100) {
            cout << "0";
            cout << fractionPart << '\n';
        } else {
            cout << fractionPart << '\n';
        }
    }
};

class FloatingPoint {
private:
    int sign;
    int exponent;
    int exponentSize;
    int significand;
    int significandSize;
    char type;
    bool isNaN = false;
    bool isInf = false;
    bool isZero = false;
    bool isSubnormal = false;

    void check() {
        if (exponent == 0) {
            if (significand == 0) {
                isZero = true;
            } else {
                isSubnormal = true;
                exponent = 1;
            }
        } else if (exponent == ((1 << exponentSize) - 1)) {
            if (significand == 0) {
                isInf = true;
            } else {
                isNaN = true;
            }
        }
    }
    void setConstants(char _type) {
        type = _type;
        if (_type == 'f') {
            significandSize = 23;
            exponentSize = 8;
        } else {
            significandSize = 10;
            exponentSize = 5;
        }
    }
    int realSignificand() const {
        if (!isSubnormal) {
            return significand | (1 << significandSize);
        }
        return significand;
    }
    void denormalize() {
        int maxExponent = (1 << exponentSize) - (1 << (exponentSize - 1)) - 1;
        while ((significand >> (significandSize + 1)) > 0) {
            significand >>= 1;
            exponent++;
        }
        if (exponent > maxExponent) {
            isInf = true;
        }
        significand &= (1 << significandSize) - 1;
    }
    void normalize() {
        int minExponent = 1;
        while (((significand >> significandSize) & 1) == 0) {
            significand <<= 1;
            exponent--;
        }
        if (exponent < minExponent) {
            isZero = true;
        }
        significand &= (1 << significandSize) - 1;
    }
    FloatingPoint returnNaN() const {
        return {type, 0, (1 << exponentSize) - 1, 1};
    }
    FloatingPoint returnZero() const {
        return {type, 0, 0, 0};
    }

public:
    FloatingPoint(char _type, string& _input) {
        setConstants(_type);
        unsigned int input = stoll(_input, 0, 16);
        sign = (int)(input >> (significandSize + exponentSize));
        exponent = (int)(input >> significandSize) & ((1 << exponentSize) - 1);
        significand = (int)input & ((1 << significandSize) - 1);
        check();
    }
    FloatingPoint(char _type, int _sign, int _exponent, int _significand) {
        setConstants(_type);
        sign = _sign;
        significand = _significand;
        exponent = _exponent;
        check();
    }
    FloatingPoint(char _type) {
        setConstants(_type);
        sign = 0;
        exponent = 0;
        significand = 0;
    }
    FloatingPoint operator + (const FloatingPoint other) const {
        if (isNaN) {
            return *this;
        }
        if (other.isNaN) {
            return other;
        }
        if (isInf && !other.isInf) {
            return *this;
        }
        if (!isInf && other.isInf) {
            return other;
        }
        if (isInf && other.isInf) {
            if (sign == other.sign) {
                return other;
            } else {
                return {type, 0, 1, (1 << exponentSize) - 1};
            }
        }

        if (!sign && other.sign) {
            return *this - other;
        }
        if (sign && !other.sign) {
            return other - *this;
        }

        int newExponent = max(exponent, other.exponent);
        int value1 = realSignificand();
        int value2 = other.realSignificand();
        if (exponent != newExponent) {
            value1 >>= newExponent - exponent;
        }
        if (other.exponent != newExponent) {
            value2 >>= newExponent - other.exponent;
        }
        FloatingPoint result = FloatingPoint(type, sign, newExponent, value1 + value2);
        result.denormalize();
        result.normalize();
        return result;
    }
    FloatingPoint operator - (const FloatingPoint other) const {
        if (isNaN) {
            return *this;
        }
        if (other.isNaN) {
            return other;
        }
        if (isInf && !other.isInf) {
            return *this;
        }
        if (!isInf && other.isInf) {
            return {type, other.sign ^ 1, (1 << other.exponentSize) - 1, other.significand};
        }
        if (isInf && other.isInf) {
            if (sign ^ other.sign) {
                return *this;
            } else {
                return returnNaN();
            }
        }

        if (sign ^ other.sign) {
            return *this + FloatingPoint(other.type, other.sign ^ 1, other.exponent, other.significand);
        }
        int newExponent = max(exponent, other.exponent);
        int value1 = realSignificand();
        int value2 = other.realSignificand();
        if (exponent != newExponent) {
            value1 >>= newExponent - exponent;
        }
        if (other.exponent != newExponent) {
            value2 >>= newExponent - exponent;
        }
        if (value1 == value2) {
            return returnZero();
        }
        int newSign = 0;
        if (value1 > value2) {
            newSign = sign;
        } else {
            newSign = other.sign;
        }
        int newValue = abs(value1 - value2);
        FloatingPoint result = FloatingPoint(type, newSign, newExponent, newValue);
        result.denormalize();
        result.normalize();
        return result;
    }
};



int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 6) {
        cerr << "Invalid number of arguments\n";
        return 1;
    }
    if (string(argv[2]) != "0") {
        cerr << "Program works only with rounding to zero\n";
        return 1;
    }
    string format = string(argv[1]);
    if (argc == 4) {
        string input = string(argv[3]);
        if (format == "f" || format == "h") {
            FloatingPoint num(format[0], input);
            //num.out();
        } else {
            FixedPoint num(format, input);
            num.out();
        }
    } else {
        string input1 = string(argv[3]);
        string input2 = string(argv[5]);
        string operation = string(argv[4]);
        if (format == "f" || format == "h") {
            FloatingPoint num1(format[0], input1);
            FloatingPoint num2(format[0], input2);
            FloatingPoint res(format[0]);
            if (operation == "+") {
                res = num1 + num2;
            } else if (operation == "-") {
                res = num1 - num2;
            }
        } else {
            FixedPoint num1(format, input1);
            FixedPoint num2(format, input2);
            FixedPoint res;
            if (operation == "+") {
                res = num1 + num2;
            } else if (operation == "-") {
                res = num1 - num2;
            } else if (operation == "*") {
                res = num1 * num2;
            } else {
                res = num1 / num2;
            }
            res.out();
        }
    }
    return 0;
}
