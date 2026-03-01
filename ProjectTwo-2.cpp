/* 
 * Project_Two.cpp
 * CS 300 – ABCU Advising Assistance Program
 * Jacob S. Hary
 * 02.21.2026 .. v2
 *
 * Data structure: hash table (unordered_map) per my Project 1 
 *
 * Build:
 *   g++ -std=c++17 ProjectTwo.cpp -o advising
 * Run:
 *   ./ProjectTwo-2
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// -----------------------------
// very small Course struct
// -----------------------------
struct Course {
    string number;                 // example: CSCI200
    string title;                  // example: Data Structures
    vector<string> prerequisites;  // store codes; resolve titles on print
};

// -----------------------------
// tiny helpers (trim, uppercase, csv split)
// -----------------------------
static inline string trim(const string& s) {
    size_t i = 0; while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) ++i;
    if (i == s.size()) return "";
    size_t j = s.size() - 1; while (j > i && isspace(static_cast<unsigned char>(s[j]))) --j;
    return s.substr(i, j - i + 1);
}

static inline string up(string s) {
    for (char &c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

// quick CSV split; assumes no quoted commas in input
static vector<string> splitCSV(const string& line) {
    vector<string> out; string field;
    for (char ch : line) {
        if (ch == ',') { out.push_back(trim(field)); field.clear(); }
        else field.push_back(ch);
    }
    out.push_back(trim(field));
    for (auto &f : out) if (!f.empty() && f.back()=='\r') f.pop_back(); // windows CRLF guard
    return out;
}

// -----------------------------
// AdvisingProgram (hash map of courses)
// -----------------------------
class AdvisingProgram {
public:
    // load csv into hash table; returns count (just in case I need it)
    size_t loadFromFile(const string& fileName) {
        courses.clear();
        ifstream fin(fileName);
        if (!fin.is_open()) {
            throw runtime_error("Unable to open file: " + fileName);
        }
        string line; size_t lineNo = 0; size_t inserted = 0;
        while (getline(fin, line)) {
            ++lineNo;
            string raw = trim(line);
            if (raw.empty()) continue; // skip blanks

            vector<string> f = splitCSV(raw);
            if (f.size() < 2) {
                cerr << "[Warning] Line " << lineNo << ": expected Course Number and Title" << '\n';
                continue;
            }
            string num = up(f[0]);
            string title = f[1];
            if (num.empty() || title.empty()) {
                cerr << "[Warning] Line " << lineNo << ": missing number or title" << '\n';
                continue;
            }

            vector<string> prereqs;
            for (size_t i = 2; i < f.size(); ++i) {
                string p = up(f[i]);
                if (!p.empty()) prereqs.push_back(p);
            }

            auto it = courses.find(num);
            if (it == courses.end()) {
                courses.emplace(num, Course{num, title, prereqs});
                ++inserted;
            } else {
                // merge prereqs if duplicate rows show up (happens sometimes)
                auto &ex = it->second.prerequisites;
                for (const string& p : prereqs) {
                    if (find(ex.begin(), ex.end(), p) == ex.end()) ex.push_back(p);
                }
                // Keeping original title (feels safer to me for duplicates)
            }
        }
        lastLoadCount = inserted; // I learned that this command could help me debug, so i am trying it out
        return inserted;
    }

    // print all courses sorted by number (alphanumeric)
    void printCourseList() const {
        if (courses.empty()) {
            cout << "No course data loaded. Please load data first (Option 1)." << '\n';
            return;
        }
        vector<const Course*> list; list.reserve(courses.size());
        for (const auto& kv : courses) list.push_back(&kv.second);
        sort(list.begin(), list.end(), [](const Course* a, const Course* b){ return a->number < b->number; });

        // Match sample header
        cout << "Here is a sample schedule:" << '\n';
        for (const Course* c : list) {
            cout << c->number << ", " << c->title << '\n';
        }
    }

    // print one course's info (Per my rubric, prereq numbers AND titles)
    void printCourseInfo(const string& input) const {
        if (courses.empty()) {
            cout << "No course data loaded. Please load data first (Option 1)." << '\n';
            return;
        }
        string code = up(trim(input));
        if (code.empty()) { cout << "Invalid input: course number cannot be empty." << '\n'; return; }
        auto it = courses.find(code);
        if (it == courses.end()) { cout << "Course '" << code << "' not found." << '\n'; return; }

        const Course& c = it->second;
        cout << c.number << ", " << c.title << '\n';

        if (c.prerequisites.empty()) {
            cout << "Prerequisites: None" << '\n';
            return;
        }

        // Build a single line like: Prerequisites: CSCI301: Advanced Programming in C++, CSCI350: Operating Systems
        cout << "Prerequisites: ";
        for (size_t i = 0; i < c.prerequisites.size(); ++i) {
            const string& p = c.prerequisites[i];
            auto jt = courses.find(p);
            if (jt != courses.end()) {
                cout << jt->second.number << ": " << jt->second.title;
            } else {
                // if prereq isn't in file, still show code
                cout << p << " (not found in data)";
            }
            if (i + 1 < c.prerequisites.size()) cout << ", ";
        }
        cout << '\n';
    }

private:
    unordered_map<string, Course> courses;
    size_t lastLoadCount = 0; // not used by UI, but I like to keep it for those 'sanity' checks
};

// -----------------------------
// menu printing 
// -----------------------------
static void printMenu() {
    cout << "Welcome to the course planner." << '\n';
    cout << "1. Load Data Structure." << '\n';
    cout << "2. Print Course List." << '\n';
    cout << "3. Print Course." << '\n';
    cout << "9. Exit" << '\n';
    cout << "What would you like to do? ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    AdvisingProgram app;

    while (true) {
        printMenu();
        string opt; if (!getline(cin, opt)) break; opt = trim(opt);
        if (opt.empty()) { cout << "Please enter a menu option." << '\n'; continue; }

        bool numeric = !opt.empty();
        for (char ch : opt) if (!isdigit(static_cast<unsigned char>(ch))) { numeric = false; break; }
        if (!numeric) { cout << opt << " is not a valid option." << '\n'; continue; }

        int choice = stoi(opt);
        if (choice == 1) {
            cout << "Enter course data file name: ";
            string fileName; if (!getline(cin, fileName)) { cout << "Input error." << '\n'; break; }
            fileName = trim(fileName);
            if (fileName.empty()) { cout << "File name cannot be empty." << '\n'; continue; }
            try {
                // not printing count to keep the UI less noisy
                (void)app.loadFromFile(fileName);
            } catch (const exception& ex) {
                cout << "Error: " << ex.what() << '\n';
            }
        } else if (choice == 2) {
            app.printCourseList();
        } else if (choice == 3) {
            cout << "What course do you want to know about? ";
            string code; if (!getline(cin, code)) { cout << "Input error." << '\n'; break; }
            app.printCourseInfo(code);
        } else if (choice == 9) {
            cout << "Thank you for using the course planner!" << '\n';
            return 0;
        } else {
            cout << choice << " is not a valid option." << '\n';
        }
    }

    return 0; // EOF or something weird
}
