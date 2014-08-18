/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "static.h"
#include "staticfilecontroller.h"
#include "controllers/dumpcontroller.h"
#include "controllers/templatecontroller.h"
#include "controllers/formcontroller.h"
#include "controllers/fileuploadcontroller.h"
#include "controllers/sessioncontroller.h"

#include "controllers/librariescontroller.h"
#include "controllers/foldercontroller.h"
#include "controllers/covercontroller.h"
#include "controllers/comiccontroller.h"
#include "controllers/folderinfocontroller.h"
#include "controllers/pagecontroller.h"
#include "controllers/updatecomiccontroller.h"
#include "controllers/errorcontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "QsLog.h"

RequestMapper::RequestMapper(QObject* parent)
	:HttpRequestHandler(parent) {}

void RequestMapper::loadSession(HttpRequest & request, HttpResponse& response)
{
    HttpSession session=Static::sessionStore->getSession(request,response);
    if(session.contains("ySession")) //session is already alive check if it is needed to update comics
    {
        QString postData = QString::fromUtf8(request.getBody());
        if(postData.length()>0) {
            QList<QString> data = postData.split("\n");
            if(data.length() > 2) {
                session.setDeviceType(data.at(0).split(":").at(1));
                session.setDisplayType(data.at(1).split(":").at(1));
                QList<QString> comics = data.at(2).split(":").at(1).split("\t");
                session.clearComics();
                foreach(QString hash,comics) {
                    session.setComicOnDevice(hash);
                }
            }
            else
            {
                if(data.length()>1)
                {
                    session.setDeviceType(data.at(0).split(":").at(1));
                    session.setDisplayType(data.at(1).split(":").at(1));
                }
            }
        }
    }
    else
    {
        session.set("ySession","ok");

        session.clearNavigationPath();
        session.clearFoldersPath();

        QString postData = QString::fromUtf8(request.getBody());
        //response.writeText(postData);

        QList<QString> data = postData.split("\n");

        QLOG_INFO() << "Data lenght : " << data.length();

        if(data.length() > 2)
        {
            session.setDeviceType(data.at(0).split(":").at(1));
            session.setDisplayType(data.at(1).split(":").at(1));
            QList<QString> comics = data.at(2).split(":").at(1).split("\t");
            foreach(QString hash,comics)
            {
                session.setComicOnDevice(hash);
            }
        }
        else //values by default, only for debug purposes.
        {
            session.setDeviceType("ipad");
            session.setDisplayType("@2x");
        }

    }
}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
	QByteArray path=request.getPath();
	qDebug("RequestMapper: path=%s",path.data());

	QRegExp folder("/library/.+/folder/[0-9]+/?");//get comic content
	QRegExp folderInfo("/library/.+/folder/[0-9]+/info/?"); //get folder info
	QRegExp comic("/library/.+/comic/[0-9]+/?"); //get comic info
    QRegExp comicOpen("/library/.+/comic/[0-9]+/remote/?"); //the server will open for reading the comic
    QRegExp comicUpdate("/library/.+/comic/[0-9]+/update/?"); //get comic info
	QRegExp comicClose("/library/.+/comic/[0-9]+/close/?"); //the server will close the comic and free memory
	QRegExp cover("/library/.+/cover/[0-9a-f]+.jpg"); //get comic cover (navigation)
	QRegExp comicPage("/library/.+/comic/[0-9]+/page/[0-9]+/?"); //get comic page
    QRegExp comicPageRemote("/library/.+/comic/[0-9]+/page/[0-9]+/remote?"); //get comic page (remote reading)

    QRegExp library("/library/([0-9]+)/.+"); //permite verificar que la biblioteca solicitada existe

	path = QUrl::fromPercentEncoding(path).toLatin1();

    loadSession(request, response);

	//primera petici�n, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no est� habilitada
	if(path == "/")
	{
		LibrariesController().service(request, response);
	}

	else 
	{

        //se comprueba que la sesi�n sea la correcta con el fin de evitar accesos no autorizados
        HttpSession session=Static::sessionStore->getSession(request,response,false);
        if(!session.isNull() && session.contains("ySession"))
        {
		if(library.indexIn(path)!=-1 && DBHelper::getLibraries().contains(library.cap(1).toInt()) )
			{
				//listar el contenido del folder
				if(folder.exactMatch(path))
				{
					FolderController().service(request, response);
				}
				else if (folderInfo.exactMatch(path))
				{
					FolderInfoController().service(request, response);
				}
				else if(cover.exactMatch(path))
				{
					CoverController().service(request, response);
				}
                else if(comic.exactMatch(path) || comicOpen.exactMatch(path))
				{
                    ComicController().service(request, response);
				}
                else if(comicPage.exactMatch(path) || comicPageRemote.exactMatch(path))
				{
                    PageController().service(request,response);
                }
                else if(comicUpdate.exactMatch(path))
                {
                    UpdateComicController().service(request, response);
                }
			}
			else
			{
				//response.writeText(library.cap(1));
				Static::staticFileController->service(request, response);
			}
        }
        else //acceso no autorizado, redirecci�n
        {
            ErrorController(300).service(request,response);
        }
	}

}
