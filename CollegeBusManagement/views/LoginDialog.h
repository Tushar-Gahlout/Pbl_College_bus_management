#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include "../services/FileManager.h"
#include "../models/User.h"

/**
 * @brief Login Dialog providing role-based authentication.
 */
class LoginDialog : public QDialog {
    Q_OBJECT

private:
    FileManager* fileManager;
    User authenticatedUser;

    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QComboBox* roleHintCombo;
    QPushButton* loginBtn;
    QPushButton* exitBtn;
    QLabel* errorLabel;

    void setupUI();

private slots:
    void handleLogin();
    void handleQuickFill(int index);

public:
    explicit LoginDialog(FileManager* fm, QWidget* parent = nullptr);
    ~LoginDialog() override = default;

    User getAuthenticatedUser() const { return authenticatedUser; }
};

#endif // LOGINDIALOG_H
