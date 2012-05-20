#include "library_window.h"
#include "custom_widgets.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QDir>
#include <QDirModel>
#include <QHeaderView>
#include <QProcess>
#include <QtCore>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QMatrix>

#include <iterator>

//

LibraryWindow::LibraryWindow()
:QMainWindow(),skip(0),fullscreen(false),fetching(false)
{
	setupUI();
	loadLibraries();
}

void LibraryWindow::setupUI()
{
	createLibraryDialog = new CreateLibraryDialog(this);
	updateLibraryDialog = new UpdateLibraryDialog(this);
	renameLibraryDialog = new RenameLibraryDialog(this);
	propertiesDialog = new PropertiesDialog(this);
	exportLibraryDialog = new ExportLibraryDialog(this);
	importLibraryDialog = new ImportLibraryDialog(this);

	libraryCreator = new LibraryCreator();
	packageManager = new PackageManager();

	addLibraryDialog = new AddLibraryDialog(this);

	QSplitter * sVertical = new QSplitter(Qt::Vertical);
	QSplitter * sHorizontal = new QSplitter(Qt::Horizontal);
	//TODO: flowType is a global variable
	optionsDialog = new OptionsDialog(this);
	optionsDialog->restoreOptions();
    comicFlow = new ComicFlow(0,flowType);

	comicFlow->setFocusPolicy(Qt::StrongFocus);
	comicFlow->setShowMarks(true);
	QMatrix m;
	m.rotate(-90);
	m.scale(-1,1);
	comicFlow->setMarkImage(QImage(":/images/setRead.png").transformed(m,Qt::SmoothTransformation));
	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int height,width;
	height = heightDesktopResolution*0.39;
	width = height*0.65;
	slideSizeW = QSize(width,height);
	height = heightDesktopResolution*0.55;
	width = height*0.70;
	slideSizeF = QSize(width,height);
	comicFlow->setSlideSize(slideSizeW);
	setFocusProxy(comicFlow);

	comicView = new QListView;
	foldersView = new QTreeView;


	sVertical->addWidget(comicFlow);
	sVertical->addWidget(comicView);
	/*sVertical->setStretchFactor(0,1);
	sVertical->setStretchFactor(1,0);
	*/

	left = new QWidget;
	QVBoxLayout * l = new QVBoxLayout;
	selectedLibrary = new QComboBox;
	l->setContentsMargins(2,2,0,0);
	l->addWidget(new QLabel(tr("Select a library:")));
	l->addWidget(selectedLibrary);
	treeActions = new QToolBar(left);
	treeActions->setIconSize(QSize(16,16));
	l->addWidget(treeActions);
	l->addWidget(foldersView);
	
	QVBoxLayout * searchLayout = new QVBoxLayout;
	
	QHBoxLayout * filter = new QHBoxLayout;
	filter->addWidget(foldersFilter = new QLineEdit());
	previousFilter = "";
	filter->addWidget(clearFoldersFilter = new QPushButton(tr("Clear")));
	
	searchLayout->addWidget(new QLabel(tr("Search folders/comics"),this));

	searchLayout->addLayout(filter);
	searchLayout->addWidget(includeComicsCheckBox = new QCheckBox(tr("Include files (slower)"),this));
	
	l->addLayout(searchLayout);
	l->setSpacing(1);
	left->setLayout(l);

	sHorizontal->addWidget(left);
	sHorizontal->addWidget(sVertical);
	sHorizontal->setStretchFactor(0,0);
	sHorizontal->setStretchFactor(1,1);
	setCentralWidget(sHorizontal);

	//dirmodels
	dm = new QFileSystemModel();
	dm->setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
	dm->sort(0);//,QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	dmCV =  new QFileSystemModel();
	dmCV->setNameFilters(QStringList() << "*jpg");
	dmCV->setFilter(QDir::Files|QDir::CaseSensitive|QDir::NoDotAndDotDot);//,QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	dmCV->setNameFilterDisables(true);

	dm->setReadOnly(true);
	//dm->setLazyChildCount(false);
	dmCV->setReadOnly(true);

	dm->setIconProvider(&fip);
	dmCV->setIconProvider(&fip);
	//dmCV->sort(1);

	proxyFilter = new  YACReaderTreeSearch();
	proxyFilter->setSourceModel(dm);
	proxyFilter->setFilterRole(Qt::DisplayRole);

	/*proxySort = new YACReaderSortComics();
	proxySort->setSourceModel(dmCV);
	proxySort->setFilterRole(Qt::DisplayRole);*/

	//views
	foldersView->header()->hideSection(1);
	foldersView->header()->hideSection(2);
	foldersView->header()->hideSection(3);
	foldersView->header()->adjustSize();
	foldersView->header()->hide();
	foldersView->setAnimated(true);
	foldersView->setContextMenuPolicy(Qt::ActionsContextMenu);
	foldersView->setContextMenuPolicy(Qt::ActionsContextMenu);

	comicView->setAlternatingRowColors(true);
	comicView->setItemDelegate(new YACReaderComicViewDelegate());
	comicView->setContextMenuPolicy(Qt::ActionsContextMenu);

	had = new HelpAboutDialog(this); //TODO load data.
	QString sufix = QLocale::system().name();
	if(QFile(":/files/about_"+sufix+".html").exists())
		had->loadAboutInformation(":/files/about_"+sufix+".html");
	else
		had->loadAboutInformation(":/files/about.html");

	if(QFile(":/files/helpYACReaderLibrary_"+sufix+".html").exists())
		had->loadHelp(":/files/helpYACReaderLibrary_"+sufix+".html");
	else
		had->loadHelp(":/files/helpYACReaderLibrary.html");

        fullScreenToolTip = new QLabel(this);
        fullScreenToolTip->setText(tr("<font color='white'> press 'F' to close fullscreen mode </font>"));
        fullScreenToolTip->setPalette(QPalette(QColor(0,0,0)));
        fullScreenToolTip->setFont(QFont("courier new",15,234));
        fullScreenToolTip->setAutoFillBackground(true);
        fullScreenToolTip->hide();
        fullScreenToolTip->adjustSize();

	createActions();
	createToolBars();
	createMenus();
	createConnections();

	setFoldersFilter("");

	comicFlow->setFocus(Qt::OtherFocusReason);

	setWindowTitle(tr("YACReader Library"));
}

