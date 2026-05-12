#pragma once

#include <QWidget>
#include "AccountTableModel.h"
#include "AuditLogModel.h"

class SessionManager;
class QComboBox;
class QLineEdit;
class QLabel;
class QTableView;
class QStackedWidget;
class QPushButton;

class CustomerDashboard : public QWidget {
	Q_OBJECT
public:
	explicit CustomerDashboard(SessionManager *session,
	                           QWidget *parent = nullptr);

public slots:
	void refreshData();
	void onAccountsReady(const QVector<AccountDisplay> &accounts);
	void onTransactionsReady(
	    const QVector<TransactionDisplay> &transactions);
	void onOperationResult(const QString &message, bool success);

signals:
	void requestCustomerData(uint64_t customerId);
	void requestDeposit(uint64_t accountId, int64_t amount);
	void requestWithdraw(uint64_t accountId, int64_t amount);
	void requestTransfer(uint64_t from, uint64_t to, int64_t amount);

private:
	SessionManager *m_session;
	AccountTableModel *m_accountModel;
	AuditLogModel *m_auditModel;
	QStackedWidget *m_contentStack;
	QPushButton *m_sidebarBtns[3];

	QLabel *m_welcomeLabel;
	QLabel *m_totalBalanceLabel;
	QTableView *m_accountTable;

	QComboBox *m_txTypeCombo;
	QComboBox *m_fromAccountCombo;
	QLineEdit *m_amountEdit;
	QLineEdit *m_destAccountEdit;
	QLabel *m_destLabel;

	QTableView *m_historyTable;

	void setupUI();
	QWidget *createDashboardPage();
	QWidget *createTransactPage();
	QWidget *createHistoryPage();
	void switchPage(int index);
	void onExecuteTransaction();
	void onTransactionTypeChanged(int index);
	void onLogout();
	void populateAccountCombo();
};
