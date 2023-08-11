#include <iostream>
#include <algorithm>
#include <string>
#include <string_view> //C++ 17
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem> //C++ 17
#include <vector>
#include <unordered_map>

#include "BitMap.h"
#include "TitanicPerson.h"
#include "BPlusTreeFile.h"

template<typename T>
class FixedLengthFile {
	template<typename R> friend class BufferPoolManager;
private:
	class MainHeader {
		template<typename R> friend class FixedLengthFile;
	private:
		unsigned total_pages{};
		unsigned number_records{};
	public:
		MainHeader(const unsigned& num_total_pags, const unsigned& campos_por_pag) : total_pages(num_total_pags), number_records(campos_por_pag) {}
		MainHeader() : total_pages(0), number_records(0) {}
		friend std::ofstream& operator<<(std::ofstream& out, const MainHeader& c) {
			out.write(reinterpret_cast<const char*>(&c), sizeof(c));
			return out;
		}
		friend std::ifstream& operator>>(std::ifstream& in, MainHeader& c) {
			in.read(reinterpret_cast<char*>(&c), sizeof(c));
			return in;
		}
	};

	class PageHeader {
		template<typename R> friend class FixedLengthFile;
	public:
		std::streampos next_header_pos{};
		size_t available_space{};
		BitMap bitmap{};
	public:
		PageHeader(const std::streampos& next_header_pos, const unsigned& tam_mp) : next_header_pos(next_header_pos), available_space(0) {
			bitmap.inicializarNuevoMapa(tam_mp);
		}
		PageHeader() : next_header_pos(0), available_space(0) {}
		friend std::ofstream& operator<<(std::ofstream& out, const PageHeader& c) {
			out.write(reinterpret_cast<const char*>(&c.next_header_pos), sizeof(c.next_header_pos));
			out.write(reinterpret_cast<const char*>(&c.available_space), sizeof(c.available_space));
			out << c.bitmap;
			return out;
		}
		friend std::ifstream& operator>>(std::ifstream& in, PageHeader& c) {
			in.read(reinterpret_cast<char*>(&c.next_header_pos), sizeof(c.next_header_pos));
			in.read(reinterpret_cast<char*>(&c.available_space), sizeof(c.available_space));
			in >> c.bitmap;
			return in;
		}
		friend std::ostream& operator<<(std::ostream& os, const PageHeader& c) {
			return os << "Espacio Disponible: " << c.available_space << "\nSiguiente Cabezera: " << c.next_header_pos
				<< "\nMapa de Bits: " << c.bitmap << '\n';
		}
	public:
		static auto getFixedSize() {
			return sizeof(next_header_pos) + sizeof(available_space);
		}
	};

