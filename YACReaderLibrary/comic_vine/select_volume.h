#ifndef SELECT_VOLUME_H
#define SELECT_VOLUME_H

#include "scraper_selector.h"

class QLabel;
class VolumesModel;
class QModelIndex;
class QToolButton;
class QSortFilterProxyModel;

class ScraperScrollLabel;
class ScraperTableView;
class ScraperLineEdit;

class SelectVolume : public ScraperSelector
{
    Q_OBJECT
public:
    SelectVolume(QWidget *parent = nullptr);
    void load(const QString &json, const QString &searchString) override;
    void clearFilter();
    virtual ~SelectVolume();

public slots:
    void loadVolumeInfo(const QModelIndex &mi);
    void setCover(const QByteArray &);
    void setDescription(const QString &jsonDetail);
    QString getSelectedVolumeId();
    int getSelectedVolumeNumIssues();
    QString getSelectedVolumePublisher();

private:
    QLabel *cover;
    ScraperScrollLabel *detailLabel;
    ScraperTableView *tableVolumes;
    VolumesModel *model;
    QSortFilterProxyModel *proxyModel;
    ScraperLineEdit *filterEdit;
};

#endif // SELECT_VOLUME_H
