#include "CustomerDashboard.h"
#include "AccountTableModel.h"
#include "AuditLogModel.h"
#include "SessionManager.h"
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>

CustomerDashboard::CustomerDashboard(SessionManager *session,
                                     QWidget *parent)
    : QWidget(parent), m_session(session),
      m_accountModel(new AccountTableModel(this)),
      m_auditModel(new AuditLogModel(this)) {
	setupUI();
}

void CustomerDashboard::setupUI() {
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

	m_welcomeLabel = new QLabel;
	m_welcomeLabel->setObjectName("sidebarRole");
	m_welcomeLabel->setWordWrap(true);

	sideLayout->addWidget(brandLabel);
	sideLayout->addSpacing(4);
	sideLayout->addWidget(m_welcomeLabel);
	sideLayout->addSpacing(32);

	QStringList pageNames = {
	    QStringLiteral("Dashboard"),
	    QStringLiteral("Transact"),
	    QStringLiteral("History"),
	};

	for (int i = 0; i < 3; ++i) {
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
	        &CustomerDashboard::refreshData);
	connect(logoutBtn, &QPushButton::clicked, this,
	        &CustomerDashboard::onLogout);

	m_contentStack = new QStackedWidget;
	m_contentStack->addWidget(createDashboardPage());
	m_contentStack->addWidget(createTransactPage());
	m_contentStack->addWidget(createHistoryPage());

	mainLayout->addWidget(sidebar);
	mainLayout->addWidget(m_contentStack, 1);
}

