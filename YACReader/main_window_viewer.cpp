#include "main_window_viewer.h"
#include "configuration.h"
#include "viewer.h"
#include "goto_dialog.h"
#include "custom_widgets.h"
#include "options_dialog.h"
#include "check_new_version.h"
#include "comic.h"
#include "bookmarks_dialog.h"
#include "shortcuts_dialog.h"
#include "width_slider.h"

MainWindowViewer::MainWindowViewer()
:QMainWindow(),fullscreen(false),toolbars(true),alwaysOnTop(false),currentDirectory("."),currentDirectoryImgDest(".")
{
	loadConfiguration();
	setupUI();
}

void MainWindowViewer::loadConfiguration()
{
	settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);

	Configuration & config = Configuration::getConfiguration();
	config.load(settings);
	currentDirectory = config.getDefaultPath();
	fullscreen = config.getFullScreen();
}

void MainWindowViewer::setupUI()
{
	setWindowIcon(QIcon(":/images/icon.png"));

	viewer = new Viewer(this);
	connect(viewer,SIGNAL(reset()),this,SLOT(disableActions()));

	setCentralWidget(viewer);
	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int height,width;
	height = static_cast<int>(heightDesktopResolution*0.84);
	width = static_cast<int>(height*0.70);
	Configuration & conf = Configuration::getConfiguration();
	QPoint p = conf.getPos();
	QSize s = conf.getSize();
	if(s.width()!=0)
	{
		move(p);
		resize(s);
	}
	else
	{
		move(QPoint((widthDesktopResolution-width)/2,((heightDesktopResolution-height)-40)/2));
		resize(QSize(width,height));
	}

	had = new HelpAboutDialog(this); //TODO load data

	had->loadAboutInformation(":/files/about.html");
	had->loadHelp(":/files/helpYACReader.html");

	optionsDialog = new OptionsDialog(this);
	connect(optionsDialog,SIGNAL(accepted()),viewer,SLOT(updateOptions()));
	connect(optionsDialog,SIGNAL(fitToWidthRatioChanged(float)),viewer,SLOT(updateFitToWidthRatio(float)));
	connect(optionsDialog, SIGNAL(optionsChanged()),this,SLOT(reloadOptions()));
	connect(optionsDialog,SIGNAL(changedImageOptions()),viewer,SLOT(updateImageOptions()));

	optionsDialog->restoreOptions(settings);
	shortcutsDialog = new ShortcutsDialog(this);

	createActions();
	createToolBars();

	setWindowTitle("YACReader");

	if(QCoreApplication::argc()>1)
	{
		//TODO: new method open(QString)
		QString pathFile = QCoreApplication::arguments().at(1);
		currentDirectory = pathFile;
		QFileInfo fi(pathFile);
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
		
		enableActions();
		
		viewer->open(pathFile);
		
	}

	versionChecker = new HttpVersionChecker();

	connect(versionChecker,SIGNAL(newVersionDetected()),
		this,SLOT(newVersion()));

	versionChecker->get();
	
	viewer->setFocusPolicy(Qt::StrongFocus);
	

	if(Configuration::getConfiguration().getAlwaysOnTop())
	{
		setWindowFlags(this->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	}

	if(fullscreen)
		toFullScreen();
	if(conf.getMaximized())
		showMaximized();

	setAcceptDrops(true);

	if(Configuration::getConfiguration().getShowToolbars() && !Configuration::getConfiguration().getFullScreen())
		showToolBars();
	else
		hideToolBars();
}

void MainWindowViewer::createActions()
{
	openAction = new QAction(tr("&Open"),this);
	openAction->setShortcut(tr("O"));
	openAction->setIcon(QIcon(":/images/open.png"));
	openAction->setToolTip(tr("Open a comic"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	openFolderAction = new QAction(tr("Open Folder"),this);
	openFolderAction->setShortcut(tr("Ctrl+O"));
	openFolderAction->setIcon(QIcon(":/images/openFolder.png"));
	openFolderAction->setToolTip(tr("Open image folder"));
	connect(openFolderAction, SIGNAL(triggered()), this, SLOT(openFolder()));

	saveImageAction = new QAction(tr("Save"),this);
	saveImageAction->setIcon(QIcon(":/images/save.png"));
	saveImageAction->setToolTip(tr("Save current page"));
	saveImageAction->setDisabled(true);
	connect(saveImageAction,SIGNAL(triggered()),this,SLOT(saveImage()));

	openPreviousComicAction = new QAction(tr("Previous Comic"),this);
	openPreviousComicAction->setIcon(QIcon(":/images/previousComic.png"));
	openPreviousComicAction->setShortcut(Qt::CTRL + Qt::Key_Left);
	openPreviousComicAction->setToolTip(tr("Open previous comic"));
	openPreviousComicAction->setDisabled(true);
	connect(openPreviousComicAction,SIGNAL(triggered()),this,SLOT(openPreviousComic()));

	openNextComicAction = new QAction(tr("Next Comic"),this);
	openNextComicAction->setIcon(QIcon(":/images/nextComic.png"));
	openNextComicAction->setShortcut(Qt::CTRL + Qt::Key_Right);
	openNextComicAction->setToolTip(tr("Open next comic"));
	openNextComicAction->setDisabled(true);
	connect(openNextComicAction,SIGNAL(triggered()),this,SLOT(openNextComic()));

	prevAction = new QAction(tr("&Previous"),this);
	prevAction->setIcon(QIcon(":/images/prev.png"));
	prevAction->setShortcut(Qt::Key_Left);
	prevAction->setToolTip(tr("Go to previous page"));
	prevAction->setDisabled(true);
	connect(prevAction, SIGNAL(triggered()),viewer,SLOT(prev()));

	nextAction = new QAction(tr("&Next"),this);
	nextAction->setIcon(QIcon(":/images/next.png"));
	nextAction->setShortcut(Qt::Key_Right);
	nextAction->setToolTip(tr("Go to next page"));
	nextAction->setDisabled(true);
	connect(nextAction, SIGNAL(triggered()),viewer,SLOT(next()));

	adjustWidth = new QAction(tr("Fit Width"),this);
	adjustWidth->setShortcut(tr("A"));
	adjustWidth->setIcon(QIcon(":/images/fit.png"));
	adjustWidth->setCheckable(true);
	adjustWidth->setDisabled(true);
	adjustWidth->setChecked(Configuration::getConfiguration().getAdjustToWidth());
	adjustWidth->setToolTip(tr("Fit image to ..."));
	//adjustWidth->setIcon(QIcon(":/images/fitWidth.png"));
	connect(adjustWidth, SIGNAL(triggered()),this,SLOT(changeFit()));

	leftRotationAction = new QAction(tr("Rotate image to the left"),this);
	leftRotationAction->setShortcut(tr("L"));
	leftRotationAction->setIcon(QIcon(":/images/rotateL.png"));
	leftRotationAction->setDisabled(true);
	connect(leftRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateLeft()));

	rightRotationAction = new QAction(tr("Rotate image to the right"),this);
	rightRotationAction->setShortcut(tr("R"));
	rightRotationAction->setIcon(QIcon(":/images/rotateR.png"));
	rightRotationAction->setDisabled(true);
	connect(rightRotationAction, SIGNAL(triggered()),viewer,SLOT(rotateRight()));

	doublePageAction = new QAction(tr("Double page mode"),this);
	doublePageAction->setToolTip(tr("Switch to double page mode"));
	doublePageAction->setShortcut(tr("D"));
	doublePageAction->setIcon(QIcon(":/images/doublePage.png"));
	doublePageAction->setDisabled(true);
	doublePageAction->setCheckable(true);
	doublePageAction->setChecked(Configuration::getConfiguration().getDoublePage());
	connect(doublePageAction, SIGNAL(triggered()),viewer,SLOT(doublePageSwitch()));

	goToPage = new QAction(tr("Go To"),this);
	goToPage->setShortcut(tr("G"));
	goToPage->setIcon(QIcon(":/images/goto.png"));
	goToPage->setDisabled(true);
	goToPage->setToolTip(tr("Go to page ..."));
	connect(goToPage, SIGNAL(triggered()),viewer,SLOT(showGoToDialog()));

	optionsAction = new QAction(tr("Options"),this);
	optionsAction->setShortcut(tr("C"));
	optionsAction->setToolTip(tr("YACReader options"));
	optionsAction->setIcon(QIcon(":/images/options.png"));

	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));

	helpAboutAction = new QAction(tr("Help"),this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
	helpAboutAction->setShortcut(Qt::Key_F1);
	helpAboutAction->setIcon(QIcon(":/images/help.png"));
	connect(helpAboutAction, SIGNAL(triggered()),had,SLOT(show()));

	showMagnifyingGlass = new QAction(tr("Magnifying glass"),this);
	showMagnifyingGlass->setToolTip(tr("Switch Magnifying glass"));
	showMagnifyingGlass->setShortcut(tr("Z"));
	showMagnifyingGlass->setIcon(QIcon(":/images/zoom.png"));
	showMagnifyingGlass->setDisabled(true);
	showMagnifyingGlass->setCheckable(true);
	connect(showMagnifyingGlass, SIGNAL(triggered()),viewer,SLOT(magnifyingGlassSwitch()));

	setBookmark = new QAction(tr("Set bookmark"),this);
	setBookmark->setToolTip(tr("Set a bookmark on the current page"));
	setBookmark->setShortcut(Qt::CTRL+Qt::Key_M);
	setBookmark->setIcon(QIcon(":/images/setBookmark.png"));
	setBookmark->setDisabled(true);
	setBookmark->setCheckable(true);
	connect(setBookmark,SIGNAL(triggered (bool)),viewer,SLOT(setBookmark(bool)));
	connect(viewer,SIGNAL(pageAvailable(bool)),setBookmark,SLOT(setEnabled(bool)));
	connect(viewer,SIGNAL(pageIsBookmark(bool)),setBookmark,SLOT(setChecked(bool)));

	showBookmarks = new QAction(tr("Show bookmarks"),this);
	showBookmarks->setToolTip(tr("Show the bookmarks of the current comic"));
	showBookmarks->setShortcut(tr("M"));
	showBookmarks->setIcon(QIcon(":/images/bookmark.png"));
	showBookmarks->setDisabled(true);
	connect(showBookmarks, SIGNAL(triggered()),viewer->getBookmarksDialog(),SLOT(show()));

	showShorcutsAction = new QAction(tr("Show keyboard shortcuts"), this );
	showShorcutsAction->setIcon(QIcon(":/images/shortcuts.png"));
	connect(showShorcutsAction, SIGNAL(triggered()),shortcutsDialog,SLOT(show()));

	showInfo = new QAction(tr("Show Info"),this);
	showInfo->setShortcut(tr("I"));
	showInfo->setIcon(QIcon(":/images/properties.png"));
	showInfo->setDisabled(true);
	connect(showInfo, SIGNAL(triggered()),viewer,SLOT(informationSwitch()));

	closeAction = new QAction(tr("Close"),this);
	closeAction->setShortcut(Qt::Key_Escape);
	closeAction->setIcon(QIcon(":/images/deleteLibrary.png"));
	connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

	showDictionaryAction = new QAction(tr("Show Dictionary"),this);
	showDictionaryAction->setShortcut(Qt::Key_T);
	showDictionaryAction->setIcon(QIcon(":/images/dictionary.png"));
	showDictionaryAction->setCheckable(true);
	showDictionaryAction->setDisabled(true);
	connect(showDictionaryAction,SIGNAL(triggered()),viewer,SLOT(translatorSwitch()));

	alwaysOnTopAction = new QAction(tr("Always on top"),this);
	alwaysOnTopAction->setShortcut(Qt::Key_Q);
	alwaysOnTopAction->setIcon(QIcon(":/images/alwaysOnTop.png"));
	alwaysOnTopAction->setCheckable(true);
	alwaysOnTopAction->setDisabled(true);
	alwaysOnTopAction->setChecked(Configuration::getConfiguration().getAlwaysOnTop());
	connect(alwaysOnTopAction,SIGNAL(triggered()),this,SLOT(alwaysOnTopSwitch()));

	adjustToFullSizeAction = new QAction(tr("Show full size"),this);
	adjustToFullSizeAction->setShortcut(Qt::Key_W);
	adjustToFullSizeAction->setIcon(QIcon(":/images/adjustToFullSize.png"));
	adjustToFullSizeAction->setCheckable(true);
	adjustToFullSizeAction->setDisabled(true);
	adjustToFullSizeAction->setChecked(Configuration::getConfiguration().getAdjustToFullSize());
	connect(adjustToFullSizeAction,SIGNAL(triggered()),this,SLOT(adjustToFullSizeSwitch()));
}

void MainWindowViewer::createToolBars()
{
	comicToolBar = addToolBar(tr("&File"));

	QToolButton * tb = new QToolButton();
	tb->addAction(openAction);
	tb->addAction(openFolderAction);
	tb->setPopupMode(QToolButton::MenuButtonPopup);
	tb->setDefaultAction(openAction);

	comicToolBar->addWidget(tb);
	comicToolBar->addAction(saveImageAction);
	comicToolBar->addAction(openPreviousComicAction);
	comicToolBar->addAction(openNextComicAction);

	comicToolBar->addSeparator();
	comicToolBar->addAction(prevAction);
	comicToolBar->addAction(nextAction);
	comicToolBar->addAction(goToPage);

	comicToolBar->addSeparator();
	comicToolBar->addAction(alwaysOnTopAction);

	comicToolBar->addSeparator();

	//QWidget * widget = new QWidget();

	//QToolButton * tbW = new QToolButton(widget);
	//tbW->addAction(adjustWidth);
	//tbW->setPopupMode(QToolButton::MenuButtonPopup);
	//tbW->setDefaultAction(adjustWidth);

	//QHBoxLayout *layout = new QHBoxLayout;
	//layout->addWidget(tbW);
	//layout->setContentsMargins(0,0,0,0);
	//widget->setLayout(layout);
	//widget->setContentsMargins(0,0,0,0);

	//comicToolBar->addWidget(widget);

	//comicToolBar->addAction(adjustWidth);
	QMenu * menu = new QMenu();
	sliderAction = new YACReaderSliderAction(this);
	menu->setAutoFillBackground(false);
	menu->setStyleSheet(" QMenu {background:transparent; border: 0px;padding: 0px; }"
		);
	menu->addAction(sliderAction);
		QToolButton * tb2 = new QToolButton();
	tb2->addAction(adjustWidth);
	tb2->setMenu(menu);

	connect(sliderAction,SIGNAL(fitToWidthRatioChanged(float)),viewer,SLOT(updateFitToWidthRatio(float)));

	//tb2->addAction();
	tb2->setPopupMode(QToolButton::MenuButtonPopup);
	tb2->setDefaultAction(adjustWidth);
	comicToolBar->addWidget(tb2);
	comicToolBar->addAction(adjustToFullSizeAction);
	comicToolBar->addAction(leftRotationAction);
	comicToolBar->addAction(rightRotationAction);
    comicToolBar->addAction(doublePageAction);

	comicToolBar->addSeparator();
	comicToolBar->addAction(showMagnifyingGlass);

	comicToolBar->addSeparator();
	comicToolBar->addAction(setBookmark);
	comicToolBar->addAction(showBookmarks);
	
	comicToolBar->addSeparator();
	comicToolBar->addAction(showDictionaryAction);
	
	comicToolBar->addWidget(new QToolBarStretch());

	comicToolBar->addAction(showShorcutsAction);
	comicToolBar->addAction(optionsAction);
	comicToolBar->addAction(helpAboutAction);
	comicToolBar->addAction(closeAction);

	comicToolBar->setMovable(false);


	viewer->addAction(openAction);
	viewer->addAction(openFolderAction);
	viewer->addAction(saveImageAction);
	viewer->addAction(openPreviousComicAction);
	viewer->addAction(openNextComicAction);
	QAction * separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	viewer->addAction(prevAction);
	viewer->addAction(nextAction);
	viewer->addAction(goToPage);
	viewer->addAction(adjustWidth);
	viewer->addAction(adjustToFullSizeAction);
	viewer->addAction(leftRotationAction);
	viewer->addAction(rightRotationAction);
        viewer->addAction(doublePageAction);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	
	viewer->addAction(setBookmark);
	viewer->addAction(showBookmarks);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);

	viewer->addAction(showDictionaryAction);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);

	viewer->addAction(showMagnifyingGlass);
	viewer->addAction(optionsAction);
	viewer->addAction(helpAboutAction);
	viewer->addAction(showInfo);
	viewer->addAction(showShorcutsAction);
	separator = new QAction("",this);
	separator->setSeparator(true);
	viewer->addAction(separator);
	viewer->addAction(closeAction);

	viewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MainWindowViewer::reloadOptions()
{
	viewer->updateConfig(settings);
}

