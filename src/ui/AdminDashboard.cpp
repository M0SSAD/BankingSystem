#include "AdminDashboard.h"
#include "AccountTableModel.h"
#include "AuditLogModel.h"
#include "SessionManager.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>

AdminDashboard::AdminDashboard(SessionManager *session,
                               QWidget *parent)
    : QWidget(parent), m_session(session),
      m_accountModel(new AccountTableModel(this)),
      m_customerModel(new CustomerTableModel(this)),
      m_auditModel(new AuditLogModel(this)) {
	setupUI();
}

void AdminDashboard::setupUI() {
	auto *mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	auto *sidebar = new QFrame;
	sidebar->setObjectName("sidebar");
	sidebar->setFixedWidth(220);
	auto *sideLayout = new QVBoxLayout(sidebar);
	sideLayout->setContentsMargins(16, 24, 16, 16);
	sideLayout->setSpacing(0);

	auto *brandLabel =
	    new QLabel(QStringLiteral("BANKING SYSTEM"));
	brandLabel->setObjectName("sidebarBrand");

	auto *roleLabel =
	    new QLabel(QStringLiteral("Admin Panel"));
	roleLabel->setObjectName("sidebarRole");

	sideLayout->addWidget(brandLabel);
	sideLayout->addSpacing(4);
	sideLayout->addWidget(roleLabel);
	sideLayout->addSpacing(32);

	QStringList pageNames = {
	    QStringLiteral("Overview"),
	    QStringLiteral("Customers"),
	    QStringLiteral("Accounts"),
	    QStringLiteral("Ledger"),
	};

	for (int i = 0; i < 4; ++i) {
		m_sidebarBtns[i] = new QPushButton(pageNames[i]);
		m_sidebarBtns[i]->setObjectName("sidebarBtn");
		m_sidebarBtns[i]->setCheckable(true);
		m_sidebarBtns[i]->setFixedHeight(40);
		m_sidebarBtns[i]->setCursor(Qt::PointingHandCursor);
		sideLayout->addWidget(m_sidebarBtns[i]);
		connect(m_sidebarBtns[i], &QPushButton::clicked, this,
		        [this, i]() { switchPage(i); });
	}
	m_sidebarBtns[0]->setChecked(true);

	sideLayout->addStretch();

	auto *refreshBtn =
	    new QPushButton(QStringLiteral("Refresh"));
	refreshBtn->setObjectName("sidebarActionBtn");
	refreshBtn->setFixedHeight(36);
	refreshBtn->setCursor(Qt::PointingHandCursor);

	auto *logoutBtn =
	    new QPushButton(QStringLiteral("Logout"));
	logoutBtn->setObjectName("sidebarLogoutBtn");
	logoutBtn->setFixedHeight(36);
	logoutBtn->setCursor(Qt::PointingHandCursor);

	sideLayout->addWidget(refreshBtn);
	sideLayout->addSpacing(8);
	sideLayout->addWidget(logoutBtn);

	connect(refreshBtn, &QPushButton::clicked, this,
	        &AdminDashboard::refreshData);
	connect(logoutBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onLogout);

	m_contentStack = new QStackedWidget;
	m_contentStack->addWidget(createOverviewPage());
	m_contentStack->addWidget(createCustomersPage());
	m_contentStack->addWidget(createAccountsPage());
	m_contentStack->addWidget(createLedgerPage());

	mainLayout->addWidget(sidebar);
	mainLayout->addWidget(m_contentStack, 1);
}