QWidget *CustomerDashboard::createDashboardPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("My Accounts"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	auto *balanceCard = new QFrame;
	balanceCard->setObjectName("statCard");
	auto *balLayout = new QVBoxLayout(balanceCard);
	auto *balTitle =
	    new QLabel(QStringLiteral("Total Balance"));
	balTitle->setObjectName("statLabel");
	m_totalBalanceLabel =
	    new QLabel(QStringLiteral("EGP 0"));
	m_totalBalanceLabel->setObjectName("statValue");
	balLayout->addWidget(balTitle);
	balLayout->addWidget(m_totalBalanceLabel);

	layout->addWidget(balanceCard);

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

QWidget *CustomerDashboard::createTransactPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("New Transaction"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	auto *formCard = new QFrame;
	formCard->setObjectName("formCard");
	auto *formLayout = new QVBoxLayout(formCard);
	formLayout->setContentsMargins(24, 24, 24, 24);

	auto *formRow = new QHBoxLayout;
	formRow->setSpacing(24);

	auto *leftCol = new QVBoxLayout;
	leftCol->setSpacing(12);

	auto *typeLabel =
	    new QLabel(QStringLiteral("Transaction Type"));
	typeLabel->setObjectName("fieldLabel");
	m_txTypeCombo = new QComboBox;
	m_txTypeCombo->addItem(QStringLiteral("Deposit"));
	m_txTypeCombo->addItem(QStringLiteral("Withdraw"));
	m_txTypeCombo->addItem(QStringLiteral("Transfer"));
	m_txTypeCombo->setFixedHeight(38);

	auto *acctLabel =
	    new QLabel(QStringLiteral("From Account"));
	acctLabel->setObjectName("fieldLabel");
	m_fromAccountCombo = new QComboBox;
	m_fromAccountCombo->setFixedHeight(38);

	leftCol->addWidget(typeLabel);
	leftCol->addWidget(m_txTypeCombo);
	leftCol->addSpacing(8);
	leftCol->addWidget(acctLabel);
	leftCol->addWidget(m_fromAccountCombo);

	auto *rightCol = new QVBoxLayout;
	rightCol->setSpacing(12);

	auto *amountLabel =
	    new QLabel(QStringLiteral("Amount (EGP)"));
	amountLabel->setObjectName("fieldLabel");
	m_amountEdit = new QLineEdit;
	m_amountEdit->setPlaceholderText(
	    QStringLiteral("Enter amount..."));
	m_amountEdit->setFixedHeight(38);

	m_destLabel =
	    new QLabel(QStringLiteral("Destination Account"));
	m_destLabel->setObjectName("fieldLabel");
	m_destAccountEdit = new QLineEdit;
	m_destAccountEdit->setPlaceholderText(
	    QStringLiteral("Destination account ID..."));
	m_destAccountEdit->setFixedHeight(38);

	rightCol->addWidget(amountLabel);
	rightCol->addWidget(m_amountEdit);
	rightCol->addSpacing(8);
	rightCol->addWidget(m_destLabel);
	rightCol->addWidget(m_destAccountEdit);

	formRow->addLayout(leftCol, 1);
	formRow->addLayout(rightCol, 1);

	formLayout->addLayout(formRow);
	formLayout->addSpacing(20);

	auto *execBtn =
	    new QPushButton(QStringLiteral("Execute Transaction"));
	execBtn->setObjectName("primaryBtn");
	execBtn->setFixedHeight(42);
	execBtn->setCursor(Qt::PointingHandCursor);
	execBtn->setMaximumWidth(280);

	formLayout->addWidget(execBtn, 0, Qt::AlignLeft);
	formLayout->addStretch();

	layout->addWidget(formCard);
	layout->addStretch();

	connect(m_txTypeCombo, &QComboBox::currentIndexChanged,
	        this,
	        &CustomerDashboard::onTransactionTypeChanged);
	connect(execBtn, &QPushButton::clicked, this,
	        &CustomerDashboard::onExecuteTransaction);

	return page;
}

QWidget *CustomerDashboard::createHistoryPage() {
	auto *page = new QWidget;
	page->setObjectName("contentPage");
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(32, 28, 32, 28);
	layout->setSpacing(20);

	auto *pageTitle =
	    new QLabel(QStringLiteral("Transaction History"));
	pageTitle->setObjectName("pageTitle");
	layout->addWidget(pageTitle);

	m_historyTable = new QTableView;
	m_historyTable->setModel(m_auditModel);
	m_historyTable->setAlternatingRowColors(true);
	m_historyTable->setSelectionBehavior(
	    QAbstractItemView::SelectRows);
	m_historyTable->setSelectionMode(
	    QAbstractItemView::SingleSelection);
	m_historyTable->verticalHeader()->setVisible(false);
	m_historyTable->setSortingEnabled(true);
	m_historyTable->horizontalHeader()->setStretchLastSection(
	    true);
	m_historyTable->verticalHeader()->setDefaultSectionSize(36);

	layout->addWidget(m_historyTable, 1);

	return page;
}

void CustomerDashboard::switchPage(int index) {
	for (int i = 0; i < 3; ++i)
		m_sidebarBtns[i]->setChecked(i == index);
	m_contentStack->setCurrentIndex(index);
}

void CustomerDashboard::refreshData() {
	QString name = m_session->customerName();
	uint64_t cid = m_session->customerId();
	m_welcomeLabel->setText(
	    QStringLiteral("Welcome, %1\n(ID: %2)")
	        .arg(name)
	        .arg(cid));

	populateAccountCombo();
	emit requestCustomerData(cid);
}

void CustomerDashboard::populateAccountCombo() {
	m_fromAccountCombo->clear();
	auto accountIds = m_session->accountIds();
	for (uint64_t aid : accountIds) {
		m_fromAccountCombo->addItem(
		    QString::number(aid), QVariant::fromValue(aid));
	}
}

void CustomerDashboard::onAccountsReady(
    const QVector<AccountDisplay> &accounts) {
	if (!isVisible())
		return;
	m_accountModel->setAccounts(accounts);

	int64_t total = 0;
	for (const auto &acc : accounts) {
		if (acc.state != AccountState::CLOSED)
			total += acc.balance;
	}
	static const QLocale loc(QLocale::English);
	m_totalBalanceLabel->setText(
	    QStringLiteral("EGP %1")
	        .arg(loc.toString(static_cast<qlonglong>(total))));
}

void CustomerDashboard::onTransactionsReady(
    const QVector<TransactionDisplay> &transactions) {
	if (!isVisible())
		return;
	m_auditModel->setTransactions(transactions);
}

void CustomerDashboard::onOperationResult(const QString &message,
                                          bool success) {
	if (!isVisible())
		return;
	QMessageBox::information(
	    this,
	    success ? QStringLiteral("Success")
	            : QStringLiteral("Notice"),
	    message);
	if (success) {
		m_amountEdit->clear();
		m_destAccountEdit->clear();
		emit requestCustomerData(m_session->customerId());
	}
}

void CustomerDashboard::onTransactionTypeChanged(int index) {
	bool isTransfer = (index == 2);
	m_destLabel->setVisible(isTransfer);
	m_destAccountEdit->setVisible(isTransfer);
}

void CustomerDashboard::onExecuteTransaction() {
	bool ok = false;
	int64_t amount = m_amountEdit->text().toLongLong(&ok);
	if (!ok || amount <= 0) {
		QMessageBox::warning(
		    this, QStringLiteral("Validation"),
		    QStringLiteral("Enter a valid positive amount"));
		return;
	}

	uint64_t accountId =
	    m_fromAccountCombo->currentData().toULongLong();
	int type = m_txTypeCombo->currentIndex();

	switch (type) {
	case 0:
		emit requestDeposit(accountId, amount);
		break;
	case 1:
		emit requestWithdraw(accountId, amount);
		break;
	case 2: {
		uint64_t destId =
		    m_destAccountEdit->text().toULongLong(&ok);
		if (!ok) {
			QMessageBox::warning(
			    this, QStringLiteral("Validation"),
			    QStringLiteral(
			        "Enter a valid destination account"));
			return;
		}
		emit requestTransfer(accountId, destId, amount);
		break;
	}
	}
}

void CustomerDashboard::onLogout() { m_session->logout(); }
