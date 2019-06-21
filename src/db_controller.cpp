#include "db_controller.h"
DbController::DbController(QObject* parent) :
    QObject(parent)
{

}

DbController::~DbController()
{
    if (db.isOpen())
        db.close();
}

void DbController::connectToServerRequested(QString engine, QString driver, QString server, int port, QString database,
                                            QString login, QString password, bool is_sql_authentication)
{
    db = QSqlDatabase();
    db.removeDatabase("example-connection");

    if (engine == "mssql")
    {
        db = QSqlDatabase::addDatabase("QODBC", "example-connection");
    }
    else
    {
        emit serverErrorWithConnection("Unknown database engine");
        return;
    }

    bool connection_succesfull;

    if (engine == "mssql")
    {
        connection_succesfull = connectToServerMSSQL(driver, server, port, database, login, password);
    }
    else
    {
        emit serverErrorWithConnection("Unknown database engine");
        return;
    }

    if (connection_succesfull)
        emit serverConnected();
    else
        emit serverErrorWithConnection(getLastError().driverText());
}

void DbController::disconnectFromServerRequested()
{
    disconnectFromServer();

    emit serverDisconnected();
}

bool DbController::checkIfTableExists(QString table)
{
    return db.tables().contains(table);
}

bool DbController::checkIfConnected()
{
    return db.isOpen();
}

void DbController::selectTableRequested(QString table, QString field, QString le_name)
{
    QSqlQueryModel* model = selectTable(table, field, le_name);

    emit tableSelected(model);
}
void DbController::deleteRowRequested(QString table, QString field, QString le_name)
{
    QSqlQueryModel* model = new QSqlQueryModel;
    QString query ("DELETE FROM " + table + " WHERE " + field + " = '" + le_name + "'");
    model->setQuery(query, db);
    emit tableSelected(model);
}
void DbController::updateRowRequested (QString table, QString field, QString le_name, QString le_where)
{
    QSqlQueryModel* model = new QSqlQueryModel;
    QString query2 ("UPDATE "+ table + " SET " + field + " = '" + le_name + "' WHERE " + field + " = '"+ le_where + "'");
    model->setQuery(query2, db);
    emit tableSelected(model);
}

void DbController::readTableRequested(QString table, QString le_name){
    QSqlQueryModel* model = new QSqlQueryModel;
    QString query ("SELECT " + le_name +" FROM " + table );
    model->setQuery(query, db);
    emit tableSelected(model);
}

void DbController::createRowRequested(QString table, QString le_name){
    QSqlQueryModel* model = new QSqlQueryModel;
    QString query ("INSERT INTO " + table +" VALUES (" + le_name + ")" );
    model->setQuery(query, db);
    emit tableSelected(model);
}



void DbController::getTablesNamesRequested()
{
    emit gotTablesNames(db.tables());
}
void DbController::getFieldsNamesRequested(QString table_name)
{
    QSqlRecord Rec = db.driver()->record(table_name);
    QStringList list_of_fields;
    for (int i = 0 ; i < Rec.count() ; i++){
        list_of_fields.append(Rec.fieldName(i));
    }
    emit gotFieldsNames(list_of_fields);

}
bool DbController::connectToServerMSSQL(QString driver, QString server, int port, QString database,
                                   QString login, QString password)
{
    db.setDatabaseName(connection_string_sqlauth.arg(driver).arg(server).arg(port).arg(database)
                       .arg(login).arg(password));

    return db.open();
}



void DbController::disconnectFromServer()
{
    db.close();
}

QSqlQueryModel* DbController::selectTable(QString name, QString field, QString le_name)
{
    QSqlQueryModel* model = new QSqlQueryModel;
    QString query ("SELECT * FROM " + name + " WHERE " + field + " = '" + le_name + "'");
    model->setQuery(query, db);

    return model;
}

QSqlError DbController::getLastError()
{
    return db.lastError();
}

const QString DbController::connection_string_sqlauth =
        QString("DRIVER={%1};SERVER=%2;PORT=%3;DATABASE=%4;UID=%5;PWD=%6");

const QString DbController::connection_string_winauth =
        QString("DRIVER={%1};SERVER=%2;PORT=%3;DATABASE=%4");