QWidget *AdminDashboard::createOverviewPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("Overview"));
	pageTitle->setObjectName("pageTitle");

	layout->addWidget(pageTitle);

	auto *cardsRow = new QHBoxLayout;
	cardsRow->setSpacing(16);

	auto *card1 = new QFrame;
	card1->setObjectName("statCard");
	auto *c1Layout = new QVBoxLayout(card1);
	auto *c1Title =
	    new QLabel(QStringLiteral("Total Customers"));
	c1Title->setObjectName("statLabel");
	m_statCustomers =
	    new QLabel(QStringLiteral("0"));
	m_statCustomers->setObjectName("statValue");
	c1Layout->addWidget(c1Title);
	c1Layout->addWidget(m_statCustomers);

	auto *card2 = new QFrame;
	card2->setObjectName("statCard");
	auto *c2Layout = new QVBoxLayout(card2);
	auto *c2Title =
	    new QLabel(QStringLiteral("Total Accounts"));
	c2Title->setObjectName("statLabel");
	m_statAccounts =
	    new QLabel(QStringLiteral("0"));
	m_statAccounts->setObjectName("statValue");
	c2Layout->addWidget(c2Title);
	c2Layout->addWidget(m_statAccounts);

	auto *card3 = new QFrame;
	card3->setObjectName("statCard");
	auto *c3Layout = new QVBoxLayout(card3);
	auto *c3Title =
	    new QLabel(QStringLiteral("Active Accounts"));
	c3Title->setObjectName("statLabel");
	m_statActive =
	    new QLabel(QStringLiteral("0"));
	m_statActive->setObjectName("statValue");
	c3Layout->addWidget(c3Title);
	c3Layout->addWidget(m_statActive);

	auto *card4 = new QFrame;
	card4->setObjectName("statCard");
	auto *c4Layout = new QVBoxLayout(card4);
	auto *c4Title =
	    new QLabel(QStringLiteral("Total Deposits"));
	c4Title->setObjectName("statLabel");
	m_statBalance =
	    new QLabel(QStringLiteral("EGP 0"));
	m_statBalance->setObjectName("statValue");
	c4Layout->addWidget(c4Title);
	c4Layout->addWidget(m_statBalance);

	cardsRow->addWidget(card1);
	cardsRow->addWidget(card2);
	cardsRow->addWidget(card3);
	cardsRow->addWidget(card4);

	layout->addLayout(cardsRow);

	auto *recentTitle =
	    new QLabel(QStringLiteral("Recent Transactions"));
	recentTitle->setObjectName("sectionTitle");
	layout->addWidget(recentTitle);

	m_ledgerTable = new QTableView;
	m_ledgerTable->setModel(m_auditModel);
	m_ledgerTable->setAlternatingRowColors(true);
	m_ledgerTable->setSelectionBehavior(
	    QAbstractItemView::SelectRows);
	m_ledgerTable->setSelectionMode(
	    QAbstractItemView::SingleSelection);
	m_ledgerTable->verticalHeader()->setVisible(false);
	m_ledgerTable->setSortingEnabled(true);
	m_ledgerTable->horizontalHeader()->setStretchLastSection(
	    true);
	m_ledgerTable->verticalHeader()->setDefaultSectionSize(36);

	layout->addWidget(m_ledgerTable, 1);

	return page;
}

