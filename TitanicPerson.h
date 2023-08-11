#pragma once
#include <string_view>
#include <vector>
#include <tuple>

class TitanicPerson {
private:
	int passenger_Id{};
	bool survived{};
	int P_class{};
	char sex[7]{};
	int age{};
	int sib_Sp{};
	int parch{};
	char embarked{};
	double fare{};
	char ticket[25]{};
	char cabin[15]{};
	char name[80]{};
public:
	TitanicPerson();
	void setPassenger_Id(const int& passenger_Id);
	void setSurvived(bool survived);
	void setP_class(const int& P_class);
	void setName(const std::string_view& name);
	void setSex(const std::string_view& sex);
	void setAge(const int& age);
	void setSib_Sp(const int& sib_Sp);
	void setParch(const int& parch);
	void setFare(const double& fare);
	void setTicket(const std::string_view& ticket);
	void setCabin(const std::string_view& cabin);
	void setEmbarked(const char& embarked);
	int getPassenget_Id() const;

public:
	static std::tuple<std::vector<std::string>, std::vector<std::vector<std::string>>> getFullRegister();
	static std::vector<TitanicPerson> getAllRecods();
	static std::vector<TitanicPerson> getRecords(const unsigned& begin, const unsigned& end);
	static void printFieldTittle();
public:
	friend std::ostream& operator<<(std::ostream& os, const TitanicPerson& p);
	friend std::ifstream& operator>>(std::ifstream& in, TitanicPerson& p);
	friend std::ofstream& operator<<(std::ofstream& out, const TitanicPerson& p);
	bool operator<(const TitanicPerson& p1);
	bool operator>(const TitanicPerson& p1);
	bool operator==(const TitanicPerson& p1);
	bool operator>=(const TitanicPerson& p1);
	bool operator<=(const TitanicPerson& p1);
};

