#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <limits>
#include <map>

using namespace std;

namespace clr {
    const string reset   = "\033[0m";
    const string bold    = "\033[1m";
    const string dim     = "\033[2m";

    const string red     = "\033[91m";
    const string green   = "\033[92m";
    const string yellow  = "\033[93m";
    const string blue    = "\033[94m";
    const string magenta = "\033[95m";
    const string cyan    = "\033[96m";
    const string white   = "\033[97m";
    const string gray    = "\033[90m";

    const string bred    = "\033[1;91m";
    const string bgreen  = "\033[1;92m";
    const string byellow = "\033[1;93m";
    const string bblue   = "\033[1;94m";
    const string bmagenta= "\033[1;95m";
    const string bcyan   = "\033[1;96m";
    const string bwhite  = "\033[1;97m";
}

void enableColors() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode)) return;
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

string currentTimestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

string generateID(const string& prefix, int num) {
    ostringstream ss;
    ss << prefix << setw(4) << setfill('0') << num;
    return ss.str();
}

class Transaction {
public:
    string transactionID;
    string type;
    double amount;
    double balanceAfter;
    string timestamp;
    string description;

    Transaction() {}
    Transaction(string id, string t, double amt, double bal, string desc)
        : transactionID(id), type(t), amount(amt), balanceAfter(bal),
          timestamp(currentTimestamp()), description(desc) {}

    string serialize() const {
        ostringstream ss;
        ss << transactionID << "|" << type << "|" << fixed << setprecision(2)
           << amount << "|" << balanceAfter << "|" << timestamp << "|" << description;
        return ss.str();
    }

    static Transaction deserialize(const string& line) {
        Transaction t;
        istringstream ss(line);
        string token;
        getline(ss, t.transactionID, '|');
        getline(ss, t.type, '|');
        getline(ss, token, '|'); t.amount = stod(token);
        getline(ss, token, '|'); t.balanceAfter = stod(token);
        getline(ss, t.timestamp, '|');
        getline(ss, t.description);
        return t;
    }
};

class Account {
public:
    string accountNumber;
    string customerID;
    string accountType;
    double balance;
    vector<Transaction> transactions;
    int transactionCounter;
    bool active;

    Account() : balance(0), transactionCounter(0), active(true) {}

    Account(string accNum, string custID, string type, double initialDeposit)
        : accountNumber(accNum), customerID(custID), accountType(type),
          balance(initialDeposit), transactionCounter(0), active(true) {
        if (initialDeposit > 0)
            addTransaction("CREDIT", initialDeposit, "Initial deposit");
    }

    void addTransaction(const string& type, double amount, const string& desc) {
        transactionCounter++;
        string txID = accountNumber + "-TX" + to_string(transactionCounter);
        transactions.push_back(Transaction(txID, type, amount, balance, desc));
    }

