#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent, DbController* dbc, QThread* dbt) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db_controller = dbc;
    db_thread = dbt;


    //signal-slot binding
    connect(ui->button_connect, SIGNAL(clicked()), this, SLOT(connectToServerRequested()));

    // ui => db_controller

    connect(this, SIGNAL(connectToServer(QString,QString,QString,int,QString,QString,QString,bool)),
            db_controller, SLOT(connectToServerRequested(QString,QString,QString,int,QString,QString,QString,bool)));
    connect(this, SIGNAL(disconnectFromServer()), db_controller, SLOT(disconnectFromServerRequested()));
    connect(this, SIGNAL(deleteRow(QString, QString, QString)), db_controller, SLOT(deleteRowRequested(QString, QString, QString)));
    connect(this, SIGNAL(updateRow(QString, QString, QString, QString)), db_controller, SLOT(updateRowRequested(QString, QString, QString, QString)));
    connect(this, SIGNAL(readTable(QString, QString)), db_controller, SLOT(readTableRequested(QString, QString)));
    connect(this, SIGNAL(createRow(QString, QString)), db_controller, SLOT(createRowRequested(QString, QString)));
    connect(this, SIGNAL(selectTable(QString, QString, QString)), db_controller, SLOT(selectTableRequested(QString, QString, QString)));
    connect(this, SIGNAL(getTablesNames()), db_controller, SLOT(getTablesNamesRequested()));
    connect(this, SIGNAL(getFieldsNames(QString)), db_controller, SLOT(getFieldsNamesRequested(QString)));

     // db_controller => ui

    connect(db_controller, SIGNAL(serverConnected()), this, SLOT(serverConnected()));
    connect(db_controller, SIGNAL(serverErrorWithConnection(QString)),
            this, SLOT(serverErrorWithConnection(QString)));
    connect(db_controller, SIGNAL(serverDisconnected()), this, SLOT(serverDisconnected()));
    connect(db_controller, SIGNAL(tableSelected(QSqlQueryModel*)), this, SLOT(displayTable(QSqlQueryModel*)));
    connect(db_controller, SIGNAL(gotTablesNames(QStringList)), this, SLOT(fillTablesNames(QStringList)));
    connect(db_controller, SIGNAL(gotFieldsNames(QStringList)), this, SLOT(fillFieldsNames(QStringList)));

    // load settings
    ui->lineEdit_where->setHidden(true);
    ui->label_where->setHidden(true);
    QString inifile("config.ini");
    QFileInfo check_file(inifile);
    if (check_file.exists() && check_file.isFile())
    {
        QSettings settings(inifile, QSettings::IniFormat);

        QString engine = settings.value("sql/engine", "").toString();


        ui->radioButton_search->setChecked(true);
        ui->lineEdit_driver->setText(settings.value("sql/driver", "").toString());
        ui->lineEdit_server_address->setText(settings.value("sql/address", "").toString());
        ui->spinBox_server_port->setValue(settings.value("sql/port", 0).toInt());
        ui->lineEdit_login->setText(settings.value("sql/login", "").toString());
        ui->lineEdit_password->setText(settings.value("sql/password", "").toString()); // plain text, so secure...
        ui->lineEdit_database_name->setText(settings.value("sql/database", "").toString());
        ui->statusBar->showMessage("Settings file config.ini loaded", 3000);


    }
    else
    {
        ui->statusBar->showMessage("Settings file config.ini does not exist", 5000);
    }


}

MainWindow::~MainWindow()
{
    db_thread->exit();
    db_thread->wait();
    delete ui;
}

void MainWindow::connectToServerRequested()
{
    QString engine;
    engine = "mssql";

    QString driver   = ui->lineEdit_driver->text(),
            server   = ui->lineEdit_server_address->text(),
            database = ui->lineEdit_database_name->text(),
            login    = ui->lineEdit_login->text(),
            password = ui->lineEdit_password->text();
    int port = ui->spinBox_server_port->value();

    if (server == "")
    {
        QMessageBox::information(this,
                                 "Invalid Connection Data",
                                 "Insert server address to connect",
                                 QMessageBox::Ok);
        return;
    }


    if ( login == "")
    {
        QMessageBox::information(this,
                                 "Invalid Connection Data",
                                 "Insert login to connect",
                                 QMessageBox::Ok);
        return;
    }

    if (database == "")
    {
        QMessageBox::information(this,
                                 "Invalid Connection Data",
                                 "Insert database name to connect",
                                 QMessageBox::Ok);
        return;
    }

    ui->button_connect->setEnabled(false);
    ui->statusBar->showMessage("Connecting...");

    emit connectToServer(engine, driver, server, port, database, login, password, 1);
}

