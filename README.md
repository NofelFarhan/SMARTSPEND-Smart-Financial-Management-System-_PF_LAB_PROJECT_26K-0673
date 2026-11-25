# SmartSpend – Personal Finance Management System

## About the Project

SmartSpend is a personal finance management application developed using the C programming language. It helps users manage their daily income and expenses, set budgets, track upcoming payments, and monitor their savings goals.

The main purpose of this project is to make personal money management easier by providing important financial information in one place.

## Features

SmartSpend includes the following features:

### 1. Dashboard
- Displays total income and expenses.
- Shows the available balance.
- Displays savings progress and financial alerts.
- Shows upcoming payments.

### 2. Add Transaction
- Add income and expense records.
- Enter the amount, date, category, and description.
- Checks that the amount and date are valid.

### 3. Transaction History
- View previously added transactions.
- Search transactions by category or description.
- View transactions in pages.

### 4. Budget Management
- Set a monthly spending budget.
- Set separate budgets for different categories.
- Monitor spending using visual progress bars.

### 5. Payment Management
- Add upcoming payments and bills.
- Set payment amounts and due dates.
- Support recurring monthly payments.
- Mark payments as paid.

### 6. Savings Goal and Emergency Reserve
- Create a savings goal.
- Set a target amount.
- Update the amount saved.
- Keep track of an emergency reserve.

### 7. Financial Advisor
- Check whether a planned purchase is affordable.
- Estimate how long it may take to reach a savings goal.
- Compare monthly spending.
- Identify spending categories that have increased.

## Technologies Used

- **Programming Language:** C
- **Graphics Library:** Raylib
- **GUI Library:** Raygui
- **File Handling:** C file handling for saving and loading data
- **Compiler:** GCC (C99)

## Programming Concepts Used

This project demonstrates the following Programming Fundamentals concepts:

- Variables and data types
- Conditional statements (`if`, `else`)
- Loops (`for`, `while`)
- Functions
- Arrays
- Structures
- String handling
- File handling
- Basic calculations and logical operations

## Project Files

| File | Purpose |
|---|---|
| `main.c` | Main application and graphical interface |
| `data.c` | Saving and loading application data |
| `finance.c` | Financial calculations and analysis |
| `smartspend.h` | Shared structures, constants, and function declarations |
| `test_console.c` | Console-based tests |

## Data Storage

SmartSpend uses local text files to store information:

- `data_txns.txt` – Transaction records
- `data_pays.txt` – Payment records
- `data_settings.txt` – Budget and savings settings

The application loads saved information when it starts and saves changes locally.

## How to Run the Project

1. Download or clone this repository.
2. Make sure GCC and the required Raylib and Raygui dependencies are installed.
3. Open the project in your C development environment.
4. Configure the compiler and linker to use Raylib.
5. Compile and run the application.

**Note:** The exact compilation command depends on your operating system and Raylib installation.

## Project Limitations

- Data is stored locally rather than in an online database.
- The application does not provide cloud synchronization.
- Transaction editing and deletion are not currently included.
- Financial suggestions are based on the project's own calculations and should not be treated as professional financial advice.

## Future Improvements

- Add transaction editing and deletion.
- Add graphical reports and charts.
- Introduce data backup and export options.
- Add more financial analysis features.
- Improve data security and storage.

## Project Information

**Project Name:** SmartSpend  
**Course:** Programming Fundamentals  
**Language:** C  
**Project Type:** Desktop Application

**Developed by:**
- Muhammad Nofel
- Muhammad Hassan

**Institution:** FAST-NUCES, Karachi

---

*SmartSpend – Record, Plan, and Understand Your Spending.*