void LibraryWindow::createActions()
{
	createLibraryAction = new QAction(this);
	createLibraryAction->setToolTip(tr("Create a new library"));
	createLibraryAction->setShortcut(Qt::Key_C);
	createLibraryAction->setIcon(QIcon(":/images/new.png"));

	openLibraryAction = new QAction(this);
	openLibraryAction->setToolTip(tr("Open an existing library"));
	openLibraryAction->setShortcut(Qt::Key_O);
	openLibraryAction->setIcon(QIcon(":/images/openLibrary.png"));

	exportLibraryAction = new QAction(this);
	exportLibraryAction->setToolTip(tr("Pack the covers of the selected library"));
	exportLibraryAction->setIcon(QIcon(":/images/exportLibrary.png"));

	importLibraryAction = new QAction(this);
	importLibraryAction->setToolTip(tr("Unpack a catalog"));
	importLibraryAction->setIcon(QIcon(":/images/importLibrary.png"));

	updateLibraryAction = new QAction(this);
	updateLibraryAction->setToolTip(tr("Update current library"));
	updateLibraryAction->setShortcut(Qt::Key_U);
	updateLibraryAction->setIcon(QIcon(":/images/updateLibrary.png"));

	renameLibraryAction = new QAction(this);
	renameLibraryAction->setToolTip(tr("Rename current library"));
	renameLibraryAction->setShortcut(Qt::Key_R);
	renameLibraryAction->setIcon(QIcon(":/images/edit.png"));

	deleteLibraryAction = new QAction(this);
	deleteLibraryAction->setToolTip(tr("Delete current library from disk"));
	deleteLibraryAction->setIcon(QIcon(":/images/deleteLibrary.png"));

	removeLibraryAction = new QAction(this);
	removeLibraryAction->setToolTip(tr("Remove current library from your collection"));
	removeLibraryAction->setIcon(QIcon(":/images/removeLibrary.png"));

	openComicAction = new QAction(this);
	openComicAction->setToolTip(tr("Open current comic on YACReader"));
	openComicAction->setShortcut(Qt::Key_Return);
	openComicAction->setIcon(QIcon(":/images/icon.png"));
	
	setAsReadAction = new QAction(tr("Set as read"),this);
	setAsReadAction->setToolTip(tr("Set comic as read"));
	setAsReadAction->setIcon(QIcon(":/images/setRead.png"));
	
	setAsNonReadAction = new QAction(tr("Set as unread"),this);
	setAsNonReadAction->setToolTip(tr("Set comic as unread"));
	setAsNonReadAction->setIcon(QIcon(":/images/setUnread.png"));
	
	setAllAsReadAction = new QAction(tr("Set all as read"),this);
	setAllAsReadAction->setToolTip(tr("Set all comics as read"));
	setAllAsReadAction->setIcon(QIcon(":/images/setAllRead.png"));
	
	setAllAsNonReadAction = new QAction(tr("Set all as unread"),this);
	setAllAsNonReadAction->setToolTip(tr("Set all comics as unread"));
	setAllAsNonReadAction->setIcon(QIcon(":/images/setAllUnread.png"));

	showHideMarksAction = new QAction(tr("Show/Hide marks"),this);
	showHideMarksAction->setToolTip(tr("Show or hide readed marks"));
	showHideMarksAction->setShortcut(Qt::Key_M);
	showHideMarksAction->setCheckable(true);
	showHideMarksAction->setIcon(QIcon(":/images/showMarks.png"));
	showHideMarksAction->setChecked(true);


	showPropertiesAction = new QAction(this);
	showPropertiesAction->setToolTip(tr("Show properties of current comic"));
	showPropertiesAction->setShortcut(Qt::Key_P);
	showPropertiesAction->setIcon(QIcon(":/images/properties.png"));

	toggleFullScreenAction = new QAction(this);
	toggleFullScreenAction->setToolTip(tr("Fullscreen mode on/off (F)"));
	toggleFullScreenAction->setShortcut(Qt::Key_F);
	toggleFullScreenAction->setIcon(QIcon(":/images/fit.png"));

	helpAboutAction = new QAction(this);
	helpAboutAction->setToolTip(tr("Help, About YACReader"));
	helpAboutAction->setShortcut(Qt::Key_F1);
	helpAboutAction->setIcon(QIcon(":/images/help.png"));

	setRootIndexAction = new QAction(this);
	setRootIndexAction->setToolTip(tr("Select root node"));
	setRootIndexAction->setIcon(QIcon(":/images/setRoot.png"));

	expandAllNodesAction = new QAction(this);
	expandAllNodesAction->setToolTip(tr("Expand all nodes"));
	expandAllNodesAction->setIcon(QIcon(":/images/expand.png"));

	colapseAllNodesAction = new QAction(this);
	colapseAllNodesAction->setToolTip(tr("Colapse all nodes"));
	colapseAllNodesAction->setIcon(QIcon(":/images/colapse.png"));

	optionsAction = new QAction(this);
	optionsAction->setToolTip(tr("Show options dialog"));
	optionsAction->setIcon(QIcon(":/images/options.png"));

	//disable actions
	updateLibraryAction->setEnabled(false);
	renameLibraryAction->setEnabled(false);
	deleteLibraryAction->setEnabled(false);
	removeLibraryAction->setEnabled(false);
	openComicAction->setEnabled(false);
	showPropertiesAction->setEnabled(false);
	setAsReadAction->setEnabled(false);
	setAsNonReadAction->setEnabled(false);
	setAllAsReadAction->setEnabled(false);
	setAllAsNonReadAction->setEnabled(false);

	openContainingFolderAction = new QAction(this);
	openContainingFolderAction->setText(tr("Open folder..."));
	openContainingFolderAction->setIcon(QIcon(":/images/open.png"));

	openContainingFolderComicAction = new QAction(this);
	openContainingFolderComicAction->setText(tr("Open containing folder..."));
	openContainingFolderComicAction->setIcon(QIcon(":/images/open.png"));
}

