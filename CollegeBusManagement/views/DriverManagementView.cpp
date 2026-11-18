#include "DriverManagementView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>

DriverManagementView::DriverManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void DriverManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Driver Management", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // Action Buttons Row
    auto* actionLayout = new QHBoxLayout();
    addBtn = new QPushButton("➕ Add Driver", this);
    addBtn->setProperty("class", "primaryButton");
    addBtn->setCursor(Qt::PointingHandCursor);

    editBtn = new QPushButton("✏️ Edit Driver", this);
    editBtn->setProperty("class", "secondaryButton");
    editBtn->setCursor(Qt::PointingHandCursor);

    deleteBtn = new QPushButton("🗑️ Delete Driver", this);
    deleteBtn->setProperty("class", "dangerButton");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    viewDetailsBtn = new QPushButton("ℹ️ View Profile", this);
    viewDetailsBtn->setProperty("class", "secondaryButton");
    viewDetailsBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    actionLayout->addWidget(addBtn);
    actionLayout->addWidget(editBtn);
    actionLayout->addWidget(deleteBtn);
    actionLayout->addWidget(viewDetailsBtn);
    actionLayout->addWidget(refreshBtn);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);

    // Search Bar
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Search driver by ID, Name, Phone, License, or Bus...");
    searchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(searchEdit);

    // Drivers Table
    driverTable = new QTableWidget(this);
    driverTable->setColumnCount(7);
    driverTable->setHorizontalHeaderLabels({
        "Driver ID", "Name", "Phone Number", "Driving License", "Assigned Bus", "Experience (Yrs)", "Email"
    });
    driverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    driverTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    driverTable->setSelectionMode(QAbstractItemView::SingleSelection);
    driverTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    driverTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(driverTable);

    // Connect signals
    connect(searchEdit, &QLineEdit::textChanged, this, &DriverManagementView::handleSearch);
    connect(addBtn, &QPushButton::clicked, this, &DriverManagementView::handleAdd);
    connect(editBtn, &QPushButton::clicked, this, &DriverManagementView::handleEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &DriverManagementView::handleDelete);
    connect(viewDetailsBtn, &QPushButton::clicked, this, &DriverManagementView::handleViewDetails);
    connect(refreshBtn, &QPushButton::clicked, this, &DriverManagementView::refreshData);
    connect(driverTable, &QTableWidget::doubleClicked, this, &DriverManagementView::handleViewDetails);
}

void DriverManagementView::refreshData() {
    handleSearch();
}

void DriverManagementView::handleSearch() {
    std::string query = searchEdit->text().trimmed().toStdString();
    std::vector<Driver> list = fileManager->searchDrivers(query);

    driverTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& d = list[i];
        driverTable->insertRow(i);

        driverTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(d.getId())));
        driverTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(d.getName())));
        driverTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(d.getPhone())));
        driverTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(d.getLicenseNumber())));
        driverTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(d.getAssignedBus())));
        driverTable->setItem(i, 5, new QTableWidgetItem(QString::number(d.getExperienceYears())));
        driverTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(d.getEmail())));
    }
}

void DriverManagementView::handleAdd() {
    showDriverFormDialog(nullptr);
}

void DriverManagementView::handleEdit() {
    int row = driverTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Driver", "Please select a driver from the table to edit.");
        return;
    }

    std::string dId = driverTable->item(row, 0)->text().toStdString();
    Driver d = fileManager->getDriverById(dId);
    if (!d.getId().empty()) {
        showDriverFormDialog(&d);
    }
}

void DriverManagementView::handleDelete() {
    int row = driverTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Driver", "Please select a driver to delete.");
        return;
    }

    QString dId = driverTable->item(row, 0)->text();
    QString name = driverTable->item(row, 1)->text();

    auto reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete Driver '%1' (%2)?").arg(name, dId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            fileManager->deleteDriver(dId.toStdString());
            QMessageBox::information(this, "Success", "Driver record deleted successfully.");
            refreshData();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete driver: %1").arg(e.what()));
        }
    }
}

void DriverManagementView::handleViewDetails() {
    int row = driverTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Driver", "Please select a driver to view profile details.");
        return;
    }

    std::string dId = driverTable->item(row, 0)->text().toStdString();
    Driver d = fileManager->getDriverById(dId);
    if (!d.getId().empty()) {
        showDriverDetailsDialog(d);
    }
}

