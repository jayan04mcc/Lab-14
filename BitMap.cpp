#include "BitMap.h"
#include <fstream>

BitMap::BitMap(const unsigned& size) : size(size) {
	for (unsigned i = 0; i < size; i++) {
		bits.push_back(false);
	}
}
BitMap::BitMap() : size(0) {};

void BitMap::inicializarNuevoMapa(const unsigned& size) {
	this->size = size;
	if (!bits.empty()) {
		bits.clear();
	}
	for (unsigned i = 0; i < size; i++) {
		bits.push_back(false);
	}
}
void BitMap::setBit(const unsigned& index) {
	if (index >= bits.size())
		throw std::out_of_range("El indice ingresado sale del rango del mapa de bits!\n");
	bits[index] = true;
}
void BitMap::clearBit(const unsigned& index) {
	if (index >= bits.size())
		throw std::out_of_range("El indice ingresado sale del rango del mapa de bits!\n");
	bits[index] = false;
}
bool BitMap::getBit(const unsigned& index) const {
	if (index >= bits.size()) {
		throw std::out_of_range("El indice ingresado sale del rango del mapa de bits!\n");
	}
	return bits[index];
}
auto BitMap::getSize() const {
	return bits.size();
}
auto BitMap::getTotalSize() {
	return sizeof(size) + sizeof(bool) * bits.size();
}
auto BitMap::getBitsActives() const {
	return std::count(bits.cbegin(), bits.cend(), true);
}
unsigned BitMap::getBitsNulls() const {
	return bits.size() - getBitsActives();
}
auto BitMap::getPosFirstNullBit() const {
	int c{ 0 };
	for (const auto& i : bits) {
		if (!i) { return c; }
		c++;
	}
	return -1;
}

std::ofstream& operator<<(std::ofstream& out, const BitMap& m_b) {
	out.write(reinterpret_cast<const char*>(&m_b.size), sizeof(m_b.size));
	for (const auto& b : m_b.bits) {
		out.write(reinterpret_cast<const char*>(&b), sizeof(b));
	}
	return out;
}
std::ifstream& operator>>(std::ifstream& in, BitMap& m_b) {
	in.read(reinterpret_cast<char*>(&m_b.size), sizeof(m_b.size));
	if (!m_b.bits.empty()) { m_b.bits.clear(); }
	bool aux;
	for (unsigned i = 0; i < m_b.size; i++) {
		in.read(reinterpret_cast<char*>(&aux), sizeof(aux));
		m_b.bits.push_back(aux);
	}
	return in;
}
std::ostream& operator<<(std::ostream& os, const BitMap& m_b) {
	for (const auto& b : m_b.bits) { os << b << ' '; }
	return os;
}