void LibraryWindow::createToolBars()
{
	libraryToolBar = addToolBar(tr("Library"));
	libraryToolBar->setIconSize(QSize(32,32)); //TODO make icon size dynamic
	libraryToolBar->addAction(createLibraryAction);
	libraryToolBar->addAction(openLibraryAction);
	libraryToolBar->addAction(exportLibraryAction);
	libraryToolBar->addAction(importLibraryAction);

	libraryToolBar->addSeparator();
	libraryToolBar->addAction(updateLibraryAction); 
	libraryToolBar->addAction(renameLibraryAction);
	libraryToolBar->addAction(removeLibraryAction);
	libraryToolBar->addAction(deleteLibraryAction);

	libraryToolBar->addSeparator();
	libraryToolBar->addAction(openComicAction);
	libraryToolBar->addAction(showPropertiesAction);
	
	QToolButton * tb = new QToolButton();
	tb->addAction(setAsReadAction);
	tb->addAction(setAllAsReadAction);
	tb->setPopupMode(QToolButton::MenuButtonPopup);
	tb->setDefaultAction(setAsReadAction);

	QToolButton * tb2 = new QToolButton();
	tb2->addAction(setAsNonReadAction);
	tb2->addAction(setAllAsNonReadAction);
	tb2->setPopupMode(QToolButton::MenuButtonPopup);
	tb2->setDefaultAction(setAsNonReadAction);

	libraryToolBar->addWidget(tb);
	libraryToolBar->addWidget(tb2);

	libraryToolBar->addAction(showHideMarksAction);

	libraryToolBar->addSeparator();
	libraryToolBar->addAction(toggleFullScreenAction);

	libraryToolBar->addWidget(new QToolBarStretch());
	libraryToolBar->addAction(optionsAction);
	libraryToolBar->addAction(helpAboutAction);
	

	libraryToolBar->setMovable(false);

	treeActions->addAction(setRootIndexAction);
	treeActions->addAction(expandAllNodesAction);
	treeActions->addAction(colapseAllNodesAction);

	comicFlow->addAction(toggleFullScreenAction);
	comicFlow->addAction(openComicAction);
}

void LibraryWindow::createMenus()
{
    comicView->addAction(openContainingFolderComicAction);
    foldersView->addAction(openContainingFolderAction);
}

