/***************************************************************************

                    kappwizard.cpp - the kde-application-wizard
                             -------------------                                         

    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qstringlist.h>
#include <keditcl.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kiconloaderdialog.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <qregexp.h>


#include "cproject.h"
#include "ckappwizard.h"
#include "ctoolclass.h"



CKAppWizard::CKAppWizard(QWidget* parent,const char* name,QString author_name,QString author_email) : KWizard(parent,name,true){
  q = new KShellProcess();
  gen_prj = false;
  modifyDirectory = false;
  modifyVendor = false;
  nameold = "";
  setCaption(i18n("Application Wizard"));
  init();
  initPages();
  m_author_email = author_email;
  m_author_name = author_name;
  project=0l;
  //cerr << ":" << m_author_name << ":";
  //cerr << ":" << m_author_email << ":";
  slotDefaultClicked(0);
}

CKAppWizard::~CKAppWizard () {
  
}

// slot slotAppEnd -> reject()
// slot slotOkClicked -> accept()

void CKAppWizard::init(){

  QToolTip::add(cancelButton(), i18n("exit the CKAppWizard"));
  QToolTip::add(finishButton(), i18n("creating the project"));
#warning FIXME
#if 0
  setDefaultButton();
  defaultButton = getDefaultButton();
  QToolTip::add(defaultButton,i18n("set all changes back"));
  connect(this,SIGNAL(defaultclicked(int)),SLOT(slotDefaultClicked(int)));
#endif
}

void CKAppWizard::initPages(){
  
  // create the first page
  widget0 = new QWidget(this);
  addPage(widget0, i18n("Applications"));
  
  QPixmap iconpm;    
  QPixmap minipm;
  QPixmap pm;
  pm.load(locate("appdata", "pics/kAppWizard.bmp"));
  
  // create a widget and paint a picture on it
  widget1a = new QWidget( widget0, "widget1a" );
  widget1a->setGeometry( 0, 0, 500, 160 );
  widget1a->setMinimumSize( 0, 0 );
  widget1a->setMaximumSize( 32767, 32767 );
  widget1a->setFocusPolicy( QWidget::NoFocus );
  widget1a->setBackgroundMode( QWidget::PaletteBackground );
  widget1a->setFontPropagation( QWidget::NoChildren );
  widget1a->setPalettePropagation( QWidget::NoChildren );
  widget1a->setBackgroundPixmap (pm);
  
  applications = new QListView( widget0, "applications" );
  applications->setGeometry( 20, 170, 170, 250 );
  applications->setMinimumSize( 0, 0 );
  applications->setMaximumSize( 32767, 32767 );
  applications->setFocusPolicy( QWidget::TabFocus );
  applications->setBackgroundMode( QWidget::PaletteBackground );
  applications->setFontPropagation( QWidget::NoChildren );
  applications->setPalettePropagation( QWidget::NoChildren );
  applications->setResizePolicy( QScrollView::Manual );
  applications->setVScrollBarMode( QScrollView::Auto );
  applications->setHScrollBarMode( QScrollView::Auto );
  applications->setTreeStepSize( 20 );
  applications->setMultiSelection( FALSE );
  applications->setAllColumnsShowFocus( FALSE );
  applications->setItemMargin( 1 );
  applications->setRootIsDecorated( TRUE );
  applications->addColumn( i18n("Applications"), -1 );
  applications->setColumnWidthMode( 0, QListView::Maximum );
  applications->setColumnAlignment( 0, 1 );
  applications->setSorting (-1,FALSE);
  applications->header()->hide();
  
  // create another widget for a picture
  widget1b = new QWidget( widget0, "widget1b" );
  widget1b->setGeometry( 255, 180, 190, 140 );
  widget1b->setMinimumSize( 0, 0 );
  widget1b->setMaximumSize( 32767, 32767 );
  widget1b->setFocusPolicy( QWidget::NoFocus );
  widget1b->setBackgroundMode( QWidget::PaletteBackground );
  widget1b->setFontPropagation( QWidget::NoChildren );
  widget1b->setPalettePropagation( QWidget::NoChildren );
  
  apphelp = new QLabel( widget0, "apphelp" );
  apphelp->setGeometry( 230, 330, 240, 90 );
  apphelp->setMinimumSize( 0, 0 );
  apphelp->setMaximumSize( 32767, 32767 );
  apphelp->setFocusPolicy( QWidget::NoFocus );
  apphelp->setBackgroundMode( QWidget::PaletteBackground );
  apphelp->setFontPropagation( QWidget::NoChildren );
  apphelp->setPalettePropagation( QWidget::NoChildren );
  apphelp->setText( i18n("Label:") );
  apphelp->setAlignment( 1313 );
  apphelp->setMargin( -1 );
  
  othersentry = new QListViewItem (applications, i18n("Others"));
  othersentry->setExpandable (true);
  othersentry->setOpen (TRUE);
  othersentry->sortChildItems (0,FALSE);
  customprojitem = new QListViewItem (othersentry,i18n("custom project"));
  
  /*	gtkentry = new QListViewItem (applications, "GTK");
	gtkentry->setExpandable (true);
	gtkentry->setOpen (TRUE);
	gtkentry->sortChildItems (0,FALSE);	
	gtkminiitem = new QListViewItem (gtkentry,"Mini");
	gtknormalitem = new QListViewItem (gtkentry,"Normal");
  */
  ccppentry = new QListViewItem (applications, i18n("Terminal"));
  ccppentry->setExpandable (true);
  ccppentry->setOpen (TRUE);
  ccppentry->sortChildItems (0,FALSE);
  cppitem = new QListViewItem (ccppentry,i18n("C++"));
  citem = new QListViewItem (ccppentry,i18n("C"));

  qtentry = new QListViewItem (applications, i18n("Qt"));
  qtentry->setExpandable (true);
  qtentry->setOpen (TRUE);
  qtentry->sortChildItems (0,FALSE);
  //qtminiitem = new QListViewItem (qtentry,"Mini");
  qtnormalitem = new QListViewItem (qtentry,i18n("Normal"));
  
  kdeentry = new QListViewItem (applications,i18n("KDE"));
  kdeentry->setExpandable (true);
  kdeentry->setOpen (TRUE);
  kdeentry->sortChildItems (0,FALSE);
  //komitem = new QListViewItem (kdeentry,"KOM");
  //corbaitem = new QListViewItem (kdeentry,"Corba");
  kdeminiitem = new QListViewItem (kdeentry,i18n("Mini"));
  kdenormalitem = new QListViewItem (kdeentry,i18n("Normal"));
  
  applications->setFrameStyle( QListView::Panel | QListView::Sunken );
  applications->setLineWidth( 2 );
  
  separator0 = new KSeparator (widget0);
  separator0->setGeometry(0,160,515,5);
  
  connect (applications, SIGNAL(selectionChanged ()),SLOT(slotApplicationClicked()));
  
  /************************************************************/
  
  // create the second page
  widget1 = new QWidget(this);
  addPage(widget1, i18n("Generate settings"));
  
  name = new QLabel( widget1, "name" );
  name->setGeometry( 30, 10, 100, 30 );
  name->setMinimumSize( 0, 0 );
  name->setMaximumSize( 32767, 32767 );
  name->setFocusPolicy( QWidget::NoFocus );
  name->setBackgroundMode( QWidget::PaletteBackground );
  name->setFontPropagation( QWidget::NoChildren );
  name->setPalettePropagation( QWidget::NoChildren );
  name->setText( i18n("Projectname:") );
  name->setAlignment( 289 );
  name->setMargin( -1 );

  nameline = new QLineEdit( widget1, "nameline" );
  nameline->setGeometry( 140, 10, 290, 30 );
  nameline->setMinimumSize( 0, 0 );
  nameline->setMaximumSize( 32767, 32767 );
  nameline->setFocusPolicy( QWidget::StrongFocus );
  nameline->setBackgroundMode( QWidget::PaletteBase );
  nameline->setFontPropagation( QWidget::NoChildren );
  nameline->setPalettePropagation( QWidget::NoChildren );
  nameline->setText( "" );
  nameline->setMaxLength( 32767 );
  nameline->setEchoMode( QLineEdit::Normal );
  nameline->setFrame( TRUE );

  directory = new QLabel( widget1, "directory" );
  directory->setGeometry( 30, 50, 100, 30 );
  directory->setMinimumSize( 0, 0 );
  directory->setMaximumSize( 32767, 32767 );
  directory->setFocusPolicy( QWidget::NoFocus );
  directory->setBackgroundMode( QWidget::PaletteBackground );
  directory->setFontPropagation( QWidget::NoChildren );
  directory->setPalettePropagation( QWidget::NoChildren );
  directory->setText( i18n("Projectdirectory:") );
  directory->setAlignment( 289 );
  directory->setMargin( -1 );

  directoryline = new QLineEdit( widget1, "directoryline" );
  directoryline->setGeometry( 140, 50, 290, 30 );
  directoryline->setMinimumSize( 0, 0 );
  directoryline->setMaximumSize( 32767, 32767 );
  directoryline->setFocusPolicy( QWidget::StrongFocus );
  directoryline->setBackgroundMode( QWidget::PaletteBase );
  directoryline->setFontPropagation( QWidget::NoChildren );
  directoryline->setPalettePropagation( QWidget::NoChildren );
  directoryline->setText( "" );
  directoryline->setMaxLength( 32767 );
  directoryline->setEchoMode( QLineEdit::Normal );
  directoryline->setFrame( TRUE );

  directoryload = new QPushButton( widget1, "directoryload" );
  directoryload->setGeometry( 440, 50, 30, 30 );
  directoryload->setMinimumSize( 0, 0 );
  directoryload->setMaximumSize( 32767, 32767 );
  directoryload->setFocusPolicy( QWidget::TabFocus );
  directoryload->setBackgroundMode( QWidget::PaletteBackground );
  directoryload->setFontPropagation( QWidget::NoChildren );
  directoryload->setPalettePropagation( QWidget::NoChildren );
  directoryload->setPixmap(BarIcon("open"));
  directoryload->setAutoRepeat( FALSE );
  directoryload->setAutoResize( FALSE );
  
  versionnumber = new QLabel( widget1, "versionnumber" );
  versionnumber->setGeometry( 30, 90, 100, 30 );
  versionnumber->setMinimumSize( 0, 0 );
  versionnumber->setMaximumSize( 32767, 32767 );
  versionnumber->setFocusPolicy( QWidget::NoFocus );
  versionnumber->setBackgroundMode( QWidget::PaletteBackground );
  versionnumber->setFontPropagation( QWidget::NoChildren );
  versionnumber->setPalettePropagation( QWidget::NoChildren );
  versionnumber->setText( i18n("Versionnumber:") );
  versionnumber->setAlignment( 289 );
  versionnumber->setMargin( -1 );

  versionline = new QLineEdit( widget1, "versionline" );
  versionline->setGeometry( 140, 90, 290, 30 );
  versionline->setMinimumSize( 0, 0 );
  versionline->setMaximumSize( 32767, 32767 );
  versionline->setFocusPolicy( QWidget::StrongFocus );
  versionline->setBackgroundMode( QWidget::PaletteBase );
  versionline->setFontPropagation( QWidget::NoChildren );
  versionline->setPalettePropagation( QWidget::NoChildren );
  versionline->setText( "" );
  versionline->setMaxLength( 32767 );
  versionline->setEchoMode( QLineEdit::Normal );
  versionline->setFrame( TRUE );

  authorname = new QLabel( widget1, "authorname" );
  authorname->setGeometry( 30, 130, 100, 30 );
  authorname->setMinimumSize( 0, 0 );
  authorname->setMaximumSize( 32767, 32767 );
  authorname->setFocusPolicy( QWidget::NoFocus );
  authorname->setBackgroundMode( QWidget::PaletteBackground );
  authorname->setFontPropagation( QWidget::NoChildren );
  authorname->setPalettePropagation( QWidget::NoChildren );
  authorname->setText( i18n("Author:") );
  authorname->setAlignment( 289 );
  authorname->setMargin( -1 );

  authorline = new QLineEdit( widget1, "authorline" );
  authorline->setGeometry( 140, 130, 290, 30 );
  authorline->setMinimumSize( 0, 0 );
  authorline->setMaximumSize( 32767, 32767 );
  authorline->setFocusPolicy( QWidget::StrongFocus );
  authorline->setBackgroundMode( QWidget::PaletteBase );
  authorline->setFontPropagation( QWidget::NoChildren );
  authorline->setPalettePropagation( QWidget::NoChildren );
  authorline->setText( "" );
  authorline->setMaxLength( 32767 );
  authorline->setEchoMode( QLineEdit::Normal );
  authorline->setFrame( TRUE );
  
  email = new QLabel( widget1, "email" );
  email->setGeometry( 30, 170, 100, 30 );
  email->setMinimumSize( 0, 0 );
  email->setMaximumSize( 32767, 32767 );
  email->setFocusPolicy( QWidget::NoFocus );
  email->setBackgroundMode( QWidget::PaletteBackground );
  email->setFontPropagation( QWidget::NoChildren );
  email->setPalettePropagation( QWidget::NoChildren );
  email->setText( i18n("Email:") );
  email->setAlignment( 289 );
  email->setMargin( -1 );

  emailline = new QLineEdit( widget1, "emailline" );
  emailline->setGeometry( 140, 170, 290, 30 );
  emailline->setMinimumSize( 0, 0 );
  emailline->setMaximumSize( 32767, 32767 );
  emailline->setFocusPolicy( QWidget::StrongFocus );
  emailline->setBackgroundMode( QWidget::PaletteBase );
  emailline->setFontPropagation( QWidget::NoChildren );
  emailline->setPalettePropagation( QWidget::NoChildren );
  emailline->setText( "" );
  emailline->setMaxLength( 32767 );
  emailline->setEchoMode( QLineEdit::Normal );
  emailline->setFrame( TRUE );

  generatesource = new QCheckBox( widget1, "generatesource" );
  generatesource->setGeometry( 30, 220, 440, 30 );
  generatesource->setMinimumSize( 0, 0 );
  generatesource->setMaximumSize( 32767, 32767 );
  generatesource->setFocusPolicy( QWidget::TabFocus );
  generatesource->setBackgroundMode( QWidget::PaletteBackground );
  generatesource->setFontPropagation( QWidget::NoChildren );
  generatesource->setPalettePropagation( QWidget::NoChildren );
  generatesource->setText( i18n("generate sources and headers") );
  generatesource->setAutoRepeat( FALSE );
  generatesource->setAutoResize( FALSE );

  gnufiles = new QCheckBox( widget1, "gnufiles" );
  gnufiles->setGeometry( 30, 270, 440, 30 );
  gnufiles->setMinimumSize( 0, 0 );
  gnufiles->setMaximumSize( 32767, 32767 );
  gnufiles->setFocusPolicy( QWidget::TabFocus );
  gnufiles->setBackgroundMode( QWidget::PaletteBackground );
  gnufiles->setFontPropagation( QWidget::NoChildren );
  gnufiles->setPalettePropagation( QWidget::NoChildren );
  gnufiles->setText( i18n("GNU-Standard-Files (INSTALL,README,COPYING...)" ));
  gnufiles->setAutoRepeat( FALSE );
  gnufiles->setAutoResize( FALSE );

  userdoc = new QCheckBox( widget1, "userdoc" );
  userdoc->setGeometry( 30, 300, 330, 30 );
  userdoc->setMinimumSize( 0, 0 );
  userdoc->setMaximumSize( 32767, 32767 );
  userdoc->setFocusPolicy( QWidget::TabFocus );
  userdoc->setBackgroundMode( QWidget::PaletteBackground );
  userdoc->setFontPropagation( QWidget::NoChildren );
  userdoc->setPalettePropagation( QWidget::NoChildren );
  userdoc->setText( i18n("User-Documentation") );
  userdoc->setAutoRepeat( FALSE );
  userdoc->setAutoResize( FALSE );

  apidoc = new QCheckBox( widget1, "apidoc" );
  apidoc->setGeometry( 30, 330, 200, 30 );
  apidoc->setMinimumSize( 0, 0 );
  apidoc->setMaximumSize( 32767, 32767 );
  apidoc->setFocusPolicy( QWidget::TabFocus );
  apidoc->setBackgroundMode( QWidget::PaletteBackground );
  apidoc->setFontPropagation( QWidget::NoChildren );
  apidoc->setPalettePropagation( QWidget::NoChildren );
  apidoc->setText( i18n("API-Documentation") );
  apidoc->setAutoRepeat( FALSE );
  apidoc->setAutoResize( FALSE );

  lsmfile = new QCheckBox( widget1, "lsmfile" );
  lsmfile->setGeometry( 30, 360, 340, 30 );
  lsmfile->setMinimumSize( 0, 0 );
  lsmfile->setMaximumSize( 32767, 32767 );
  lsmfile->setFocusPolicy( QWidget::TabFocus );
  lsmfile->setBackgroundMode( QWidget::PaletteBackground );
  lsmfile->setFontPropagation( QWidget::NoChildren );
  lsmfile->setPalettePropagation( QWidget::NoChildren );
  lsmfile->setText( i18n("lsm-File - Linux Software Map") );
  lsmfile->setAutoRepeat( FALSE );
  lsmfile->setAutoResize( FALSE );

  datalink = new QCheckBox( widget1, "datalink" );
  datalink->setGeometry( 30, 390, 200, 30 );
  datalink->setMinimumSize( 0, 0 );
  datalink->setMaximumSize( 32767, 32767 );
  datalink->setFocusPolicy( QWidget::TabFocus );
  datalink->setBackgroundMode( QWidget::PaletteBackground );
  datalink->setFontPropagation( QWidget::NoChildren );
  datalink->setPalettePropagation( QWidget::NoChildren );
  datalink->setText( i18n(".kdelnk-File") );
  datalink->setAutoRepeat( FALSE );
  datalink->setAutoResize( FALSE );
  
  progicon = new QCheckBox( widget1, "progicon" );
  progicon->setGeometry( 290, 330, 110, 30 );
  progicon->setMinimumSize( 0, 0 );
  progicon->setMaximumSize( 32767, 32767 );
  progicon->setFocusPolicy( QWidget::TabFocus );
  progicon->setBackgroundMode( QWidget::PaletteBackground );
  progicon->setFontPropagation( QWidget::NoChildren );
  progicon->setPalettePropagation( QWidget::NoChildren );
  progicon->setText( i18n("Program-Icon") );
  progicon->setAutoRepeat( FALSE );
  progicon->setAutoResize( FALSE );

  iconload = new QPushButton( widget1, "iconload" );
  iconload->setGeometry( 410, 310, 60, 60 );
  iconload->setMinimumSize( 0, 0 );
  iconload->setMaximumSize( 32767, 32767 );
  iconload->setFocusPolicy( QWidget::TabFocus );
  iconload->setBackgroundMode( QWidget::PaletteBackground );
  iconload->setFontPropagation( QWidget::NoChildren );
  iconload->setPalettePropagation( QWidget::NoChildren );
  iconload->setText( "" );
  iconload->setAutoRepeat( FALSE );
  iconload->setAutoResize( FALSE );

  miniicon = new QCheckBox( widget1, "miniicon" );
  miniicon->setGeometry( 290, 390, 110, 30 );
  miniicon->setMinimumSize( 0, 0 );
  miniicon->setMaximumSize( 32767, 32767 );
  miniicon->setFocusPolicy( QWidget::TabFocus );
  miniicon->setBackgroundMode( QWidget::PaletteBackground );
  miniicon->setFontPropagation( QWidget::NoChildren );
  miniicon->setPalettePropagation( QWidget::NoChildren );
  miniicon->setText( i18n("Mini-Icon") );
  miniicon->setAutoRepeat( FALSE );
  miniicon->setAutoResize( FALSE );
  
  miniload = new QPushButton( widget1, "miniload" );
  miniload->setGeometry( 440, 390, 30, 30 );
  miniload->setMinimumSize( 0, 0 );
  miniload->setMaximumSize( 32767, 32767 );
  miniload->setFocusPolicy( QWidget::TabFocus );
  miniload->setBackgroundMode( QWidget::PaletteBackground );
  miniload->setFontPropagation( QWidget::NoChildren );
  miniload->setPalettePropagation( QWidget::NoChildren );
  miniload->setText( "" );
  miniload->setAutoRepeat( FALSE );
  miniload->setAutoResize( FALSE );
  
  separator1 = new KSeparator (widget1);
  separator1->setGeometry(0,210,515,5);
  
  separator2 = new KSeparator (widget1);
  separator2->setGeometry(0,255,515,5);

  QString text;

  text = i18n("Insert your project name here. This is\n"
              "also the name of the directory where your project\n"
              "will be created.");
  QWhatsThis::add(name, text);
  QWhatsThis::add(nameline, text);
  
  text = i18n("Enter the toplevel directory of your project.\n");
  QWhatsThis::add(directory, text);
  QWhatsThis::add(directoryline, text);
  QWhatsThis::add(directoryload, text);
  
  text =  i18n("Set the initial version number of your project here.\n"
               "The number will be used in the about-dialog as well as for\n"
               "determining the project's package numbering for distribution.");
  QWhatsThis::add(versionnumber, text);
  QWhatsThis::add(versionline, text);

  text = i18n("Insert your name or the development team name here. This will be used\n"
              "for adding your name as the author to all generated files of your project.");
  QWhatsThis::add(authorname, text);
  QWhatsThis::add(authorline, text);
      
  text = i18n("Enter your email adress here. This will be\n"
              "used for file header information.");
  QWhatsThis::add(email, text);
  QWhatsThis::add(emailline, text);
  
  text = i18n("Generate a HTML-based documentation set\n"
              "for your project classes including cross-references to\n"
              "the used libraries.");
  QWhatsThis::add(apidoc, text);

  text = i18n("Generate a preset documentation handbook in HTML\n"
                                "by an SGML file included with your package.");
  QWhatsThis::add(userdoc, text);
  
  text = i18n("Create a lsm-file for your project. The Linux Software Map\n"
              "is a file generally used for projects for distribution purpose and contains\n"
              "a short description of the project including the requirements on the side of\n"
              "the end-user.");
  QWhatsThis::add(lsmfile, text);

  
  text = i18n("Generate a set of GNU standard files for your project.\n"
              "These will give the end-user of the sourcepackage an overview about\n"
              "the licensing, readme's etc, as well as a ChangeLog file for you to\n"
              "protocol your changes.");
  QWhatsThis::add(gnufiles, text);
                
  text = i18n("Add a program icon to your project that represents\n"
              "your application in the window manager.");
  QWhatsThis::add(progicon, text);

  text = i18n("Add a mini program icon to your project that is used\n"
                                 "for window-manager popup menus.");
  QWhatsThis::add(miniicon, text);
                
  text = i18n("Add a KDE link file which is installed in\n"
              "the KDE panel of the end user. By default, your application's\n"
              "link file will be installed in the Applications menu. You can\n"
              "change this destination by editing the installation properties\n"
              "for the link file later.");
  QWhatsThis::add(datalink, text);

  text = i18n("Lets you select another icon for your program\n"
              "than the sample program icon provided by the Application\n"
              "Wizard.");
  QWhatsThis::add(iconload, text);
  
  text = i18n("Lets you select another mini-icon for your program\n"
              "than the sample program icon provided by the Application\n"
              "Wizard.");
  QWhatsThis::add(miniload, text);  
  
  connect(nameline,SIGNAL(textChanged(const QString &)),SLOT(slotProjectnameEntry()));
	connect(directoryline,SIGNAL(textChanged(const QString &)),SLOT(slotDirectoryEntry()));
  connect(directoryload,SIGNAL(clicked()),SLOT(slotDirDialogClicked()));
  connect(miniload,SIGNAL(clicked()),SLOT(slotMiniIconButtonClicked()));
  connect(progicon,SIGNAL(clicked()),SLOT(slotProgIconClicked()));   
  connect(miniicon,SIGNAL(clicked()),SLOT(slotMiniIconClicked()));   
  connect(iconload,SIGNAL(clicked()),SLOT(slotIconButtonClicked()));

  /************************************************************/

  // create the thirth page
  widget1c = new QWidget(this);
  addPage(widget1c, i18n("Version Control System Support"));

	qtarch_ButtonGroup_1 = new QButtonGroup( widget1c, "ButtonGroup_1" );
	qtarch_ButtonGroup_1->setGeometry( 20, 50, 460, 360 );
	qtarch_ButtonGroup_1->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_1->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_1->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setFrameStyle( 49 );
	qtarch_ButtonGroup_1->setAlignment( 1 );

	vsSupport = new QLabel( widget1c, "vsSupport" );
	vsSupport->setGeometry( 30, 10, 150, 30 );
	vsSupport->setMinimumSize( 0, 0 );
	vsSupport->setMaximumSize( 32767, 32767 );
	vsSupport->setFocusPolicy( QWidget::NoFocus );
	vsSupport->setBackgroundMode( QWidget::PaletteBackground );
	vsSupport->setFontPropagation( QWidget::NoChildren );
	vsSupport->setPalettePropagation( QWidget::NoChildren );
	vsSupport->setText(i18n( "vcs support" ));
	vsSupport->setAlignment( 289 );
	vsSupport->setMargin( -1 );	
	
	vsBox = new QComboBox( FALSE, widget1c, "vsBox" );
	vsBox->setGeometry( 180, 10, 100, 30 );
	vsBox->setMinimumSize( 0, 0 );
	vsBox->setMaximumSize( 32767, 32767 );
	vsBox->setFocusPolicy( QWidget::StrongFocus );
	vsBox->setBackgroundMode( QWidget::PaletteBackground );
	vsBox->setFontPropagation( QWidget::AllChildren );
	vsBox->setPalettePropagation( QWidget::AllChildren );
	vsBox->setSizeLimit( 10 );
	vsBox->setAutoResize( FALSE );
	vsBox->insertItem( "NONE" );
	vsBox->insertItem( "CVS" );	

	vsInstall = new QLabel( widget1c, "vsInstall" );
	vsInstall->setGeometry( 40, 80, 140, 30 );
	vsInstall->setMinimumSize( 0, 0 );
	vsInstall->setMaximumSize( 32767, 32767 );
	vsInstall->setFocusPolicy( QWidget::NoFocus );
	vsInstall->setBackgroundMode( QWidget::PaletteBackground );
	vsInstall->setFontPropagation( QWidget::NoChildren );
	vsInstall->setPalettePropagation( QWidget::NoChildren );
	vsInstall->setText(i18n( "vcs location") );
	vsInstall->setAlignment( 289 );
	vsInstall->setMargin( -1 );
	
	vsLocation = new QLineEdit( widget1c, "vsLocation" );
	vsLocation->setGeometry( 180, 80, 240, 30 );
	vsLocation->setMinimumSize( 0, 0 );
	vsLocation->setMaximumSize( 32767, 32767 );
	vsLocation->setFocusPolicy( QWidget::StrongFocus );
	vsLocation->setBackgroundMode( QWidget::PaletteBase );
	vsLocation->setFontPropagation( QWidget::NoChildren );
	vsLocation->setPalettePropagation( QWidget::NoChildren );
	vsLocation->setText( "" );
	vsLocation->setMaxLength( 32767 );
	vsLocation->setEchoMode( QLineEdit::Normal );
	vsLocation->setFrame( TRUE );

	locationbutton = new QPushButton( widget1c, "locationbutton" );
	locationbutton->setGeometry( 430, 80, 30, 30 );
	locationbutton->setMinimumSize( 0, 0 );
	locationbutton->setMaximumSize( 32767, 32767 );
	locationbutton->setFocusPolicy( QWidget::TabFocus );
	locationbutton->setBackgroundMode( QWidget::PaletteBackground );
	locationbutton->setFontPropagation( QWidget::NoChildren );
	locationbutton->setPalettePropagation( QWidget::NoChildren );
	locationbutton->setText( "..." );
	locationbutton->setAutoRepeat( FALSE );
	locationbutton->setAutoResize( FALSE );	

	projectVSLocation = new QLabel( widget1c, "projectVSLocation" );
	projectVSLocation->setGeometry( 40, 150, 140, 30 );
	projectVSLocation->setMinimumSize( 0, 0 );
	projectVSLocation->setMaximumSize( 32767, 32767 );
	projectVSLocation->setFocusPolicy( QWidget::NoFocus );
	projectVSLocation->setBackgroundMode( QWidget::PaletteBackground );
	projectVSLocation->setFontPropagation( QWidget::NoChildren );
	projectVSLocation->setPalettePropagation( QWidget::NoChildren );
	projectVSLocation->setText(i18n( "repository in vcs") );
	projectVSLocation->setAlignment( 289 );
	projectVSLocation->setMargin( -1 );	
	
	projectlocationline = new QLineEdit( widget1c, "projectlocationline" );
	projectlocationline->setGeometry( 180, 150, 280, 30 );
	projectlocationline->setMinimumSize( 0, 0 );
	projectlocationline->setMaximumSize( 32767, 32767 );
	projectlocationline->setFocusPolicy( QWidget::StrongFocus );
	projectlocationline->setBackgroundMode( QWidget::PaletteBase );
	projectlocationline->setFontPropagation( QWidget::NoChildren );
	projectlocationline->setPalettePropagation( QWidget::NoChildren );
	projectlocationline->setText( "" );
	projectlocationline->setMaxLength( 32767 );
	projectlocationline->setEchoMode( QLineEdit::Normal );
	projectlocationline->setFrame( TRUE );

	vendorTag = new QLabel( widget1c, "vendorTag" );
	vendorTag->setGeometry( 40, 220, 140, 30 );
	vendorTag->setMinimumSize( 0, 0 );
	vendorTag->setMaximumSize( 32767, 32767 );
	vendorTag->setFocusPolicy( QWidget::NoFocus );
	vendorTag->setBackgroundMode( QWidget::PaletteBackground );
	vendorTag->setFontPropagation( QWidget::NoChildren );
	vendorTag->setPalettePropagation( QWidget::NoChildren );
	vendorTag->setText(i18n( "vendor tag") );
	vendorTag->setAlignment( 289 );
	vendorTag->setMargin( -1 );

	vendorline = new QLineEdit( widget1c, "vendorline" );
	vendorline->setGeometry( 180, 220, 280, 30 );
	vendorline->setMinimumSize( 0, 0 );
	vendorline->setMaximumSize( 32767, 32767 );
	vendorline->setFocusPolicy( QWidget::StrongFocus );
	vendorline->setBackgroundMode( QWidget::PaletteBase );
	vendorline->setFontPropagation( QWidget::NoChildren );
	vendorline->setPalettePropagation( QWidget::NoChildren );
	vendorline->setText( "" );
	vendorline->setMaxLength( 32767 );
	vendorline->setEchoMode( QLineEdit::Normal );
	vendorline->setFrame( TRUE );
							
	logMessage = new QLabel( widget1c, "logMessage" );
	logMessage->setGeometry( 40, 290, 140, 30 );
	logMessage->setMinimumSize( 0, 0 );
	logMessage->setMaximumSize( 32767, 32767 );
	logMessage->setFocusPolicy( QWidget::NoFocus );
	logMessage->setBackgroundMode( QWidget::PaletteBackground );
	logMessage->setFontPropagation( QWidget::NoChildren );
	logMessage->setPalettePropagation( QWidget::NoChildren );
	logMessage->setText(i18n( "log message") );
	logMessage->setAlignment( 289 );
	logMessage->setMargin( -1 );
	
	messageline = new QLineEdit( widget1c, "messageline" );
	messageline->setGeometry( 180, 290, 280, 30 );
	messageline->setMinimumSize( 0, 0 );
	messageline->setMaximumSize( 32767, 32767 );
	messageline->setFocusPolicy( QWidget::StrongFocus );
	messageline->setBackgroundMode( QWidget::PaletteBase );
	messageline->setFontPropagation( QWidget::NoChildren );
	messageline->setPalettePropagation( QWidget::NoChildren );
	messageline->setText( "" );
	messageline->setMaxLength( 32767 );
	messageline->setEchoMode( QLineEdit::Normal );
	messageline->setFrame( TRUE );
	
	releaseTag = new QLabel( widget1c, "releaseTag" );
	releaseTag->setGeometry( 40, 360, 140, 30 );
	releaseTag->setMinimumSize( 0, 0 );
	releaseTag->setMaximumSize( 32767, 32767 );
	releaseTag->setFocusPolicy( QWidget::NoFocus );
	releaseTag->setBackgroundMode( QWidget::PaletteBackground );
	releaseTag->setFontPropagation( QWidget::NoChildren );
	releaseTag->setPalettePropagation( QWidget::NoChildren );
	releaseTag->setText(i18n( "release tag") );
	releaseTag->setAlignment( 289 );
	releaseTag->setMargin( -1 );

	releaseline = new QLineEdit( widget1c, "releaseline" );
	releaseline->setGeometry( 180, 360, 280, 30 );
	releaseline->setMinimumSize( 0, 0 );
	releaseline->setMaximumSize( 32767, 32767 );
	releaseline->setFocusPolicy( QWidget::StrongFocus );
	releaseline->setBackgroundMode( QWidget::PaletteBase );
	releaseline->setFontPropagation( QWidget::NoChildren );
	releaseline->setPalettePropagation( QWidget::NoChildren );
	releaseline->setText( "" );
	releaseline->setMaxLength( 32767 );
	releaseline->setEchoMode( QLineEdit::Normal );
	releaseline->setFrame( TRUE );			
		
	qtarch_ButtonGroup_1->insert( locationbutton );
  					
	connect(locationbutton,SIGNAL(clicked()),SLOT(slotLocationButtonClicked()));
	connect(vsBox,SIGNAL(activated(int)),SLOT(slotVSBoxChanged(int)));
	connect(vendorline,SIGNAL(textChanged(const QString &)),SLOT(slotVendorEntry()));
	
	projectlocationline->setEnabled(false);

        text = i18n("Here you can enter the log message for the\n"
                    "version control system.");
	QWhatsThis::add(messageline, text);
        QWhatsThis::add(logMessage, text);
				
	text = i18n("Here you can choose the vendor tag, which your project\n"
                    "has in the version control system.");
	QWhatsThis::add(vendorTag, text);
        QWhatsThis::add(vendorline, text);

        text = i18n("Here you can choose a special pointer for the first\n"
                    "entry in the version control system.");
	QWhatsThis::add(releaseTag, text);
        QWhatsThis::add(releaseline, text);
				
	text = i18n("Here you can choose the version control system,\n"
                    "which you want to use.");
	QWhatsThis::add(vsBox, text);
        QWhatsThis::add(vsSupport, text);
				
        text = i18n("Here you can choose where your vcsroot loction should be.\n"
                    "At the moment we only support local vs. And be sure, you\n"
                    "have read and write access in the location.");
	QWhatsThis::add(vsInstall, text);
        QWhatsThis::add(vsLocation, text);
        QWhatsThis::add(locationbutton, text);
				
        text = i18n("Here you can see the repository of your project in the\n"
                    "version control system. The repository is dependend on\n"
                    "the directory of your project. You can not change the\n"
                    "repository directly.");
	QWhatsThis::add(projectVSLocation, text);
        QWhatsThis::add(projectlocationline, text);
				
				
  /************************************************************/
  
  // create the forth page
  widget2 = new QWidget(this);
  addPage(widget2, i18n("Headertemplate for .h-files"));
  
  hheader = new QCheckBox( widget2, "hheader" );
  hheader->setGeometry( 20, 20, 230, 30 );
  hheader->setMinimumSize( 0, 0 );
  hheader->setMaximumSize( 32767, 32767 );
  hheader->setFocusPolicy( QWidget::TabFocus );
  hheader->setBackgroundMode( QWidget::PaletteBackground );
  hheader->setFontPropagation( QWidget::NoChildren );
  hheader->setPalettePropagation( QWidget::NoChildren );
  hheader->setText( i18n("headertemplate for .h-files") );
  hheader->setAutoRepeat( FALSE );
  hheader->setAutoResize( FALSE );

  hload = new QPushButton( widget2, "hload" );
  hload->setGeometry( 260, 20, 100, 30 );
  hload->setMinimumSize( 0, 0 );
  hload->setMaximumSize( 32767, 32767 );
  hload->setFocusPolicy( QWidget::TabFocus );
  hload->setBackgroundMode( QWidget::PaletteBackground );
  hload->setFontPropagation( QWidget::NoChildren );
  hload->setPalettePropagation( QWidget::NoChildren );
  hload->setText(i18n( "Load..." ));
  hload->setAutoRepeat( FALSE );
  hload->setAutoResize( FALSE );
  
  hnew = new QPushButton( widget2, "hnew" );
  hnew->setGeometry( 380, 20, 100, 30 );
  hnew->setMinimumSize( 0, 0 );
  hnew->setMaximumSize( 32767, 32767 );
  hnew->setFocusPolicy( QWidget::TabFocus );
  hnew->setBackgroundMode( QWidget::PaletteBackground );
  hnew->setFontPropagation( QWidget::NoChildren );
  hnew->setPalettePropagation( QWidget::NoChildren );
  hnew->setText( i18n("New" ));
  hnew->setAutoRepeat( FALSE );
  hnew->setAutoResize( FALSE );
  
  hedit = new KEdit( kapp,widget2 );
  QFont f("fixed",10);
  hedit->setFont(f);
  hedit->setGeometry( 20, 70, 460, 350 );
  hedit->setMinimumSize( 0, 0 );
  hedit->setMaximumSize( 32767, 32767 );
  hedit->setFocusPolicy( QWidget::StrongFocus );
  hedit->setBackgroundMode( QWidget::PaletteBase );
  hedit->setFontPropagation( QWidget::SameFont );
  hedit->setPalettePropagation( QWidget::SameFont );
  hedit->insertLine( "" );
  hedit->setReadOnly( FALSE );
  hedit->setOverwriteMode( FALSE );
  
  QToolTip::add(hload,i18n("Press this button to select an\n"
  													"existing header template file"));
  QToolTip::add(hnew,i18n("Clears the pre-set headertemplate"));
  QToolTip::add(hedit,i18n("Edit your headertemplate here"));
  
  QWhatsThis::add(hheader, i18n("Use a standard\n"
				"headertemplate for your headerfiles"));
  
  
  connect(hheader,SIGNAL(clicked()),SLOT(slotHeaderHeaderClicked()));   
  connect(hload,SIGNAL(clicked()),SLOT(slotHeaderDialogClicked()));
  connect(hnew,SIGNAL(clicked()),SLOT(slotNewHeaderButtonClicked()));

  /************************************************************/
  
  // create the fifth page
  widget3 = new QWidget(this);
  addPage(widget3, i18n("Headertemplate for .cpp/.c-files"));
  
  cppheader = new QCheckBox( widget3, "cppheader" );
  cppheader->setGeometry( 20, 20, 230, 30 );
  cppheader->setMinimumSize( 0, 0 );
  cppheader->setMaximumSize( 32767, 32767 );
  cppheader->setFocusPolicy( QWidget::TabFocus );
  cppheader->setBackgroundMode( QWidget::PaletteBackground );
  cppheader->setFontPropagation( QWidget::NoChildren );
  cppheader->setPalettePropagation( QWidget::NoChildren );
  cppheader->setText( i18n("headertemplate for .cpp-files") );
  cppheader->setAutoRepeat( FALSE );
  cppheader->setAutoResize( FALSE );

  cppload = new QPushButton( widget3, "cppload" );
  cppload->setGeometry( 260, 20, 100, 30 );
  cppload->setMinimumSize( 0, 0 );
  cppload->setMaximumSize( 32767, 32767 );
  cppload->setFocusPolicy( QWidget::TabFocus );
  cppload->setBackgroundMode( QWidget::PaletteBackground );
  cppload->setFontPropagation( QWidget::NoChildren );
  cppload->setPalettePropagation( QWidget::NoChildren );
  cppload->setText( i18n("Load...") );
  cppload->setAutoRepeat( FALSE );
  cppload->setAutoResize( FALSE );
  
  cppnew = new QPushButton( widget3, "cppnew" );
  cppnew->setGeometry( 380, 20, 100, 30 );
  cppnew->setMinimumSize( 0, 0 );
  cppnew->setMaximumSize( 32767, 32767 );
  cppnew->setFocusPolicy( QWidget::TabFocus );
  cppnew->setBackgroundMode( QWidget::PaletteBackground );
  cppnew->setFontPropagation( QWidget::NoChildren );
  cppnew->setPalettePropagation( QWidget::NoChildren );
  cppnew->setText( i18n("New") );
  cppnew->setAutoRepeat( FALSE );
  cppnew->setAutoResize( FALSE );

  cppedit = new KEdit(kapp,widget3);
  cppedit->setFont(f);
  cppedit->setGeometry( 20, 70, 460, 350 );
  cppedit->setMinimumSize( 0, 0 );
  cppedit->setMaximumSize( 32767, 32767 );
  cppedit->setFocusPolicy( QWidget::StrongFocus );
  cppedit->setBackgroundMode( QWidget::PaletteBase );
  cppedit->setFontPropagation( QWidget::SameFont );
  cppedit->setPalettePropagation( QWidget::SameFont );
  cppedit->insertLine( "" );
  cppedit->setReadOnly( FALSE );
  cppedit->setOverwriteMode( FALSE );
  
  QToolTip::add(cppload,i18n("Press this button to select an\n"
  													"existing header template file"));
  QToolTip::add(cppnew,i18n("Clears the pre-set headertemplate"));
  QToolTip::add(cppedit,i18n("Edit your headertemplate here"));

  QWhatsThis::add(cppheader, i18n("Use a standard\n"
				"headertemplate for your implementation files"));


  connect(cppheader,SIGNAL(clicked()),SLOT(slotCppHeaderClicked()));   
  connect(cppload,SIGNAL(clicked()),SLOT(slotCppDialogClicked()));
  connect(cppnew,SIGNAL(clicked()),SLOT(slotNewCppButtonClicked()));   

  /************************************************************/
  
  // create the sixth page
  widget4 = new QWidget(this);
  addPage(widget4, i18n("Processes"));
  
  // create a MultiLineEdit for the processes of kAppWizard
  output = new QMultiLineEdit( widget4, "output" );
  output->setGeometry( 10, 10, 480, 330 );
  output->setMinimumSize( 0, 0 );
  output->setMaximumSize( 32767, 32767 );
  output->setFocusPolicy( QWidget::StrongFocus );
  output->setBackgroundMode( QWidget::PaletteBase );
  output->setFontPropagation( QWidget::SameFont );
  output->setPalettePropagation( QWidget::SameFont );
  output->insertLine( "" );
  output->setReadOnly( FALSE );
  output->setOverwriteMode( FALSE );
  
  errOutput = new QMultiLineEdit( widget4, "errOutput" );
  errOutput->setGeometry( 10, 340, 480, 80 );
  errOutput->setMinimumSize( 0, 0 );
  errOutput->setMaximumSize( 32767, 32767 );
  errOutput->setFocusPolicy( QWidget::StrongFocus );
  errOutput->setBackgroundMode( QWidget::PaletteBase );
  errOutput->setFontPropagation( QWidget::SameFont );
  errOutput->setPalettePropagation( QWidget::SameFont );
  errOutput->insertLine( "" );
  errOutput->setReadOnly( FALSE );
  errOutput->setOverwriteMode( FALSE );
  QFont font("helvetica",10);
  output->setFont(font);
  QToolTip::add(output,i18n("Displays the normal output of the project generator"));
  errOutput->setFont(font);
  QToolTip::add(errOutput,i18n("Displays warnings and errormessages of the project generator"));
  // go to page 2 then to page 1
