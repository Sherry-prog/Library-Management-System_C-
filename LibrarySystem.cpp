// Basic Book Library System (Console, C++11)
// Features: add, delete, view inventory, view available books, issue book,
// return book, view issued books, search, and save/load to a text file.

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int LOAN_DAYS = 14;
const string DATA_FILE = "library_data.txt";

// ---------- Helpers ----------

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return s;
}

string truncate(const string& s, size_t width) {
    if (s.size() <= width) return s;
    return s.substr(0, width - 3) + "...";
}

string formatDate(time_t t) {
    char buf[32];
    struct tm* info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d", info);
    return string(buf);
}

string readLine(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            // Input closed (e.g. Ctrl+D); exit cleanly
            cout << "\nInput closed. Exiting.\n";
            exit(0);
        }
        // trim spaces
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start != string::npos) {
            line = line.substr(start, end - start + 1);
            // '|' is used as a separator in the save file
            replace(line.begin(), line.end(), '|', '/');
            return line;
        }
        cout << "This field cannot be empty.\n";
    }
}

int readInt(const string& prompt) {
    while (true) {
        string line = readLine(prompt);
        try {
            size_t pos = 0;
            int value = stoi(line, &pos);
            if (pos == line.size()) return value;
        } catch (...) {
        }
        cout << "Please enter a valid whole number.\n";
    }
}

// ---------- Model ----------

struct Book {
    int id;
    string title;
    string author;
    bool issued;
    string borrower;
    time_t issueTime;
};

class Library {
private:
    vector<Book> books;
    int nextId;

    int findIndex(int id) const {
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].id == id) return static_cast<int>(i);
        }
        return -1;
    }

    static void printHeader() {
        cout << left << setw(6) << "ID" << setw(30) << "Title" << setw(22) << "Author"
             << setw(12) << "Status" << "\n";
        cout << string(70, '-') << "\n";
    }

    static void printRow(const Book& b) {
        cout << left << setw(6) << b.id << setw(30) << truncate(b.title, 28)
             << setw(22) << truncate(b.author, 20) << setw(12)
             << (b.issued ? "Issued" : "Available") << "\n";
    }