void LibraryWindow::createConnections()
{
	//libraryCreator connections
	connect(createLibraryDialog,SIGNAL(createLibrary(QString,QString,QString)),this,SLOT(create(QString,QString,QString)));
	connect(libraryCreator,SIGNAL(coverExtracted(QString)),createLibraryDialog,SLOT(showCurrentFile(QString)));
	connect(libraryCreator,SIGNAL(finished()),createLibraryDialog,SLOT(close()));
	connect(libraryCreator,SIGNAL(coverExtracted(QString)),updateLibraryDialog,SLOT(showCurrentFile(QString)));
	connect(libraryCreator,SIGNAL(finished()),updateLibraryDialog,SLOT(close()));
	connect(libraryCreator,SIGNAL(finished()),this,SLOT(openLastCreated()));

	//packageManager connections
	connect(exportLibraryDialog,SIGNAL(exportPath(QString)),this,SLOT(exportLibrary(QString)));
	connect(exportLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(packageManager,SIGNAL(exported()),exportLibraryDialog,SLOT(close()));
	connect(importLibraryDialog,SIGNAL(unpackCLC(QString,QString,QString)),this,SLOT(importLibrary(QString,QString,QString)));
	connect(importLibraryDialog,SIGNAL(rejected()),packageManager,SLOT(cancel()));
	connect(importLibraryDialog,SIGNAL(rejected()),this,SLOT(deleteCurrentLibrary()));
	connect(packageManager,SIGNAL(imported()),importLibraryDialog,SLOT(hide()));
	connect(packageManager,SIGNAL(imported()),this,SLOT(openLastCreated()));
	

	//create and update dialogs
	connect(createLibraryDialog,SIGNAL(cancelCreate()),this,SLOT(cancelCreating()));
	connect(updateLibraryDialog,SIGNAL(cancelUpdate()),this,SLOT(stopLibraryCreator()));

	//open existing library from dialog.
	connect(addLibraryDialog,SIGNAL(addLibrary(QString,QString)),this,SLOT(openLibrary(QString,QString)));

	//load library when selected library changes
	connect(selectedLibrary,SIGNAL(currentIndexChanged(QString)),this,SLOT(loadLibrary(QString)));

	//rename library dialog
	connect(renameLibraryDialog,SIGNAL(renameLibrary(QString)),this,SLOT(rename(QString)));

	//navigations between view modes (tree,list and flow)
	connect(foldersView, SIGNAL(pressed(QModelIndex)), this, SLOT(loadCovers(QModelIndex)));
	connect(comicView, SIGNAL(pressed(QModelIndex)), this, SLOT(centerComicFlow(QModelIndex)));
	connect(comicFlow, SIGNAL(centerIndexChanged(int)), this, SLOT(updateComicView(int)));

	//actions
	connect(createLibraryAction,SIGNAL(triggered()),this,SLOT(createLibrary()));
	connect(exportLibraryAction,SIGNAL(triggered()),exportLibraryDialog,SLOT(show()));
	connect(importLibraryAction,SIGNAL(triggered()),importLibraryDialog,SLOT(show()));
	
	connect(openLibraryAction,SIGNAL(triggered()),this,SLOT(showAddLibrary()));
	connect(showPropertiesAction,SIGNAL(triggered()),this,SLOT(showProperties()));
	connect(setAsReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicReaded()));
	connect(setAsNonReadAction,SIGNAL(triggered()),this,SLOT(setCurrentComicUnreaded()));
	connect(setAllAsReadAction,SIGNAL(triggered()),this,SLOT(setComicsReaded()));
	connect(setAllAsNonReadAction,SIGNAL(triggered()),this,SLOT(setComicsUnreaded()));

	connect(showHideMarksAction,SIGNAL(toggled(bool)),comicFlow,SLOT(setShowMarks(bool)));
	

	connect(updateLibraryAction,SIGNAL(triggered()),this,SLOT(updateLibrary()));
	connect(renameLibraryAction,SIGNAL(triggered()),this,SLOT(renameLibrary()));
	connect(deleteLibraryAction,SIGNAL(triggered()),this,SLOT(deleteLibrary()));
	connect(removeLibraryAction,SIGNAL(triggered()),this,SLOT(removeLibrary()));
	connect(openComicAction,SIGNAL(triggered()),this,SLOT(openComic()));
	connect(helpAboutAction,SIGNAL(triggered()),had,SLOT(show()));
	connect(setRootIndexAction,SIGNAL(triggered()),this,SLOT(setRootIndex()));
	connect(expandAllNodesAction,SIGNAL(triggered()),foldersView,SLOT(expandAll()));
	connect(colapseAllNodesAction,SIGNAL(triggered()),foldersView,SLOT(collapseAll()));
	connect(toggleFullScreenAction,SIGNAL(triggered()),this,SLOT(toggleFullScreen()));
	connect(optionsAction, SIGNAL(triggered()),optionsDialog,SLOT(show()));
	connect(optionsDialog, SIGNAL(optionsChanged()),this,SLOT(reloadOptions()));
	//ComicFlow
	connect(comicFlow,SIGNAL(selected(unsigned int)),this,SLOT(openComic()));
	connect(comicView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openComic()));
	//Folders filter
	connect(clearFoldersFilter,SIGNAL(clicked()),foldersFilter,SLOT(clear()));
	connect(foldersFilter,SIGNAL(textChanged(QString)),this,SLOT(setFoldersFilter(QString)));
	connect(includeComicsCheckBox,SIGNAL(stateChanged(int)),this,SLOT(searchInFiles(int)));
	
	//ContextMenus
	connect(openContainingFolderComicAction,SIGNAL(triggered()),this,SLOT(openContainingFolderComic()));
	connect(openContainingFolderAction,SIGNAL(triggered()),this,SLOT(openContainingFolder()));

	//connect(dm,SIGNAL(directoryLoaded(QString)),foldersView,SLOT(expandAll()));
	connect(dm,SIGNAL(directoryLoaded(QString)),this,SLOT(updateFoldersView(QString)));
}

