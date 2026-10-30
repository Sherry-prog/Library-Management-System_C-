# Book Library System (C++ Console)

A basic, menu-driven library management program that runs in the terminal. Written in standard C++11 with no external libraries. Data is saved automatically to a text file, so your books and loans are still there next time you run it.

## Features

- Add a book (title and author, ID assigned automatically)
- Delete a book (blocked if the book is currently issued)
- View the full book inventory
- View only available books
- Issue a book to someone, with issue date and due date (14 days)
- Return a book, with a late notice if it is overdue
- View all issued books, with borrower names and an OVERDUE flag
- Search books by title or author keyword
- Library statistics (total, available and issued counts)
- Automatic saving to `library_data.txt`
- Input validation for numbers and empty fields

The library starts with 5 sample books the first time you run it.

## Requirements

- A C++ compiler with C++11 support (g++, clang++ or MSVC)

Check your installation:

```
g++ --version
```

## How to Compile and Run

```
g++ -std=c++11 -o library LibrarySystem.cpp
./library
```

On Windows (MinGW):

```
g++ -std=c++11 -o library.exe LibrarySystem.cpp
library.exe
```

## Menu

```
==========================================
        BOOK LIBRARY SYSTEM
==========================================
 1. Add a book
 2. Delete a book
 3. View book inventory
 4. View available books
 5. Issue a book to someone
 6. Return a book
 7. View issued books
 8. Search books
 9. Library statistics
 0. Exit
```

## Example: Issuing a Book

```
Choose an option: 5

--- Issue a Book ---
Enter book ID: 2
Borrower name: Alice
"Clean Code" issued to Alice.
Issue date: 2026-10-30 | Due date: 2026-11-13
```

## Data File

Books are stored in `library_data.txt` in the same folder as the program, one book per line:

```
id|title|author|issued(0/1)|borrower|issueTimestamp
```

The `|` character is replaced with `/` in any text you type so it doesn't break the file. Delete `library_data.txt` to reset the library to the sample books.

## Limitations

- One copy per book (no quantity tracking)
- No member accounts, borrowers are just names
- No fines are calculated, only overdue warnings
- Single user, no login system

## Possible Improvements

- Track multiple copies of each book
- Add member registration and borrowing limits
- Calculate late fees
- Add categories or ISBN numbers
- Split the code into header and source files, and use a database such as SQLite

## License

Free to use and modify for learning and personal projects.