public:
    Library() : nextId(1) { load(); }

    // ----- Persistence -----

    void save() const {
        ofstream out(DATA_FILE.c_str());
        if (!out) {
            cout << "Warning: could not save data to " << DATA_FILE << "\n";
            return;
        }
        for (size_t i = 0; i < books.size(); i++) {
            const Book& b = books[i];
            out << b.id << '|' << b.title << '|' << b.author << '|' << (b.issued ? 1 : 0)
                << '|' << b.borrower << '|' << static_cast<long long>(b.issueTime) << '\n';
        }
    }

    void load() {
        ifstream in(DATA_FILE.c_str());
        if (!in) {
            seedSampleBooks();
            return;
        }
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            vector<string> parts;
            stringstream ss(line);
            string field;
            while (getline(ss, field, '|')) parts.push_back(field);
            // A trailing empty borrower field can be dropped by getline
            while (parts.size() < 6) parts.push_back("");
            try {
                Book b;
                b.id = stoi(parts[0]);
                b.title = parts[1];
                b.author = parts[2];
                b.issued = (parts[3] == "1");
                b.borrower = parts[4];
                b.issueTime = static_cast<time_t>(parts[5].empty() ? 0 : stoll(parts[5]));
                books.push_back(b);
                if (b.id >= nextId) nextId = b.id + 1;
            } catch (...) {
                // skip corrupted line
            }
        }
    }

    void seedSampleBooks() {
        addBookInternal("The Pragmatic Programmer", "Andrew Hunt");
        addBookInternal("Clean Code", "Robert C. Martin");
        addBookInternal("Introduction to Algorithms", "Thomas H. Cormen");
        addBookInternal("The C++ Programming Language", "Bjarne Stroustrup");
        addBookInternal("Atomic Habits", "James Clear");
    }

    void addBookInternal(const string& title, const string& author) {
        Book b;
        b.id = nextId++;
        b.title = title;
        b.author = author;
        b.issued = false;
        b.borrower = "";
        b.issueTime = 0;
        books.push_back(b);
    }

    // ----- Operations -----

    void addBook() {
        cout << "\n--- Add a Book ---\n";
        string title = readLine("Title : ");
        string author = readLine("Author: ");
        addBookInternal(title, author);
        cout << "Book added with ID " << (nextId - 1) << ".\n";
        save();
    }

    void deleteBook() {
        cout << "\n--- Delete a Book ---\n";
        int id = readInt("Enter book ID to delete: ");
        int idx = findIndex(id);
        if (idx < 0) {
            cout << "No book found with ID " << id << ".\n";
            return;
        }
        if (books[idx].issued) {
            cout << "This book is currently issued to " << books[idx].borrower
                 << " and cannot be deleted.\n";
            return;
        }
        string confirm =
            readLine("Delete \"" + books[idx].title + "\"? (y/n): ");
        if (toLower(confirm) == "y") {
            books.erase(books.begin() + idx);
            cout << "Book deleted.\n";
            save();
        } else {
            cout << "Deletion cancelled.\n";
        }
    }

    void viewInventory() const {
        cout << "\n--- Book Inventory (" << books.size() << " total) ---\n";
        if (books.empty()) {
            cout << "The library has no books.\n";
            return;
        }
        printHeader();
        for (size_t i = 0; i < books.size(); i++) printRow(books[i]);
    }

    void viewAvailable() const {
        cout << "\n--- Available Books ---\n";
        bool any = false;
        for (size_t i = 0; i < books.size(); i++) {
            if (!books[i].issued) {
                if (!any) printHeader();
                printRow(books[i]);
                any = true;
            }
        }
        if (!any) cout << "No books are available right now.\n";
    }

    void issueBook() {
        cout << "\n--- Issue a Book ---\n";
        int id = readInt("Enter book ID: ");
        int idx = findIndex(id);
        if (idx < 0) {
            cout << "No book found with ID " << id << ".\n";
            return;
        }
        Book& b = books[idx];
        if (b.issued) {
            cout << "Sorry, this book is already issued to " << b.borrower << ".\n";
            return;
        }
        string borrower = readLine("Borrower name: ");
        b.issued = true;
        b.borrower = borrower;
        b.issueTime = time(NULL);

        time_t due = b.issueTime + static_cast<time_t>(LOAN_DAYS) * 24 * 60 * 60;
        cout << "\"" << b.title << "\" issued to " << borrower << ".\n";
        cout << "Issue date: " << formatDate(b.issueTime) << " | Due date: "
             << formatDate(due) << "\n";
        save();
    }

    void returnBook() {
        cout << "\n--- Return a Book ---\n";
        int id = readInt("Enter book ID: ");
        int idx = findIndex(id);
        if (idx < 0) {
            cout << "No book found with ID " << id << ".\n";
            return;
        }
        Book& b = books[idx];
        if (!b.issued) {
            cout << "This book is not currently issued.\n";
            return;
        }
        time_t due = b.issueTime + static_cast<time_t>(LOAN_DAYS) * 24 * 60 * 60;
        if (time(NULL) > due) {
            int daysLate = static_cast<int>((time(NULL) - due) / (24 * 60 * 60));
            cout << "Note: this book was returned " << daysLate << " day(s) late.\n";
        }
        cout << "\"" << b.title << "\" returned by " << b.borrower << ".\n";
        b.issued = false;
        b.borrower = "";
        b.issueTime = 0;
        save();
    }

    void viewIssued() const {
        cout << "\n--- Issued Books ---\n";
        bool any = false;
        for (size_t i = 0; i < books.size(); i++) {
            const Book& b = books[i];
            if (!b.issued) continue;
            if (!any) {
                cout << left << setw(6) << "ID" << setw(26) << "Title" << setw(18)
                     << "Borrower" << setw(13) << "Issued" << setw(13) << "Due" << "\n";
                cout << string(76, '-') << "\n";
                any = true;
            }
            time_t due = b.issueTime + static_cast<time_t>(LOAN_DAYS) * 24 * 60 * 60;
            bool overdue = time(NULL) > due;
            cout << left << setw(6) << b.id << setw(26) << truncate(b.title, 24)
                 << setw(18) << truncate(b.borrower, 16) << setw(13)
                 << formatDate(b.issueTime) << setw(13) << formatDate(due)
                 << (overdue ? "OVERDUE" : "") << "\n";
        }
        if (!any) cout << "No books are currently issued.\n";
    }

    void searchBooks() const {
        cout << "\n--- Search Books ---\n";
        string keyword = toLower(readLine("Enter title or author keyword: "));
        bool any = false;
        for (size_t i = 0; i < books.size(); i++) {
            if (toLower(books[i].title).find(keyword) != string::npos ||
                toLower(books[i].author).find(keyword) != string::npos) {
                if (!any) printHeader();
                printRow(books[i]);
                any = true;
            }
        }
        if (!any) cout << "No matching books found.\n";
    }

    void showStats() const {
        int issuedCount = 0;
        for (size_t i = 0; i < books.size(); i++) {
            if (books[i].issued) issuedCount++;
        }
        cout << "\n--- Library Statistics ---\n";
        cout << "Total books : " << books.size() << "\n";
        cout << "Available   : " << (books.size() - issuedCount) << "\n";
        cout << "Issued      : " << issuedCount << "\n";
    }
};

// ---------- Menu ----------

void printMenu() {
    cout << "\n==========================================\n";
    cout << "        BOOK LIBRARY SYSTEM\n";
    cout << "==========================================\n";
    cout << " 1. Add a book\n";
    cout << " 2. Delete a book\n";
    cout << " 3. View book inventory\n";
    cout << " 4. View available books\n";
    cout << " 5. Issue a book to someone\n";
    cout << " 6. Return a book\n";
    cout << " 7. View issued books\n";
    cout << " 8. Search books\n";
    cout << " 9. Library statistics\n";
    cout << " 0. Exit\n";
    cout << "------------------------------------------\n";
}

int main() {
    Library library;
    bool running = true;

    while (running) {
        printMenu();
        int choice = readInt("Choose an option: ");
        switch (choice) {
            case 1: library.addBook(); break;
            case 2: library.deleteBook(); break;
            case 3: library.viewInventory(); break;
            case 4: library.viewAvailable(); break;
            case 5: library.issueBook(); break;
            case 6: library.returnBook(); break;
            case 7: library.viewIssued(); break;
            case 8: library.searchBooks(); break;
            case 9: library.showStats(); break;
            case 0:
                library.save();
                cout << "\nData saved. Goodbye!\n";
                running = false;
                break;
            default:
                cout << "Invalid option. Please choose between 0 and 9.\n";
        }
    }
    return 0;
}