void MainWindowViewer::open()
{
	QFileDialog openDialog;
	QString pathFile = openDialog.getOpenFileName(this,tr("Open Comic"),currentDirectory,tr("Comic files (*.cbr *.cbz *.rar *.zip *.pdf *.tar *.arj)"));
	if (!pathFile.isEmpty())
	{
		openComicFromPath(pathFile);
	}
}

void MainWindowViewer::openComicFromPath(QString pathFile)
{
	currentDirectory = pathFile;
	QFileInfo fi(pathFile);
	getSiblingComics(fi.absolutePath(),fi.fileName());

	setWindowTitle("YACReader - " + fi.fileName());

	enableActions();

	viewer->open(pathFile);
	
}

void MainWindowViewer::openFolder()
{
	QFileDialog openDialog;
	QString pathDir = openDialog.getExistingDirectory(this,tr("Open folder"),currentDirectory);
	if (!pathDir.isEmpty())
	{
		openFolderFromPath(pathDir);
	}
}

void MainWindowViewer::openFolderFromPath(QString pathDir)
{
	currentDirectory = pathDir; //TODO ??
	QFileInfo fi(pathDir);
	getSiblingComics(fi.absolutePath(),fi.fileName());

	setWindowTitle("YACReader - " + fi.fileName());

	enableActions();

	viewer->open(pathDir);
	
}

