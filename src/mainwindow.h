#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "db_controller.h"

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent, DbController* dbc, QThread* dbt);
    ~MainWindow();

public slots:
    void setEnableCreate();
    void setEnableRead();
    void setEnableDelete();
    void setEnableUpdate();
    void setEnableMethod();
    void setEnableSearch();
    void connectToServerRequested();
    void disconnectFromServerRequested();
    void showTableRequested();
    void deleteRequested();
    void updateRequested();
    void createRequested();
    void readRequested();
    void comboboxTableChanged(const QString&);
    void serverConnected();
    void serverErrorWithConnection(QString);
    void serverDisconnected();
    void displayTable(QSqlQueryModel*);
    void fillTablesNames(QStringList);
    void fillFieldsNames(QStringList);


signals:
    void connectToServer(QString, QString, QString, int, QString, QString, QString, bool);
    void disconnectFromServer();
    void updateRow(QString, QString, QString, QString);
    void selectTable(QString, QString, QString);
    void readTable(QString, QString);
    void createRow(QString, QString);
    void deleteRow(QString, QString, QString);
    void getTablesNames();
    void getFieldsNames(QString);

    
private:
    Ui::MainWindow* ui;
    DbController*   db_controller;
    QThread*        db_thread;

protected:
    virtual void keyPressEvent(QKeyEvent*);
};

#endif // MAINWINDOW_H
