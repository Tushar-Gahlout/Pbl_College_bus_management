# College Bus Management System (ERP)
> **Course:** Object-Oriented Programming (OOPS with C++) Project-Based Learning (PBL)  
> **Technologies:** C++17 • Qt 6 Widgets • File Handling (No Database) • CMake

---

## 📌 Project Overview

The **College Bus Management System** is a professional desktop application designed to manage student transportation, bus fleets, routes, drivers, seat allocation, daily attendance, fee collections, vehicle servicing, and CSV reports.

Built strictly using **C++17**, **Qt 6 Widgets**, and **C++ File Handling** (`ifstream`, `ofstream`, `fstream`), the project strictly avoids any external database (like SQLite/MySQL), web engines, or non-C++ languages.

---

## 🏗️ Folder Structure

```
CollegeBusManagement/
├── CMakeLists.txt                # Main CMake build configuration
├── main.cpp                      # Application entry point & Qt event loop
├── models/                       # OOPS Data Models
│   ├── Person.h                  # Abstract Base Class (Abstraction & Inheritance)
│   ├── Student.h / Student.cpp   # Derived from Person (Polymorphism)
│   ├── Driver.h / Driver.cpp     # Derived from Person (Polymorphism)
│   ├── Bus.h / Bus.cpp           # Bus fleet encapsulation & capacity logic
│   ├── Route.h / Route.cpp       # Transit routes & stops
│   ├── Seat.h / Seat.cpp         # Seat allocation records
│   ├── Attendance.h / .cpp       # Daily attendance records
│   ├── FeeRecord.h / .cpp        # Fee payment records & receipts
│   ├── MaintenanceRecord.h / .cpp# Bus servicing & expense records
│   └── User.h                    # Authentication model
├── services/                     # Business Logic & Persistence
│   ├── Exceptions.h              # Custom OOP Exceptions
│   └── FileManager.h / .cpp      # C++ File Handling engine (CRUD + Temp files)
├── views/                        # Modern Qt 6 GUI Widgets
│   ├── LoginDialog.h / .cpp      # Role-based login with evaluation quick-fill
│   ├── MainWindow.h / .cpp       # Main shell with dark blue sidebar
│   ├── AdminDashboardView.h/.cpp # Metrics cards & fleet status
│   ├── StudentManagementView.h/.cpp # Add, Edit, Delete, Search students
│   ├── BusManagementView.h/.cpp  # Bus fleet & capacity limits
│   ├── RouteManagementView.h/.cpp# Routes, stops & timings
│   ├── DriverManagementView.h/.cpp# Driver profiles & bus assignment
│   ├── SeatAllocationView.h/.cpp # Interactive 2x2 visual bus grid
│   ├── AttendanceView.h/.cpp     # Daily passenger attendance & history
│   ├── FeeManagementView.h/.cpp  # Fees, dues, payments & receipts
│   ├── MaintenanceView.h/.cpp    # Servicing records & bus status
│   ├── ReportsView.h/.cpp        # Tabbed reports & CSV exports
│   ├── StudentPortalView.h/.cpp  # Dedicated Student Portal
│   ├── DriverPortalView.h/.cpp   # Dedicated Driver Portal
│   └── BusMapWidget.h/.cpp       # Live GPS Bus Tracker & Interactive Map
├── data/                         # Persistent text data files
│   ├── users.txt                 # User credentials & roles
│   ├── students.txt              # Student directory
│   ├── buses.txt                 # Bus fleet
│   ├── drivers.txt               # Driver directory
│   ├── routes.txt                # Transit routes & stops
│   ├── seats.txt                 # Seat allocations
│   ├── attendance.txt            # Attendance records
│   ├── fees.txt                  # Fee payment records
│   └── maintenance.txt           # Servicing expense records
└── resources/
    ├── style.qss                 # Modern Dark Blue stylesheet
    └── resources.qrc             # Qt resource bundle
```

---

## 🔑 Demo Credentials (For Evaluation)

The application features **Role-Based Access Control (RBAC)**. On the login screen, you can also use the **Quick Demo Login dropdown** to populate credentials in one click:

| Role | Username | Password | Access / Dashboard |
| :--- | :--- | :--- | :--- |
| **Admin** | `admin` | `admin123` | Full control over all 9 modules, management & reports |
| **Student** | `student1` | `pass123` | Personal bus & seat info, route stops, fee receipt, attendance history |
| **Student** | `student2` | `pass123` | Student portal for Priya Patel |
| **Driver** | `driver1` | `pass123` | Assigned bus & route timings, today's passenger list & attendance |
| **Driver** | `driver2` | `pass123` | Driver portal for Vikram Singh |

---

## 🎓 Demonstrating OOPS Concepts in Evaluation

This project was architected specifically to highlight core Object-Oriented Programming concepts:

### 1. Classes and Objects
- Every real-world entity is represented as an independent class: `Student`, `Driver`, `Bus`, `Route`, `Seat`, `Attendance`, `FeeRecord`, `MaintenanceRecord`.
- Objects are dynamically managed in vectors and persisted to file storage.

### 2. Encapsulation
- Member variables are declared `private` or `protected`.
- Access and modification are mediated through getters, setters, and business methods with validation (e.g., `Bus::isFull()`, `Bus::allocateSeat()`, `Student::setSemester()`).

