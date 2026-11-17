#ifndef STUDENTMANAGEMENTVIEW_H
#define STUDENTMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "../services/FileManager.h"

/**
 * @brief Student Management Module for adding, editing, deleting, and searching students.
 */
class StudentManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLineEdit* searchEdit;
    QComboBox* deptFilterCombo;
    QTableWidget* studentTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* refreshBtn;
    QPushButton* assignBusBtn;
    QPushButton* bstViewBtn;
    QPushButton* hashLookupBtn;

    void setupUI();
    void showStudentFormDialog(Student* existing = nullptr);

private slots:
    void handleSearch();
    void handleAdd();
    void handleEdit();
    void handleDelete();
    void handleAssignBus();
    void handleViewBST();
    void handleHashLookup();

public slots:
    void refreshData();

public:
    explicit StudentManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~StudentManagementView() override = default;
};

#endif // STUDENTMANAGEMENTVIEW_H
