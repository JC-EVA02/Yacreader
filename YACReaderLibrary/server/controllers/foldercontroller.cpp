#include "foldercontroller.h"
#include "library_window.h"  //get libraries

#include "folder.h"

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

FolderController::FolderController() {}

void FolderController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

	HttpSession session=Static::sessionStore->getSession(request,response);



	QString y = session.get("xxx").toString();
	response.writeText(QString("session xxx : %1 <br/>").arg(y));

	Template t=Static::templateLoader->getTemplate("folder",request.getHeader("Accept-Language"));
	t.enableWarnings();
	QString path = request.getPath();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	qulonglong parentId = pathElements.at(4).toULongLong();
	QList<LibraryItem *> folderContent = mw->getFolderContentFromLibrary(libraryName,parentId);
	QList<LibraryItem *> folderComics = mw->getFolderComicsFromLibrary(libraryName,parentId);

	qulonglong backId = mw->getParentFromComicFolderId(libraryName,parentId);


	int page = 0;
	QByteArray p = request.getParameter("page");
	if(p.length() != 0)
		page = p.toInt();

	// /comicIdi/pagei/comicIdj/pagej/....../comicIdn/pagen
	//QString currentPath = session.get("currentPath").toString();
	//QStringList pathSize = currentPath.split("/").last().toInt;


	if(backId == 1 && parentId == 1)
		t.setVariable(QString("upurl"),"/?page=0");
	else
		t.setVariable(QString("upurl"),"/library/" + libraryName + "/folder/" +QString("%1").arg(backId));//.arg(upPage));		

	/*if(currentPath.length()>0)
	{
		if(currentPath.contains(QString("%1").arg(parentId))
		{

		}
		else
		{
			session.set("currentPath",currentPath+QString("/%1/%2").arg(parentId).arg(page);
		}
	}*/


	//t.loop("element",folderContent.length());

	int elementsPerPage = 10;

	int numFolders = folderContent.length();
	int numComics = folderComics.length();
	int totalLength = folderContent.length() + folderComics.length();

	int numFolderPages = numFolders / 10 + ((numFolders%10)>0?1:0);
	int numPages = totalLength / 10 + ((totalLength%10)>0?1:0);

	response.writeText(QString("Number of pages : %1 <br/>").arg(numPages));

	if(page < 0)
		page = 0;
	else if(page >= numPages)
		page = numPages-1;

	int indexCurrentPage = page*10;
	int numFoldersAtCurrentPage = qMax(0,qMin(numFolders - indexCurrentPage, 10));

	//response.writeText(QString("indexCurrentPage : %1 <br/>").arg(indexCurrentPage));
	//response.writeText(QString("numFoldersAtCurrentPage : %1 <br/>").arg(numFoldersAtCurrentPage));
	//response.writeText(QString("foldersLength : %1 <br/>").arg(folderContent.length()));

	t.loop("element",numFoldersAtCurrentPage);
	int i = 0;
	while(i<numFoldersAtCurrentPage)
	{
		t.setVariable(QString("element%1.name").arg(i),folderContent.at(i + (page*10))->name);
		t.setVariable(QString("element%1.url").arg(i),"/library/"+libraryName+"/folder/"+QString("%1").arg(folderContent.at(i + (page*10))->id));
		t.setVariable(QString("element%1.downloadurl").arg(i),"/library/"+libraryName+"/folder/"+QString("%1/info").arg(folderContent.at(i + (page*10))->id));
		i++;
	}

	int comicsOffset;// = qMax(0,((page - (numFolderPages - 1)) * 10) - (numFolders%10));

	int comicPage = numFolderPages!=0?page-(numFolderPages - 1):page;

	if(comicPage > 0)
	{
		comicsOffset = 10 - (numFolders%10);
		comicsOffset += (comicPage-1) *10;
	}
	else
		comicsOffset = 0;

	

	int globalComicsOffset = 10 - (numFolders%10);
	int numComicsAtCurrentPage = 0;

	if(comicPage == 0) //primera p�gina de los c�mics
			numComicsAtCurrentPage = qMin(globalComicsOffset,numComics);
		else if (page == (numPages-1)) //�ltima p�gina de los c�mics
			numComicsAtCurrentPage = 10-globalComicsOffset + (numComics%10);
		else
			numComicsAtCurrentPage = 10 - numFoldersAtCurrentPage;

	if(numComics == 0)
		numComicsAtCurrentPage = 0;
	response.writeText(QString("numComicsAtCurrentPage : %1 <br/>").arg(numComicsAtCurrentPage));
	response.writeText(QString("comicsOffset : %1 <br/>").arg(comicsOffset));

	t.loop("elementcomic",numComicsAtCurrentPage);
	//
	int j = 0;

	while(j<numComicsAtCurrentPage)
	{
		const ComicDB * comic = (ComicDB *)folderComics.at(j+comicsOffset);
		//if(comic->info.title == 0 || comic->info.title->isEmpty())
			t.setVariable(QString("elementcomic%1.name").arg(j),comic->name);
		//else
		//	t.setVariable(QString("elementcomic%1.name").arg(i),*comic->info.title);
		t.setVariable(QString("elementcomic%1.url").arg(j),"/library/"+libraryName+"/comic/"+QString("%1").arg(comic->id));
		t.setVariable(QString("elementcomic%1.coverulr").arg(j),"/library/"+libraryName+"/cover/"+QString("%1").arg(comic->info.hash + ".jpg"));
		j++;
	}

	if(numPages > 1)
	{
	t.loop("page",numPages);
	int z = 0;
	while(z < numPages)
	{
		
		t.setVariable(QString("page%1.url").arg(z),"/library/"+libraryName+"/folder/"+QString("%1").arg(parentId)+QString("?page=%1").arg(z));
		if(page == z)
			t.setVariable(QString("page%1.number").arg(z),QString("<strong>%1</strong>").arg(z));
		else
			t.setVariable(QString("page%1.number").arg(z),QString("%1").arg(z));
		z++;
	}
	}
	else
		t.loop("page",0);

	response.write(t.toLatin1(),true);

}