QWidget *AdminDashboard::createCustomersPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("Customer Management"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	auto *topRow = new QHBoxLayout;
	topRow->setSpacing(20);

	auto *regCard = new QFrame;
	regCard->setObjectName("formCard");
	auto *regLayout = new QVBoxLayout(regCard);
	regLayout->setContentsMargins(20, 20, 20, 20);

	auto *regTitle =
	    new QLabel(QStringLiteral("Register New Customer"));
	regTitle->setObjectName("formTitle");
	regLayout->addWidget(regTitle);
	regLayout->addSpacing(12);

	auto *nameLabel =
	    new QLabel(QStringLiteral("Customer Name"));
	nameLabel->setObjectName("fieldLabel");
	m_regNameEdit = new QLineEdit;
	m_regNameEdit->setPlaceholderText(
	    QStringLiteral("Full name..."));
	m_regNameEdit->setFixedHeight(38);

	auto *natIdLabel =
	    new QLabel(QStringLiteral("National ID"));
	natIdLabel->setObjectName("fieldLabel");
	m_regNationalIdEdit = new QLineEdit;
	m_regNationalIdEdit->setPlaceholderText(
	    QStringLiteral("National ID number..."));
	m_regNationalIdEdit->setFixedHeight(38);

	auto *regBtn =
	    new QPushButton(QStringLiteral("Register Customer"));
	regBtn->setObjectName("primaryBtn");
	regBtn->setFixedHeight(38);
	regBtn->setCursor(Qt::PointingHandCursor);

	regLayout->addWidget(nameLabel);
	regLayout->addWidget(m_regNameEdit);
	regLayout->addSpacing(8);
	regLayout->addWidget(natIdLabel);
	regLayout->addWidget(m_regNationalIdEdit);
	regLayout->addSpacing(12);
	regLayout->addWidget(regBtn);
	regLayout->addStretch();

	connect(regBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onRegisterCustomer);

	auto *searchCard = new QFrame;
	searchCard->setObjectName("formCard");
	auto *searchLayout = new QVBoxLayout(searchCard);
	searchLayout->setContentsMargins(20, 20, 20, 20);

	auto *searchTitle =
	    new QLabel(QStringLiteral("Search Customers"));
	searchTitle->setObjectName("formTitle");
	searchLayout->addWidget(searchTitle);
	searchLayout->addSpacing(12);

	auto *searchFieldLabel =
	    new QLabel(QStringLiteral("Search by Name Prefix"));
	searchFieldLabel->setObjectName("fieldLabel");

	auto *searchInputRow = new QHBoxLayout;
	searchInputRow->setSpacing(8);
	m_searchEdit = new QLineEdit;
	m_searchEdit->setPlaceholderText(
	    QStringLiteral("Type a name prefix..."));
	m_searchEdit->setFixedHeight(38);
	auto *searchBtn =
	    new QPushButton(QStringLiteral("Search"));
	searchBtn->setObjectName("primaryBtn");
	searchBtn->setFixedSize(90, 38);
	searchBtn->setCursor(Qt::PointingHandCursor);
	searchInputRow->addWidget(m_searchEdit);
	searchInputRow->addWidget(searchBtn);

	m_searchResults = new QListWidget;
	m_searchResults->setMaximumHeight(120);
	m_searchResults->setAlternatingRowColors(true);

	m_searchStatusLabel =
	    new QLabel(QStringLiteral("Enter a name to search"));
	m_searchStatusLabel->setObjectName("hintLabel");

	searchLayout->addWidget(searchFieldLabel);
	searchLayout->addLayout(searchInputRow);
	searchLayout->addSpacing(6);
	searchLayout->addWidget(m_searchResults);
	searchLayout->addWidget(m_searchStatusLabel);
	searchLayout->addStretch();

	connect(searchBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onSearchCustomer);
	connect(m_searchEdit, &QLineEdit::returnPressed, this,
	        &AdminDashboard::onSearchCustomer);
	connect(m_searchResults, &QListWidget::itemDoubleClicked,
	        this, [this](QListWidgetItem *item) {
		        m_openAcctCustIdEdit->setText(
		            QString::number(
		                item->data(Qt::UserRole).toULongLong()));
		        switchPage(2);
	        });

	topRow->addWidget(regCard, 1);
	topRow->addWidget(searchCard, 1);

	layout->addLayout(topRow);

	auto *tableTitle =
	    new QLabel(QStringLiteral("All Customers"));
	tableTitle->setObjectName("sectionTitle");
	layout->addWidget(tableTitle);

	m_customerTable = new QTableView;
	m_customerTable->setModel(m_customerModel);
	m_customerTable->setAlternatingRowColors(true);
	m_customerTable->setSelectionBehavior(
	    QAbstractItemView::SelectRows);
	m_customerTable->setSelectionMode(
	    QAbstractItemView::SingleSelection);
	m_customerTable->verticalHeader()->setVisible(false);
	m_customerTable->setSortingEnabled(true);
	m_customerTable->horizontalHeader()->setStretchLastSection(
	    true);
	m_customerTable->verticalHeader()->setDefaultSectionSize(36);

	layout->addWidget(m_customerTable, 1);

	return page;
}