void LibraryWindow::loadLibrary(const QString & name)
{
	if(libraries.size()>0)
	{	
		QString path=libraries.value(name)+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
		if(d.exists(path))
		{
			//renew dirmodels (because QFileSystemModel is.......crap ;) )
			QFileSystemModel * oldDm = dm;
			QFileSystemModel * oldDmCV = dmCV;
			YACReaderTreeSearch * oldTreeSearch = proxyFilter;
			//YACReaderSortComics * oldSortProxy = proxySort;

			dm = new QFileSystemModel();
			dm->setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
			dm->sort(0);//,QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
			dmCV =  new QFileSystemModel();
			dmCV->setNameFilters(QStringList() << "*jpg");
			dmCV->setFilter(QDir::Files|QDir::CaseSensitive|QDir::NoDotAndDotDot);//,QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
			dmCV->setNameFilterDisables(true);

			dm->setReadOnly(true);
			//dm->setLazyChildCount(false);
			dmCV->setReadOnly(true);

			dm->setIconProvider(&fip);
			dmCV->setIconProvider(&fip);
			//dmCV->sort(1);

			proxyFilter = new  YACReaderTreeSearch();
			proxyFilter->setSourceModel(dm);
			proxyFilter->setFilterRole(Qt::DisplayRole);

			/*proxySort = new YACReaderSortComics();
			proxySort->setSourceModel(dmCV);
			proxySort->setFilterRole(Qt::DisplayRole);*/

			connect(dm,SIGNAL(directoryLoaded(QString)),this,SLOT(updateFoldersView(QString)));

			includeComicsCheckBox->setCheckState(Qt::Unchecked);

			if(oldDm !=0)
				delete oldDm;
			if(oldDmCV !=0)
				delete oldDmCV;
			if(oldTreeSearch !=0)
				delete oldTreeSearch;
			/*if(oldSortProxy !=0)
				delete oldSortProxy;*/
			//end renew dirmodels
			//TODO review this code, some sentences could be not necessary
			foldersView->setModel(proxyFilter);
			comicView->setModel(dmCV);
			_rootIndex = dm->setRootPath(path);
			i=0;

			comicView->setRootIndex(dmCV->setRootPath(path));
			foldersView->setRootIndex(proxyFilter->mapFromSource(_rootIndex));//dm->setRootPath(path)));

			foldersView->header()->hideSection(1);
			foldersView->header()->hideSection(2);
			foldersView->header()->hideSection(3);
			foldersView->header()->adjustSize();
			foldersView->header()->hide();

			loadCovers(proxyFilter->mapFromSource(dm->index(path)));
			//foldersView->expandAll();

		}
		else
		{
			comicView->setModel(NULL);
			foldersView->setModel(NULL);
			comicFlow->clear();
		}
		d.setCurrent(libraries.value(name));
		d.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		if(d.count()<=1)
		{
			//QMessageBox::critical(NULL,QString::number(d.count()),QString::number(d.count()));
			updateLibraryAction->setEnabled(false);
			openComicAction->setEnabled(false);
			showPropertiesAction->setEnabled(false);
			openContainingFolderAction->setEnabled(false);
			openContainingFolderComicAction->setEnabled(false);
			setAsReadAction->setEnabled(false);
			setAsNonReadAction->setEnabled(false);
			setAllAsReadAction->setEnabled(false);
			setAllAsNonReadAction->setEnabled(false);
			importedCovers = true;
		}
		else
		{
			updateLibraryAction->setEnabled(true);
			openComicAction->setEnabled(true);
			showPropertiesAction->setEnabled(true);
			openContainingFolderAction->setEnabled(true);
			openContainingFolderComicAction->setEnabled(true);
			setAsReadAction->setEnabled(true);
			setAsNonReadAction->setEnabled(true);
			setAllAsReadAction->setEnabled(true);
			setAllAsNonReadAction->setEnabled(true);
			importedCovers = false;
		}
		renameLibraryAction->setEnabled(true);
		deleteLibraryAction->setEnabled(true);
		removeLibraryAction->setEnabled(true);
		foldersFilter->setEnabled(true);
		clearFoldersFilter->setEnabled(true);
	}
	else
	{
		updateLibraryAction->setEnabled(false);
		renameLibraryAction->setEnabled(false);
		deleteLibraryAction->setEnabled(false);
		removeLibraryAction->setEnabled(false);
		foldersFilter->setEnabled(false);
		clearFoldersFilter->setEnabled(false);
		setAsReadAction->setEnabled(false);
		setAsNonReadAction->setEnabled(false);
		setAllAsReadAction->setEnabled(false);
		setAllAsNonReadAction->setEnabled(false);
	}
}

