#pragma once
#include "BPlusTree.h";
#include <vector>
#include <fstream>
#include <string_view>
#include <filesystem>

template<typename key_type, typename val_type, size_t DEGREE = 3>
class BPlusTreeFile {
private:
	BPlusTree<key_type, val_type, DEGREE> b_tree;
	const std::string FILE_PATH;

public:

	BPlusTreeFile(const std::string_view& _FILE_PATH, bool create_new_tree = true) : FILE_PATH(_FILE_PATH) {
		//Crea un nuevo arbol
		if (create_new_tree) {
			std::ofstream output_file(this->FILE_PATH, std::ios::out | std::ios::binary);
			if (!output_file) {
				std::cerr << "No se pudo crear el archivo '" + this->FILE_PATH + "'!\n";
				exit(EXIT_FAILURE);
			}
		//Carga desde un archivo y reecrea un arbol
		} 
		else {
			std::filesystem::path file_path(FILE_PATH);
			if (!std::filesystem::exists(file_path)) {
				throw std::invalid_argument("La direccion del archivo ingresado no es valida!\n");
			}

			std::ifstream input_file(this->FILE_PATH, std::ios::in, std::ios::binary);

			if (!input_file) {
				std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
				exit(EXIT_FAILURE);
			}

			//Cargando datos del cabezal
			size_t aux_degree{};

			input_file.read(reinterpret_cast<char*>(&aux_degree), sizeof(aux_degree));

			if (aux_degree != DEGREE) {
				throw std::invalid_argument("¡El grado del arbol ingresado, no coincide con el del archivo!\n");
			}

			std::cout << "degree: " << aux_degree << "\n";

			//Cargando cantidad de valores y llaves
			size_t size_keys{}, size_values{};

			input_file.read(reinterpret_cast<char*>(&size_keys), sizeof(size_keys));
			input_file.read(reinterpret_cast<char*>(&size_values), sizeof(size_values));

			std::cout << size_keys << " " << size_values << "\n";

			//Cargando llaves y valores
			std::vector<key_type> vec_keys{};
			std::vector<val_type> vec_values{};

			key_type aux_key{};
			for (size_t i = 0; i < size_keys; i++) {
				input_file.read(reinterpret_cast<char*>(&aux_key), sizeof(aux_key));
				vec_keys.push_back(aux_key);
			}

			val_type aux_value{};
			for (size_t i = 0; i < size_values; i++) {
				input_file.read(reinterpret_cast<char*>(&aux_value), sizeof(aux_value));
				vec_values.push_back(aux_value);
			}
		
			//Insertanto las llaves y sus valores al arbol
			for (size_t i = 0; i < size_keys; i++) {
				insert_key_value(vec_keys[i], vec_values[i]);
			}
		}
	}

	~BPlusTreeFile() { save_b_tree(); }

	/*Guarda las llaves y valores del arbol en un archivo para posteriormente cargar el arbol*/
	void save_b_tree() {
		std::filesystem::path file_path(FILE_PATH);
		if (!std::filesystem::exists(file_path)) {
			throw std::invalid_argument("La direccion del archivo ingresado no es valida!\n");
		}

		std::ofstream output_file(this->FILE_PATH, std::ios::in, std::ios::binary);

		if (!output_file) {
			std::cerr << "No se pudo abrir el archivo '" + this->FILE_PATH + "'!\n";
			exit(EXIT_FAILURE);
		}

		const auto aux_degree = DEGREE;

		//HEADER
		output_file.write(reinterpret_cast<const char*>(&aux_degree), sizeof(aux_degree));

		//vectores de igual tamaño que contienen las llaves y valores que se insertaron 
		//al arbol b_tree
		const auto vec_keys{ b_tree.get_keys() };
		const auto vec_values{ b_tree.get_vals() };

		const auto size_vec_keys{ vec_keys.size() };
		const auto size_vec_values{ vec_values.size() };

		output_file.write(reinterpret_cast<const char*>(&size_vec_keys), sizeof(size_vec_keys));
		output_file.write(reinterpret_cast<const char*>(&size_vec_values), sizeof(size_vec_values));

		//guarda las llaves en el archivo
		for (size_t i = 0; i < vec_keys.size(); i++) {
			output_file.write(reinterpret_cast<const char*>(&i), sizeof(i));
		}

		//guarda los valores en el archivo
		for (size_t i = 0; i < vec_values.size(); i++) {
			output_file.write(reinterpret_cast<const char*>(&i), sizeof(i));
		}

		//std::cout << aux_degree << " " << size_vec_keys << " " << size_vec_values;
	}

	void insert_key_value(const key_type& key, const val_type& value) {
		b_tree.insert(key, value);
	}

	void delete_value(const key_type& key) {
		b_tree.erase(key);
	}

	auto get_value(const key_type& key) {
		return b_tree.find(key).get_val();
	}
};

