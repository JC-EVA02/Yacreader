#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlDatabase>

class TableItem;

//! [0]
class TableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	TableModel(QObject *parent = 0);
    TableModel( QSqlQuery &sqlquery, QObject *parent = 0);
    ~TableModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	void setupModelData(unsigned long long int parentFolder,QSqlDatabase & db);
	
	//M�todos de conveniencia
	QStringList getPaths(const QString & _source);
	QString getComicPath(QModelIndex & mi);
	//getComicInfo(QModelIndex & mi); --> para la edici�n
	//getComicsInfo(QList<QModelIndex> list); --> recupera la informaci�n com�n a los comics seleccionados
	//setcomicInfo(QModelIndex & mi); --> inserta en la base datos
	//setComicInfoForAllComics(); --> inserta la informaci�n com�n a todos los c�mics de una sola vez.
	//setComicInfoForSelectedComis(QList<QModelIndex> list); -->inserta la informaci�n com�n para los comics seleccionados
private:
    void setupModelData( QSqlQuery &sqlquery);

    QList<TableItem *> _data;

signals:
	void beforeReset();
	void reset();
};
//! [0]

#endif
