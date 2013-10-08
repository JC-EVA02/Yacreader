#include "search_single_comic.h"

#include "scrapper_lineedit.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

SearchSingleComic::SearchSingleComic(QWidget * parent)
	:QWidget(parent)
{

	QLabel * label = new QLabel(tr("No results found, please provide some aditional information. At least one field is needed."));
	label->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	titleEdit = new ScrapperLineEdit(tr("Title:"));
	numberEdit = new ScrapperLineEdit(tr("Number:"));
	volumeEdit = new ScrapperLineEdit(tr("Series:"));

	numberEdit->setMaximumWidth(126);

	QVBoxLayout * l = new QVBoxLayout;
	QHBoxLayout * hl = new QHBoxLayout;
	hl->addWidget(titleEdit);
	hl->addWidget(numberEdit);

	l->addSpacing(35);
	l->addWidget(label);
	l->addLayout(hl);
	l->addWidget(volumeEdit);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

QString SearchSingleComic::getVolumeInfo()
{
	return volumeEdit->text();
}

QString SearchSingleComic::getComicInfo()
{
	return titleEdit->text();
}

int SearchSingleComic::getComicNumber()
{
	QString numberText = numberEdit->text();
	if(numberText.isEmpty())
		return -1;
	return numberText.toInt();
}