void LibraryWindow::loadCovers(const QModelIndex & mi)
{
if(foldersFilter->text()!="")
{
	setFoldersFilter("");
	foldersFilter->clear();
}
	QString path = dm->filePath(proxyFilter->mapToSource(mi));
	delete dmCV;
	dmCV = new QFileSystemModel();
	dmCV->setFilter(QDir::Files|QDir::CaseSensitive|QDir::NoDotAndDotDot);
	dmCV->setNameFilters(QStringList() << "*jpg");
	dmCV->setNameFilterDisables(false);
	dmCV->setReadOnly(true);

	dmCV->setIconProvider(&fip);
	//dmCV->sort(1);
	//delete proxySort;

	//TODO
	/*proxySort = new YACReaderSortComics();
	proxySort->setSourceModel(dmCV);
	proxySort->setFilterRole(Qt::DisplayRole);*/

	comicView->setModel(dmCV);
	comicView->setRootIndex(dmCV->setRootPath(path));

	comicFlow->setImagePath(path);
	comicFlow->setFocus(Qt::OtherFocusReason);
	paths = comicFlow->getImageFiles();
	if(paths.size()>0 && !importedCovers)
	{
		openComicAction->setEnabled(true);
		showPropertiesAction->setEnabled(true);
					setAsReadAction->setEnabled(true);
			setAsNonReadAction->setEnabled(true);
			setAllAsReadAction->setEnabled(true);
			setAllAsNonReadAction->setEnabled(true);
	}
	else
	{
		openComicAction->setEnabled(false);
		showPropertiesAction->setEnabled(false);
				setAsReadAction->setEnabled(false);
		setAsNonReadAction->setEnabled(false);
		setAllAsReadAction->setEnabled(false);
		setAllAsNonReadAction->setEnabled(false);
	}
	if(paths.size()>0)
		comicView->setCurrentIndex(dmCV->index(paths[0]));
	
}

void LibraryWindow::centerComicFlow(const QModelIndex & mi)
{
	int distance = comicFlow->centerIndex()-mi.row();
	if(abs(distance)>10)
	{
		if(distance<0)
			comicFlow->setCenterIndex(comicFlow->centerIndex()+(-distance)-10);
		else
			comicFlow->setCenterIndex(comicFlow->centerIndex()-distance+10);
		skip = 10;
	}
	else
		skip = abs(comicFlow->centerIndex()-mi.row());
	comicFlow->showSlide(mi.row());
	comicFlow->setFocus(Qt::OtherFocusReason);
}

void LibraryWindow::updateComicView(int i)
{

	if((paths.size()>0)&&skip==0)
		comicView->setCurrentIndex(dmCV->index(paths[i]));
	skip?(--skip):0;
}

void LibraryWindow::openComic()
{
	//int index = comicFlow->centerIndex();
	if(!importedCovers)
	{
		QModelIndex mi = comicView->currentIndex();
		QString path = QDir::cleanPath(dmCV->filePath(mi));
		
		path.remove("/.yacreaderlibrary");
		path.remove(path.size()-4,4);
		QProcess::startDetached(QDir::cleanPath(QCoreApplication::applicationDirPath())+"/YACReader",QStringList() << path);
		//Comic is readed
		setCurrentComicReaded();


	}
}

void LibraryWindow::setCurrentComicReaded()
{
	comicFlow->markSlide(comicFlow->centerIndex());
	QModelIndex mi = comicView->currentIndex();
	QString path = QDir::cleanPath(dmCV->filePath(mi));
	QFile f(path.remove(path.size()-3,3)+"r");
	f.open(QIODevice::WriteOnly);
	f.close();

	comicFlow->updateMarks();
}

void LibraryWindow::setComicsReaded()
{
	QModelIndex mi = proxyFilter->mapToSource(foldersView->currentIndex());
	QString path;
	
	if(mi.isValid())
		path = QDir::cleanPath(dm->filePath(mi));
	else
		path = dm->rootPath();

	QDir d(path,"*.jpg");
	d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QFileInfoList list = d.entryInfoList();
	int nFiles = list.size();
	for(int i=0;i<nFiles;i++)
	{
		QFileInfo info = list.at(i);
		QString filePath = info.absoluteFilePath();
		QFile f(filePath.remove(filePath.size()-3,3)+"r");
		f.open(QIODevice::WriteOnly);
		f.close();
		comicFlow->markSlide(i);
	}

	comicFlow->updateMarks();
}

void LibraryWindow::setCurrentComicUnreaded()
{
	comicFlow->unmarkSlide(comicFlow->centerIndex());
	QModelIndex mi = comicView->currentIndex();
	QString path = QDir::cleanPath(dmCV->filePath(mi));
	QFile f(path.remove(path.size()-3,3)+"r");
	f.open(QIODevice::WriteOnly);
	f.remove();
	f.close();

	comicFlow->updateMarks();
}