#warning Substitution would be senseless??
/*
  showPage(widget1);
  showPage(widget0);
*/
  //  gotoPage(1);
  //  gotoPage(0);

  setMinimumSize(515,500);

}

// connection to directoryload
void CKAppWizard::slotDirDialogClicked() {
  QString projname;
  KDirDialog *dirdialog = new KDirDialog(dir,this,"Directory");
  dirdialog->setCaption (i18n("Directory"));
  dirdialog->show();
  projname = nameline->text();
  if (!modifyDirectory) {
  	dir = dirdialog->dirPath() + projname.lower();
  	directoryline->setText(dir);
  }
  else {
  	dir = dirdialog->dirPath();
    directoryline->setText(dir);
  }
  dir = dirdialog->dirPath();
  delete (dirdialog);
}

// connection of hload
void CKAppWizard::slotHeaderDialogClicked() {
  KFileDialog *headerdialog = new KFileDialog(QDir::homeDirPath(),"*",this,"Headertemplate",true,true);
  headerdialog->setCaption (i18n("Select your template for Header-file headers"));
  if(headerdialog->exec()){
    hedit->loadFile(headerdialog->selectedFile(),cppedit->OPEN_READWRITE);
  }
  delete (headerdialog);
}

// connection of cppload
void CKAppWizard::slotCppDialogClicked() {
  KFileDialog *cppdialog = new KFileDialog(QDir::homeDirPath(),"*",this,"Cpptemplate",true,true);
  cppdialog->setCaption( (citem->isSelected()) ?
                         i18n("Select your template for C-file headers")
                         :
                         i18n("Select your template for Cpp-file headers")
			);
  if(cppdialog->exec()){
    cppedit->loadFile(cppdialog->selectedFile(),cppedit->OPEN_READWRITE);
  }
  delete (cppdialog);
}

