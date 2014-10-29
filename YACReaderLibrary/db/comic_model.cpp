
#include <QtGui>
#include <QtDebug>
#include <limits>

#include "comic_item.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "comic_db.h"
#include "db_helper.h"

//ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
#include "QsLog.h"


ComicModel::ComicModel(QObject *parent)
    : QAbstractItemModel(parent)
{
	connect(this,SIGNAL(beforeReset()),this,SIGNAL(modelAboutToBeReset()));
	connect(this,SIGNAL(reset()),this,SIGNAL(modelReset()));
}

//! [0]
ComicModel::ComicModel( QSqlQuery &sqlquery, QObject *parent)
    : QAbstractItemModel(parent)
{
	setupModelData(sqlquery);
}
//! [0]

//! [1]
ComicModel::~ComicModel()
{
	qDeleteAll(_data);
}
//! [1]

//! [2]
int ComicModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	if(_data.isEmpty())
		return 0;
	return _data.first()->columnCount();
}
//! [2]

QHash<int, QByteArray> ComicModel::roleNames() const {
    QHash<int, QByteArray> roles;

    roles[NumberRole] = "number";
    roles[TitleRole] = "title";
    roles[FileNameRole] = "file_name";
    roles[NumPagesRole] = "num_pages";
    roles[IdRole] = "id";
    roles[Parent_IdRole] = "parent_id";
    roles[PathRole] = "path";
    roles[HashRole] = "hash";
    roles[ReadColumnRole] = "read_column";
    roles[IsBisRole] = "is_bis";
    roles[CurrentPageRole] = "current_page";
    roles[RatingRole] = "rating";
    roles[HasBeenOpenedRole] = "has_been_opened";
    roles[CoverPathRole] = "cover_path";

    return roles;
}

//! [3]
QVariant ComicModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	/*if (index.column() == TableModel::Rating && role == Qt::DecorationRole)
	{
		TableItem *item = static_cast<TableItem*>(index.internalPointer());
		return QPixmap(QString(":/images/rating%1.png").arg(item->data(index.column()).toInt()));
	}*/

	if (role == Qt::DecorationRole)
	{
			return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		switch(index.column())//TODO obtener esto de la query
		{
        case ComicModel::Number:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::NumPages:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::Hash:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default:
			return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
		}
	}


    //TODO check here if any view is asking for TableModel::Roles
    //these roles will be used from QML/GridView

    ComicItem *item = static_cast<ComicItem*>(index.internalPointer());

    if (role == NumberRole)
        return item->data(Number);
    else if (role == TitleRole)
        return item->data(Title).isNull()?item->data(FileName):item->data(Title);
    else if (role == RatingRole)
        return item->data(Rating);
    else if (role == CoverPathRole)
        return "file:///"+_databasePath+"/covers/"+item->data(Hash).toString()+".jpg";
    else if (role == NumPagesRole)
        return item->data(NumPages);
    else if (role == CurrentPageRole)
        return item->data(CurrentPage);
    else if (role == ReadColumnRole)
        return item->data(ReadColumn).toBool();
    else if (role == HasBeenOpenedRole)
        return item->data(ComicModel::HasBeenOpened);

    if (role != Qt::DisplayRole)
        return QVariant();

    if(index.column() == ComicModel::Hash)
		return QString::number(item->data(index.column()).toString().right(item->data(index.column()).toString().length()-40).toInt()/1024.0/1024.0,'f',2)+"Mb";
    if(index.column() == ComicModel::ReadColumn)
        return (item->data(ComicModel::CurrentPage).toInt()==item->data(ComicModel::NumPages).toInt() || item->data(ComicModel::ReadColumn).toBool())?QVariant(tr("yes")):QVariant(tr("no"));
    if(index.column() == ComicModel::CurrentPage)
        return item->data(ComicModel::HasBeenOpened).toBool()?item->data(index.column()):QVariant("-");
	
    if (index.column() == ComicModel::Rating)
		return QVariant();

	return item->data(index.column());
}
//! [3]

//! [4]
Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
    if(index.column() == ComicModel::Rating)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [4]