void LibraryWindow::setComicsUnreaded()
{
	QModelIndex mi = proxyFilter->mapToSource(foldersView->currentIndex());
	QString path;
	
	if(mi.isValid())
		path = QDir::cleanPath(dm->filePath(mi));
	else
		path = dm->rootPath();

	QDir d(path,"*.jpg");
	d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QFileInfoList list = d.entryInfoList();
	int nFiles = list.size();
	for(int i=0;i<nFiles;i++)
	{
		QFileInfo info = list.at(i);
		QString filePath = info.absoluteFilePath();
		QFile f(filePath.remove(filePath.size()-3,3)+"r");
		f.open(QIODevice::WriteOnly);
		f.remove();
		f.close();
		comicFlow->unmarkSlide(i);
	}

	comicFlow->updateMarks();
}

void LibraryWindow::createLibrary()
{
	createLibraryDialog->show();
}

void LibraryWindow::create(QString source, QString dest, QString name)
{
	_lastAdded = name;
	libraries.insert(name,source);
	selectedLibrary->addItem(name,source);
	libraryCreator->createLibrary(source,dest);
	libraryCreator->start();
	saveLibraries();
}

void LibraryWindow::openLastCreated()
{
	loadLibrary(_lastAdded);
	selectedLibrary->setCurrentIndex(selectedLibrary->findText(_lastAdded));
}

void LibraryWindow::showAddLibrary()
{
	addLibraryDialog->show();
}

void LibraryWindow::openLibrary(QString path, QString name)
{	
	_lastAdded = name;
	libraries.insert(name,path);
	selectedLibrary->addItem(name,path);
	openLastCreated();
	saveLibraries();
}

void LibraryWindow::loadLibraries()
{
	QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	QStringList lines = content.split('\n');
	QString line,name;
	int i=0;
	foreach(line,lines)
	{
		if((i%2)==0)
		{
			name = line;
		}
		else
		{
			libraries.insert(name.trimmed(),line.trimmed());
			selectedLibrary->addItem(name.trimmed(),line.trimmed());
		}
		i++;
	}
}

void LibraryWindow::saveLibraries()
{
	QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
        if(!f.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(NULL,tr("Saving libraries file...."),tr("There was a problem saving YACReaderLibrary libraries file. Please, check if you have enough permissions in the YACReader root folder."));
        }
        else
        {
	QTextStream txtS(&f);
	for(QMap<QString,QString>::iterator i = libraries.begin();i!=libraries.end();i++)
	{
		txtS << i.key() << "\n";
		txtS << i.value() << "\n";
	}
    }
}

void LibraryWindow::updateLibrary()
{
	delete dm;
	delete dmCV;
	delete proxyFilter;
	dm = 0;
	dmCV = 0;
	proxyFilter = 0;

	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary);
	_lastAdded = currentLibrary;
	libraryCreator->updateLibrary(path,path+"/.yacreaderlibrary");
	libraryCreator->start();
	updateLibraryDialog->show();
}