void MainWindowViewer::saveImage()
{
	QFileDialog saveDialog;
	QString pathFile = saveDialog.getSaveFileName(this,tr("Save current page"),currentDirectoryImgDest,tr("Image files (*.jpg)"));
	if (!pathFile.isEmpty())
	{
		currentDirectoryImgDest = pathFile;
		const QPixmap * p = viewer->pixmap();
		if(p!=NULL)
			p->save(pathFile);
	}
}

void MainWindowViewer::enableActions()
{
	saveImageAction->setDisabled(false);
	prevAction->setDisabled(false);
	nextAction->setDisabled(false);
	adjustWidth->setDisabled(false);
	goToPage->setDisabled(false);
	alwaysOnTopAction->setDisabled(false);
	leftRotationAction->setDisabled(false);
	rightRotationAction->setDisabled(false);
	showMagnifyingGlass->setDisabled(false);
    doublePageAction->setDisabled(false);
	adjustToFullSizeAction->setDisabled(false);
	//setBookmark->setDisabled(false);
	showBookmarks->setDisabled(false);
	showInfo->setDisabled(false); //TODO enable goTo and showInfo (or update) when numPages emited
	showDictionaryAction->setDisabled(false);
}
void MainWindowViewer::disableActions()
{
    saveImageAction->setDisabled(true);
    prevAction->setDisabled(true);
    nextAction->setDisabled(true);
    adjustWidth->setDisabled(true);
    goToPage->setDisabled(true);
	alwaysOnTopAction->setDisabled(true);
    leftRotationAction->setDisabled(true);
    rightRotationAction->setDisabled(true);
    showMagnifyingGlass->setDisabled(true);
    doublePageAction->setDisabled(true);
	adjustToFullSizeAction->setDisabled(true);
    setBookmark->setDisabled(true);
    showBookmarks->setDisabled(true);
    showInfo->setDisabled(true); //TODO enable goTo and showInfo (or update) when numPages emited
    openPreviousComicAction->setDisabled(true);
    openNextComicAction->setDisabled(true);
	showDictionaryAction->setDisabled(true);
}