//! [5]
QVariant ComicModel::headerData(int section, Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)//TODO obtener esto de la query
		{
        case ComicModel::Number:
			return QVariant(QString("#"));
        case ComicModel::Title:
			return QVariant(QString(tr("Title")));
        case ComicModel::FileName:
			return QVariant(QString(tr("File Name")));
        case ComicModel::NumPages:
			return QVariant(QString(tr("Pages")));
        case ComicModel::Hash:
			return QVariant(QString(tr("Size")));
        case ComicModel::ReadColumn:
			return QVariant(QString(tr("Read")));
        case ComicModel::CurrentPage:
			return QVariant(QString(tr("Current Page")));
        case ComicModel::Rating:
			return QVariant(QString(tr("Rating")));
		}
	}

	if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole)
	{
		switch(section)//TODO obtener esto de la query
		{
        case ComicModel::Number:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::NumPages:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::Hash:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default:
			return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
		}
	}


	if(orientation == Qt::Vertical && role == Qt::DecorationRole)
	{
        QString fileName = _data.value(section)->data(ComicModel::FileName).toString();
		QFileInfo fi(fileName);
		QString ext = fi.suffix();

		if (ext.compare("cbr",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicRar.png"));
		else if (ext.compare("cbz",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicZip.png"));
		else if(ext.compare("pdf",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/pdf.png"));
		else if (ext.compare("tar",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/tar.png"));
		else if(ext.compare("zip",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/zip.png"));
		else if(ext.compare("rar",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/rar.png"));
		else if (ext.compare("7z",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/7z.png"));
		else if (ext.compare("cb7",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comic7z.png"));
		else if (ext.compare("cb7",Qt::CaseInsensitive) == 0)
			return QVariant(QIcon(":/images/comicTar.png"));

	}

	return QVariant();
}
//! [5]

//! [6]
QModelIndex ComicModel::index(int row, int column, const QModelIndex &parent)
			const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	return createIndex(row, column, _data.at(row));
}
//! [6]

//! [7]
QModelIndex ComicModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}
//! [7]

//! [8]
int ComicModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		return _data.count();

	return 0;
}
//! [8]

QStringList ComicModel::getPaths(const QString & _source)
{
	QStringList paths;
	QString source = _source + "/.yacreaderlibrary/covers/";
    QList<ComicItem *>::ConstIterator itr;
	for(itr = _data.constBegin();itr != _data.constEnd();itr++)
	{
        QString hash = (*itr)->data(ComicModel::Hash).toString();
		paths << source+ hash +".jpg";
	}

	return paths;
}

void ComicModel::setupModelData(unsigned long long int folderId,const QString & databasePath)
{
	//QFile f(QCoreApplication::applicationDirPath()+"/performance.txt");
	//f.open(QIODevice::Append);
	beginResetModel();
	//QElapsedTimer timer;
	//timer.start();
	qDeleteAll(_data);
	_data.clear();

	//QTextStream txtS(&f);
	//txtS << "TABLEMODEL: Tiempo de borrado: " << timer.elapsed() << "ms\r\n";
	_databasePath = databasePath;
	QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
	{
	//crear la consulta
	//timer.restart();
    QSqlQuery selectQuery(db);
	selectQuery.prepare("select ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", folderId);
	selectQuery.exec();
	//txtS << "TABLEMODEL: Tiempo de consulta: " << timer.elapsed() << "ms\r\n";
	//timer.restart();
	setupModelData(selectQuery);
	//txtS << "TABLEMODEL: Tiempo de creaciï¿½n del modelo: " << timer.elapsed() << "ms\r\n";
	//selectQuery.finish();
	}
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	endResetModel();

    if(_data.length()==0)
        emit isEmpty();
}

void ComicModel::setupModelData(const SearchModifiers modifier, const QString &filter, const QString &databasePath)
{
    //QFile f(QCoreApplication::applicationDirPath()+"/performance.txt");
    //f.open(QIODevice::Append);
    beginResetModel();
    //QElapsedTimer timer;
    //timer.start();
    qDeleteAll(_data);
    _data.clear();

    //QTextStream txtS(&f);
    //txtS << "TABLEMODEL: Tiempo de borrado: " << timer.elapsed() << "ms\r\n";
    _databasePath = databasePath;
    QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
    {
    //crear la consulta
    //timer.restart();
    QSqlQuery selectQuery(db);

    switch (modifier) {
    case YACReader::NoModifiers:
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE UPPER(ci.title) LIKE UPPER(:filter) OR UPPER(c.fileName) LIKE UPPER(:filter) LIMIT :limit");
        selectQuery.bindValue(":filter", "%%"+filter+"%%");
        selectQuery.bindValue(":limit",500); //TODO, load this value from settings
        break;

    case YACReader::OnlyRead:
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE (UPPER(ci.title) LIKE UPPER(:filter) OR UPPER(c.fileName) LIKE UPPER(:filter)) AND ci.read = 1 LIMIT :limit");
        selectQuery.bindValue(":filter", "%%"+filter+"%%");
        selectQuery.bindValue(":limit",500); //TODO, load this value from settings
        break;

    case YACReader::OnlyUnread:
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE (UPPER(ci.title) LIKE UPPER(:filter) OR UPPER(c.fileName) LIKE UPPER(:filter)) AND ci.read = 0 LIMIT :limit");
        selectQuery.bindValue(":filter", "%%"+filter+"%%");
        selectQuery.bindValue(":limit",500); //TODO, load this value from settings
        break;

    default:
        QLOG_ERROR() << "not implemented";
        break;
    }


    selectQuery.exec();

    QLOG_DEBUG() << selectQuery.lastError() << "--";

    //txtS << "TABLEMODEL: Tiempo de consulta: " << timer.elapsed() << "ms\r\n";
    //timer.restart();
    setupModelData(selectQuery);
    //txtS << "TABLEMODEL: Tiempo de creaciï¿½n del modelo: " << timer.elapsed() << "ms\r\n";
    //selectQuery.finish();
    }
    db.close();
    QSqlDatabase::removeDatabase(_databasePath);
    endResetModel();

    emit searchNumResults(_data.length());
}

QString ComicModel::getComicPath(QModelIndex mi)
{
	if(mi.isValid())
        return _data.at(mi.row())->data(ComicModel::Path).toString();
	return "";
}

void ComicModel::setupModelData(QSqlQuery &sqlquery)
{
    ComicItem * currentItem;
	while (sqlquery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = sqlquery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);

        currentItem = new ComicItem(data);
		bool lessThan = false;
		if(_data.isEmpty())
			_data.append(currentItem);
		else
		{
            ComicItem * last = _data.back();
            QString nameLast = last->data(ComicModel::FileName).toString();
            QString nameCurrent = currentItem->data(ComicModel::FileName).toString();
			int numberLast,numberCurrent;
			int max = (std::numeric_limits<int>::max)();
			numberLast = numberCurrent = max;

            if(!last->data(ComicModel::Number).isNull())
            numberLast = last->data(ComicModel::Number).toInt();
			
            if(!currentItem->data(ComicModel::Number).isNull())
            numberCurrent = currentItem->data(ComicModel::Number).toInt();
			
            QList<ComicItem *>::iterator i;
			i = _data.end();
			i--;

			if(numberCurrent != max)
			{
				while ((lessThan =numberCurrent < numberLast) && i != _data.begin())
				{
					i--;
					numberLast = max;

                    if(!(*i)->data(ComicModel::Number).isNull())
                        numberLast = (*i)->data(ComicModel::Number).toInt();
				}
			}
			else
			{
				while ((lessThan = naturalSortLessThanCI(nameCurrent,nameLast)) && i != _data.begin() && numberLast == max)
				{
					i--;
                    nameLast = (*i)->data(ComicModel::FileName).toString();
					numberLast = max;

                    if(!(*i)->data(ComicModel::Number).isNull())
                        numberLast = (*i)->data(ComicModel::Number).toInt();
				}

			}
			if(!lessThan) //si se ha encontrado un elemento menor que current, se inserta justo despuï¿½s
			{
				if(numberCurrent != max)
				{
					if(numberCurrent == numberLast)
                        if(currentItem->data(ComicModel::IsBis).toBool())
						{
							_data.insert(++i,currentItem);
						}
						else
							_data.insert(i,currentItem);
					else
						_data.insert(++i,currentItem);
				}
				else
					_data.insert(++i,currentItem);
			}
			else
			{
				_data.insert(i,currentItem);
			}

		}
	}
}

ComicDB ComicModel::getComic(const QModelIndex & mi)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    ComicDB c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}

ComicDB ComicModel::_getComic(const QModelIndex & mi)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
    ComicDB c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(),db);
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return c;
}


QVector<YACReaderComicReadStatus> ComicModel::getReadList()
{
	int numComics = _data.count();
	QVector<YACReaderComicReadStatus> readList(numComics);
	for(int i=0;i<numComics;i++)
	{
        if(_data.value(i)->data(ComicModel::ReadColumn).toBool())
			readList[i] = YACReader::Read;
        else if (_data.value(i)->data(ComicModel::CurrentPage).toInt() == _data.value(i)->data(ComicModel::NumPages).toInt())
			 readList[i] = YACReader::Read;
        else if (_data.value(i)->data(ComicModel::HasBeenOpened).toBool())
			readList[i] = YACReader::Opened;
		else
			readList[i] = YACReader::Unread;
	}
	return readList;
}
//TODO untested, this method is no longer used
QVector<YACReaderComicReadStatus> ComicModel::setAllComicsRead(YACReaderComicReadStatus read)
{
	return setComicsRead(persistentIndexList(),read);
}

QList<ComicDB> ComicModel::getAllComics()
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();

	QList<ComicDB> comics;
	int numComics = _data.count();
	for(int i=0;i<numComics;i++)
	{
        comics.append(DBHelper::loadComic(_data.value(i)->data(ComicModel::Id).toULongLong(),db));
	}

	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	return comics;
}

QList<ComicDB> ComicModel::getComics(QList<QModelIndex> list)
{
	QList<ComicDB> comics;

	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	QList<QModelIndex>::const_iterator itr;
	for(itr = list.constBegin(); itr!= list.constEnd();itr++)
	{
		comics.append(_getComic(*itr));
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	return comics;
}
//TODO
QVector<YACReaderComicReadStatus> ComicModel::setComicsRead(QList<QModelIndex> list,YACReaderComicReadStatus read)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
	foreach (QModelIndex mi, list)
	{
		if(read == YACReader::Read)
		{
        _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(true));
        ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(),db);
		c.info.read = true;
		DBHelper::update(&(c.info),db);
		}
		if(read == YACReader::Unread)
		{
        _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(false));
        _data.value(mi.row())->setData(ComicModel::CurrentPage, QVariant(1));
        _data.value(mi.row())->setData(ComicModel::HasBeenOpened, QVariant(false));
        ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(),db);
		c.info.read = false;
		c.info.currentPage = 1;
		c.info.hasBeenOpened = false;
		DBHelper::update(&(c.info),db);
		}
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

    emit dataChanged(index(list.first().row(),ComicModel::ReadColumn),index(list.last().row(),ComicModel::HasBeenOpened),QVector<int>() << ReadColumnRole << CurrentPageRole << HasBeenOpenedRole);

	return getReadList();
}
qint64 ComicModel::asignNumbers(QList<QModelIndex> list,int startingNumber)
{
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	db.transaction();
    qint64 idFirst = _data.value(list[0].row())->data(ComicModel::Id).toULongLong();
	int i = 0;
	foreach (QModelIndex mi, list)
	{
        ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(),db);
        c.info.number = startingNumber+i;
		c.info.edited = true;
		DBHelper::update(&(c.info),db);
		i++;
	}

	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(_databasePath);

	//emit dataChanged(index(list.first().row(),READ),index(list.last().row(),READ));

	return idFirst;
}
QModelIndex ComicModel::getIndexFromId(quint64 id)
{
    QList<ComicItem *>::ConstIterator itr;
	int i=0;
	for(itr = _data.constBegin();itr != _data.constEnd();itr++)
	{
        if((*itr)->data(ComicModel::Id).toULongLong() == id)
			break;
		i++;
	}

	return index(i,0);
}

void ComicModel::startTransaction()
{
	
	dbTransaction = DataBaseManagement::loadDatabase(_databasePath);
	dbTransaction.transaction();
}

void ComicModel::finishTransaction()
{
	dbTransaction.commit();
	dbTransaction.close();
	QSqlDatabase::removeDatabase(_databasePath);


}

void ComicModel::removeInTransaction(int row)
{
    ComicDB c = DBHelper::loadComic(_data.at(row)->data(ComicModel::Id).toULongLong(),dbTransaction);

	DBHelper::removeFromDB(&c,dbTransaction);
	beginRemoveRows(QModelIndex(),row,row);
	removeRow(row);
	delete _data.at(row);
	_data.removeAt(row);

	endRemoveRows();
}

void ComicModel::remove(ComicDB * comic, int row)
{
	beginRemoveRows(QModelIndex(),row,row);
	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

	DBHelper::removeFromDB(comic,db);
	
	removeRow(row);
	delete _data.at(row);
	_data.removeAt(row);

	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
	endRemoveRows();
}

/*ComicDB TableModel::getComic(int row)
{
	return getComic(index(row,0));
}*/

void ComicModel::remove(int row)
{
	removeInTransaction(row);
}

void ComicModel::reload(const ComicDB & comic)
{
	int row = 0;
	bool found = false;
    foreach(ComicItem * item,_data)
	{
        if(item->data(ComicModel::Id).toULongLong() == comic.id)
		{
			found = true;
            item->setData(ComicModel::ReadColumn,comic.info.read);
            item->setData(ComicModel::CurrentPage,comic.info.currentPage);
            item->setData(ComicModel::HasBeenOpened,true);
			break;
				
		}
		row++;
	}
    if(found)
        emit dataChanged(index(row,ReadColumn),index(row,HasBeenOpened), QVector<int>() << ReadColumnRole << CurrentPageRole << HasBeenOpenedRole);
}

void ComicModel::resetComicRating(const QModelIndex &mi)
{
    ComicDB comic = getComic(mi);

    QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

    comic.info.rating = 0;
    _data[mi.row()]->setData(ComicModel::Rating,0);
    DBHelper::update(&(comic.info),db);

    emit dataChanged(mi,mi);

    db.close();
    QSqlDatabase::removeDatabase(_databasePath);
}


void ComicModel::updateRating(int rating, QModelIndex mi)
{
	ComicDB comic = getComic(mi);

	QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
	//TODO optimize update
	
	comic.info.rating = rating;
    _data[mi.row()]->setData(ComicModel::Rating,rating);
	DBHelper::update(&(comic.info),db);

	emit dataChanged(mi,mi);

	db.close();
	QSqlDatabase::removeDatabase(_databasePath);
}