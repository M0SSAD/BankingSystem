#pragma once

#include <QWidget>

class SessionManager;
class QLineEdit;
class QLabel;
class QPushButton;

class LoginScreen : public QWidget {
	Q_OBJECT
public:
	explicit LoginScreen(SessionManager *session,
	                     QWidget *parent = nullptr);

public slots:
	void setLoginStatus(const QString &message, bool isError);

signals:
	void loginAsAdmin();
	void loginAsCustomer(uint64_t nationalId);

private:
	SessionManager *m_session;
	QLineEdit *m_nationalIdInput;
	QLabel *m_statusLabel;
	QPushButton *m_loginBtn;

	void onCustomerLoginClicked();
};