QWidget *AdminDashboard::createAccountsPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("Account Management"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	auto *topRow = new QHBoxLayout;
	topRow->setSpacing(20);

	auto *openCard = new QFrame;
	openCard->setObjectName("formCard");
	auto *openLayout = new QVBoxLayout(openCard);
	openLayout->setContentsMargins(20, 20, 20, 20);

	auto *openTitle =
	    new QLabel(QStringLiteral("Open New Account"));
	openTitle->setObjectName("formTitle");
	openLayout->addWidget(openTitle);
	openLayout->addSpacing(12);

	auto *custIdLabel =
	    new QLabel(QStringLiteral("Customer ID"));
	custIdLabel->setObjectName("fieldLabel");
	m_openAcctCustIdEdit = new QLineEdit;
	m_openAcctCustIdEdit->setPlaceholderText(
	    QStringLiteral("Customer ID..."));
	m_openAcctCustIdEdit->setFixedHeight(38);

	auto *openBtn =
	    new QPushButton(QStringLiteral("Open Account"));
	openBtn->setObjectName("successBtn");
	openBtn->setFixedHeight(38);
	openBtn->setCursor(Qt::PointingHandCursor);

	openLayout->addWidget(custIdLabel);
	openLayout->addWidget(m_openAcctCustIdEdit);
	openLayout->addSpacing(12);
	openLayout->addWidget(openBtn);
	openLayout->addStretch();

	connect(openBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onOpenAccount);

	auto *mgmtCard = new QFrame;
	mgmtCard->setObjectName("formCard");
	auto *mgmtLayout = new QVBoxLayout(mgmtCard);
	mgmtLayout->setContentsMargins(20, 20, 20, 20);

	auto *mgmtTitle =
	    new QLabel(QStringLiteral("Account Operations"));
	mgmtTitle->setObjectName("formTitle");
	mgmtLayout->addWidget(mgmtTitle);
	mgmtLayout->addSpacing(12);

	auto *acctIdLabel =
	    new QLabel(QStringLiteral("Account ID"));
	acctIdLabel->setObjectName("fieldLabel");
	m_acctMgmtIdEdit = new QLineEdit;
	m_acctMgmtIdEdit->setPlaceholderText(
	    QStringLiteral("Enter account ID..."));
	m_acctMgmtIdEdit->setFixedHeight(38);

	auto *btnRow = new QHBoxLayout;
	btnRow->setSpacing(8);

	auto *freezeBtn =
	    new QPushButton(QStringLiteral("Freeze"));
	freezeBtn->setObjectName("dangerBtn");
	freezeBtn->setFixedHeight(36);
	freezeBtn->setCursor(Qt::PointingHandCursor);

	auto *unfreezeBtn =
	    new QPushButton(QStringLiteral("Unfreeze"));
	unfreezeBtn->setObjectName("successBtn");
	unfreezeBtn->setFixedHeight(36);
	unfreezeBtn->setCursor(Qt::PointingHandCursor);

	auto *closeBtn =
	    new QPushButton(QStringLiteral("Close"));
	closeBtn->setObjectName("dangerBtn");
	closeBtn->setFixedHeight(36);
	closeBtn->setCursor(Qt::PointingHandCursor);

	btnRow->addWidget(freezeBtn);
	btnRow->addWidget(unfreezeBtn);
	btnRow->addWidget(closeBtn);

	mgmtLayout->addWidget(acctIdLabel);
	mgmtLayout->addWidget(m_acctMgmtIdEdit);
	mgmtLayout->addSpacing(8);
	mgmtLayout->addLayout(btnRow);
	mgmtLayout->addStretch();

	connect(freezeBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onFreezeAccount);
	connect(unfreezeBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onUnfreezeAccount);
	connect(closeBtn, &QPushButton::clicked, this,
	        &AdminDashboard::onCloseAccount);

	topRow->addWidget(openCard, 1);
	topRow->addWidget(mgmtCard, 1);

	layout->addLayout(topRow);

	auto *tableTitle =
	    new QLabel(QStringLiteral("All Accounts"));
	tableTitle->setObjectName("sectionTitle");
	layout->addWidget(tableTitle);

	m_accountTable = new QTableView;
	m_accountTable->setModel(m_accountModel);
	m_accountTable->setAlternatingRowColors(true);
	m_accountTable->setSelectionBehavior(
	    QAbstractItemView::SelectRows);
	m_accountTable->setSelectionMode(
	    QAbstractItemView::SingleSelection);
	m_accountTable->verticalHeader()->setVisible(false);
	m_accountTable->setSortingEnabled(true);
	m_accountTable->horizontalHeader()->setStretchLastSection(
	    true);
	m_accountTable->verticalHeader()->setDefaultSectionSize(36);

	layout->addWidget(m_accountTable, 1);

	return page;
}

QWidget *AdminDashboard::createLedgerPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("Transaction Ledger"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	auto *table = new QTableView;
	table->setModel(m_auditModel);
	table->setAlternatingRowColors(true);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->verticalHeader()->setVisible(false);
	table->setSortingEnabled(true);
	table->horizontalHeader()->setStretchLastSection(true);
	table->verticalHeader()->setDefaultSectionSize(36);

	layout->addWidget(table, 1);

	return page;
}

void AdminDashboard::switchPage(int index) {
	for (int i = 0; i < 4; ++i)
		m_sidebarBtns[i]->setChecked(i == index);
	m_contentStack->setCurrentIndex(index);
}

void AdminDashboard::refreshData() { emit requestAllData(); }

