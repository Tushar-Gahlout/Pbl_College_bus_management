#include "StudentManagementView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QLabel>

StudentManagementView::StudentManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void StudentManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header & Action Bar
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Student Management", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    addBtn = new QPushButton("➕ Add New Student", this);
    addBtn->setProperty("class", "primaryButton");
    addBtn->setCursor(Qt::PointingHandCursor);

    editBtn = new QPushButton("✏️ Edit Student", this);
    editBtn->setProperty("class", "secondaryButton");
    editBtn->setCursor(Qt::PointingHandCursor);

    deleteBtn = new QPushButton("🗑️ Delete Student", this);
    deleteBtn->setProperty("class", "dangerButton");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    assignBusBtn = new QPushButton("🚌 Assign Bus / Route", this);
    assignBusBtn->setProperty("class", "secondaryButton");
    assignBusBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(addBtn);
    topLayout->addWidget(editBtn);
    topLayout->addWidget(deleteBtn);
    topLayout->addWidget(assignBusBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Search and Filter Bar
    auto* filterLayout = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Search student by ID, Name, Department, Bus or Stop...");
    searchEdit->setClearButtonEnabled(true);
    filterLayout->addWidget(searchEdit, 3);

    deptFilterCombo = new QComboBox(this);
    deptFilterCombo->addItem("All Departments");
    deptFilterCombo->addItem("Computer Science");
    deptFilterCombo->addItem("Electronics");
    deptFilterCombo->addItem("Mechanical");
    deptFilterCombo->addItem("Civil");
    deptFilterCombo->addItem("Information Tech");
    deptFilterCombo->addItem("Management");
    filterLayout->addWidget(deptFilterCombo, 1);

    mainLayout->addLayout(filterLayout);

    // Student Records Table
    studentTable = new QTableWidget(this);
    studentTable->setColumnCount(9);
    studentTable->setHorizontalHeaderLabels({
        "Student ID", "Name", "Department", "Sem", "Phone", "Bus No", "Pickup Point", "Seat", "Address"
    });
    studentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(studentTable);

    // Signal connections
    connect(searchEdit, &QLineEdit::textChanged, this, &StudentManagementView::handleSearch);
    connect(deptFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentManagementView::handleSearch);
    connect(addBtn, &QPushButton::clicked, this, &StudentManagementView::handleAdd);
    connect(editBtn, &QPushButton::clicked, this, &StudentManagementView::handleEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &StudentManagementView::handleDelete);
    connect(assignBusBtn, &QPushButton::clicked, this, &StudentManagementView::handleAssignBus);
    connect(refreshBtn, &QPushButton::clicked, this, &StudentManagementView::refreshData);
    connect(studentTable, &QTableWidget::doubleClicked, this, &StudentManagementView::handleEdit);
}

void StudentManagementView::refreshData() {
    handleSearch();
}

void StudentManagementView::handleSearch() {
    std::string query = searchEdit->text().trimmed().toStdString();
    std::string dept = deptFilterCombo->currentText().toStdString();
    if (dept == "All Departments") dept = "";

    std::vector<Student> list;
    if (dept.empty()) {
        list = fileManager->searchStudents(query);
    } else {
        // Use overloaded search
        std::vector<Student> deptStudents = fileManager->searchStudents(dept, 0);
        if (query.empty()) {
            list = deptStudents;
        } else {
            for (const auto& s : deptStudents) {
                if (s.getId().find(query) != std::string::npos ||
                    s.getName().find(query) != std::string::npos) {
                    list.push_back(s);
                }
            }
        }
    }

    studentTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& s = list[i];
        studentTable->insertRow(i);

        studentTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
        studentTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
        studentTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.getDepartment())));
        studentTable->setItem(i, 3, new QTableWidgetItem(QString::number(s.getSemester())));
        studentTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(s.getPhone())));
        studentTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(s.getBusNumber())));
        studentTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));

        QString seatStr = (s.getSeatNumber() > 0) ? QString::number(s.getSeatNumber()) : "None";
        studentTable->setItem(i, 7, new QTableWidgetItem(seatStr));
        studentTable->setItem(i, 8, new QTableWidgetItem(QString::fromStdString(s.getAddress())));
    }
}

void StudentManagementView::handleAdd() {
    showStudentFormDialog(nullptr);
}

void StudentManagementView::handleEdit() {
    int row = studentTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Student", "Please select a student from the table to edit.");
        return;
    }

    std::string sId = studentTable->item(row, 0)->text().toStdString();
    Student s = fileManager->getStudentById(sId);
    if (!s.getId().empty()) {
        showStudentFormDialog(&s);
    }
}

void StudentManagementView::handleAssignBus() {
    handleEdit();
}

