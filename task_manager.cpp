#include <iostream>
#include <vector>
#include <string>
#include <clocale>
#include <limits>
#include <functional>
#include <fstream>
#include <filesystem>
#define NOMINMAX
#include <windows.h>
#include <chrono>
#include <format>

namespace fs = std::filesystem;

struct task {
	std::string text;
	int number;
	std::string status;
	std::string date;
	task(std::string text, int number, std::string status, std::string date) : text(text), number(number), status(status), date(date) {}
};

std::string get_current_date() {
	using namespace std::chrono;
	auto const now = system_clock::now();
	auto const local = zoned_time{ current_zone(), now };
	auto const local_min = floor<minutes>(local.get_local_time());
	auto const ymd = year_month_day{ floor<days>(local_min) };
	auto const time = hh_mm_ss{ local_min - floor<days>(local_min) };
	return std::format("{:%d.%m.%Y} {:02}:{:02}", ymd, time.hours().count(), time.minutes().count());
}

void save_to_file(const std::vector<task>& tasks, const std::string& file_name) {
	std::ofstream file(file_name);
	if (!file) {
		std::cerr << "Ошибка сохранения файла!" << std::endl;
		return;
	}
	for (const auto& t : tasks) {
		file << t.text << '|' << t.status << '|' << t.date << '\n';
	}
}

void print_file(const std::vector<task>& tasks_vec) {
	if (tasks_vec.empty()) {
		std::cout << "Файл пуст" << std::endl;
	}
	else {
		for (const auto& obj : tasks_vec) {
			std::cout << obj.number << ". " << obj.text << " [Статус: " << obj.status << "]" << " Дата: " << obj.date <<  std::endl;
		}
		std::cout << std::endl;
	}
}

void fill_tasks_vec(std::vector<task>& tasks_vec, const std::string& file_name) {
	std::ifstream file(file_name);
	if (!file) {
		std::cerr << "Ошибка открытия файла для чтения!" << std::endl;
		return;
	}
	std::string line;
	int count = 0;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		++count;

		size_t pos1 = line.find('|');
		if (pos1 == std::string::npos) {
			tasks_vec.emplace_back(line, count, "Не выполнено", "");
			continue;
		}
		std::string text = line.substr(0, pos1);
		size_t pos2 = line.find('|', pos1 + 1);
		std::string status, date;
		if (pos2 == std::string::npos) {
			status = line.substr(pos1 + 1);
			date = "";
		}
		else {
			status = line.substr(pos1 + 1, pos2 - pos1 - 1);
			date = line.substr(pos2 + 1);
		}
		tasks_vec.emplace_back(text, count, status, date);
	}
}

bool is_valid_file_name(std::string& file_name) {
	if (file_name.empty() || file_name.length() > 250) return false;
	const std::string forbidden = "\\/:*?\"<>|";
	for (char ch : file_name) {
		if (forbidden.find(ch) != std::string::npos) return false;
	}
	return true;
}