// connection of hnew
void CKAppWizard::slotNewHeaderButtonClicked() {
  hedit->clear();
}

// connection of cppnew
void CKAppWizard::slotNewCppButtonClicked() {
  cppedit->clear();
}

// connection of this (defaultButton)
void CKAppWizard::accept() {

  if (!(CToolClass::searchInstProgram("sgml2html") || CToolClass::searchInstProgram("ksgml2html"))) {
  	userdoc->setChecked(false);
  	KMessageBox::sorry(0,i18n("sgml2html and ksgml2html do not exist!\n"
                                  "If you want to generate the user-documentation, you need one of these programs.\n"
    		 "If you do not have one, the user-documentation will not be generate."));
  }


  if (vsBox->currentItem() == 1) {
    if (!CToolClass::searchProgram("cvs")) {
      return;
    }
  }
  QDir dir;
  QString direct = directoryline->text();

  if (direct.right(1) == "/")
      direct.truncate(direct.length()-1); 
  direct.truncate(direct.findRev('/'));
  dir.setPath(direct);

  KShellProcess p;
  if (!dir.exists()) {
 		p.clearArguments();
		p << "mkdirhier";
		p << direct;
		p.start(KProcess::Block,KProcess::AllOutput);	
  }

  dir.setPath(directoryline->text());
  if (dir.exists()) {
    QString msg = i18n("%1 already exists!\nIt isn't possible to generate a new project into an existing directory.").arg(directoryline->text());
    KMessageBox::sorry( this, msg);
    return;
  }
  else {
    okPermited();
  }
}