### 3. Inheritance
- `Person` acts as the base class for human entities.
- `Student` and `Driver` derive from `Person` (`class Student : public Person`, `class Driver : public Person`), reusing fields (`id`, `name`, `phone`, `email`).

### 4. Polymorphism
- `Person` declares pure virtual methods:
  ```cpp
  virtual std::string getRole() const = 0;
  virtual std::string getDetails() const = 0;
  virtual std::string serialize() const = 0;
  ```
- `Student` and `Driver` provide concrete polymorphic implementations (`override`).
- `Person` features a `virtual ~Person() = default;` to ensure proper destruction of derived objects via base pointers.

### 5. Abstraction
- High-level business operations (`FileManager`, `LoginDialog`, `ReportsView`) interact through clean, well-defined class interfaces without exposing the underlying storage mechanisms.

### 6. Function Overloading
- Implemented in `FileManager` for different search and payment workflows:
  ```cpp
  // Search by single string query (ID, Name, Dept, Bus)
  std::vector<Student> searchStudents(const std::string& query);

  // Search by Department AND Semester
  std::vector<Student> searchStudents(const std::string& department, int semester);

  // Record payment with all details
  bool recordPayment(const std::string& studentId, double amount, const std::string& date, const std::string& method, const std::string& receipt);

  // Overloaded record payment with automated defaults
  bool recordPayment(const std::string& studentId, double amount);
  ```

### 7. Exception Handling
- Robust error handling using custom exception classes derived from `std::exception`:
  - `DuplicateIdException`: Thrown when trying to insert an existing Student or Bus ID.
  - `BusFullException`: Thrown when assigning a student to a bus whose capacity is reached.
  - `NotFoundException`: Thrown when updating/deleting non-existent IDs.
  - `FileIOException`: Thrown if a data file cannot be accessed.
  - Handled in views and presented via `QMessageBox::critical` without crashing.

---

## 💾 C++ File Handling Architecture

The project adheres strictly to **pure C++ file streams**:
- **Reading:** `std::ifstream` parses pipe-delimited (`|`) records line-by-line using `std::getline()`.
- **Appending:** `std::ofstream` with `std::ios::app` adds new entries directly to the file.
- **Safe Update & Delete (Temporary Files):**
  When updating or deleting records, `FileManager` writes valid records to a temporary file (`temp_students.txt`, `temp_buses.txt`), deletes the original file, and atomically renames the temporary file using `std::filesystem::rename()`. This protects against data corruption.
- **CSV Export:** Reports are exported to standard CSV files using `std::ofstream` and can be opened in Microsoft Excel or Apple Numbers.

---

## 🚀 How to Run the Project on macOS

### Prerequisites
Make sure you have CMake and Qt 6 installed. If not already installed, run:
```bash
brew install cmake qt@6
```

---

### Option 1: Using Qt Creator (Recommended & Easiest)
1. Open **Qt Creator**.
2. Select **File > Open File or Project...**
3. Navigate to `CollegeBusManagement/CMakeLists.txt` and click **Open**.
4. Choose the **Qt 6 (Desktop)** kit.
5. Click **Configure Project**.
6. Press the green **Run (▶)** button or press `Cmd + R`.

---

### Option 2: Using Terminal CLI
Open Terminal in the project directory:

```bash
# 1. Navigate to the project folder
cd /Users/harshgahlot/Desktop/tushar/CollegeBusManagement

# 2. Tell CMake where Homebrew installed Qt 6
export CMAKE_PREFIX_PATH="$(brew --prefix qt@6):$CMAKE_PREFIX_PATH"

# 3. Configure the build
cmake -B build -S .

# 4. Compile the project
cmake --build build

# 5. Run the application
./build/CollegeBusManagement
```

---

### Option 3: Using Visual Studio Code
1. Open Visual Studio Code.
2. Open the folder: `/Users/harshgahlot/Desktop/tushar`.
3. Install the **CMake Tools** and **C/C++** extensions if prompted.
4. If CMake does not find Qt6 automatically, add this setting in `.vscode/settings.json`:
   ```json
   {
       "cmake.configureSettings": {
           "CMAKE_PREFIX_PATH": "/opt/homebrew/opt/qt@6"
       }
   }
   ```
5. Press `F5` or click **Build & Run** on the bottom status bar.

---

## 💡 Evaluation / Viva Questions & Answers

**Q1: Why did you use File Handling instead of SQLite or MySQL?**  
*Answer:* As per the project guidelines, the goal is to demonstrate standard C++ stream handling (`ifstream`, `ofstream`, `fstream`) and file operations (`ios::app`, temporary file swapping) without third-party database dependencies.

**Q2: How do you prevent bus overloading?**  
*Answer:* The `Bus` class tracks `capacity` and `occupiedSeats`. Before assigning any student to a bus, `Bus::canAllocate()` is checked. If `occupiedSeats >= capacity`, a custom `BusFullException` is thrown and caught by the UI, alerting the user that the bus is full.

**Q3: Where is Polymorphism used in the application?**  
*Answer:* The abstract base class `Person` declares pure virtual methods (`getRole()`, `getDetails()`, `serialize()`). `Student` and `Driver` override these methods to provide role-specific behaviors.

**Q4: How do updates and deletes avoid data loss?**  
*Answer:* Instead of rewriting files in place, we stream data to a temporary file (`temp_*.txt`). If any write error or exception occurs, the original file is untouched. Once complete, `std::filesystem::rename` atomically swaps the files.