	template<typename U>
	class Record {
		template<typename R> friend class FixedLengthFile;
	private:
		unsigned id{};
		U dato;
	public:
		Record(const U& dato, const unsigned& id) : dato(dato), id(id) {}
		Record() : id(1) {}
		friend std::ofstream& operator<<(std::ofstream& out, const Record<U>& e) {
			out.write(reinterpret_cast<const char*>(&e), sizeof(e));
			return out;
		}
		friend std::ifstream& operator>>(std::ifstream& in, Record<U>& e) {
			in.read(reinterpret_cast<char*>(&e), sizeof(e));
			return in;
		}
		friend std::ostream& operator>>(std::ostream& os, const Record<U>& e) {
			return os << std::left << std::setw(4) << e.id << e.dato;
		};
	};

private:
	using PointerPos_PageHeader = std::tuple<std::streampos, PageHeader>;
	unsigned total_pages{};
	unsigned number_records{};
	unsigned actual_page{};
	size_t PAGE_SIZE{};
	size_t PAGE_HEADER_SIZE{};
	const std::string FILE_PATH{};
	std::unordered_map<unsigned, PointerPos_PageHeader> map_header_page{};

private:
	void setCabezalPagina(const unsigned& page) {
		if (page >= total_pages)
			throw std::invalid_argument("El archivo no contiene el numero de paginas ingresado\n");

		std::ofstream output_file(FILE_PATH, std::ios::in, std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		output_file.seekp(std::get<0>(map_header_page[page]));
		output_file << std::get<1>(map_header_page[page]);
		output_file.close();
	}
	auto getPosCampo(const unsigned& page, const unsigned& pos) {
		if (pos >= number_records)
			throw std::invalid_argument("La indicada sale del rango de la cantidad maxima de campos\n");
		const auto record_pos_bytes{ static_cast<size_t>(std::get<0>(map_header_page[page])) + PAGE_HEADER_SIZE + sizeof(Record<T>) * (pos - 1) };
		return record_pos_bytes;
	}
public:
	FixedLengthFile(const std::string_view& FILE_PATH, const unsigned& total_pages, const unsigned& number_records)
		: FILE_PATH(FILE_PATH) {

		this->total_pages = total_pages;
		this->number_records = number_records;
		this->actual_page = 0;

		PAGE_HEADER_SIZE = PageHeader::getFixedSize() + sizeof(unsigned) + sizeof(bool) * number_records;
		PAGE_SIZE = sizeof(Record<T>) * number_records + PAGE_HEADER_SIZE;

		std::ofstream output_file(this->FILE_PATH, std::ios::out | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		auto main_header = MainHeader(total_pages, number_records);
		output_file << main_header;

		auto header_position{ static_cast<size_t>(output_file.tellp()) };

		Record<T> empty_record;

		for (unsigned i = 0; i < total_pages; i++) {
			PageHeader page_header(header_position + (i + 1u) * PAGE_SIZE, number_records);
			map_header_page.insert(std::make_pair(i, std::make_tuple(header_position + i * PAGE_SIZE, page_header)));

			output_file << page_header;

			for (unsigned e = 1; e <= number_records; e++) {
				empty_record.id = e + number_records * i;
				output_file << empty_record;
			}
		}
		output_file.close();
	}

	FixedLengthFile(const std::string_view& FILE_PATH) : FILE_PATH(FILE_PATH) {
		std::filesystem::path file_path(FILE_PATH);
		if (!std::filesystem::exists(file_path)) {
			throw std::invalid_argument("La direccion del archivo ingresado no es valida!\n");
		}

		actual_page = 0;

		std::ifstream input_file(this->FILE_PATH, std::ios::in, std::ios::binary);

		if (!input_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		MainHeader main_header;
		input_file >> main_header;

		this->total_pages = main_header.total_pages;
		this->number_records = main_header.number_records;

		PAGE_HEADER_SIZE = PageHeader::getFixedSize() + sizeof(unsigned) + sizeof(bool) * number_records;
		PAGE_SIZE = PAGE_HEADER_SIZE + sizeof(Record<T>) * number_records;

		auto header_position{ sizeof(MainHeader) };

		for (unsigned i = 0; i < total_pages; i++) {
			PageHeader page_header;
			input_file >> page_header;

			map_header_page.insert(std::make_pair(i, std::make_tuple(header_position + i * PAGE_SIZE, page_header)));

			input_file.seekg(page_header.next_header_pos);
		}

		input_file.close();
	}

	~FixedLengthFile() {}

	auto getPageSize() const {
		return PAGE_SIZE;
	}

	auto getActualPage() const {
		return actual_page;
	}

	void setActualPage(const unsigned& actual_page) {
		if (actual_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");
		this->actual_page = actual_page;
	}

	auto getTotalPages() const {
		return total_pages;
	}

	auto getFilePath() const {
		return FILE_PATH;
	}

	void fileInfo() const {
		std::cout << "CANTIDAD DE PAGINAS: " << total_pages << "\nCAMPOS POR PAGINA: " << number_records << "\nBYTES DEL TIPO DE DATO: " << sizeof(Record<T>)
			<< "\BYTES CABEZERAL INICIAL: " << sizeof(MainHeader) << "\nBYTES CABEZERAL DE PAGINA: " << PAGE_HEADER_SIZE << "\nBYTES POR PAGINA: "
			<< PAGE_SIZE << "\nBYTES TOTALES DEL ARCHIVO: " << sizeof(MainHeader) + PAGE_SIZE * total_pages << '\n';
	}

	void PageHeaderInfo() const {
		for (const auto& i : map_header_page) {
			std::cout << std::get<1>(i.second) << '\n';
		}
	}

	void readPage(const unsigned& actual_page) {
		if (actual_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indicada!\n");

		std::ifstream input_file(FILE_PATH, std::ios::in | std::ios::binary);
		if (!input_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'\n";
			exit(EXIT_FAILURE);
		}
		input_file.seekg(static_cast<size_t>(std::get<0>(map_header_page[actual_page])) + PAGE_HEADER_SIZE);

		auto pos_limite{ std::get<1>(map_header_page[actual_page]).next_header_pos };

		Record<T> campo;

		while (input_file.tellg() < pos_limite && !input_file.eof()) {
			input_file >> campo;
			std::cout << campo.dato << '\n';
		}

		input_file.close();
	}

	void readFirstPage() {
		readPage(0);
	}

	void readLastPage() {
		readPage(total_pages - 1);
	}

	void readPreviousPage() {
		readPage(actual_page - 1);
	}

	void readNextPage() {
		readPage(actual_page + 1);
	}

	void readCurrentPage() {
		readPage(actual_page);
	}

	void readAllPages() {
		for (unsigned i = 0; i < total_pages; i++) { 
			std::cout << "PAGINA: " << i << "\n\n";
			readPage(i); }
	}

	void writePage(const unsigned& current_page, const std::vector<T>& vec_records) {
		if (current_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");

		std::ofstream output_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		output_file.seekp(static_cast<size_t>(std::get<0>(map_header_page[current_page])) + PAGE_HEADER_SIZE);

		std::vector<Record<T>> new_vec_records;

		unsigned id{ 1 + number_records * current_page };
		for (const auto& r : vec_records) {
			new_vec_records.push_back(Record<T>(r, id));
			id++;
		}

		const auto max_size{ new_vec_records.size() <= number_records ? new_vec_records.size() : number_records };

		unsigned index{ 0 };
		for (unsigned i = 0; i < max_size; i++) {
			std::get<1>(map_header_page[current_page]).bitmap.setBit(index);
			output_file << new_vec_records[i];
			index++;
		}

		output_file.close();
		setCabezalPagina(current_page);
	}

	void writePage(const unsigned& actual_page, const T& record, const unsigned& record_pos = 0) {
		if (actual_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indica!\n");

		std::ofstream output_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		auto aux_record_pos{ record_pos };

		if (record_pos == 0) {
			const auto empty_bit_pos{ std::get<1>(map_header_page[actual_page]).bitmap.getPosFirstNullBit() };
			if (empty_bit_pos == -1)
				return;
			aux_record_pos = empty_bit_pos + 1;
		}

		const auto records_pos_bytes{ getPosCampo(actual_page, aux_record_pos) };

		output_file.seekp(records_pos_bytes);

		output_file << Record<T>(record, aux_record_pos + number_records * actual_page);

		output_file.close();

		std::get<1>(map_header_page[actual_page]).bitmap.setBit(aux_record_pos - 1);
		setCabezalPagina(actual_page);
		return;
	}

	void writeCurrentPage(const std::vector<T>& registro) {
		writePage(actual_page, registro);
	}

	void writeCurrentPage(const T& campo, const unsigned& numero_campo = 0) {
		writePage(actual_page, campo, numero_campo);
	}

	void addNewPage() {

		std::ofstream out_file(FILE_PATH, std::ios::in | std::ios::binary);

		if (!out_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		const auto pos_new_header{ std::get<1>(map_header_page[total_pages - 1]).next_header_pos };

		PageHeader new_header(pos_new_header + PAGE_SIZE, number_records);
		map_header_page.insert(std::make_pair(total_pages, std::make_tuple(pos_new_header, new_header)));
		total_pages++;

		out_file.seekp(pos_new_header);
		out_file << new_header;

		Record<T> empty_record;
		for (unsigned e = 1; e <= number_records; e++) {
			empty_record.id = e + number_records * (total_pages - 1);
			out_file << empty_record;
		}

		out_file.close();
		setCabezalPagina(std::get<1>(map_header_page[total_pages - 1]));
	}

	auto getRecods(const unsigned& actual_page) {

		if (actual_page >= total_pages)
			throw std::invalid_argument("El archivo no contiene la cantidad de paginas indicada!\n");

		std::ifstream input_file(FILE_PATH, std::ios::in | std::ios::binary);
		if (!input_file) {
			std::cerr << "No se pudo abrir el archivo '" + FILE_PATH + "'\n";
			exit(EXIT_FAILURE);
		}
		input_file.seekg(static_cast<size_t>(std::get<0>(map_header_page[actual_page])) + PAGE_HEADER_SIZE);

		auto pos_limite{ std::get<1>(map_header_page[actual_page]).next_header_pos };

		Record<T> campo;
		std::vector<T> records{};

		while (input_file.tellg() < pos_limite && !input_file.eof()) {
			input_file >> campo;
			records.push_back(campo.dato);
		}

		input_file.close();

		return records;
	}
};

constexpr std::string_view FILE_TITANIC_PATH{ "personas_titanic.bin" };
constexpr unsigned NUMBER_PAGES{ 36 };
constexpr unsigned NUMBER_RECORDS{ 25 };

constexpr std::string_view FILE_B_TREE_PATH{ "b_plus_tree_index.bin" };
constexpr unsigned DEGREE{ 5 };


void create_file(FixedLengthFile<TitanicPerson>* file);
void insert_records_b_tree(BPlusTreeFile<unsigned, unsigned, DEGREE>* _b_tree, FixedLengthFile<TitanicPerson>* _file);
void b_plus_tree_test_1();
void b_plus_tree_test_2();


int main() {

	b_plus_tree_test_1();

	//b_plus_tree_test_2();


	return 0;
}

void create_file(FixedLengthFile<TitanicPerson>* file) {

	std::vector<TitanicPerson> vec{};
	unsigned current_page{ 0 };

	for (unsigned i = 0; i < NUMBER_PAGES - 1; i++) {
		//std::cout << i * NUMBER_RECORDS + 1 << ' ' << (i + 1) * NUMBER_RECORDS << '\n';
		vec = TitanicPerson::getRecords(i * NUMBER_RECORDS, (i + 1) * NUMBER_RECORDS);
		file->writeCurrentPage(vec);
		current_page++;
		file->setActualPage(current_page);
	}
}

// Crea un arbol b - tree ingresando los registros almacenados no vacios de el archivo de longitud fija
void insert_records_b_tree(BPlusTreeFile<unsigned, unsigned, DEGREE>* _b_tree, FixedLengthFile<TitanicPerson>* _file) { 
	std::vector<TitanicPerson> vec_ti_person{};

	unsigned total_pages{ _file->getTotalPages() };
	int id_passanget{};

	for (unsigned i = 0; i < total_pages; i++) {
		vec_ti_person = _file->getRecods(i);

		for (const auto& j : vec_ti_person) {

			id_passanget = j.getPassenget_Id();
			if (id_passanget != -1) {
				_b_tree->insert_key_value(id_passanget, i); //insertar un nodo llave, valor
			}
		}
		vec_ti_person.clear();
	}
}

void b_plus_tree_test_1() {
	//Cargamos desde un archivo todos su contenido al objeto FixedLengthFile
	auto file{ FixedLengthFile<TitanicPerson>(FILE_TITANIC_PATH, NUMBER_PAGES, NUMBER_RECORDS) };
	create_file(&file);

	file.readAllPages();

	//Creamos un objeto BPlusTreeFile que contiene un BPlusTree y guarda en un archivo su contenido
	//para poder recuperar su contenido
	auto b_tree_file{ BPlusTreeFile<unsigned, unsigned, DEGREE>(FILE_B_TREE_PATH) };
	insert_records_b_tree(&b_tree_file, &file);

	//Buscar Nodo
	std::cout << b_tree_file.get_value(435) << "\n";

	//Eliminar Nodo
	b_tree_file.delete_value(435); //delete

	try {
		std::cout << b_tree_file.get_value(435);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}
}

void b_plus_tree_test_2() {
	//Creamos un Objeto BPlusTreeFile cargando el contenido guardado de un archivo de un
	//BPlusTreeFile anterior

	auto new_file{ BPlusTreeFile<unsigned, unsigned, DEGREE>(FILE_B_TREE_PATH, false) };
	//Eliminar Nodo
	new_file.delete_value(435); //delete

	try {
		std::cout << new_file.get_value(435);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}
}