    bool deposit(double amount) {
        if (amount <= 0) return false;
        balance += amount;
        addTransaction("CREDIT", amount, "Deposit");
        return true;
    }

    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        addTransaction("DEBIT", amount, "Withdrawal");
        return true;
    }

    bool transferOut(double amount, const string& toAccount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        addTransaction("TRANSFER-OUT", amount, "Transfer to " + toAccount);
        return true;
    }

    void transferIn(double amount, const string& fromAccount) {
        balance += amount;
        addTransaction("TRANSFER-IN", amount, "Transfer from " + fromAccount);
    }

    void printStatement(int count = 10) const {
        cout << "\n" << clr::bcyan << "  Account Statement : " << clr::bwhite << accountNumber << clr::reset << "\n";
        cout << clr::cyan << "  Type    : " << clr::white << accountType
             << clr::cyan << "   |   Balance : " << clr::bgreen << "$"
             << fixed << setprecision(2) << balance << clr::reset << "\n\n";

        cout << clr::gray << "  +" << string(18, '-') << "+" << string(16, '-')
             << "+" << string(13, '-') << "+" << string(13, '-') << "+" << string(21, '-') << "+" << clr::reset << "\n";

        cout << clr::bold << clr::yellow
             << "  | " << left << setw(17) << "Transaction ID"
             << "| " << setw(15) << "Type"
             << "| " << setw(12) << "Amount"
             << "| " << setw(12) << "Balance"
             << "| " << setw(20) << "Date & Time"
             << "|" << clr::reset << "\n";

        cout << clr::gray << "  +" << string(18, '-') << "+" << string(16, '-')
             << "+" << string(13, '-') << "+" << string(13, '-') << "+" << string(21, '-') << "+" << clr::reset << "\n";

        int start = max(0, (int)transactions.size() - count);
        for (int i = start; i < (int)transactions.size(); i++) {
            const Transaction& tx = transactions[i];
            string typeColor = clr::white;
            string amtColor  = clr::white;
            if (tx.type == "CREDIT" || tx.type == "TRANSFER-IN") {
                typeColor = clr::bgreen; amtColor = clr::bgreen;
            } else if (tx.type == "DEBIT" || tx.type == "TRANSFER-OUT") {
                typeColor = clr::bred; amtColor = clr::bred;
            }
            cout << clr::gray << "  | " << clr::reset
                 << clr::cyan << left << setw(17) << tx.transactionID << clr::gray << "| " << clr::reset
                 << typeColor << setw(15) << tx.type << clr::gray << "| " << clr::reset
                 << amtColor << "$" << setw(12) << fixed << setprecision(2) << tx.amount << clr::gray << "| " << clr::reset
                 << clr::white << "$" << setw(12) << tx.balanceAfter << clr::gray << "| " << clr::reset
                 << clr::dim << setw(20) << tx.timestamp << clr::gray << "|" << clr::reset << "\n";
        }

        cout << clr::gray << "  +" << string(18, '-') << "+" << string(16, '-')
             << "+" << string(13, '-') << "+" << string(13, '-') << "+" << string(21, '-') << "+" << clr::reset << "\n";
    }
};

class Customer {
public:
    string customerID;
    string fullName;
    string email;
    string phone;
    string address;
    string dateJoined;
    vector<string> accountNumbers;
    bool active;

    Customer() : active(true) {}

    Customer(string id, string name, string mail, string ph, string addr)
        : customerID(id), fullName(name), email(mail), phone(ph),
          address(addr), dateJoined(currentTimestamp()), active(true) {}

    void printInfo() const {
        cout << "\n";
        cout << clr::bcyan  << "  +--------------------------------------------------+" << clr::reset << "\n";
        cout << clr::bwhite << "  |           CUSTOMER PROFILE                      |" << clr::reset << "\n";
        cout << clr::bcyan  << "  +--------------------------------------------------+" << clr::reset << "\n";
        cout << clr::yellow << "  | ID       : " << clr::white  << left << setw(38) << customerID << clr::yellow << "|" << clr::reset << "\n";
        cout << clr::yellow << "  | Name     : " << clr::white  << setw(38) << fullName << clr::yellow << "|" << clr::reset << "\n";
        cout << clr::yellow << "  | Email    : " << clr::cyan   << setw(38) << email    << clr::yellow << "|" << clr::reset << "\n";
        cout << clr::yellow << "  | Phone    : " << clr::white  << setw(38) << phone    << clr::yellow << "|" << clr::reset << "\n";
        cout << clr::yellow << "  | Address  : " << clr::white  << setw(38) << address  << clr::yellow << "|" << clr::reset << "\n";
        cout << clr::yellow << "  | Joined   : " << clr::dim    << setw(38) << dateJoined<< clr::yellow<< "|" << clr::reset << "\n";
        cout << clr::bcyan  << "  +--------------------------------------------------+" << clr::reset << "\n";

        cout << clr::yellow << "  Accounts: ";
        if (accountNumbers.empty()) {
            cout << clr::gray << "None\n" << clr::reset;
        } else {
            for (size_t i = 0; i < accountNumbers.size(); i++) {
                if (i) cout << clr::gray << ", " << clr::reset;
                cout << clr::bgreen << accountNumbers[i];
            }
            cout << clr::reset << "\n";
        }
    }
};