void DriverManagementView::showDriverDetailsDialog(const Driver& driver) {
    QDialog dlg(this);
    dlg.setWindowTitle("Driver Profile Details");
    dlg.setFixedSize(450, 360);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel("🧑‍✈️ " + QString::fromStdString(driver.getName()), &dlg);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e3a8a;");
    layout->addWidget(title);

    auto* box = new QFrame(&dlg);
    box->setStyleSheet("background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 16px;");
    auto* formLayout = new QFormLayout(box);
    formLayout->setSpacing(10);

    formLayout->addRow("Driver ID:", new QLabel(QString::fromStdString(driver.getId()), box));
    formLayout->addRow("Role (Polymorphism):", new QLabel(QString::fromStdString(driver.getRole()), box));
    formLayout->addRow("Phone Number:", new QLabel(QString::fromStdString(driver.getPhone()), box));
    formLayout->addRow("License Number:", new QLabel(QString::fromStdString(driver.getLicenseNumber()), box));
    formLayout->addRow("Assigned Bus:", new QLabel(QString::fromStdString(driver.getAssignedBus()), box));
    formLayout->addRow("Driving Experience:", new QLabel(QString::number(driver.getExperienceYears()) + " years", box));
    formLayout->addRow("Email:", new QLabel(QString::fromStdString(driver.getEmail()), box));

    layout->addWidget(box);

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "primaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

void DriverManagementView::showDriverFormDialog(Driver* existing) {
    bool isEditing = (existing != nullptr);

    QDialog dlg(this);
    dlg.setWindowTitle(isEditing ? "Edit Driver Details" : "Add New Driver");
    dlg.setFixedSize(480, 450);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("e.g. DRV04");
    if (isEditing) {
        idEdit->setText(QString::fromStdString(existing->getId()));
        idEdit->setEnabled(false);
    }

    auto* nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("Full Driver Name");
    if (isEditing) nameEdit->setText(QString::fromStdString(existing->getName()));

    auto* phoneEdit = new QLineEdit(&dlg);
    phoneEdit->setPlaceholderText("10-digit mobile number");
    if (isEditing) phoneEdit->setText(QString::fromStdString(existing->getPhone()));

    auto* licenseEdit = new QLineEdit(&dlg);
    licenseEdit->setPlaceholderText("e.g. DL-2019-123456");
    if (isEditing) licenseEdit->setText(QString::fromStdString(existing->getLicenseNumber()));

    auto* busCombo = new QComboBox(&dlg);
    busCombo->addItem("None");
    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    if (isEditing) busCombo->setCurrentText(QString::fromStdString(existing->getAssignedBus()));

    auto* expSpin = new QSpinBox(&dlg);
    expSpin->setRange(0, 50);
    expSpin->setSuffix(" years");
    if (isEditing) expSpin->setValue(existing->getExperienceYears());
    else expSpin->setValue(3);

    auto* emailEdit = new QLineEdit(&dlg);
    emailEdit->setPlaceholderText("driver@college.edu");
    if (isEditing) emailEdit->setText(QString::fromStdString(existing->getEmail()));

    formLayout->addRow("Driver ID *:", idEdit);
    formLayout->addRow("Full Name *:", nameEdit);
    formLayout->addRow("Phone Number *:", phoneEdit);
    formLayout->addRow("Driving License *:", licenseEdit);
    formLayout->addRow("Assigned Bus:", busCombo);
    formLayout->addRow("Experience:", expSpin);
    formLayout->addRow("Email Address:", emailEdit);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton(isEditing ? "Save Changes" : "Add Driver", &dlg);
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
        QString lic = licenseEdit->text().trimmed();

        if (id.isEmpty() || name.isEmpty() || phone.isEmpty() || lic.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation Error", "Please fill in all mandatory fields (ID, Name, Phone, License).");
            return;
        }

        Driver d(id.toStdString(), name.toStdString(), phone.toStdString(),
                 lic.toStdString(), busCombo->currentText().toStdString(),
                 expSpin->value(), emailEdit->text().trimmed().toStdString());

        try {
            if (isEditing) {
                fileManager->updateDriver(d);
                QMessageBox::information(&dlg, "Success", "Driver updated successfully!");
            } else {
                fileManager->addDriver(d);
                QMessageBox::information(&dlg, "Success", "Driver added successfully!");
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
