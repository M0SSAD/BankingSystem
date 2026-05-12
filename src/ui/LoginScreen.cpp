#include "LoginScreen.h"
#include "SessionManager.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginScreen::LoginScreen(SessionManager *session, QWidget *parent)
    : QWidget(parent), m_session(session) {
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(60, 60, 60, 60);

	mainLayout->addStretch(1);

	auto *titleLabel = new QLabel(QStringLiteral("BankingSystem"));
	titleLabel->setObjectName("titleLabel");
	titleLabel->setAlignment(Qt::AlignCenter);

	auto *subtitleLabel =
	    new QLabel(QStringLiteral("Professional Banking Platform v1.0"));
	subtitleLabel->setObjectName("subtitleLabel");
	subtitleLabel->setAlignment(Qt::AlignCenter);

	mainLayout->addWidget(titleLabel);
	mainLayout->addSpacing(8);
	mainLayout->addWidget(subtitleLabel);
	mainLayout->addSpacing(50);

	auto *cardsLayout = new QHBoxLayout;
	cardsLayout->setSpacing(40);
	cardsLayout->setAlignment(Qt::AlignCenter);

	auto *adminCard = new QFrame;
	adminCard->setObjectName("card");
	adminCard->setFixedSize(320, 260);
	auto *adminLayout = new QVBoxLayout(adminCard);

	auto *adminTitle =
	    new QLabel(QStringLiteral("ADMIN PORTAL"));
	adminTitle->setObjectName("headingLabel");
	adminTitle->setAlignment(Qt::AlignCenter);

	auto *adminDesc =
	    new QLabel(QStringLiteral("Full system access.\nNo login required."));
	adminDesc->setObjectName("subtitleLabel");
	adminDesc->setAlignment(Qt::AlignCenter);

	auto *adminBtn =
	    new QPushButton(QStringLiteral("Enter Admin Panel"));
	adminBtn->setObjectName("primaryBtn");
	adminBtn->setFixedHeight(42);
	adminBtn->setCursor(Qt::PointingHandCursor);

	adminLayout->addStretch();
	adminLayout->addWidget(adminTitle);
	adminLayout->addSpacing(12);
	adminLayout->addWidget(adminDesc);
	adminLayout->addSpacing(24);
	adminLayout->addWidget(adminBtn);
	adminLayout->addStretch();

	connect(adminBtn, &QPushButton::clicked, this,
	        &LoginScreen::loginAsAdmin);

	auto *customerCard = new QFrame;
	customerCard->setObjectName("card");
	customerCard->setFixedSize(320, 340);
	auto *customerLayout = new QVBoxLayout(customerCard);

	auto *customerTitle =
	    new QLabel(QStringLiteral("CUSTOMER PORTAL"));
	customerTitle->setObjectName("headingLabel");
	customerTitle->setAlignment(Qt::AlignCenter);

	auto *idLabel = new QLabel(QStringLiteral("National ID"));
	idLabel->setObjectName("sectionLabel");

	m_nationalIdInput = new QLineEdit;
	m_nationalIdInput->setPlaceholderText(
	    QStringLiteral("Enter your National ID..."));
	m_nationalIdInput->setFixedHeight(42);

	m_loginBtn =
	    new QPushButton(QStringLiteral("Login"));
	m_loginBtn->setObjectName("successBtn");
	m_loginBtn->setFixedHeight(42);
	m_loginBtn->setCursor(Qt::PointingHandCursor);

	m_statusLabel = new QLabel(
	    QStringLiteral("Enter your National ID to login"));
	m_statusLabel->setAlignment(Qt::AlignCenter);
	m_statusLabel->setStyleSheet(
	    "color: #6B7280; font-size: 12px;");

	customerLayout->addStretch();
	customerLayout->addWidget(customerTitle);
	customerLayout->addSpacing(20);
	customerLayout->addWidget(idLabel);
	customerLayout->addSpacing(4);
	customerLayout->addWidget(m_nationalIdInput);
	customerLayout->addSpacing(12);
	customerLayout->addWidget(m_loginBtn);
	customerLayout->addSpacing(8);
	customerLayout->addWidget(m_statusLabel);
	customerLayout->addStretch();

	connect(m_loginBtn, &QPushButton::clicked, this,
	        &LoginScreen::onCustomerLoginClicked);
	connect(m_nationalIdInput, &QLineEdit::returnPressed, this,
	        &LoginScreen::onCustomerLoginClicked);

	cardsLayout->addWidget(adminCard);
	cardsLayout->addWidget(customerCard);

	mainLayout->addLayout(cardsLayout);
	mainLayout->addStretch(2);
}

void LoginScreen::setLoginStatus(const QString &message, bool isError) {
	m_statusLabel->setText(message);
	if (isError) {
		m_statusLabel->setStyleSheet(
		    "color: #EF4444; font-size: 12px;");
	} else {
		m_statusLabel->setStyleSheet(
		    "color: #10B981; font-size: 12px;");
	}
}

void LoginScreen::onCustomerLoginClicked() {
	bool ok = false;
	uint64_t nationalId = m_nationalIdInput->text().toULongLong(&ok);
	if (!ok || m_nationalIdInput->text().isEmpty()) {
		setLoginStatus(
		    QStringLiteral("Please enter a valid National ID"), true);
		return;
	}
	m_statusLabel->setText(QStringLiteral("Logging in..."));
	m_statusLabel->setStyleSheet("color: #6B7280; font-size: 12px;");
	emit loginAsCustomer(nationalId);
}