class Bank {
private:
    string bankName;
    map<string, Customer> customers;
    map<string, Account>  accounts;
    int customerCounter;
    int accountCounter;
    string dataDir;

    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void pause() {
        cout << "\n  " << clr::gray << "Press Enter to continue..." << clr::reset;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    void printHeader() {
        clearScreen();
        cout << "\n";
        cout << clr::bblue  << "  #============================================================#" << clr::reset << "\n";
        cout << clr::bwhite << "  #" << clr::bcyan << "           N E X U S   B A N K                          " << clr::bwhite << "#" << clr::reset << "\n";
        cout << clr::bblue  << "  #" << clr::dim   << "              Core Banking System v1.0                  " << clr::bblue << "#" << clr::reset << "\n";
        cout << clr::bblue  << "  #============================================================#" << clr::reset << "\n\n";
    }

    void printDivider() {
        cout << clr::gray << "  " << string(56, '-') << clr::reset << "\n";
    }

    void printSectionTitle(const string& title) {
        cout << clr::bmagenta << "  [ " << clr::bwhite << title << clr::bmagenta << " ]" << clr::reset << "\n";
        printDivider();
    }

    void successMsg(const string& msg) {
        cout << "\n  " << clr::bgreen << "[OK] " << clr::green << msg << clr::reset << "\n";
    }

    void errorMsg(const string& msg) {
        cout << "\n  " << clr::bred << "[!!] " << clr::red << msg << clr::reset << "\n";
    }

    double getAmount(const string& prompt) {
        double val;
        while (true) {
            cout << clr::yellow << prompt << clr::white;
            if (cin >> val && val > 0) { cin.ignore(); return val; }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            errorMsg("Invalid amount. Try again.");
        }
    }

    int getMenuChoice(int mn, int mx) {
        int choice;
        while (true) {
            cout << "\n  " << clr::byellow << "Your choice: " << clr::bwhite;
            if (cin >> choice && choice >= mn && choice <= mx) { cin.ignore(); return choice; }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            errorMsg("Enter a number between " + to_string(mn) + " and " + to_string(mx));
        }
    }

    string getInput(const string& label) {
        string val;
        cout << clr::yellow << label << clr::white;
        getline(cin, val);
        return val;
    }

    Customer* findCustomer(const string& id) {
        auto it = customers.find(id);
        if (it != customers.end() && it->second.active) return &it->second;
        return nullptr;
    }

    Account* findAccount(const string& num) {
        auto it = accounts.find(num);
        if (it != accounts.end() && it->second.active) return &it->second;
        return nullptr;
    }

    void saveData() {
        ofstream cf(dataDir + "customers.dat");
        for (auto& p : customers) {
            Customer& c = p.second;
            cf << c.customerID << "|" << c.fullName << "|" << c.email << "|"
               << c.phone << "|" << c.address << "|" << c.dateJoined << "|"
               << (c.active ? 1 : 0) << "|";
            for (size_t i = 0; i < c.accountNumbers.size(); i++) {
                if (i) cf << ",";
                cf << c.accountNumbers[i];
            }
            cf << "\n";
        }

        ofstream af(dataDir + "accounts.dat");
        for (auto& p : accounts) {
            Account& a = p.second;
            af << a.accountNumber << "|" << a.customerID << "|" << a.accountType
               << "|" << fixed << setprecision(2) << a.balance << "|"
               << (a.active ? 1 : 0) << "|" << a.transactionCounter << "\n";
            for (auto& tx : a.transactions)
                af << "TX:" << tx.serialize() << "\n";
        }

        ofstream meta(dataDir + "meta.dat");
        meta << customerCounter << "\n" << accountCounter << "\n";
    }

    void loadData() {
        ifstream meta(dataDir + "meta.dat");
        if (meta) meta >> customerCounter >> accountCounter;

        ifstream cf(dataDir + "customers.dat");
        string line;
        while (getline(cf, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            Customer c; string token;
            getline(ss, c.customerID, '|'); getline(ss, c.fullName, '|');
            getline(ss, c.email, '|');      getline(ss, c.phone, '|');
            getline(ss, c.address, '|');    getline(ss, c.dateJoined, '|');
            getline(ss, token, '|'); c.active = (token == "1");
            getline(ss, token);
            if (!token.empty()) {
                istringstream acc(token); string an;
                while (getline(acc, an, ',')) c.accountNumbers.push_back(an);
            }
            customers[c.customerID] = c;
        }

        ifstream af(dataDir + "accounts.dat");
        Account* cur = nullptr;
        while (getline(af, line)) {
            if (line.empty()) continue;
            if (line.size() >= 3 && line.substr(0, 3) == "TX:") {
                if (cur) cur->transactions.push_back(Transaction::deserialize(line.substr(3)));
            } else {
                Account a; istringstream ss(line); string token;
                getline(ss, a.accountNumber, '|'); getline(ss, a.customerID, '|');
                getline(ss, a.accountType, '|');
                getline(ss, token, '|'); a.balance = stod(token);
                getline(ss, token, '|'); a.active = (token == "1");
                getline(ss, token); a.transactionCounter = stoi(token);
                accounts[a.accountNumber] = a;
                cur = &accounts[a.accountNumber];
            }
        }
    }

    void createCustomer() {
        printHeader(); printSectionTitle("New Customer Registration");
        string name  = getInput("  Full Name    : ");
        string email = getInput("  Email        : ");
        string phone = getInput("  Phone        : ");
        string addr  = getInput("  Address      : ");

        if (name.empty() || email.empty()) { errorMsg("Name and email cannot be empty."); pause(); return; }

        customerCounter++;
        string cid = generateID("CUST", customerCounter);
        customers[cid] = Customer(cid, name, email, phone, addr);
        saveData();

        successMsg("Customer registered successfully!");
        cout << "  " << clr::cyan << "Customer ID  : " << clr::bgreen << cid << clr::reset << "\n";
        cout << "  " << clr::cyan << "Name         : " << clr::white << name << clr::reset << "\n";
        pause();
    }

    void createAccount() {
        printHeader(); printSectionTitle("Open New Account");
        string cid = getInput("  Customer ID  : ");
        Customer* c = findCustomer(cid);
        if (!c) { errorMsg("Customer not found."); pause(); return; }

        cout << "\n  " << clr::bcyan << "Account Type:" << clr::reset << "\n";
        cout << "    " << clr::bwhite << "1." << clr::green   << " Savings\n" << clr::reset;
        cout << "    " << clr::bwhite << "2." << clr::yellow  << " Checking\n" << clr::reset;
        cout << "    " << clr::bwhite << "3." << clr::magenta << " Fixed Deposit\n" << clr::reset;

        int choice = getMenuChoice(1, 3);
        string types[] = {"Savings", "Checking", "Fixed Deposit"};
        string type = types[choice - 1];

        double initial = getAmount("  Initial Deposit ($) : ");

        accountCounter++;
        string accNum = generateID("ACC", accountCounter);
        accounts[accNum] = Account(accNum, cid, type, initial);
        c->accountNumbers.push_back(accNum);
        saveData();

        successMsg("Account opened successfully!");
        cout << "  " << clr::cyan << "Account No   : " << clr::bgreen  << accNum  << clr::reset << "\n";
        cout << "  " << clr::cyan << "Type         : " << clr::yellow  << type    << clr::reset << "\n";
        cout << "  " << clr::cyan << "Balance      : " << clr::bgreen  << "$" << fixed << setprecision(2) << initial << clr::reset << "\n";
        pause();
    }

    void depositMenu() {
        printHeader(); printSectionTitle("Deposit Funds");
        string accNum = getInput("  Account No   : ");
        Account* a = findAccount(accNum);
        if (!a) { errorMsg("Account not found."); pause(); return; }

        cout << "  " << clr::cyan << "Current Balance : " << clr::bgreen << "$" << fixed << setprecision(2) << a->balance << clr::reset << "\n";
        double amt = getAmount("  Deposit Amount ($) : ");
        a->deposit(amt);
        saveData();

        successMsg("Deposit successful!");
        cout << "  " << clr::cyan << "Amount       : " << clr::bgreen << "+$" << fixed << setprecision(2) << amt << clr::reset << "\n";
        cout << "  " << clr::cyan << "New Balance  : " << clr::bgreen << "$" << a->balance << clr::reset << "\n";
        pause();
    }

    void withdrawMenu() {
        printHeader(); printSectionTitle("Withdraw Funds");
        string accNum = getInput("  Account No   : ");
        Account* a = findAccount(accNum);
        if (!a) { errorMsg("Account not found."); pause(); return; }

        cout << "  " << clr::cyan << "Current Balance : " << clr::bgreen << "$" << fixed << setprecision(2) << a->balance << clr::reset << "\n";
        double amt = getAmount("  Withdraw Amount ($) : ");

        if (!a->withdraw(amt)) {
            errorMsg("Insufficient funds.");
        } else {
            saveData();
            successMsg("Withdrawal successful!");
            cout << "  " << clr::cyan << "Amount       : " << clr::bred << "-$" << fixed << setprecision(2) << amt << clr::reset << "\n";
            cout << "  " << clr::cyan << "New Balance  : " << clr::bgreen << "$" << a->balance << clr::reset << "\n";
        }
        pause();
    }

    void transferMenu() {
        printHeader(); printSectionTitle("Fund Transfer");
        string fromAcc = getInput("  From Account : ");
        Account* from = findAccount(fromAcc);
        if (!from) { errorMsg("Source account not found."); pause(); return; }

        string toAcc = getInput("  To Account   : ");
        Account* to = findAccount(toAcc);
        if (!to) { errorMsg("Destination account not found."); pause(); return; }

        if (fromAcc == toAcc) { errorMsg("Cannot transfer to the same account."); pause(); return; }

        cout << "  " << clr::cyan << "Available Balance : " << clr::bgreen << "$" << fixed << setprecision(2) << from->balance << clr::reset << "\n";
        double amt = getAmount("  Transfer Amount ($) : ");

        if (!from->transferOut(amt, toAcc)) {
            errorMsg("Insufficient funds.");
        } else {
            to->transferIn(amt, fromAcc);
            saveData();
            successMsg("Transfer completed!");
            cout << "  " << clr::cyan << "Transferred  : " << clr::bgreen << "$" << fixed << setprecision(2) << amt << clr::reset << "\n";
            cout << "  " << clr::cyan << "From Balance : " << clr::bred   << "$" << from->balance << clr::reset << "\n";
            cout << "  " << clr::cyan << "To Balance   : " << clr::bgreen << "$" << to->balance   << clr::reset << "\n";
        }
        pause();
    }

    void viewAccount() {
        printHeader(); printSectionTitle("Account Details");
        string accNum = getInput("  Account No   : ");
        Account* a = findAccount(accNum);
        if (!a) { errorMsg("Account not found."); pause(); return; }

        Customer* c = findCustomer(a->customerID);
        if (c) {
            cout << "\n  " << clr::bcyan << "Holder : " << clr::bwhite << c->fullName
                 << clr::gray << "  (" << c->customerID << ")" << clr::reset << "\n";
            cout << "  " << clr::cyan << "Email  : " << clr::white << c->email
                 << clr::cyan << "   Phone: " << clr::white << c->phone << clr::reset << "\n";
        }
        a->printStatement(10);
        pause();
    }

    void viewCustomer() {
        printHeader(); printSectionTitle("Customer Profile");
        string cid = getInput("  Customer ID  : ");
        Customer* c = findCustomer(cid);
        if (!c) { errorMsg("Customer not found."); pause(); return; }

        c->printInfo();
        cout << "\n  " << clr::bcyan << "Linked Accounts:\n" << clr::reset;
        for (auto& accNum : c->accountNumbers) {
            Account* a = findAccount(accNum);
            if (a) {
                cout << "    " << clr::bwhite << "[" << clr::bgreen << a->accountNumber << clr::bwhite << "] "
                     << clr::yellow << a->accountType << clr::gray << "  --  "
                     << clr::cyan << "Balance: " << clr::bgreen << "$" << fixed << setprecision(2) << a->balance
                     << clr::reset << "\n";
            }
        }
        pause();
    }

    void listAllCustomers() {
        printHeader(); printSectionTitle("All Registered Customers");
        if (customers.empty()) { cout << "  " << clr::gray << "No customers yet.\n" << clr::reset; pause(); return; }

        cout << clr::bold << clr::yellow
             << "  " << left << setw(11) << "ID"
             << setw(22) << "Full Name"
             << setw(26) << "Email"
             << "Accounts" << clr::reset << "\n";
        printDivider();

        for (auto& p : customers) {
            Customer& c = p.second;
            if (!c.active) continue;
            cout << "  " << clr::bgreen << left << setw(11) << c.customerID
                 << clr::white  << setw(22) << c.fullName.substr(0, 21)
                 << clr::cyan   << setw(26) << c.email.substr(0, 25)
                 << clr::yellow << c.accountNumbers.size() << clr::reset << "\n";
        }
        printDivider();
        pause();
    }

    void miniStatement() {
        printHeader(); printSectionTitle("Mini Statement");
        string accNum = getInput("  Account No   : ");
        Account* a = findAccount(accNum);
        if (!a) { errorMsg("Account not found."); pause(); return; }
        a->printStatement(5);
        pause();
    }

public:
    Bank(const string& name, const string& dir = "data/")
        : bankName(name), customerCounter(0), accountCounter(0), dataDir(dir) {
#ifdef _WIN32
        system(("mkdir " + dir + " 2>nul").c_str());
#else
        system(("mkdir -p " + dir).c_str());
#endif
        loadData();
    }

    void run() {
        while (true) {
            printHeader();
            cout << clr::bwhite << "  MAIN MENU\n" << clr::reset;
            printDivider();
            cout << "  " << clr::bwhite << "1." << clr::white   << "  Register New Customer\n";
            cout << "  " << clr::bwhite << "2." << clr::white   << "  Open New Account\n";
            cout << "  " << clr::bwhite << "3." << clr::bgreen  << "  Deposit\n";
            cout << "  " << clr::bwhite << "4." << clr::bred    << "  Withdraw\n";
            cout << "  " << clr::bwhite << "5." << clr::byellow << "  Fund Transfer\n";
            cout << "  " << clr::bwhite << "6." << clr::cyan    << "  View Account & Transactions\n";
            cout << "  " << clr::bwhite << "7." << clr::cyan    << "  Mini Statement (Last 5)\n";
            cout << "  " << clr::bwhite << "8." << clr::magenta << "  View Customer Profile\n";
            cout << "  " << clr::bwhite << "9." << clr::white   << "  List All Customers\n";
            cout << "  " << clr::bwhite << "0." << clr::gray    << "  Exit\n" << clr::reset;
            printDivider();

            int choice = getMenuChoice(0, 9);
            switch (choice) {
                case 1: createCustomer();   break;
                case 2: createAccount();    break;
                case 3: depositMenu();      break;
                case 4: withdrawMenu();     break;
                case 5: transferMenu();     break;
                case 6: viewAccount();      break;
                case 7: miniStatement();    break;
                case 8: viewCustomer();     break;
                case 9: listAllCustomers(); break;
                case 0:
                    printHeader();
                    cout << "  " << clr::bgreen << "Thank you for banking with Nexus Bank. Goodbye!" << clr::reset << "\n\n";
                    return;
            }
        }
    }
};

int main() {
    enableColors();
    Bank bank("NEXUS BANK  --  Core Banking System");
    bank.run();
    return 0;
}
