#include "TitanicPerson.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iostream>

TitanicPerson::TitanicPerson() {
	passenger_Id = -1;
	P_class = -1;
	age = -1;
	sib_Sp = -1;
	parch = -1;
	fare = -1.0;
	survived = false;
}

void TitanicPerson::setPassenger_Id(const int& passenger_Id) { this->passenger_Id = passenger_Id; }

void TitanicPerson::setSurvived(bool survived) { this->survived = survived; }

void TitanicPerson::setP_class(const int& P_class) { this->P_class = P_class; }

void TitanicPerson::setName(const std::string_view& name) {
	auto longitud = name.length();
	longitud = longitud < 80 ? longitud : 80 - 1;
	name.copy(this->name, longitud);
	this->name[longitud] = '\0';
}
void TitanicPerson::setSex(const std::string_view& sex) {

	if (sex == "male") {
		sex.copy(this->sex, 4);
		this->sex[4] = '\0';
	}
	else if (sex == "female") {
		sex.copy(this->sex, 6);
		this->sex[6] = '\0';
	}
}

void TitanicPerson::setAge(const int& age) { this->age = age; }

void TitanicPerson::setSib_Sp(const int& sib_Sp) { this->sib_Sp = sib_Sp; }

void TitanicPerson::setParch(const int& parch) { this->parch = parch; }

void TitanicPerson::setFare(const double& fare) { this->fare = fare; }

void TitanicPerson::setTicket(const std::string_view& ticket) {
	auto longitud = ticket.length();
	longitud = longitud < 25 ? longitud : 25 - 1;
	ticket.copy(this->ticket, longitud);
	this->ticket[longitud] = '\0';
}

void TitanicPerson::setCabin(const std::string_view& cabin) {
	auto longitud = cabin.length();
	longitud = longitud < 15 ? longitud : 15 - 1;
	cabin.copy(this->cabin, longitud);
	this->cabin[longitud] = '\0';
}

void TitanicPerson::setEmbarked(const char& embarked) { this->embarked = embarked; }

int TitanicPerson::getPassenget_Id() const {
	return this->passenger_Id;
}

std::tuple<std::vector<std::string>, std::vector<std::vector<std::string>>> TitanicPerson::getFullRegister() {
		std::ifstream in_file("titanic.csv", std::ios::in);

		if (!in_file) {
			std::cerr << "No se pudo abrir el archivo 'titanic.csv'\n";
			exit(EXIT_FAILURE);
		}

		std::string line{};
		std::stringstream ss{};
		std::string field{};
		std::vector<std::vector<std::string>> vec_records{};
		std::vector<std::string> vec_fields{};
		std::vector<std::string> vec_field_name{};

		std::getline(in_file, line);
		ss << line;

		while (std::getline(ss, field, ',')) {
			vec_field_name.push_back(field);
		}
		ss.str("");
		ss.clear();

		while (std::getline(in_file, line)) {

			ss << line;

			while (std::getline(ss, field, ',')) {
				vec_fields.push_back(field);
			}

			vec_fields[3].erase(vec_fields[3].begin()); //Borrar el '"' inicial del campo name
			vec_fields[4].erase(vec_fields[4].end() - 1); //Borrar el '"' final del campo name

			if (vec_fields.size() == 12) { //En el campo 'Embarked' hay campos que estan vacias y al estar al final no se lee el vacio
				vec_fields.push_back("");  //y vec_campos solo alamcena 12 valores y no 13, provocando el error de out_range al acceder a estos datos
			}

			ss.str("");
			ss.clear();
			vec_records.push_back(vec_fields);
			vec_fields.clear();
		}

		in_file.close();

		return std::make_tuple(vec_field_name, vec_records);
	}

