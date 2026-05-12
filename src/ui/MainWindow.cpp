#include "MainWindow.h"
#include "AdminDashboard.h"
#include "BackendWorker.h"
#include "CustomerDashboard.h"
#include "LoginScreen.h"
#include "SessionManager.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle(
	    QStringLiteral("BankingSystem - Professional Banking"));
	setMinimumSize(1100, 750);
	resize(1200, 800);

	m_session = new SessionManager(this);

	m_worker = new BackendWorker;
	m_workerThread = new QThread(this);
	m_worker->moveToThread(m_workerThread);
	connect(m_workerThread, &QThread::finished, m_worker,
	        &QObject::deleteLater);
	m_workerThread->start();

	m_loginScreen = new LoginScreen(m_session);
	m_adminDashboard = new AdminDashboard(m_session);
	m_customerDashboard = new CustomerDashboard(m_session);

	m_stack = new QStackedWidget;
	m_stack->addWidget(m_loginScreen);
	m_stack->addWidget(m_adminDashboard);
	m_stack->addWidget(m_customerDashboard);

	setCentralWidget(m_stack);

	connect(m_session, &SessionManager::stateChanged, this,
	        &MainWindow::onStateChanged);

	connect(m_loginScreen, &LoginScreen::loginAsAdmin,
	        m_session, &SessionManager::enterAdmin);
	connect(m_loginScreen, &LoginScreen::loginAsCustomer,
	        m_worker, &BackendWorker::onLookupCustomer);
	connect(m_worker, &BackendWorker::customerLookupResult,
	        this, &MainWindow::onCustomerLookupResult);

	connect(m_adminDashboard, &AdminDashboard::requestAllData,
	        m_worker, &BackendWorker::onRequestAllAccounts);
	connect(m_adminDashboard, &AdminDashboard::requestAllData,
	        m_worker, &BackendWorker::onRequestAllCustomers);
	connect(m_adminDashboard, &AdminDashboard::requestAllData,
	        m_worker, &BackendWorker::onRequestAllTransactions);
	connect(m_adminDashboard,
	        &AdminDashboard::requestRegisterCustomer, m_worker,
	        &BackendWorker::onRegisterCustomer);
	connect(m_adminDashboard,
	        &AdminDashboard::requestOpenAccount, m_worker,
	        &BackendWorker::onOpenAccount);
	connect(m_adminDashboard,
	        &AdminDashboard::requestFreezeAccount, m_worker,
	        &BackendWorker::onFreezeAccount);
	connect(m_adminDashboard,
	        &AdminDashboard::requestUnfreezeAccount, m_worker,
	        &BackendWorker::onUnfreezeAccount);
	connect(m_adminDashboard,
	        &AdminDashboard::requestCloseAccount, m_worker,
	        &BackendWorker::onCloseAccount);
	connect(m_adminDashboard,
	        &AdminDashboard::requestSearchCustomer, m_worker,
	        &BackendWorker::onSearchCustomer);

	connect(m_worker, &BackendWorker::accountsReady,
	        m_adminDashboard,
	        &AdminDashboard::onAccountsReady);
	connect(m_worker, &BackendWorker::customersReady,
	        m_adminDashboard,
	        &AdminDashboard::onCustomersReady);
	connect(m_worker, &BackendWorker::transactionsReady,
	        m_adminDashboard,
	        &AdminDashboard::onTransactionsReady);
	connect(m_worker, &BackendWorker::operationResult,
	        m_adminDashboard,
	        &AdminDashboard::onOperationResult);
	connect(m_worker, &BackendWorker::searchResultsReady,
	        m_adminDashboard,
	        &AdminDashboard::onSearchResultsReady);

	connect(m_customerDashboard,
	        &CustomerDashboard::requestCustomerData, m_worker,
	        &BackendWorker::onRequestCustomerAccounts);
	connect(m_customerDashboard,
	        &CustomerDashboard::requestCustomerData, m_worker,
	        &BackendWorker::onRequestCustomerTransactions);
	connect(m_customerDashboard,
	        &CustomerDashboard::requestDeposit, m_worker,
	        &BackendWorker::onDeposit);
	connect(m_customerDashboard,
	        &CustomerDashboard::requestWithdraw, m_worker,
	        &BackendWorker::onWithdraw);
	connect(m_customerDashboard,
	        &CustomerDashboard::requestTransfer, m_worker,
	        &BackendWorker::onTransfer);

	connect(m_worker, &BackendWorker::accountsReady,
	        m_customerDashboard,
	        &CustomerDashboard::onAccountsReady);
	connect(m_worker, &BackendWorker::transactionsReady,
	        m_customerDashboard,
	        &CustomerDashboard::onTransactionsReady);
	connect(m_worker, &BackendWorker::operationResult,
	        m_customerDashboard,
	        &CustomerDashboard::onOperationResult);
}

MainWindow::~MainWindow() {
	m_workerThread->quit();
	m_workerThread->wait();
}

void MainWindow::onStateChanged(SessionState newState) {
	switch (newState) {
	case SessionState::Login:
		m_stack->setCurrentWidget(m_loginScreen);
		break;
	case SessionState::Admin:
		m_stack->setCurrentWidget(m_adminDashboard);
		m_adminDashboard->refreshData();
		break;
	case SessionState::Customer:
		m_stack->setCurrentWidget(m_customerDashboard);
		m_customerDashboard->refreshData();
		break;
	}
}

void MainWindow::onCustomerLookupResult(
    bool found, uint64_t customerId, const QString &name,
    const QVector<uint64_t> &accountIds) {
	if (found) {
		m_session->enterCustomerSession(customerId, name,
		                                accountIds);
	} else {
		m_loginScreen->setLoginStatus(
		    QStringLiteral(
		        "Customer not found. Check your National ID."),
		    true);
	}
}