void MainWindowViewer::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		this->close();
		break;
	case Qt::Key_F:   
		toggleFullScreen();
		break;
	case Qt::Key_H:   
		toggleToolBars();
		break;
	case Qt::Key_O:   
		open();
		break;
	default:
		QWidget::keyPressEvent(event);
		break;
	}
}

void MainWindowViewer::mouseDoubleClickEvent ( QMouseEvent * event )
{
	toggleFullScreen();
	event->accept();
}

void MainWindowViewer::toggleFullScreen()
{
	fullscreen?toNormal():toFullScreen();
	Configuration::getConfiguration().setFullScreen(fullscreen = !fullscreen);
}

void MainWindowViewer::toFullScreen()
{
	hideToolBars();
	viewer->hide();
	viewer->fullscreen = true;//TODO, change by the right use of windowState();
	showFullScreen();
	viewer->show();
	if(viewer->magnifyingGlassIsVisible())
		viewer->showMagnifyingGlass();
}

void MainWindowViewer::toNormal()
{
	//show all
	viewer->hide();
	viewer->fullscreen = false;//TODO, change by the right use of windowState();
	//viewer->hideMagnifyingGlass();
	showNormal();
	if(Configuration::getConfiguration().getShowToolbars())
		showToolBars();
	viewer->show();
	if(viewer->magnifyingGlassIsVisible())
		viewer->showMagnifyingGlass();
}
void MainWindowViewer::toggleToolBars()
{
	toolbars?hideToolBars():showToolBars();

	Configuration::getConfiguration().setShowToolbars(toolbars);
}
void MainWindowViewer::hideToolBars()
{
	//hide all
	this->comicToolBar->hide();
	toolbars = false;
}

