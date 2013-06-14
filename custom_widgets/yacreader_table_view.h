#ifndef YACREADER_TABLE_VIEW_H
#define YACREADER_TABLE_VIEW_H

#include <QTableView>

class YACReaderDeletingProgress;
class QResizeEvent;
class QPropertyAnimation;

class YACReaderTableView : public QTableView
{
    Q_OBJECT
public:
    explicit YACReaderTableView(QWidget *parent = 0);
    
signals:
    
public slots:
	void showDeleteProgress();
	void hideDeleteProgress();

private:
    YACReaderDeletingProgress * deletingProgress;
	bool showDelete;
	QPropertyAnimation * showDeletingProgressAnimation;

	void resizeEvent(QResizeEvent * event);
};

#endif // YACREADER_TABLE_VIEW_H
