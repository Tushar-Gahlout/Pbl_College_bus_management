#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>

LoginDialog::LoginDialog(FileManager* fm, QWidget* parent)
    : QDialog(parent), fileManager(fm) {
    setWindowTitle("College Bus Management System - Login");
    setFixedSize(450, 480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUI();
}

void LoginDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(16);

    // Header with Logo / Icon
    auto* titleLabel = new QLabel("🚌 College Bus Management", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #78350f;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* subLabel = new QLabel("Sign in to your account", this);
    subLabel->setStyleSheet("font-size: 13px; color: #78716c; margin-bottom: 10px; font-weight: 500;");
    subLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subLabel);

    // Quick Login Demo Selector (Convenient for PBL project evaluation!)
    auto* demoBox = new QGroupBox("Quick Demo Login (Evaluation Helper)", this);
    demoBox->setStyleSheet("QGroupBox { font-weight: bold; color: #b45309; border: 1.5px dashed #f59e0b; border-radius: 6px; margin-top: 10px; padding: 10px; background-color: #fffbeb; }");
    auto* demoLayout = new QVBoxLayout(demoBox);

    roleHintCombo = new QComboBox(demoBox);
    roleHintCombo->addItem("-- Select Sample Account --");
    roleHintCombo->addItem("Admin (admin / admin123)");
    roleHintCombo->addItem("Student 1 - Rahul (student1 / pass123)");
    roleHintCombo->addItem("Student 2 - Priya (student2 / pass123)");
    roleHintCombo->addItem("Driver 1 - Rajesh (driver1 / pass123)");
    roleHintCombo->addItem("Driver 2 - Vikram (driver2 / pass123)");
    roleHintCombo->addItem("Conductor 1 - Ramesh (conductor1 / pass123)");
    roleHintCombo->addItem("Conductor 2 - Sunil (conductor2 / pass123)");
    demoLayout->addWidget(roleHintCombo);
    mainLayout->addWidget(demoBox);

    connect(roleHintCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginDialog::handleQuickFill);

    // Form inputs
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Enter username");
    usernameEdit->setText("admin"); // Default to admin for fast testing
    formLayout->addRow("Username:", usernameEdit);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Enter password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setText("admin123");
    formLayout->addRow("Password:", passwordEdit);

    mainLayout->addLayout(formLayout);

    errorLabel = new QLabel("", this);
    errorLabel->setStyleSheet("color: #dc2626; font-size: 12px; font-weight: bold;");
    errorLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(errorLabel);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    loginBtn = new QPushButton("Login", this);
    loginBtn->setProperty("class", "primaryButton");
    loginBtn->setMinimumHeight(38);
    loginBtn->setCursor(Qt::PointingHandCursor);

    exitBtn = new QPushButton("Exit", this);
    exitBtn->setProperty("class", "secondaryButton");
    exitBtn->setMinimumHeight(38);
    exitBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::handleLogin);
    connect(exitBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::handleLogin);
}

void LoginDialog::handleQuickFill(int index) {
    if (index == 1) { // Admin
        usernameEdit->setText("admin");
        passwordEdit->setText("admin123");
    } else if (index == 2) { // Student 1
        usernameEdit->setText("student1");
        passwordEdit->setText("pass123");
    } else if (index == 3) { // Student 2
        usernameEdit->setText("student2");
        passwordEdit->setText("pass123");
    } else if (index == 4) { // Driver 1
        usernameEdit->setText("driver1");
        passwordEdit->setText("pass123");
    } else if (index == 5) { // Driver 2
        usernameEdit->setText("driver2");
        passwordEdit->setText("pass123");
    } else if (index == 6) { // Conductor 1
        usernameEdit->setText("conductor1");
        passwordEdit->setText("pass123");
    } else if (index == 7) { // Conductor 2
        usernameEdit->setText("conductor2");
        passwordEdit->setText("pass123");
    }
}

void LoginDialog::handleLogin() {
    std::string userStr = usernameEdit->text().trimmed().toStdString();
    std::string passStr = passwordEdit->text().toStdString();

    if (userStr.empty() || passStr.empty()) {
        errorLabel->setText("Please enter both username and password.");
        return;
    }

    User u = fileManager->authenticate(userStr, passStr);
    if (!u.getUsername().empty()) {
        authenticatedUser = u;
        accept();
    } else {
        errorLabel->setText("Invalid username or password! Try sample accounts above.");
        QMessageBox::warning(this, "Login Failed", 
            "Authentication failed. Please check your credentials or pick an account from the demo selector.");
    }
}