void MainWindowViewer::showToolBars()
{
	this->comicToolBar->show();
	toolbars = true;
}

void MainWindowViewer::changeFit()
{
	Configuration & conf = Configuration::getConfiguration();
	conf.setAdjustToWidth(!conf.getAdjustToWidth());
	viewer->updatePage();
}

void MainWindowViewer::newVersion()
{
	QMessageBox msgBox;
	msgBox.setText(tr("There is a new version avaliable"));
	msgBox.setInformativeText(tr("Do you want to download the new version?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	int ret = msgBox.exec();

	if(ret==QMessageBox::Yes){
		QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));
	}
}





void MainWindowViewer::closeEvent ( QCloseEvent * event )
{
	viewer->save();
	Configuration & conf = Configuration::getConfiguration();
	if(!fullscreen && !isMaximized())
	{
		conf.setPos(pos());
		conf.setSize(size());
	}
	conf.setMaximized(isMaximized());
}

void MainWindowViewer::openPreviousComic()
{
	if(!previousComicPath.isEmpty())
	{
		viewer->open(previousComicPath);
		QFileInfo fi(previousComicPath);
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
	}
}

void MainWindowViewer::openNextComic()
{
	if(!nextComicPath.isEmpty())
	{
		viewer->open(nextComicPath);
		QFileInfo fi(nextComicPath);
		getSiblingComics(fi.absolutePath(),fi.fileName());

		setWindowTitle("YACReader - " + fi.fileName());
	}
}

