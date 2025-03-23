
#include <iostream>
#include <fstream>
#include <random>
#include <map>
#include <vector>
#include <iomanip>
#include <queue>

using namespace std;

const int TARGET_SIZE = 11000; // >10KB

class SetUpConsole {
public:
    SetUpConsole() {
        setlocale(LC_ALL, "Russian");
        cout.imbue(locale("Russian"));
    }
} consoleSetup;

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

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

void buildCodeTable(HuffmanNode* node, const string& code, map<char, string>& table) {
    if (!node) return;
    if (!node->left && !node->right) {
        table[node->ch] = code;
    }
    buildCodeTable(node->left, code + "0", table);
    buildCodeTable(node->right, code + "1", table);
}

string encodeHuffman(const string& text, map<char, string>& codeTable) {
    string encoded;
    for (char c : text) {
        encoded += codeTable[c];
    }
    return encoded;
}


// метод Хаффмана
void huffmanEncodeFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Нормализация для file3
    for (char& c : content) {
        if (ispunct(c) && c != ' ') c = '.';
        c = tolower(c);
    }
    content.erase(remove(content.begin(), content.end(), '\t'), content.end());

    // Считаем частоты символов
    map<char, int> freq;
    for (char c : content) freq[c]++;

    // Построение дерева Хаффмана
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
    for (const auto& pair : freq) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    while (pq.size() > 1) {
        HuffmanNode* l = pq.top(); pq.pop();
        HuffmanNode* r = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode('\0', l->freq + r->freq);
        parent->left = l;
        parent->right = r;
        pq.push(parent);
    }

    HuffmanNode* root = pq.top();
    map<char, string> codeTable;
    buildCodeTable(root, "", codeTable);

    // Проверка префиксности
    bool prefixFree = true;
    for (const auto& a : codeTable) {
        for (const auto& b : codeTable) {
            if (a.first != b.first && b.second.find(a.second) == 0) {
                prefixFree = false;
                break;
            }
        }
    }

    // Кодирование текста
    string encoded = encodeHuffman(content, codeTable);
    ofstream out("encoded_" + filename);
    out << encoded;
    out.close();

    // 1. Средняя длина кодового слова (Lcp)
    double Lcp = 0.0;
    int totalFreq = 0;
    for (const auto& p : freq) totalFreq += p.second;

    for (const auto& p : freq) {
        double probability = static_cast<double>(p.second) / totalFreq;
        Lcp += probability * codeTable[p.first].length();
    }

    // 2. Оценка энтропии входного текста H (до кодирования, на основе одиночных символов)
    map<string, int> freqChar;
    for (char c : content) {
        string s(1, c);
        freqChar[s]++;
    }
    double H = calculateEntropy(freqChar, totalFreq);

    // 3. Избыточность кодирования: μ = Lcp - H
    double redundancy = Lcp - H;

    cout << "\nЗакодировано методом Хаффмана: encoded_" << filename << endl;
    cout << "Код префиксный: " << (prefixFree ? "Да" : "Нет") << endl;

    // 🔽 Оценки энтропии выходной последовательности:
    map<string, int> freq1, freq2, freq3;
    for (size_t i = 0; i < encoded.size(); ++i)
        freq1[string(1, encoded[i])]++; // Частоты одиночных символов
    for (size_t i = 0; i + 1 < encoded.size(); ++i)
        freq2[encoded.substr(i, 2)]++;  // Частоты пар символов
    for (size_t i = 0; i + 2 < encoded.size(); ++i)
        freq3[encoded.substr(i, 3)]++;  // Частоты троек символов

    // 📌 1. Оценка энтропии выходной последовательности (частоты символов)
    double H1 = calculateEntropy(freq1, encoded.size());

    // 📌 2. Оценка энтропии выходной последовательности (частоты пар символов)
    double H2 = calculateEntropy(freq2, encoded.size() - 1) / 2;

    // 📌 3. Оценка энтропии выходной последовательности (частоты троек символов)
    double H3 = calculateEntropy(freq3, encoded.size() - 2) / 3;

    // 📊 Вывод результатов
    cout << left << setw(30) << "Средняя длина кода (Lcp):" << Lcp << endl;
    cout << left << setw(30) << "Избыточность кодирования (μ):" << redundancy << endl;
    cout << left << setw(30) << "H1 (частоты символов):" << H1 << endl;
    cout << left << setw(30) << "H2 (частоты пар символов):" << H2 << endl;
    cout << left << setw(30) << "H3 (частоты троек символов):" << H3 << endl;
}

int main()
{
    string command;
    cout << "Введите команду (generate для генерации файлов): ";
    cin >> command;

    if (command == "generate") {
        generateUniformFile("file1.txt");
        generateNonUniformFile("file2.txt");

       // analyzeFile("file1.txt");
      //  analyzeFile("file2.txt");
      //  analyzeFile("file3.txt");

        huffmanEncodeFile("file1.txt");
        huffmanEncodeFile("file2.txt");
        huffmanEncodeFile("file3.txt");
    }
    else {
        cout << "Неизвестная команда." << endl;
    }

    cout << "\nНажмите Enter для выхода...";
    cin.ignore();
    cin.get();

    return 0;
}


