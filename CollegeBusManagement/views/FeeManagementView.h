#ifndef FEEMANAGEMENTVIEW_H
#define FEEMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../services/FileManager.h"

/**
 * @brief Fee Management Module for handling student bus transportation fees and receipts.
 */
class FeeManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLineEdit* searchEdit;
    QLabel* totalRevenueLbl;
    QLabel* totalCollectedLbl;
    QLabel* totalPendingLbl;

    QTableWidget* feeTable;

    QPushButton* recordPaymentBtn;
    QPushButton* viewReceiptBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showPaymentDialog();
    void showReceiptDialog(const FeeRecord& fee);

private slots:
    void handleSearch();
    void handleRecordPayment();
    void handleViewReceipt();

public slots:
    void refreshData();

public:
    explicit FeeManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~FeeManagementView() override = default;
};

#endif // FEEMANAGEMENTVIEW_H
