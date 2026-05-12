#include "SessionManager.h"

SessionManager::SessionManager(QObject *parent) : QObject(parent) {}

SessionState SessionManager::state() const { return m_state; }

uint64_t SessionManager::customerId() const { return m_customerId; }

QString SessionManager::customerName() const { return m_customerName; }

QVector<uint64_t> SessionManager::accountIds() const {
	return m_accountIds;
}

void SessionManager::enterAdmin() {
	m_state = SessionState::Admin;
	m_customerId = 0;
	m_customerName.clear();
	m_accountIds.clear();
	emit stateChanged(m_state);
}

void SessionManager::enterCustomerSession(uint64_t cid,
                                          const QString &name,
                                          const QVector<uint64_t> &accounts) {
	m_state = SessionState::Customer;
	m_customerId = cid;
	m_customerName = name;
	m_accountIds = accounts;
	emit stateChanged(m_state);
}

void SessionManager::logout() {
	m_state = SessionState::Login;
	m_customerId = 0;
	m_customerName.clear();
	m_accountIds.clear();
	emit stateChanged(m_state);
}
