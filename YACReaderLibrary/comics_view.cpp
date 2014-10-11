#include "comics_view.h"
#include "comic.h"

#include "QsLog.h"

ComicsView::ComicsView(QWidget *parent) :
    QWidget(parent),model(NULL)
{
    setAcceptDrops(true);
}

void ComicsView::setModel(TableModel *m)
{
    model = m;
}

void ComicsView::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urlList;

    if (event->mimeData()->hasUrls())
    {
        urlList = event->mimeData()->urls();
        foreach (QUrl url, urlList)
        {
            if(Comic::fileIsComic(url))
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void ComicsView::dropEvent(QDropEvent *event)
{
    bool accepted = false;
QLOG_DEBUG() << "drop" << event->dropAction();
    if(event->dropAction() == Qt::CopyAction)
    {
        QLOG_DEBUG() << "copy";
        emit copyComicsToCurrentFolder(filterInvalidComicFiles(event->mimeData()->urls()));

    }
    else if(event->dropAction() & Qt::MoveAction)
    {
        QLOG_DEBUG() << "move";
        emit moveComicsToCurrentFolder(filterInvalidComicFiles(event->mimeData()->urls()));
    }

    if(accepted)
        event->acceptProposedAction();
}

QList<QString> ComicsView::filterInvalidComicFiles(const QList<QUrl> &list)
{
    QList<QString> validComicFiles;
    foreach (QUrl url, list) {
        if(Comic::fileIsComic(url))
            validComicFiles << url.toLocalFile();
    }

    return validComicFiles;
}