void MainWindow::disconnectFromServerRequested()
{
    ui->button_connect->setEnabled(false);

    delete ui->tableView_database_table->model();

    emit disconnectFromServer();
}





void MainWindow::showTableRequested()
{
    ui->button_show_table->setEnabled(false);

    delete ui->tableView_database_table->model();

    QString table_name = ui->comboBox_table_name->currentText();
    QString field_name = ui->comboBox_field_name->currentText();
    QString le_name = ui->lineEdit_search->text();

    emit selectTable(table_name, field_name, le_name);
}

void MainWindow::serverConnected()
{
    ui->button_connect->setEnabled(true);

    disconnect(ui->button_connect, SIGNAL(clicked()), this, SLOT(connectToServerRequested()));
    connect(ui->button_connect, SIGNAL(clicked()), this, SLOT(disconnectFromServerRequested()));

    ui->button_connect->setText("Disconnect");
    ui->groupBox_operation->setEnabled(true);
    connect(ui->radioButton_crud, SIGNAL(clicked()), this, SLOT(setEnableMethod()));
    connect(ui->radioButton_search, SIGNAL(clicked()), this, SLOT(setEnableSearch()));


    ui->statusBar->showMessage("Connected", 3000);

}

void MainWindow::fillTablesNames(QStringList tables_names)
{
    if (tables_names.length() == 0)
        QMessageBox::warning(this,
                             "Tables",
                             "There are no tables to display in the database",
                             QMessageBox::Ok);
    else
    {
        ui->comboBox_table_name->clear();
        tables_names.pop_back();
        tables_names.pop_back();
        ui->comboBox_table_name->addItems(tables_names);

        ui->comboBox_table_name->setEnabled(true);
        connect(ui->comboBox_table_name, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(comboboxTableChanged(const QString&)));//hasnt done ********
        ui->button_show_table->setFocus();
    }
}

void MainWindow::fillFieldsNames(QStringList fields_names)
{

        ui->comboBox_field_name->clear();
        ui->comboBox_field_name->addItems(fields_names);
        ui->comboBox_field_name->setEnabled(true);
        ui->comboBox_table_name->setFocus(); //*********remember this

}

void MainWindow::serverErrorWithConnection(QString message)
{
    QMessageBox::critical(this,
                          "Connection failed",
                          message,
                          QMessageBox::Ok);

    ui->button_connect->setEnabled(true);

    ui->statusBar->showMessage("Connection failed", 3000);
}

void MainWindow::serverDisconnected()
{
    disconnect(ui->button_connect, SIGNAL(clicked()), this, SLOT(disconnectFromServerRequested()));
    connect(ui->button_connect, SIGNAL(clicked()), this, SLOT(connectToServerRequested()));

    ui->tableView_database_table->setModel(NULL);

    ui->button_connect->setEnabled(true);
    ui->button_connect->setText("Connect");

    ui->comboBox_table_name->clear();
    ui->comboBox_table_name->setEnabled(false);

    ui->groupBox_operation->setEnabled(false);
    ui->groupBox_database_browser->setEnabled(false);
    ui->button_connect->setFocus();
}

void MainWindow::displayTable(QSqlQueryModel* model)
{
    if (!model->lastError().isValid())
        ui->tableView_database_table->setModel(model);
    else
        QMessageBox::critical(this,
                              "Select failed",
                              model->lastError().databaseText(),
                              QMessageBox::Ok);

    ui->button_show_table->setEnabled(true);
    ui->comboBox_table_name->setFocus();
}

void MainWindow::keyPressEvent(QKeyEvent* pe)
{
    if (pe->key() == Qt::Key_Enter || pe->key() == Qt::Key_Return)
    {
        if (!db_controller->checkIfConnected())
            emit connectToServerRequested();
        else if (ui->comboBox_table_name->isEnabled() && ui->comboBox_table_name->hasFocus())
            emit showTableRequested();
    }
}
void MainWindow::setEnableMethod()
{
    ui->groupBox_database_browser->setEnabled(false);
    ui->groupBox_crudMethod->setEnabled(true);
    ui->button_show_table->setText("Go!");
    connect(ui->radioButton_create, SIGNAL(clicked()), this, SLOT(setEnableCreate()));
    connect(ui->radioButton_Update, SIGNAL(clicked()), this, SLOT(setEnableUpdate()));
    connect(ui->radioButton_read, SIGNAL(clicked()), this, SLOT(setEnableRead()));
    connect(ui->radioButton_delete, SIGNAL(clicked()), this, SLOT(setEnableDelete()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));

}

