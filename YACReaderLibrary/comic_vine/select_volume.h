#ifndef SELECT_VOLUME_H
#define SELECT_VOLUME_H

#include <QWidget>

class QLabel;
class QTableView;
class VolumesModel;

class SelectVolume : public QWidget
{
	Q_OBJECT
public:
	SelectVolume(QWidget * parent = 0);
	void load(const QString & json);
	virtual ~SelectVolume();
private:
	QLabel * cover;
	QLabel * detailLabel;
	QTableView * tableVolumes;
	VolumesModel * model;
};

#endif // SELECT_VOLUME_H