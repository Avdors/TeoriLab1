// TeoriLab1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <fstream>
#include <random>
#include <map>
#include <vector>
#include <iomanip>

using namespace std;

const int TARGET_SIZE = 11000; // >10KB

class SetUpConsole {
public:
    SetUpConsole() {
        setlocale(LC_ALL, "Russian");
        cout.imbue(locale("Russian"));
    }
} consoleSetup;

// Вычисление энтропии
double calculateEntropy(const map<string, int>& freqMap, int total) {
    double entropy = 0.0;
    for (const auto& pair : freqMap) {
        double p = static_cast<double>(pair.second) / total;
        entropy -= p * log2(p);
    }
    return entropy;
}

void analyzeFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Подготовка (для file3 — нормализуем текст)
    for (char& c : content) {
        if (ispunct(c) && c != ' ') c = '.';
        c = tolower(c);
    }

    // Удаление табуляций
    content.erase(remove(content.begin(), content.end(), '\t'), content.end());

    // Счёт одиночных символов
    map<string, int> freqChar;
    for (char c : content) {
        string s(1, c);
        freqChar[s]++;
    }

    // Счёт пар символов
    map<string, int> freqPairs;
    for (size_t i = 0; i + 1 < content.size(); ++i) {
        string pair = content.substr(i, 2);
        freqPairs[pair]++;
    }

    int totalChars = content.size();
    int totalPairs = content.size() - 1;

    double H1 = calculateEntropy(freqChar, totalChars);
    double H2 = calculateEntropy(freqPairs, totalPairs) / 2;
    double H_theor = 0.0;

    if (filename == "file1.txt") {
        // Равномерное распределение для 6 символов
        double p = 1.0 / 6.0;
        H_theor = -6 * p * log2(p);
    }
    else if (filename == "file2.txt") {
        // Нерaвномерное распределение: A:0.5, B:0.2, C:0.15, D:0.1, E:0.05
        vector<double> probs = { 0.5, 0.2, 0.15, 0.1, 0.05 };
        for (double p : probs)
            H_theor -= p * log2(p);
    }

    // Табличный вывод
    cout << "\nФайл: " << filename << endl;
    cout << left << setw(25) << "Общее кол-во символов:" << totalChars << endl;
    cout << left << setw(25) << "Оценка H1 (одиночн.):" << fixed << setprecision(5) << H1 << endl;
    cout << left << setw(25) << "Оценка H2 (пары):" << fixed << setprecision(5) << H2 << endl;
    cout << left << setw(25) << "H_теор (по распредел.):" << H_theor << endl;
}

// генерация последовательности с равномерным распределение
void generateUniformFile(const string& filename) {

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    string symbols = "ABCDEF"; // 6 разных символов

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, symbols.size() - 1);

    for (int i = 0; i < TARGET_SIZE; ++i) {
        char c = symbols[dist(gen)];
        file << c;
    }

    file.close();
    cout << "Создан файл с равномерным распределением: " << filename << endl;

}
// генерация с неравновероятным  распределением
void generateNonUniformFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    // Заданные вероятности
    vector<pair<char, double>> symbolProb = {
        {'A', 0.5},  // 50%
        {'B', 0.2},  // 20%
        {'C', 0.15}, // 15%
        {'D', 0.1},  // 10%
        {'E', 0.05}  // 5%
    };
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.0, 1.0);

    for (int i = 0; i < TARGET_SIZE; ++i) {
        double r = dist(gen);
        double cumulative = 0.0;

        for (const auto& pair : symbolProb) {
            cumulative += pair.second;
            if (r <= cumulative) {
                file << pair.first;
                break;
            }
        }
    }

    file.close();
    cout << "Создан файл с неравномерным распределением: " << filename << endl;
}

int main()
{
    string command;
    cout << "Введите команду (generate для генерации файлов): ";
    cin >> command;

    if (command == "generate") {
        generateUniformFile("file1.txt");
        generateNonUniformFile("file2.txt");

        analyzeFile("file1.txt");
        analyzeFile("file2.txt");
        analyzeFile("file3.txt");
    }
    else {
        cout << "Неизвестная команда." << endl;
    }

    cout << "\nНажмите Enter для выхода...";
    cin.ignore();
    cin.get();

    return 0;
}