void MainWindow::setEnableCreate(){
    ui->lineEdit_where->setHidden(true);
    ui->label_where->setHidden(true);
    ui->groupBox_database_browser ->setHidden(false);
    ui->groupBox_database_browser->setEnabled(true);
    ui->comboBox_field_name->setHidden(true);
    ui->label_field->setHidden(true);
    ui->label_search->setText("Values");
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(readRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(deleteRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(updateRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));
    connect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(createRequested()));
    emit getTablesNames();

}
void MainWindow::setEnableRead(){
    ui->lineEdit_where->setHidden(true);
    ui->label_where->setHidden(true);
    ui->groupBox_database_browser ->setHidden(false);
    ui->groupBox_database_browser->setEnabled(true);
    ui->comboBox_field_name->setHidden(true);
    ui->label_field->setHidden(true);
    ui->label_search->setText("fields");
    connect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(readRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(deleteRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(updateRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(createRequested()));
    emit getTablesNames();

}
void MainWindow::setEnableDelete(){
    ui->comboBox_field_name->setHidden(false);
    ui->label_field->setHidden(false);
    ui->lineEdit_where->setHidden(true);
    ui->label_where->setHidden(true);
    ui->groupBox_database_browser ->setHidden(false);
    ui->groupBox_database_browser->setEnabled(true);
    ui->label_search->setText("Where");
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(readRequested()));
    connect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(deleteRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(updateRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(createRequested()));
    emit getTablesNames();

}

void MainWindow::setEnableUpdate(){
    ui->lineEdit_where->setHidden(false);
    ui->label_where->setHidden(false);
    ui->comboBox_field_name->setHidden(false);
    ui->label_field->setHidden(false);
    ui->groupBox_database_browser ->setHidden(false);
    ui->groupBox_database_browser->setEnabled(true);
    ui->label_search->setText("New value");
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(readRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(deleteRequested()));
    connect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(updateRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(createRequested()));
    emit getTablesNames();

}

void MainWindow::createRequested(){
    ui->button_show_table->setEnabled(false);
    delete ui->tableView_database_table->model();
    QString table_name = ui->comboBox_table_name->currentText();
    QString le_name = ui->lineEdit_search->text();
    emit createRow(table_name, le_name);
}

void MainWindow::readRequested(){
    ui->button_show_table->setEnabled(false);
    delete ui->tableView_database_table->model();
    QString table_name = ui->comboBox_table_name->currentText();
    QString le_name = ui->lineEdit_search->text();
    emit readTable(table_name, le_name);
}

void MainWindow::deleteRequested(){

    ui->button_show_table->setEnabled(false);
    delete ui->tableView_database_table->model();

    QString table_name = ui->comboBox_table_name->currentText();
    QString field_name = ui->comboBox_field_name->currentText();
    QString le_name = ui->lineEdit_search->text();

    emit deleteRow(table_name, field_name, le_name);
}
void MainWindow::updateRequested(){
    ui->button_show_table->setEnabled(false);
    delete ui->tableView_database_table->model();
    QString table_name = ui->comboBox_table_name->currentText();
    QString field_name = ui->comboBox_field_name->currentText();
    QString le_name = ui->lineEdit_search->text();
    QString le_where = ui->lineEdit_where->text();
    emit updateRow(table_name, field_name, le_name, le_where);
}

void MainWindow::setEnableSearch(){
    ui->lineEdit_where->setHidden(true);
    ui->label_where->setHidden(true);
    ui->comboBox_field_name->setHidden(false);
    ui->label_field->setHidden(false);
    ui->label_search->setText("Search");
    ui->button_show_table->setText("Show");
    ui->groupBox_database_browser ->setHidden(false);
    ui->groupBox_crudMethod->setEnabled(false);
    ui->groupBox_database_browser->setEnabled(true);
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(readRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(deleteRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(updateRequested()));
    disconnect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(createRequested()));
    connect(ui->button_show_table, SIGNAL(clicked()), this, SLOT(showTableRequested()));
    emit getTablesNames();

}

void MainWindow::comboboxTableChanged(const QString& table_name){
    emit getFieldsNames(table_name);
}