void check_num(int& command, int num) {
	while (!(std::cin >> command) || (command < 1) || (command > num)) {
		std::cout << "Ошибка! Введите корректное число от 1 до " << num << ": ";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


void clear_Screen() {
	std::cout << "\033[2J\033[H" << std::flush;
}

void add_task(std::vector<task>& tasks_vec, const std::string& file_name) {
	while (true) {
		std::cout << "Напишите новую задачу: ";
		std::string text;
		std::getline(std::cin, text);
		tasks_vec.emplace_back(text, tasks_vec.size() + 1, "Не выполнено", get_current_date());
		save_to_file(tasks_vec, file_name);

		clear_Screen();
		std::cout << "Задача успешно добавлена! Текущие задачи:" << std::endl;
		print_file(tasks_vec);
		std::cout << "Введите 1, чтобы добавить ещё, и 2, чтобы вернуться: ";
		int choice;
		check_num(choice, 2);
		if (choice != 1) break;
	}
	clear_Screen();
}

void change_text(std::vector<task>& tasks_vec, const std::string& file_name) {
	if (tasks_vec.empty()) {
		std::cout << "Файл пуст - нечего изменять." << std::endl << "Нажмите любую клавишу, чтобы вернуться: ";
		std::cin.get();
		return;
	}

	while (true) {
		std::cout << "Введите номер задачи, текст которой хотите изменить: ";
		int num;
		check_num(num, tasks_vec.size());

		std::cout << "Введите новый текст: ";
		std::string new_text;
		std::getline(std::cin, new_text);
		tasks_vec[num - 1].text = new_text;
		save_to_file(tasks_vec, file_name);

		clear_Screen();
		std::cout << "Текст успешно изменён! Текущие задачи:" << std::endl;
		print_file(tasks_vec);
		std::cout << "Введите 1, чтобы изменить ещё одну задачу, и 2, чтобы вернуться: ";
		int choice;
		check_num(choice, 2);
		if (choice != 1) break;
	}
	clear_Screen();
}

void task_done(std::vector<task>& tasks_vec, const std::string& file_name) {
	if (tasks_vec.empty()) {
		std::cout << "Файл пуст - нечего изменять" << std::endl << "Нажмите любую клавишу, чтобы вернуться: ";
		std::cin.get();
		return;
	}

	while (true) {
		std::cout << "Введите номер задачи, статус которой хотите изменить: ";
		int num;
		check_num(num, tasks_vec.size());

		auto& status = tasks_vec[num - 1].status;
		status = (status == "Выполнено") ? "Не выполнено" : "Выполнено";
		save_to_file(tasks_vec, file_name);

		clear_Screen();
		std::cout << "Статус успешно изменён! Текущие задачи:\n";
		print_file(tasks_vec);
		std::cout << "Введите 1, чтобы изменить статус ещё одной задачи, и 2, чтобы вернуться: ";
		int choice;
		check_num(choice, 2);
		if (choice != 1) break;
	}
	clear_Screen();
}

void delete_task(std::vector<task>& tasks_vec, const std::string& file_name){
	if (tasks_vec.empty()) {
		std::cout << "Файл пуст - нечего удалять." << std::endl << "Нажмите любую клавишу, чтобы вернуться: ";
		std::cin.get();
		return;
	}

	while (true) {
		std::cout << "Введите номер задачи, которую хотите удалить: ";
		int num;
		check_num(num, tasks_vec.size());

		tasks_vec.erase(tasks_vec.begin() + num - 1);

		for (size_t i = 0; i < tasks_vec.size(); ++i) {
			tasks_vec[i].number = i + 1;
		}
		save_to_file(tasks_vec, file_name);

		clear_Screen();
		if (tasks_vec.empty()) {
			std::cout << "Все задачи удалены. Файл пуст." << std::endl << "Нажмите любую клавишу, чтобы вернуться: ";
			std::cin.get();
			break;
		}

		std::cout << "Задача успешно удалена! Текущие задачи:" << std::endl;
		print_file(tasks_vec);
		std::cout << "Введите 1, чтобы удалить ещё одну задачу, и 2, чтобы вернуться: ";
		int choice;
		check_num(choice, 2);
		if (choice != 1) break;
	}
	clear_Screen();
}

int main() {
	#ifdef _WIN32
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
	#else
		std::setlocale(LC_ALL, "");
	#endif


	const std::vector<std::function<void(std::vector<task>&, const std::string& file_name)>> commands = {
		add_task,
		change_text,
		task_done,
		delete_task
	};

	std::vector<task> tasks_vec;
	
	std::cout << "КОНСОЛЬНЫЙ МЕНЕДЖЕР ЗАДАЧ" << std::endl << std::endl;
	std::cout << "Команды:" << std::endl;
	std::cout << "1 - добавить существующий файл" << std::endl;
	std::cout << "2 - создать новый файл" << std::endl;
	std::cout << "Введите нужную команду: ";

	int file_choice;
	check_num(file_choice, 2);
	clear_Screen();
	std::string file_name;
	if (file_choice == 1) {
		std::cout << "Введите имя файла на компьютере: ";
		while (true) {
			std::getline(std::cin, file_name);
			if (fs::exists(file_name)) {
				clear_Screen();
				fill_tasks_vec(tasks_vec, file_name);
				break;
			}
			else {
				std::cout << "Файл не найден. Попробуйте ещё раз: ";
			}
		}
	}
	else {
		std::cout << "Введите имя файла, который хотите создать: ";
		while (true) {
			std::getline(std::cin, file_name);
			if (fs::exists(file_name)) {
				std::cout << "Файл с таким названием уже есть на компьютере" << std::endl << "Введите другое название: ";
			}
			else if(is_valid_file_name(file_name)){
				std::ofstream new_file(file_name);
				new_file.close();
				break;
			}
			else {
				std::cout << "Некоректное название файла. Попробуйте ещё раз: ";
			}
		}
	}

	while (true) {
		std::cout << "КОНСОЛЬНЫЙ МЕНЕДЖЕР ЗАДАЧ" << std::endl;
		std::cout << "Выбранный файл: " << file_name << std::endl << std::endl;
		print_file(tasks_vec);
		std::cout << "Основные команды:" << std::endl;
		std::cout << "1 - добавить задачу" << std::endl;
		std::cout << "2 - изменить текст задачи" << std::endl;
		std::cout << "3 - изменить статус задачи" << std::endl;
		std::cout << "4 - удалить задачу" << std::endl;
		std::cout << "5 - выйти из приложения" << std::endl << std::endl;

		

		int command;
		check_num(command, 5);

		clear_Screen();
		if (command == 5) break;
		print_file(tasks_vec);
		std::cout << std::endl;
		commands[command - 1](tasks_vec, file_name);
	}
}