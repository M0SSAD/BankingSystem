#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <cstdint>

enum class SessionState { Login, Admin, Customer };

class SessionManager : public QObject {
	Q_OBJECT
public:
	explicit SessionManager(QObject *parent = nullptr);

	SessionState state() const;
	uint64_t customerId() const;
	QString customerName() const;
	QVector<uint64_t> accountIds() const;

public slots:
	void enterAdmin();
	void enterCustomerSession(uint64_t cid, const QString &name,
	                          const QVector<uint64_t> &accounts);
	void logout();

signals:
	void stateChanged(SessionState newState);

private:
	SessionState m_state = SessionState::Login;
	uint64_t m_customerId = 0;
	QString m_customerName;
	QVector<uint64_t> m_accountIds;
};