void MainWindowViewer::getSiblingComics(QString path,QString currentComic)
{
	QDir d(path);
	d.setFilter(QDir::Files|QDir::NoDotAndDotDot);
	d.setNameFilters(QStringList() << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar" << "*.pdf");
	d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QStringList list = d.entryList();
	int index = list.indexOf(currentComic);
        if(index == -1) //comic not found
        {
            QFile f(QCoreApplication::applicationDirPath()+"/errorLog.txt");
            if(!f.open(QIODevice::WriteOnly))
            {
                QMessageBox::critical(NULL,tr("Saving error log file...."),tr("There was a problem saving YACReader error log file. Please, check if you have enough permissions in the YACReader root folder."));
            }
            else
            {
            QTextStream txtS(&f);
            txtS << "METHOD : MainWindowViewer::getSiblingComics" << '\n';
            txtS << "ERROR : current comic not found in its own path" << '\n';
            txtS << path << '\n';
            txtS << currentComic << '\n';
            txtS << "Comic list count : " + list.count() << '\n';
            foreach(QString s, list){
                txtS << s << '\n';
            }
            f.close();
        }
        }

	previousComicPath = nextComicPath = "";
	if(index>0)
	{
		previousComicPath = path+"/"+list.at(index-1);
		openPreviousComicAction->setDisabled(false);
	}
	else
		openPreviousComicAction->setDisabled(true);

	if(index+1<list.count())
	{
		nextComicPath = path+"/"+list.at(index+1);
		openNextComicAction->setDisabled(false);
	}
	else
		openNextComicAction->setDisabled(true);
}

void MainWindowViewer::dropEvent(QDropEvent *event)
{
	QList<QUrl> urlList;
    QString fName;
    QFileInfo info;
 
    if (event->mimeData()->hasUrls())
    {
        urlList = event->mimeData()->urls();
    
        if ( urlList.size() > 0)
        {
            fName = urlList[0].toLocalFile(); // convert first QUrl to local path
            info.setFile( fName ); // information about file
            if (info.isFile()) 
				openComicFromPath(fName); // if is file, setText
			else 
				if(info.isDir())
					openFolderFromPath(fName);
        }
    }

	event->acceptProposedAction();
}
void MainWindowViewer::dragEnterEvent(QDragEnterEvent *event)
{
    // accept just text/uri-list mime format
    if (event->mimeData()->hasFormat("text/uri-list")) 
    {     
        event->acceptProposedAction();
    }
}

void MainWindowViewer::alwaysOnTopSwitch()
{
	if(!Configuration::getConfiguration().getAlwaysOnTop())
	{
		setWindowFlags(this->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint); //always on top
		show();
	}
	else
	{
		setWindowFlags(this->windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
		show();
	}
	Configuration::getConfiguration().setAlwaysOnTop(!Configuration::getConfiguration().getAlwaysOnTop());
}

void MainWindowViewer::adjustToFullSizeSwitch()
{
	Configuration::getConfiguration().setAdjustToFullSize(!Configuration::getConfiguration().getAdjustToFullSize());
	viewer->updatePage();
}