void AdminDashboard::onAccountsReady(
    const QVector<AccountDisplay> &accounts) {
	if (!isVisible())
		return;
	m_accountModel->setAccounts(accounts);

	int active = 0;
	int64_t totalBalance = 0;
	for (const auto &acc : accounts) {
		if (acc.state == AccountState::ACTIVE) {
			++active;
			totalBalance += acc.balance;
		}
	}

	static const QLocale loc(QLocale::English);
	m_statAccounts->setText(QString::number(accounts.size()));
	m_statActive->setText(QString::number(active));
	m_statBalance->setText(
	    QStringLiteral("EGP %1")
	        .arg(loc.toString(static_cast<qlonglong>(totalBalance))));
}

void AdminDashboard::onCustomersReady(
    const QVector<CustomerDisplay> &customers) {
	if (!isVisible())
		return;
	m_customerModel->setCustomers(customers);
	m_statCustomers->setText(QString::number(customers.size()));
}

void AdminDashboard::onTransactionsReady(
    const QVector<TransactionDisplay> &transactions) {
	if (!isVisible())
		return;
	m_auditModel->setTransactions(transactions);
}

void AdminDashboard::onOperationResult(const QString &message,
                                       bool success) {
	if (!isVisible())
		return;
	QMessageBox::information(
	    this,
	    success ? QStringLiteral("Success")
	            : QStringLiteral("Notice"),
	    message);
	if (success) {
		emit requestAllData();
	}
}

void AdminDashboard::onSearchResultsReady(
    const QVector<QPair<uint64_t, QString>> &results) {
	if (!isVisible())
		return;
	m_searchResults->clear();
	if (results.isEmpty()) {
		m_searchStatusLabel->setText(
		    QStringLiteral("No customers found"));
		return;
	}
	for (const auto &[cid, name] : results) {
		auto *item = new QListWidgetItem(
		    QStringLiteral("ID: %1  -  %2")
		        .arg(cid)
		        .arg(name));
		item->setData(Qt::UserRole, QVariant::fromValue(cid));
		m_searchResults->addItem(item);
	}
	m_searchStatusLabel->setText(
	    QStringLiteral("Found %1 customer(s)")
	        .arg(results.size()));
}

void AdminDashboard::onRegisterCustomer() {
	QString name = m_regNameEdit->text().trimmed();
	bool ok = false;
	uint64_t natId =
	    m_regNationalIdEdit->text().toULongLong(&ok);

	if (name.isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Name cannot be empty"));
		return;
	}
	if (!ok || m_regNationalIdEdit->text().isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Invalid National ID"));
		return;
	}

	emit requestRegisterCustomer(name, natId);
}

void AdminDashboard::onOpenAccount() {
	bool ok = false;
	uint64_t custId =
	    m_openAcctCustIdEdit->text().toULongLong(&ok);

	if (!ok || m_openAcctCustIdEdit->text().isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Invalid Customer ID"));
		return;
	}

	emit requestOpenAccount(custId);
}

void AdminDashboard::onFreezeAccount() {
	bool ok = false;
	uint64_t accountId =
	    m_acctMgmtIdEdit->text().toULongLong(&ok);

	if (!ok || m_acctMgmtIdEdit->text().isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Enter a valid Account ID"));
		return;
	}

	emit requestFreezeAccount(accountId);
}

void AdminDashboard::onUnfreezeAccount() {
	bool ok = false;
	uint64_t accountId =
	    m_acctMgmtIdEdit->text().toULongLong(&ok);

	if (!ok || m_acctMgmtIdEdit->text().isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Enter a valid Account ID"));
		return;
	}

	emit requestUnfreezeAccount(accountId);
}

void AdminDashboard::onCloseAccount() {
	bool ok = false;
	uint64_t accountId =
	    m_acctMgmtIdEdit->text().toULongLong(&ok);

	if (!ok || m_acctMgmtIdEdit->text().isEmpty()) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Enter a valid Account ID"));
		return;
	}

	emit requestCloseAccount(accountId);
}

void AdminDashboard::onSearchCustomer() {
	QString prefix = m_searchEdit->text().trimmed();
	if (prefix.isEmpty()) {
		m_searchStatusLabel->setText(
		    QStringLiteral("Enter a name prefix to search"));
		return;
	}

	m_searchResults->clear();
	m_searchStatusLabel->setText(
	    QStringLiteral("Searching..."));
	emit requestSearchCustomer(prefix);
}

void AdminDashboard::onLogout() { m_session->logout(); }