void StudentManagementView::handleDelete() {
    int row = studentTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Student", "Please select a student from the table to delete.");
        return;
    }

    QString sId = studentTable->item(row, 0)->text();
    QString name = studentTable->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete student '%1' (%2)?\nTheir bus seat allocation will also be released.").arg(name, sId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            fileManager->deleteStudent(sId.toStdString());
            QMessageBox::information(this, "Success", "Student record deleted successfully.");
            refreshData();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete student: %1").arg(e.what()));
        }
    }
}

void StudentManagementView::showStudentFormDialog(Student* existing) {
    bool isEditing = (existing != nullptr);

    QDialog dlg(this);
    dlg.setWindowTitle(isEditing ? "Edit Student Details" : "Add New Student");
    dlg.setFixedSize(500, 520);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("e.g. STU105");
    if (isEditing) {
        idEdit->setText(QString::fromStdString(existing->getId()));
        idEdit->setEnabled(false); // ID is immutable during edit
    }

    auto* nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("Full Student Name");
    if (isEditing) nameEdit->setText(QString::fromStdString(existing->getName()));

    auto* deptCombo = new QComboBox(&dlg);
    deptCombo->addItems({"Computer Science", "Electronics", "Mechanical", "Civil", "Information Tech", "Management"});
    if (isEditing) deptCombo->setCurrentText(QString::fromStdString(existing->getDepartment()));

    auto* semSpin = new QSpinBox(&dlg);
    semSpin->setRange(1, 8);
    if (isEditing) semSpin->setValue(existing->getSemester());
    else semSpin->setValue(1);

    auto* phoneEdit = new QLineEdit(&dlg);
    phoneEdit->setPlaceholderText("10-digit mobile number");
    if (isEditing) phoneEdit->setText(QString::fromStdString(existing->getPhone()));

    auto* emailEdit = new QLineEdit(&dlg);
    emailEdit->setPlaceholderText("student@college.edu");
    if (isEditing) emailEdit->setText(QString::fromStdString(existing->getEmail()));

    auto* addrEdit = new QLineEdit(&dlg);
    addrEdit->setPlaceholderText("Residential Address");
    if (isEditing) addrEdit->setText(QString::fromStdString(existing->getAddress()));

    // Bus selector
    auto* busCombo = new QComboBox(&dlg);
    busCombo->addItem("None");
    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        QString itemText = QString::fromStdString(b.getBusId()) + " (" + QString::number(b.getAvailableSeats()) + " seats avail)";
        busCombo->addItem(itemText, QString::fromStdString(b.getBusId()));
    }
    if (isEditing && existing->getBusNumber() != "None") {
        for (int i = 0; i < busCombo->count(); ++i) {
            if (busCombo->itemData(i).toString().toStdString() == existing->getBusNumber()) {
                busCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    auto* pickupEdit = new QLineEdit(&dlg);
    pickupEdit->setPlaceholderText("e.g. Sector 14 / Metro Gate");
    if (isEditing) pickupEdit->setText(QString::fromStdString(existing->getPickupPoint()));

    auto* seatSpin = new QSpinBox(&dlg);
    seatSpin->setRange(0, 60);
    seatSpin->setSpecialValueText("Auto / None");
    if (isEditing) seatSpin->setValue(existing->getSeatNumber());

    formLayout->addRow("Student ID *:", idEdit);
    formLayout->addRow("Full Name *:", nameEdit);
    formLayout->addRow("Department:", deptCombo);
    formLayout->addRow("Semester:", semSpin);
    formLayout->addRow("Phone Number *:", phoneEdit);
    formLayout->addRow("Email Address:", emailEdit);
    formLayout->addRow("Home Address:", addrEdit);
    formLayout->addRow("Assigned Bus:", busCombo);
    formLayout->addRow("Pickup Point:", pickupEdit);
    formLayout->addRow("Seat Number:", seatSpin);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton(isEditing ? "Save Changes" : "Add Student", &dlg);
    saveBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString id = idEdit->text().trimmed();
        QString name = nameEdit->text().trimmed();
        QString phone = phoneEdit->text().trimmed();

        if (id.isEmpty() || name.isEmpty() || phone.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation Error", "Please fill in all mandatory fields (ID, Name, Phone).");
            return;
        }

        std::string selectedBus = busCombo->currentData().toString().toStdString();
        if (selectedBus.empty()) selectedBus = "None";

        std::string pickup = pickupEdit->text().trimmed().toStdString();
        if (pickup.empty()) pickup = "None";

        Student s(id.toStdString(), name.toStdString(),
                  deptCombo->currentText().toStdString(),
                  semSpin->value(), phone.toStdString(),
                  addrEdit->text().trimmed().toStdString(),
                  selectedBus, pickup, seatSpin->value(),
                  emailEdit->text().trimmed().toStdString());

        try {
            if (isEditing) {
                fileManager->updateStudent(s);
                QMessageBox::information(&dlg, "Success", "Student updated successfully!");
            } else {
                fileManager->addStudent(s);
                QMessageBox::information(&dlg, "Success", "Student added successfully!");
            }
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Operation Failed", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}
