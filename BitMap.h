#pragma once
#include <vector>
#include <iostream>

class BitMap {
	template<typename T> friend class FixedLengthFile;
private:
	unsigned size{};
	std::vector<bool> bits{};
public:
	BitMap(const unsigned& size);
	BitMap();
	void inicializarNuevoMapa(const unsigned& size);
	void setBit(const unsigned& index);
	void clearBit(const unsigned& index);
	bool getBit(const unsigned& index) const;
	auto getSize() const;
	auto getTotalSize();
	auto getBitsActives() const;
	unsigned getBitsNulls() const;
	auto getPosFirstNullBit() const;
	friend std::ofstream& operator<<(std::ofstream& out, const BitMap& m_b);
	friend std::ifstream& operator>>(std::ifstream& in, BitMap& m_b);
	friend std::ostream& operator<<(std::ostream& os, const BitMap& m_b);
};

