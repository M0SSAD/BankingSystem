#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "SessionManager.h"

class LoginScreen;
class AdminDashboard;
class CustomerDashboard;
class BackendWorker;
class QThread;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void onStateChanged(SessionState newState);
	void onCustomerLookupResult(bool found, uint64_t customerId,
	                            const QString &name,
	                            const QVector<uint64_t> &accountIds);

private:
	QStackedWidget *m_stack;
	SessionManager *m_session;
	LoginScreen *m_loginScreen;
	AdminDashboard *m_adminDashboard;
	CustomerDashboard *m_customerDashboard;
	BackendWorker *m_worker;
	QThread *m_workerThread;
};
