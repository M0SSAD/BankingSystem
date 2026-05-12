#pragma once

#include <QPair>
#include <QWidget>
#include "AccountTableModel.h"
#include "AuditLogModel.h"

class SessionManager;
class QLineEdit;
class QLabel;
class QTableView;
class QListWidget;
class QStackedWidget;
class QPushButton;

class AdminDashboard : public QWidget {
	Q_OBJECT
public:
	explicit AdminDashboard(SessionManager *session,
	                        QWidget *parent = nullptr);

public slots:
	void refreshData();
	void onAccountsReady(const QVector<AccountDisplay> &accounts);
	void onCustomersReady(const QVector<CustomerDisplay> &customers);
	void onTransactionsReady(
	    const QVector<TransactionDisplay> &transactions);
	void onOperationResult(const QString &message, bool success);
	void onSearchResultsReady(
	    const QVector<QPair<uint64_t, QString>> &results);

signals:
	void requestAllData();
	void requestRegisterCustomer(const QString &name,
	                             uint64_t nationalId);
	void requestOpenAccount(uint64_t customerId);
	void requestFreezeAccount(uint64_t accountId);
	void requestUnfreezeAccount(uint64_t accountId);
	void requestCloseAccount(uint64_t accountId);
	void requestSearchCustomer(const QString &prefix);

private:
	SessionManager *m_session;
	AccountTableModel *m_accountModel;
	CustomerTableModel *m_customerModel;
	AuditLogModel *m_auditModel;
	QStackedWidget *m_contentStack;
	QPushButton *m_sidebarBtns[4];

	QLabel *m_statCustomers;
	QLabel *m_statAccounts;
	QLabel *m_statActive;
	QLabel *m_statBalance;

	QLineEdit *m_regNameEdit;
	QLineEdit *m_regNationalIdEdit;
	QTableView *m_customerTable;
	QLineEdit *m_searchEdit;
	QListWidget *m_searchResults;
	QLabel *m_searchStatusLabel;

	QLineEdit *m_openAcctCustIdEdit;
	QLineEdit *m_acctMgmtIdEdit;
	QTableView *m_accountTable;

	QTableView *m_ledgerTable;

	void setupUI();
	QWidget *createOverviewPage();
	QWidget *createCustomersPage();
	QWidget *createAccountsPage();
	QWidget *createLedgerPage();
	void switchPage(int index);
	void onRegisterCustomer();
	void onOpenAccount();
	void onFreezeAccount();
	void onUnfreezeAccount();
	void onCloseAccount();
	void onSearchCustomer();
	void onLogout();
};