void CKAppWizard::okPermited() {
  cancelButton()->setEnabled(false);
#warning FIXME
  //  defaultButton->setEnabled(false);
  cancelButton()->setText(i18n("Exit"));
  errOutput->clear();
  output->clear();
  QDir kdevelop;
  kdevelop.mkdir(locateLocal("appdata", ""));
  // cerr << locateLocal("appdata", "");
  cppedit->setName(locateLocal("appdata","cpp"));
  cppedit->toggleModified(true);
  cppedit->doSave();
  hedit->setName(locateLocal("appdata","header"));
  hedit->toggleModified(true);
  hedit->doSave();
  ofstream entries (locateLocal("appdata", "entries"));
  entries << "APPLICATION\n";
  if (kdenormalitem->isSelected()) {
    entries << "kdenormal\n";
  }
  else if (kdeminiitem->isSelected()) {
    entries << "kdemini\n";
  }
  /*  else if (corbaitem->isSelected()) {
      entries << "corba\n";
      }
      else if (komitem->isSelected()) {
      entries << "kom\n";
      }*/
  else if (qtnormalitem->isSelected()) {
    entries << "qtnormal\n";
  }
  /* else if (qtminiitem->isSelected()) {
     entries << "qtmini\n";
     }*/
  else if (cppitem->isSelected()) {
    entries << "cpp\n";
  }
  else if (citem->isSelected()) {
    entries << "c\n";
    }
  /*  else if (gtknormalitem->isSelected()) {
    entries << "gtknormal\n";
    }
    else if (gtkminiitem->isSelected()) {
    entries << "gtkmini\n";
    }*/
  else if (customprojitem->isSelected()) {
    entries << "customproj\n";
  }
  entries << "NAME\n";
  entries << nameline->text() << "\n";
  entries << "DIRECTORY\n";

	QString direct = directoryline->text();
  if (direct.right(1) == "/") {
		direct = direct.left(direct.length() - 1);
  }
	int pos;
	pos = direct.findRev ("/");
	direct = direct.left (pos);
  if(direct.right(1) == "/"){
    entries << direct << "\n";
  }
  else{
    entries << direct << "/\n";
  }
  
  
  entries << "AUTHOR\n";
  entries << authorline->text() << "\n";
  entries << "EMAIL\n";
  entries << emailline->text() << "\n";
  entries << "API\n";
  if (apidoc->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "USER\n";
  if (userdoc->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "LSM\n";
  if (lsmfile->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "GNU\n";
  if (gnufiles->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "PROGICON\n";
  if (progicon->isChecked()) {
    entries << name1 << "\n";
  }
  else entries << "no\n";
  entries << "MINIICON\n";
  if (miniicon->isChecked())
    entries << name2 << "\n";
  else entries << "no\n";
  entries << "KDELNK\n";
  if (datalink->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "HEADER\n";
  if (hheader->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "CPP\n";
  if (cppheader->isChecked())
    entries << "yes\n";
  else entries << "no\n";
  entries << "VERSION\n";
  entries << versionline->text() << "\n";
  entries << "VSSUPPORT\n";
  entries << QString(vsBox->text(vsBox->currentItem())).lower() + "\n";
  entries << "VENDORTAG\n";
  entries << QString(vendorline->text()) + "\n";
  entries << "RELEASETAG\n";
  entries << QString(releaseline->text()) + "\n";
  entries << "VSLOCATION\n";
  entries << QString(vsLocation->text()) + "\n";
  entries << "PRJVSLOCATION\n";
  entries << QString(projectlocationline->text()) + "\n";
  entries << "LOGMESSAGE\n";
  entries << QString(messageline->text()) + "\n";


  namelow = nameline->text();
  namelow = namelow.lower();
  QDir dir;
  dir.mkdir(directoryline->text() + QString("/"));
  if (!dir.exists())
     dir.setCurrent(directoryline->text() + QString("/"));
  KShellProcess p;
  QString copysrc;
  QString copydes;
  QString vcsInit;
  if (vsBox->currentItem() == 1) {
  	copydes = locateLocal("appdata", "kdeveloptemp");
  	dir.mkdir(copydes);
  	dir.setCurrent(copydes);
  	vcsInit = (QString) "cvs -d " + vsLocation->text() + (QString) " init";
  	p.clearArguments();
	  p << vcsInit;
	  p.start(KProcess::Block,KProcess::AllOutput);

 	}
 	else {
 	  copydes = directoryline->text() + QString ("/");
 	}
 	
 	p.clearArguments();
  if (kdeminiitem->isSelected()) {
    copysrc = locate("appdata",  "templates/mini.tar.gz");
    p << "cp " + (QString) "'" + copysrc + (QString) "' '" + copydes + (QString) "'";
    p.start(KProcess::Block,KProcess::AllOutput);
  }
  else if (kdenormalitem->isSelected()) {
    copysrc = locate("appdata", "templates/normal.tar.gz");
    p << "cp " + (QString) "'" + copysrc + (QString) "' '" + copydes + (QString) "'";
    p.start(KProcess::Block,KProcess::AllOutput);
  }
  else if (qtnormalitem->isSelected()) {
    copysrc = locate("appdata","templates/qt.tar.gz");
    p << "cp " + (QString) "'" + copysrc + (QString) "' '" + copydes + (QString) "'";
    p.start(KProcess::Block,KProcess::AllOutput);
  }
  else if (cppitem->isSelected()) {
    copysrc = locate("appdata", "templates/cpp.tar.gz");
    p << "cp " + (QString) "'" + copysrc + (QString) "' '" + copydes + (QString) "'";
    p.start(KProcess::Block,KProcess::AllOutput);
  }
  else if (citem->isSelected()) {
    copysrc = locate("appdata", "templates/c.tar.gz");
    p << "cp " + (QString) "'" + copysrc + (QString) "' '" + copydes + (QString) "'";
    p.start(KProcess::Block,KProcess::AllOutput);
  }

  dir.setCurrent("/");

  q->clearArguments();
  connect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  connect(q,SIGNAL(receivedStdout(KProcess *, char *, int)),
	  this,SLOT(slotPerlOut(KProcess *, char *, int)));
  connect(q,SIGNAL(receivedStderr(KProcess *, char *, int)),
	  this,SLOT(slotPerlErr(KProcess *, char *, int)));
  QString path = locate("appdata","tools/processes.pl");
  *q << "perl" << path ;
  q->start(KProcess::NotifyOnExit, KProcess::AllOutput);
  setFinishEnabled(widget4, false);
  setBackEnabled(widget4, false);
//  finishButton()->setEnable(false);

  showPage(widget4);
  //  gotoPage(5);
/*  int i;
  for (i=0;i<5;i++) {
      //    setPageEnabled(i,false);
  }
*/
  removePage(widget0);
  removePage(widget1);
  removePage(widget2);
  removePage(widget3);

  apidoc->setEnabled(false);
  lsmfile->setEnabled(false);
  cppheader->setEnabled(false);
  hheader->setEnabled(false);
  datalink->setEnabled(false);
  miniicon->setEnabled(false);
  progicon->setEnabled(false);
  gnufiles->setEnabled(false);
  userdoc->setEnabled(false);
  directoryline->setEnabled(false);
  nameline->setEnabled(false);
//  finishButton()->setEnabled(false);
  miniload->setEnabled(false);
  iconload->setEnabled(false);
  cppedit->setEnabled(false);
  hedit->setEnabled(false);
  authorline->setEnabled(false);
  emailline->setEnabled(false);
  versionline->setEnabled(false);
  hnew->setEnabled(false);
  hload->setEnabled(false);
  cppnew->setEnabled(false);
  cppload->setEnabled(false);
  applications->setEnabled(false);
  generatesource->setEnabled(false);
  apphelp->setEnabled(false);
  messageline->setEnabled(false);
 	logMessage->setEnabled(false);
 	vendorline->setEnabled(false);
 	vendorTag->setEnabled(false);
 	releaseline->setEnabled(false);
 	releaseTag->setEnabled(false);
 	vsInstall->setEnabled(false);
 	projectVSLocation->setEnabled(false);
 	vsLocation->setEnabled(false);
 	locationbutton->setEnabled(false);
 	qtarch_ButtonGroup_1->setEnabled(false);
	vsBox->setEnabled(false);
	vsSupport->setEnabled(false);
  name->setEnabled(false);
  email->setEnabled(false);
  authorname->setEnabled(false);
  versionnumber->setEnabled(false);
  directory->setEnabled(false);
}


void CKAppWizard::reject() {
  nametext = nameline->text();
  m_author_name = authorline->text();
  m_author_email = emailline->text();

  if ((!(finishButton()->isEnabled())) && (nametext.length() >= 1)) {

    delete (project);
  }
  delete (errOutput);
  delete (output);
  delete (cppedit);
  delete (cppnew);
  delete (cppload);
  delete (cppheader);
  delete (hedit);
  delete (hnew);
  delete (hload);
  delete (hheader);
  delete (miniload);
  delete (iconload);
  delete (datalink);
  delete (miniicon);
  delete (progicon);
  delete (separator1);
  delete (gnufiles);
  delete (lsmfile);
  delete (userdoc);
  delete (apidoc);
  delete (emailline);
  delete (authorline);
  delete (versionline);
  delete (directoryload);
  delete (email);
  delete (authorname);
  delete (separator2);
  delete (versionnumber);
  delete (directory);
  delete (name);
  delete (separator0);
  delete (widget1b);
  delete (widget1a);
  delete (apphelp);
  delete (generatesource);
  kdeentry->removeItem (kdenormalitem);
  kdeentry->removeItem (kdeminiitem);
  delete (kdenormalitem);
  delete (kdeminiitem);
  //delete (corbaitem);
  //delete (komitem);
  delete (kdeentry);
  qtentry->removeItem (qtnormalitem);
  delete (qtnormalitem);
  //delete (qtminiitem);
  delete (qtentry);
  ccppentry->removeItem (cppitem);
  ccppentry->removeItem (citem);
  delete (citem);
  delete (cppitem);
  delete (ccppentry);
  //delete (gtknormalitem);
  //delete (gtkminiitem);
  //delete (gtkentry);
  othersentry->removeItem (customprojitem);
  delete (customprojitem);
  delete (othersentry);
  delete (applications);
  delete (messageline);
  delete (logMessage);
  delete (vendorline);
  delete (vendorTag);
  delete (releaseline);
  delete (releaseTag);
  delete (vsInstall);
  delete (projectVSLocation);
  delete (projectlocationline);
  delete (vsLocation);
  delete (vsBox);
  delete (vsSupport);
  delete (locationbutton);
  delete (qtarch_ButtonGroup_1);
  KWizard::reject();
}

// connection of this (okButton)
void CKAppWizard::slotPerlOut(KProcess*,char* buffer,int buflen) {
  QCString str(buffer,buflen);
  output->append(str);
  output->setCursorPosition(output->numLines(),0);
}

// connection of this (okButton)
void CKAppWizard::slotPerlErr(KProcess*,char* buffer,int buflen) {
  QCString str(buffer,buflen);
  errOutput->append(str);
  errOutput->setCursorPosition(errOutput->numLines(),0);
}

void CKAppWizard::slotApplicationClicked() {

  if (kdenormalitem->isSelected() && (cancelButton()->text() != i18n("Exit"))) {
    pm.load(locate("appdata","pics/normalApp.bmp"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setEnabled(true);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
/*    if (strcmp(nameline->text(), "") && (cancelButton()->text() != i18n("Exit"))) {
     finishButton()->setEnabled(true);
    }
*/    apphelp->setText (i18n("Create a KDE-application with session-management, "
    												"menubar, toolbar, statusbar and support for a "
    												"document-view codeframe model."));
  }
  else if (kdeminiitem->isSelected() && (cancelButton()->text() != i18n("Exit"))) {
    pm.load(locate("appdata",  "pics/miniApp.bmp"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setEnabled(true);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
/*    if (strcmp(nameline->text(), "") && (cancelButton()->text() != i18n("Exit"))) {
      finishButton()->setEnabled(true);
    }
*/    apphelp->setText (i18n("Create a KDE-application with an empty main widget."));
  }
  else if (qtnormalitem->isSelected() && (cancelButton()->text() != i18n("Exit"))) {
    pm.load(locate("appdata", "pics/qtApp.bmp"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setEnabled(false);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
/*    if (strcmp(nameline->text(), "") && strcmp (cancelButton()->text(), i18n("Exit"))) {
      finishButton()->setEnabled(true);
    }
*/    apphelp->setText (i18n("Create a Qt-Application with a main window containing "
    											"a menubar, toolbar and statusbar, including support for "
    											"a generic document-view model."));
  }
  else if ((citem->isSelected() || cppitem->isSelected())
            && (cancelButton()->text() != i18n("Exit"))) {
    pm.load(locate("appdata", "pics/terminalApp.bmp"));
    widget1b->setBackgroundPixmap(pm);
    if (citem->isSelected())
    {
      cppheader->setText( i18n("headertemplate for .c-files") );
    }
    else
      cppheader->setText( i18n("headertemplate for .c-files") );

    apidoc->setEnabled(false);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
/*    if (strcmp(nameline->text(), "") && (cancelButton()->text() != i18n("Exit"))) {
      finishButton()->setEnabled(true);
    }
*/    apphelp->setText ( (citem->isSelected()) ?
		      i18n("Create a C application. The program will run in a terminal "
    			   "and doesn't contain any support for classes and Graphical User Interface.")
                      :
                      i18n("Create a C++ application. The program will run in a terminal "
    			   "and doesn't contain any support for a Graphical User Interface."));
  }
  /*    else if (corbaitem->isSelected() && strcmp (cancelButton->text(), i18n("Exit"))) {
      if (strcmp(nameline->text(), "") & strcmp (cancelButton->text(), i18n("Exit"))) {
      okButton->setEnabled(true);
      }
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }
      else if (komitem->isSelected() && strcmp (cancelButton->text(), i18n("Exit"))) {
      if (strcmp(nameline->text(), "") & strcmp (cancelButton->text(), i18n("Exit"))) {
      okButton->setEnabled(true);
      }
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }*/
  else if (customprojitem->isSelected() && (cancelButton()->text() != i18n("Exit"))) {
    pm.load(locate("appdata", "pics/customApp.bmp"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setEnabled(false);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(false);
    userdoc->setEnabled(false);
    generatesource->setChecked(false);
    generatesource->setEnabled(false);
/*    if (strcmp(nameline->text(), "") && (cancelButton()->text() != i18n("Exit"))) {
      finishButton()->setEnabled(true);
    }
*/    apphelp->setText (i18n("Creates an empty project to work with existing development projects. "
    											"KDevelop will not take care of any Makefiles as those are supposed to "
    											"be included with your old project."));
  }
  /*  else if (gtknormalitem->isSelected() && strcmp (cancelButton->text(), i18n("Exit"))) {
      if (strcmp(nameline->text(), "") && strcmp (cancelButton->text(), i18n("Exit"))) {
      okButton->setEnabled(true);
      }
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }
      else if (gtkminiitem->isSelected() && strcmp (cancelButton->text(), i18n("Exit"))) {
      if (strcmp(nameline->text(), "") && strcmp (cancelButton->text(), i18n("Exit"))) {
      okButton->setEnabled(true);
      }
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }
      else if (qtminiitem->isSelected() && strcmp (cancelButton->text(), i18n("Exit"))) {
      if (strcmp(nameline->text(), "") && strcmp (cancelButton->text(), i18n("Exit"))) {
      okButton->setEnabled(true);
      }
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }*/
  else if (kdeentry->isSelected()) {
//    finishButton()->setEnabled(false);
    apphelp->setText (i18n("Contains all KDE-compliant\n"
			   "project types."));
  }
  else if (qtentry->isSelected()) {
//    finishButton()->setEnabled(false);
    apphelp->setText (i18n("Contains all Qt-based\n"
			   "project types."));
  }
  else if (ccppentry->isSelected()) {
//    finishButton()->setEnabled(false);
    apphelp->setText (i18n("Contains all C/C++-terminal\n"
			   "project types."));
  }
  /*  else if (gtkentry->isSelected()) {
      okButton->setEnabled(false);
      apphelp->setText (i18n("With this framework you can\n"
			   "generate a program with toolsbar\n"
			   "and mainmenus"));
      }*/
  else if (othersentry->isSelected()) {
//    finishButton()->setEnabled(false);
    apphelp->setText (i18n("Contains all individual project types."));
  }
}

// connection of this
void CKAppWizard::slotDefaultClicked(int page) {
  pm.load(locate("appdata", "pics/normalApp.bmp"));

# warning seems to be obsolete, �cause QWizard doesn�t support DEFAULT-Button
// no access to the title
//    page(widget3)->t=i18n("Headertemplate for .c-files");

    if (currentPage()==widget3)
     showPage(widget3);

    cppheader->setText( i18n("headertemplate for .cpp-files") );

  widget1b->setBackgroundPixmap(pm);
  applications->setSelected(kdenormalitem,true);
  apidoc->setChecked(true);
  lsmfile->setChecked(true);
  cppheader->setChecked(true);
  hheader->setChecked(true);
  datalink->setChecked(true);
  miniicon->setChecked(true);
  progicon->setChecked(true);
  gnufiles->setChecked(true);
  userdoc->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
  generatesource->setChecked(true);
  directoryline->setText(QDir::homeDirPath()+ QString("/"));
  dir = QDir::homeDirPath()+ QString("/");
  nameline->setText(0);
//  finishButton()->setEnabled(false);
  miniload->setPixmap(BarIcon(locate("mini","application_settings.png")));
  iconload->setPixmap(locate("icon","xedit.png"));
  cppedit->loadFile(locate("appdata", "templates/cpp_template"),cppedit->OPEN_READWRITE);
  hedit->loadFile(locate("appdata", "templates/header_template"),hedit->OPEN_READWRITE);
  authorline->setText(m_author_name);
  emailline->setText(m_author_email);
  versionline->setText("0.1");
  messageline->setText ("new project started");
  vendorline->setText(QString(nameline->text()).lower());
  releaseline->setText("start");
  projectlocationline->setText(0);
  vsLocation->setText(QDir::homeDirPath() + "/cvsroot");
  vsBox->setCurrentItem(0);
  modifyVendor= false;
  modifyDirectory= false;
  slotVSBoxChanged(0);
}

// connection of nameline
void CKAppWizard::slotProjectnameEntry() {
  int position = nameline->cursorPosition();
  nametext = nameline->text();
  nametext = nametext.stripWhiteSpace();
  int length = nametext.length();
  int i = 0;
  QString endname = "";
  QString first = "";
  QString end = nametext;
  QRegExp regexp1 ("[a-zA-Z]");
  QRegExp regexp2 ("[a-zA-Z0-9_]");
  if (!length==0) {
  	for (i=0;i<length;i++) {
  	  first = end.left(1);
  	  end = end.right(length-i-1);
    	if (i==0) {	
    		if (regexp1.match(first) == -1) {
     	 		first = "";
    		}
    		else {

      		first = first.upper();
    		}
    	}
    	else {
    	  if (regexp2.match(first) == -1) {
     	 		first = "";
    		}		
    	}
    	endname = endname.append(first);
    }
  }

  nameline->setText(endname);
  if (!modifyDirectory) {
   	directoryline->setText(dir + endname.lower());
  }

  if (!modifyVendor) {
  	vendorline->setText(endname.lower());
  }

/*  if (nametext == "" || kdeentry->isSelected() || qtentry->isSelected() ||
   	  ccppentry->isSelected() || othersentry->isSelected()) {
   	finishButton()->setEnabled(false);
  }
  else {
   	finishButton()->setEnabled(true);
  }
*/
  setFinishEnabled(widget4, nametext!="");
  nameline->setCursorPosition(position);
}

// connection of directoryline
void CKAppWizard::slotDirectoryEntry() {
  if(directoryline->hasFocus()) {
  	modifyDirectory = true;
  }
  QString directory = directoryline->text();
  QString enddir;
  if (directory.right(1) == "/") {
		directory = directory.left(directory.length() - 1);
  }
	int pos;
	pos = directory.findRev ("/");
	enddir = directory.right (directory.length() - pos -1);
	
  projectlocationline->setText(enddir);

}

// connection of iconload
void CKAppWizard::slotIconButtonClicked() {
  QStringList iconlist;
  KIconLoaderDialog iload;
  iload.selectIcon(name1,"*");
  if (!name1.isNull() )
    iconload->setPixmap(KGlobal::iconLoader()->loadIcon(name1));
}

// connection of miniload
void CKAppWizard::slotMiniIconButtonClicked() {
  QStringList miniiconlist;
  KIconLoaderDialog  mload;
  mload.selectIcon(name2,"*");
  if (!name2.isNull() )
    miniload->setPixmap(KGlobal::iconLoader()->loadApplicationMiniIcon(name2));
}

// activate and deactivate the iconbutton
void CKAppWizard::slotProgIconClicked() {
  if (progicon->isChecked()) {
    iconload->setEnabled(true);
  }
  else {
    iconload->setEnabled(false);
  }
}

// activate and deactivate the miniiconbutton
void CKAppWizard::slotMiniIconClicked() {
  if (miniicon->isChecked()) {
    miniload->setEnabled(true);
  }
  else {
    miniload->setEnabled(false);
  }
}

// activate and deactivate the headerloadbutton, headernewbutton, headeredit
void CKAppWizard::slotHeaderHeaderClicked() {
  if (hheader->isChecked()) {
    hload->setEnabled(true);
    hnew->setEnabled(true);
    hedit->setEnabled(true);
  }
  else {
    hload->setEnabled(false);
    hnew->setEnabled(false);
    hedit->setEnabled(false);
  }
}

// activate and deactivate the cpploadbutton, cppnewbutton, cppedit
void CKAppWizard::slotCppHeaderClicked() {
  if (cppheader->isChecked()) {
    cppload->setEnabled(true);
    cppnew->setEnabled(true);
    cppedit->setEnabled(true);
  }
  else {
    cppload->setEnabled(false);
    cppnew->setEnabled(false);
    cppedit->setEnabled(false);
  }
}

void CKAppWizard::slotProcessExited() {

  QString directory = directoryline->text();
  QString prj_str;
  if (vsBox->currentItem() != 0) {
    prj_str = locateLocal("appdata","kdeveloptemp/" + namelow + ".kdevprj");
  }
  else {
    prj_str = directory + "/" + namelow + ".kdevprj";
  }
  project = new CProject(prj_str);
  project->readProject();
  project->setKDevPrjVersion("1.0beta2");
  if (cppitem->isSelected()) {
    project->setProjectType("normal_cpp");
  }
  else if (citem->isSelected()) {
    project->setProjectType("normal_c");
  }
  else if (kdeminiitem->isSelected()) {
    project->setProjectType("mini_kde");
  }
  else if (kdenormalitem->isSelected()) {
    project->setProjectType("normal_kde");
  }
  else if (qtnormalitem->isSelected()) {
    project->setProjectType("normal_qt");
  }
  else if (customprojitem->isSelected()) {
    project->setProjectType("normal_empty");
  }

  project->setProjectName (nameline->text());
  project->setSubDir (namelow + "/");
  project->setAuthor (authorline->text());
  project->setEmail (emailline->text());
  project->setVersion (versionline->text());
  if (userdoc->isChecked()) {
    project->setSGMLFile (directory + "/" + namelow + "/docs/en/index.sgml");
  }
  project->setBinPROGRAM (namelow);
  project->setLDFLAGS (" ");
  project->setCXXFLAGS ("-O0 -g3 -Wall");

  if (kdenormalitem->isSelected()) {
    project->setLDADD (" -lkfile -lkdeui -lkdecore -lqt -lXext -lX11");
  }
  else if (kdeminiitem->isSelected()) {
    project->setLDADD (" -lkdeui -lkdecore -lqt -lXext -lX11");
  }
  else if (qtnormalitem->isSelected()) {
    project->setLDADD (" -lqt -lXext -lX11");
  }

  QStrList sub_dir_list;
  TMakefileAmInfo makeAmInfo;
  makeAmInfo.rel_name = "Makefile.am";
  makeAmInfo.type = "normal";
  sub_dir_list.append(namelow);
  if (kdenormalitem->isSelected() || kdeminiitem->isSelected()) {
    sub_dir_list.append("po");
  }
  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  makeAmInfo.rel_name =  namelow + "/Makefile.am";
  makeAmInfo.type = "prog_main";
  sub_dir_list.clear();
  if (userdoc->isChecked()) {
    //    sub_dir_list.append("docs");
  }
  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  makeAmInfo.rel_name =  namelow + "/docs/Makefile.am";
  makeAmInfo.type = "normal";
  sub_dir_list.clear();
  //  sub_dir_list.append("en");
  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  makeAmInfo.rel_name =  namelow + "/docs/en/Makefile.am";
  makeAmInfo.type = "normal";
  sub_dir_list.clear();
  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  if (!(cppitem->isSelected() || citem->isSelected() || qtnormalitem->isSelected()) && CToolClass::searchProgram("xgettext")) {
    makeAmInfo.rel_name = "po/Makefile.am";
    makeAmInfo.type = "po";
    sub_dir_list.clear();
    makeAmInfo.sub_dirs = sub_dir_list;
    project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);
  }

	if (vsBox->currentItem()==0) {
    project->setVCSystem("None");
  }
  else {
    project->setVCSystem("CVS");
  }

  TFileInfo fileInfo;
  fileInfo.rel_name = namelow + ".kdevprj";
  fileInfo.type = DATA;
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->addFileToProject (namelow + ".kdevprj",fileInfo);

  if (gnufiles->isChecked()) {
    fileInfo.rel_name = "AUTHORS";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("AUTHORS",fileInfo);

    fileInfo.rel_name = "COPYING";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("COPYING",fileInfo);

    fileInfo.rel_name = "ChangeLog";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("ChangeLog",fileInfo);

    fileInfo.rel_name = "INSTALL";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("INSTALL",fileInfo);

    fileInfo.rel_name = "README";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("README",fileInfo);

    fileInfo.rel_name = "TODO";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("TODO",fileInfo);

  }

  if (lsmfile->isChecked()) {
    fileInfo.rel_name = namelow + ".lsm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + ".lsm",fileInfo);
  }

  if (generatesource->isChecked()) {
    QString extension= (citem->isSelected()) ? "c" : "cpp";
    fileInfo.rel_name = namelow + "/main."+extension;
    fileInfo.type = CPP_SOURCE;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/main."+extension,fileInfo);
  }

  if (!citem->isSelected() && !cppitem->isSelected()) {
    if (generatesource->isChecked()) {
      fileInfo.rel_name = namelow + "/" + namelow + ".cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + ".cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + ".h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + ".h",fileInfo);
    }
  }

  if (kdenormalitem->isSelected() || qtnormalitem->isSelected()) {
    if (generatesource->isChecked()) {
      fileInfo.rel_name = namelow + "/" + namelow + "doc.cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "doc.cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "doc.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "doc.h",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "view.cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "view.cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "view.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "view.h",fileInfo);

      fileInfo.rel_name = namelow + "/resource.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/resource.h",fileInfo);
    }
  }

  if (datalink->isChecked()) {
    fileInfo.rel_name = namelow + "/" + namelow + ".kdelnk";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = true;
    fileInfo.install_location = "$(kde_appsdir)/Applications/" + namelow + ".kdelnk";
    project->addFileToProject (namelow + "/" + namelow + ".kdelnk",fileInfo);
  }

  if (progicon->isChecked()) {
    fileInfo.rel_name = namelow + "/" + namelow + ".xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!qtnormalitem->isSelected()) {
      fileInfo.install = true;
      fileInfo.install_location = "$(kde_icondir)/" + namelow + ".xpm";
    }
    else {
      fileInfo.install = false;
      fileInfo.install_location = "";
    }
    project->addFileToProject (namelow + "/" + namelow + ".xpm",fileInfo);
  }

  if (miniicon->isChecked()) {
    fileInfo.rel_name = namelow + "/mini-" + namelow + ".xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!qtnormalitem->isSelected()) {
      fileInfo.install = true;
      fileInfo.install_location = "$(kde_minidir)/" + namelow + ".xpm";
    }
    else {
      fileInfo.install = false;
      fileInfo.install_location = "";
    }
    project->addFileToProject (namelow + "/mini-" + namelow + ".xpm",fileInfo);
  }

  if (qtnormalitem->isSelected()) {
    fileInfo.rel_name = namelow + "/filenew.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/filenew.xpm",fileInfo);

    fileInfo.rel_name = namelow + "/filesave.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/filesave.xpm",fileInfo);

    fileInfo.rel_name = namelow + "/fileopen.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/fileopen.xpm",fileInfo);
  }

  if (userdoc->isChecked()) {
    fileInfo.rel_name = namelow + "/docs/en/index-1.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-1.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-1.html";
    }

    project->addFileToProject (namelow + "/docs/en/index-1.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index-2.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-2.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-2.html";
    }
    project->addFileToProject (namelow + "/docs/en/index-2.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index-3.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-3.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-3.html";
    }
    project->addFileToProject (namelow + "/docs/en/index-3.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index-4.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-4.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-4.html";
    }
    project->addFileToProject (namelow + "/docs/en/index-4.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index-5.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-5.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-5.html";
    }
    project->addFileToProject (namelow + "/docs/en/index-5.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index-6.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index-6.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index-6.html";
    }
    project->addFileToProject (namelow + "/docs/en/index-6.html",fileInfo);

    fileInfo.rel_name = namelow + "/docs/en/index.html";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!cppitem->isSelected() && !citem->isSelected()) {
      fileInfo.install = true;
      if (qtnormalitem->isSelected()) {
	fileInfo.install_location = "$(prefix)/doc/" + namelow+ "/index.html";
      }
      else
	fileInfo.install_location = "$(kde_htmldir)/en/" + namelow+ "/index.html";
    }
    project->addFileToProject (namelow + "/docs/en/index.html",fileInfo);
    QFile gif (directory + "/" + namelow + "/docs/en/" + namelow + ".gif");
    if (gif.exists()) {
      fileInfo.rel_name = namelow + "/docs/en/" + namelow + ".gif";
      fileInfo.type = DATA;
      fileInfo.dist = true;
      fileInfo.install = true;
      project->addFileToProject (namelow + "/docs/en/" + namelow + ".gif",fileInfo);
    }
  }


  QStrList group_filters;
  group_filters.append("*");
  project->addLFVGroup ("Others","");
  project->setFilters("Others",group_filters);



  if (gnufiles->isChecked()) {
    group_filters.clear();
    group_filters.append("AUTHORS");
    group_filters.append("COPYING");
    group_filters.append("ChangeLog");
    group_filters.append("INSTALL");
    group_filters.append("README");
    group_filters.append("TODO");
    group_filters.append("NEWS");
    project->addLFVGroup ("GNU","");
    project->setFilters("GNU",group_filters);
  }

  if (!(cppitem->isSelected() || citem->isSelected() || qtnormalitem->isSelected())) {
    group_filters.clear();
    group_filters.append("*.po");
    project->addLFVGroup (i18n("Translations"),"");
    project->setFilters(i18n("Translations"),group_filters);
  }
  if (!cppitem->isSelected() && !citem->isSelected()) {
    group_filters.clear();
    group_filters.append("*.kdevdlg");
    project->addLFVGroup (i18n("Dialogs"),"");
    project->setFilters(i18n("Dialogs"),group_filters);
  }


  group_filters.clear();
  group_filters.append("*.cpp");
  group_filters.append("*.c");
  group_filters.append("*.cc");
  group_filters.append("*.C");
  group_filters.append("*.cxx");
  group_filters.append("*.ec");
  group_filters.append("*.ecpp");
  group_filters.append("*.lxx");
  group_filters.append("*.l++");
  group_filters.append("*.ll");
  group_filters.append("*.l");
  project->addLFVGroup (i18n("Sources"),"");
  project->setFilters(i18n("Sources"),group_filters);

  group_filters.clear();
  group_filters.append("*.h");
  group_filters.append("*.hxx");
  group_filters.append("*.hpp");
  group_filters.append("*.H");
  project->addLFVGroup (i18n("Headers"),"");
  project->setFilters(i18n("Headers"),group_filters);

  project->writeProject ();
  project->updateMakefilesAm ();

  disconnect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  connect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotMakeEnd()));

  if (CToolClass::searchInstProgram("ksgml2html")) {
    KShellProcess process;
    QString nif_template = locate("appdata", "templates/nif_template");
    process.clearArguments();
    process << "cp"; // copy is your friend :-)
    process << nif_template;
    process << QString(directoryline->text()) +"/" + QString(nameline->text()).lower() + "/docs/en/index.nif";
    process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important
  }

  KShellProcess p;
  QDir dir;
	if (vsBox->currentItem() == 1) {
	  dir.setCurrent(locateLocal("appdata", "kdeveloptemp"));
	  QString import = (QString) "cvs -d " + vsLocation->text() + (QString) " import -m \"" + messageline->text() +
	    (QString) "\" " + projectlocationline->text() + (QString) " " + vendorline->text() +
	    (QString) " " + releaseline->text();
	  p << import;
	  p.start(KProcess::Block, KProcess::AllOutput);
	}

	if (vsBox->currentItem() != 0) {
		dir.setCurrent(QDir::homeDirPath());
		QString deltemp = QString("rm -r -f ") + locateLocal("appdata", "kdeveloptemp");
		p.clearArguments();
 	 	p << deltemp;
    p.start(KProcess::Block, KProcess::AllOutput);

		QString direct = directoryline->text();
  	if (direct.right(1) == "/") {
			direct = direct.left(direct.length() - 1);
  	}
		int pos;
		pos = direct.findRev ("/");
		direct = direct.left (pos);
  	if(direct.right(1) != "/"){
    	direct = direct + "/";
  	}

  	dir.setCurrent(direct);
  	QString checkout = "cvs -d " + (QString) vsLocation->text() + " co " + (QString) projectlocationline->text();
		p.clearArguments();
 	 	p << checkout;
    p.start(KProcess::Block, KProcess::AllOutput);
  }

  QString path1 = locate("appdata", "tools/processesend.pl");
  q->clearArguments();
  *q << "perl" << path1;
  q->start(KProcess::NotifyOnExit, KProcess::AllOutput);
  
}

// enable cancelbutton if everything is done
void CKAppWizard::slotMakeEnd() {
  QString extension= (citem->isSelected()) ? "c" : "cpp";
  if (!generatesource->isChecked()) {
    QFile file;
    q->clearArguments();
    directorytext = directoryline->text();
    nametext = nameline->text();
    nametext = nametext.lower();
    file.remove (directorytext + "/" + nametext + "/main."+extension);
    if (!citem->isSelected() && !cppitem->isSelected())
    {
      file.remove (directorytext + "/" + nametext + "/" + nametext + ".cpp");
      file.remove (directorytext + "/" + nametext + "/" + nametext + ".h");
    }
    if (kdenormalitem->isSelected() || qtnormalitem->isSelected()) {
      file.remove (directorytext + "/" + nametext + "/" + nametext + "doc.cpp");
      file.remove (directorytext + "/" + nametext + "/" + nametext + "doc.h");
      file.remove (directorytext + "/" + nametext + "/" + nametext + "view.cpp");
      file.remove (directorytext + "/" + nametext + "/" + nametext + "view.h");
    }
  }
  cancelButton()->setEnabled(true);
  gen_prj = true;
}

// return the directory with the projectfile
QString CKAppWizard::getProjectFile() {
  nametext = nameline->text();
  nametext = nametext.lower();
  directorytext = directoryline->text();
  if(QString(directoryline->text()).right(1) == "/"){
    directorytext = directorytext + nametext + ".kdevprj";
  }
  else{
    directorytext = directorytext + "/" + nametext + ".kdevprj";
  }
  delete (directoryline);
  delete (nameline);
  delete (widget0);
  delete (widget1);
  delete (widget2);
  delete (widget3);
  delete (widget4);
	delete (widget1c);
  return directorytext;
}

// return TRUE if a poject is generated
bool CKAppWizard::generatedProject(){
  return gen_prj;
}

void CKAppWizard::slotLocationButtonClicked() {
  KDirDialog *dirdialog = new KDirDialog(dir,this,"Directory");
  dirdialog->setCaption (i18n("Directory"));
  dirdialog->show();
  dir = dirdialog->dirPath() + "cvsroot";
  vsLocation->setText(dir);
  delete (dirdialog);

}


void CKAppWizard::slotVSBoxChanged(int item) {
  if (item == 0) {
  	messageline->setEnabled(false);
  	logMessage->setEnabled(false);
  	vendorline->setEnabled(false);
  	vendorTag->setEnabled(false);
  	releaseline->setEnabled(false);
  	releaseTag->setEnabled(false);
  	vsInstall->setEnabled(false);
  	projectVSLocation->setEnabled(false);
  	vsLocation->setEnabled(false);
  	locationbutton->setEnabled(false);
  	qtarch_ButtonGroup_1->setEnabled(false);

  }
	else {
	  messageline->setEnabled(true);
  	logMessage->setEnabled(true);
  	vendorline->setEnabled(true);
  	vendorTag->setEnabled(true);
  	releaseline->setEnabled(true);
  	releaseTag->setEnabled(true);
  	vsInstall->setEnabled(true);
  	projectVSLocation->setEnabled(true);
  	vsLocation->setEnabled(true);
  	locationbutton->setEnabled(true);
  	qtarch_ButtonGroup_1->setEnabled(true);
	}
}

void CKAppWizard::slotVendorEntry() {
  if(vendorline->hasFocus()) {
  	modifyVendor = true;
  }
}