std::vector<TitanicPerson> TitanicPerson::getAllRecods() {
		const auto vec_records{ std::get<1>(getFullRegister()) };

		std::vector<TitanicPerson> vec_titanic_person{};

		for (const auto& vec_fields : vec_records) {
			TitanicPerson titanic_person;
			titanic_person.setPassenger_Id(vec_fields[0].empty() ? -1 : std::stoi(vec_fields[0]));
			titanic_person.setSurvived(vec_fields[1].empty() ? 0 : std::stoi(vec_fields[1]));
			titanic_person.setP_class(vec_fields[2].empty() ? -1 : std::stoi(vec_fields[2]));
			titanic_person.setName(std::string_view(vec_fields[3] + ", " + vec_fields[4]));
			titanic_person.setSex(vec_fields[5]);
			titanic_person.setAge(vec_fields[6].empty() ? -1 : std::stoi(vec_fields[6]));
			titanic_person.setSib_Sp(vec_fields[7].empty() ? -1 : std::stoi(vec_fields[7]));
			titanic_person.setParch(vec_fields[8].empty() ? -1 : std::stoi(vec_fields[8]));
			titanic_person.setTicket(vec_fields[9]);
			titanic_person.setFare(vec_fields[10].empty() ? -1.0 : std::stod(vec_fields[10]));
			titanic_person.setCabin(vec_fields[11]);
			titanic_person.setEmbarked(vec_fields[12][0]);
			vec_titanic_person.push_back(titanic_person);
		}
		return vec_titanic_person;
	}

std::vector<TitanicPerson> TitanicPerson::getRecords(const unsigned& begin, const unsigned& end) {
		if (begin > end) {
			throw std::invalid_argument("Argumento Invalido\n");
		}
		if (begin > 890 || end > 890) {
			throw std::out_of_range("Fuera de Rango\n");
		}
		const auto vec_titanic_person{ getAllRecods() };
		std::vector<TitanicPerson> vec_tperson_select{};

		vec_tperson_select.assign(vec_titanic_person.cbegin() + begin, vec_titanic_person.cbegin() + end);

		return vec_tperson_select;
	}

void TitanicPerson::printFieldTittle() {
		std::ifstream in_file("titanic.csv", std::ios::in);
		if (!in_file) {
			std::cerr << "No se pudo abrir el archivo 'titanic.csv'\n";
			exit(EXIT_FAILURE);
		}
		std::string line{};
		std::stringstream ss{};
		std::vector<std::string> vec_field_name{};
		std::getline(in_file, line);
		ss << line;
		while (std::getline(ss, line, ',')) {
			vec_field_name.push_back(line);
		}
		std::cout << std::left << std::setw(12) << vec_field_name[0] << std::setw(10) << vec_field_name[1] << std::setw(20)
			<< vec_field_name[2] << std::setw(37) << vec_field_name[3] << std::setw(14) << vec_field_name[4] << std::setw(10)
			<< vec_field_name[5] << std::setw(6) << vec_field_name[6] << std::setw(6) << vec_field_name[7] << std::setw(20)
			<< vec_field_name[8] << std::setw(10) << vec_field_name[9] << std::setw(13) << vec_field_name[10] << std::setw(4)
			<< vec_field_name[11] << '\n';
		in_file.close();
	}

std::ostream& operator<<(std::ostream& os, const TitanicPerson& p) {
	return os << std::left << std::setw(6) << p.passenger_Id << std::setw(6) << p.survived << std::setw(6)
		<< p.P_class << std::setw(60) << p.name << std::setw(16) << p.sex << std::setw(12)
		<< p.age << std::setw(6) << p.sib_Sp << std::setw(6) << p.parch << std::setw(20)
		<< p.ticket << std::setw(12) << p.fare << std::setw(15) << p.cabin << std::setw(6)
		<< p.embarked;
}

std::ifstream& operator>>(std::ifstream& in, TitanicPerson& p) {
	in.read(reinterpret_cast<char*>(&p), sizeof(p));
	return in;
}

std::ofstream& operator<<(std::ofstream& out, const TitanicPerson& p) {
	out.write(reinterpret_cast<const char*>(&p), sizeof(p));
	return out;
}

bool TitanicPerson::operator<(const TitanicPerson& p1) {
	return this->passenger_Id < p1.passenger_Id;
}

bool TitanicPerson::operator>(const TitanicPerson& p1) {
	return this->passenger_Id > p1.passenger_Id;
}

bool TitanicPerson::operator==(const TitanicPerson& p1) {
	return this->passenger_Id == p1.passenger_Id;
}

bool TitanicPerson::operator>=(const TitanicPerson& p1) {
	return this->passenger_Id >= p1.passenger_Id;
}

bool TitanicPerson::operator<=(const TitanicPerson& p1) {
	return this->passenger_Id <= p1.passenger_Id;
}