void LibraryWindow::deleteLibrary()
{
	QString currentLibrary = selectedLibrary->currentText();
	if(QMessageBox::question(this,tr("Are you sure?"),tr("Do you want delete ")+currentLibrary+" library?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
	{
	    deleteCurrentLibrary();
	}
}

void LibraryWindow::deleteCurrentLibrary()
{
    QString path = libraries.value(selectedLibrary->currentText());
    libraries.remove(selectedLibrary->currentText());
    selectedLibrary->removeItem(selectedLibrary->currentIndex());
    selectedLibrary->setCurrentIndex(0);
    path = path+"/.yacreaderlibrary";
    QDir d(path);
    delTree(d);
    d.rmdir(path);
    if(libraries.size()==0)//no more libraries avaliable.
    {
	    comicView->setModel(NULL);
	    foldersView->setModel(NULL);
	    comicFlow->clear();
    }
    saveLibraries();
}

void LibraryWindow::removeLibrary()
{
	QString currentLibrary = selectedLibrary->currentText();
	if(QMessageBox::question(this,tr("Are you sure?"),tr("Do you want remove ")+currentLibrary+tr(" library?\nFiles won't be erased from disk."),QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
	{
		libraries.remove(currentLibrary);
		selectedLibrary->removeItem(selectedLibrary->currentIndex());
		selectedLibrary->setCurrentIndex(0);
		if(libraries.size()==0)//no more libraries avaliable.
		{
			comicView->setModel(NULL);
			foldersView->setModel(NULL);
			comicFlow->clear();
		}
		saveLibraries();
	}
}

void LibraryWindow::renameLibrary()
{
	renameLibraryDialog->show();
}

void LibraryWindow::rename(QString newName)
{
	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary);
	libraries.remove(currentLibrary);
	selectedLibrary->removeItem(selectedLibrary->currentIndex());
	libraries.insert(newName,path);
	selectedLibrary->addItem(newName,path);
	saveLibraries();
	selectedLibrary->setCurrentIndex(selectedLibrary->findText(newName));
}

void LibraryWindow::cancelCreating()
{
	stopLibraryCreator();
	//TODO delete library.
}

void LibraryWindow::stopLibraryCreator()
{
	libraryCreator->stop();
	libraryCreator->wait();
}

void LibraryWindow::setRootIndex()
{
	if(libraries.size()>0)
	{	
		QString path=libraries.value(selectedLibrary->currentText())+"/.yacreaderlibrary";
		QDir d; //TODO change this by static methods (utils class?? with delTree for example) 
		if(d.exists(path))
		{
			//dmCV->refresh(dmCV->index(path));
			comicView->setRootIndex(dmCV->index(path));
                        loadCovers(proxyFilter->mapFromSource(dm->index(path)));
		}
		else
		{
			comicView->setModel(NULL);
			comicFlow->clear();
		}
	}
}


void LibraryWindow::toggleFullScreen()
{
	fullscreen?toNormal():toFullScreen();
	fullscreen = !fullscreen;
}

void LibraryWindow::toFullScreen()
{
	comicFlow->hide();
	comicFlow->setSlideSize(slideSizeF);
	comicFlow->setCenterIndex(comicFlow->centerIndex());
	comicView->hide();
	left->hide();
	libraryToolBar->hide();

	showFullScreen();

	comicFlow->show();
	comicFlow->setFocus(Qt::OtherFocusReason);
	fullScreenToolTip->move((width()-fullScreenToolTip->width())/2,0);
	fullScreenToolTip->adjustSize();
	fullScreenToolTip->show();
}

void LibraryWindow::toNormal()
{
	comicFlow->hide();
	comicFlow->setSlideSize(slideSizeW);
	comicFlow->setCenterIndex(comicFlow->centerIndex());
	comicFlow->render();
	comicView->show();
	left->show();
    fullScreenToolTip->hide();
	libraryToolBar->show();
	comicFlow->show();
	
	showMaximized();
}

void LibraryWindow::setFoldersFilter(QString filter)
{
	if(filter.contains(previousFilter))
		proxyFilter->softReset();
	else
		proxyFilter->reset();
	previousFilter = filter;
	if(!filter.isEmpty())
	{
		proxyFilter->setFilterRegExp(QRegExp(filter,Qt::CaseInsensitive,QRegExp::FixedString));
        foldersView->expandAll();
	}
	else
	{
		proxyFilter->setFilterRegExp(QRegExp());
		foldersView->scrollTo(foldersView->currentIndex(),QAbstractItemView::PositionAtTop);
        foldersView->collapseAll();
	}
}

void LibraryWindow::showProperties()
{
	//TODO create a new method for this
	QModelIndex mi = comicView->currentIndex();
	QString path = QDir::cleanPath(dmCV->filePath(mi)).remove("/.yacreaderlibrary");
	path.remove(path.size()-4,4);

	ThumbnailCreator tc(path,"");
	tc.create();
	propertiesDialog->setCover(tc.getCover());
	propertiesDialog->setFilename(path.split("/").last());
	propertiesDialog->setNumpages(tc.getNumPages());
	QFile file(path);
	propertiesDialog->setSize(file.size()/(1024.0*1024));
	file.close();
	propertiesDialog->show();
}

void LibraryWindow::openContainingFolderComic()
{
    QModelIndex modelIndex = comicView->currentIndex();
    QString path = QDir::cleanPath(dmCV->fileInfo(modelIndex).absolutePath()).remove("/.yacreaderlibrary");
    QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void LibraryWindow::openContainingFolder()
{
    QModelIndex modelIndex = foldersView->currentIndex();
    QString path = QDir::cleanPath(dm->filePath(proxyFilter->mapToSource(modelIndex))).remove("/.yacreaderlibrary");
    QDesktopServices::openUrl(QUrl("file:///"+path, QUrl::TolerantMode));
}

void LibraryWindow::exportLibrary(QString destPath)
{
	QString currentLibrary = selectedLibrary->currentText();
	QString path = libraries.value(currentLibrary)+"/.yacreaderlibrary";
	packageManager->createPackage(path,destPath+"/"+currentLibrary);
}

void LibraryWindow::importLibrary(QString clc,QString destPath,QString name)
{
	packageManager->extractPackage(clc,destPath+"/"+name);
	openLibrary(destPath+"/"+name,name);
}

void LibraryWindow::reloadOptions()
{
	comicFlow->setFlowType(flowType);
}

void LibraryWindow::updateFoldersView(QString path)
{
	QModelIndex mi = dm->index(path);
	int rowCount = dm->rowCount(mi);
	if(!fetching)
	{
		//fetching = true;
		for(int i=0;i<rowCount;i++)
		{
			dm->fetchMore(dm->index(i,0,mi));
			//int childCount = dm->rowCount(dm->index(i,0,mi));
			//if(childCount>0)
			//	QMessageBox::critical(NULL,tr("..."),tr("-----"));
			fetching = false;
		}
	}

}

void LibraryWindow::searchInFiles(int state)
{

	if(state == Qt::Checked)
	{
		dm->setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot); //crash, after update proxy filter
	}
	else
	{
		dm->setFilter(QDir::Dirs|QDir::NoDotAndDotDot); //crash
	}
}



