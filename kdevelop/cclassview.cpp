/***************************************************************************
                          cclassview.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
    based on             : cclassview.cpp by Sandy Meier
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cclassview.h"
#include <assert.h>
#include <qmessagebox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qheader.h>
#include <qprogressdialog.h> 
#include <qmessagebox.h>

#include "cclasstooldlg.h"
#include "ccvaddfolderdlg.h"
#include "cproject.h"
#include "./gfxview/GfxClassTreeWindow.h"
#include "resource.h"
#include "./classwizard/cclasswizarddlg.h"

// Initialize static members
QString CClassView::CLASSROOTNAME = "Classes";
QString CClassView::GLOBALROOTNAME = "Globals";

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CClassView::CCVToolTip::CCVToolTip()
 * CCVToolTip()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::CCVToolTip::CCVToolTip( QWidget *parent )
  : QToolTip( parent )
{
}

/*********************************************************************
 *                                                                   *
 *                        PROTECTED METHODS                          *
 *                                                                   *
 ********************************************************************/

/*-------------------------------- CClassView::CCVToolTip::maybeTip()
 * maybeTip()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::CCVToolTip::maybeTip( const QPoint &p )
{
  CClassView *cv;
  QString str;
  QRect r;

  cv = (CClassView *)parentWidget();
  cv->tip( p, r, str );

  if( !str.isEmpty() && r.isValid() )
    tip( r, str );
    
  debug( "Maybetip @ %d,%d(%s)", p.x(), p.y(), ( str.isEmpty() ? "" : str.data() ) );
}

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CClassView::CClassView()
 * CClassView()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::CClassView(QWidget* parent /* = 0 */,const char* name /* = 0 */)
  : CTreeView (parent, name)
{
  // Create the popupmenus.
  initPopups();

  // Set the store.
  store = &cp.store;

  // Create the tooltip;
  toolTip = new CCVToolTip( this );

  setTreeHandler( new CClassTreeHandler() );
  ((CClassTreeHandler *)treeH)->setStore( store );

  connect(this, SIGNAL(selectionChanged()), SLOT(slotClassViewSelected()));
}

/*------------------------------------------ CClassView::~CClassView()
 * ~CClassView()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassView::~CClassView()
{
  delete toolTip;
}

/*------------------------------------------ CClassView::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::initPopups()
{
  int id;
  // Project popup
  projectPopup.setTitle(i18n ("Project"));
  projectPopup.insertItem(i18n("New file..."), this, SLOT(slotFileNew()),0, ID_FILE_NEW);
  projectPopup.insertItem(i18n("New class..."), this, SLOT(slotClassNew()), 0, ID_PROJECT_NEW_CLASS);
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Add Folder..."), this, SLOT( slotFolderNew()),0, ID_CV_FOLDER_NEW);
  projectPopup.insertSeparator();
  projectPopup.insertItem(i18n("Options..."), this, SLOT(slotProjectOptions()),0, ID_PROJECT_OPTIONS);
  projectPopup.insertItem(i18n("Graphical classview.."), this, SLOT(slotGraphicalView()), 0, ID_CV_GRAPHICAL_VIEW);



  // Class popup
  classPopup.setTitle( i18n("Class"));
  classPopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()),ID_CV_VIEW_DECLARATION);
  classPopup.insertItem( i18n("Add member function..."), this, SLOT(slotMethodNew()), ID_CV_METHOD_NEW);
  classPopup.insertItem( i18n("Add member variable..."), this, SLOT(slotAttributeNew()),0, ID_CV_ATTRIBUTE_NEW);
  id = classPopup.insertItem( i18n("Implement virtual function..."), this, SLOT(slotImplementVirtual()),0, ID_CV_IMPLEMENT_VIRTUAL);
  classPopup.setItemEnabled( id, false );

  classPopup.insertSeparator();
  classPopup.insertItem( i18n("Parent classes..."), this, SLOT(slotClassBaseClasses()),0, ID_CV_CLASS_BASE_CLASSES);
  classPopup.insertItem( i18n("Child classes..."), this, SLOT(slotClassDerivedClasses()),0, ID_CV_CLASS_DERIVED_CLASSES);
  classPopup.insertItem( i18n("Classtool..."), this, SLOT(slotClassTool()),0, ID_CV_CLASS_TOOL);
  classPopup.insertSeparator();
  id = classPopup.insertItem( i18n( "Add slot for signal" ), this, SLOT(slotAddSlotSignal()),0, ID_CV_ADD_SLOT_SIGNAL);
  classPopup.setItemEnabled( id, false );
  id = classPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete class"), this, SLOT(slotClassDelete()), ID_CV_CLASS_DELETE);
  classPopup.setItemEnabled(id, false );
  classPopup.insertSeparator();
  classPopup.insertItem( i18n( "ClassWizard" ), this, SLOT( slotClassWizard()),0, ID_CV_CLASSWIZARD );

  // Struct popup
  structPopup.setTitle( i18n( "Struct" ) );
  structPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration() ),0,ID_CV_VIEW_DEFINITION);

  // Method popup
  methodPopup.setTitle( i18n( "Method" ) );
  methodPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()), 0, ID_CV_VIEW_DECLARATION);
  methodPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration() ),0,ID_CV_VIEW_DEFINITION);
  methodPopup.insertSeparator();
  methodPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete method" ), this, SLOT(slotMethodDelete()),0, ID_CV_METHOD_DELETE);

  // Attribute popup
  attributePopup.setTitle( i18n( "Attribute" ) );
  attributePopup.insertItem( i18n("Go to declaration" ), this, SLOT( slotViewDeclaration()),0, ID_CV_VIEW_DEFINITION);
  attributePopup.insertSeparator();
  id = attributePopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete attribute" ), this, SLOT(slotAttributeDelete()),0, ID_CV_ATTRIBUTE_DELETE);
  attributePopup.setItemEnabled( id, false );

  // Slot popup
  slotPopup.setTitle( i18n( "Slot" ) );
  slotPopup.insertItem( i18n("Go to definition" ), this, SLOT( slotViewDefinition()),0, ID_CV_VIEW_DECLARATION);
  slotPopup.insertItem( i18n("Go to declaration" ), this, SLOT(slotViewDeclaration()),0, ID_CV_VIEW_DEFINITION);
  slotPopup.insertSeparator();
  slotPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete slot" ), this, SLOT(slotMethodDelete()),0,ID_CV_METHOD_DELETE);

  // Signal popup
  signalPopup.setTitle( i18n( "Signal" ) );
  signalPopup.insertItem( i18n( "Go to declaration" ), this, SLOT(slotViewDeclaration()),0, ID_CV_VIEW_DEFINITION );
  signalPopup.insertSeparator();
  signalPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n( "Delete signal" ), this, SLOT(slotMethodDelete()),0,ID_CV_METHOD_DELETE);

  // Folder popup
  folderPopup.setTitle( i18n( "Folder" ) );
  folderPopup.insertItem( i18n("Add Folder..."), this, SLOT( slotFolderNew()),0, ID_CV_FOLDER_NEW);
  id = folderPopup.insertItem( *(treeH->getIcon( THDELETE )), i18n("Delete Folder..."), this, SLOT( slotFolderDelete()),0, ID_CV_FOLDER_DELETE);

  connect(&attributePopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&classPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&folderPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&methodPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&projectPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&slotPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
  connect(&signalPopup ,SIGNAL(highlighted(int)), this, SIGNAL(popupHighlighted(int)));
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Add all classes from the project. Reparse and redraw all classes 
 *   in the view.
 *
 * Parameters:
 *   proj          The project specification.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh( CProject *proj )
{
  QProgressDialog progressDlg(NULL, "progressDlg", true );
  QStrList src;
  QStrList header;
  char *str;
  int totalCount=0;
  int currentCount = 0;
  bool popupClassItemsEnable;

  debug( "CClassView::refresh( proj )" );

  project = proj;

  // Reset the classparser and the view.
  ((CClassTreeHandler *)treeH)->clear();
  cp.wipeout();

  // Get all header and src filenames.
  header = proj->getHeaders();
  src = proj->getSources();

  // Initialize progressbar.
  totalCount = header.count() + src.count();
  emit resetStatusbarProgress();
  emit setStatusbarProgress( 0 );
  emit setStatusbarProgressSteps( totalCount );

  // Parse headerfiles.
  for( str = header.first(); str != NULL; str = header.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    emit setStatusbarProgress( ++currentCount );
  }
	
  // Parse sourcefiles.
  for( str = src.first(); str != NULL; str = src.next() )
  {
    debug( "  parsing:[%s]", str );
    cp.parse( str );
    emit setStatusbarProgress( ++currentCount );
  }

  refresh();

  // disable certain popup items if it is a C Project
  popupClassItemsEnable=proj->getProjectType()!="normal_c";
  projectPopup.setItemEnabled(ID_PROJECT_NEW_CLASS, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_FOLDER_NEW, popupClassItemsEnable);
  projectPopup.setItemEnabled(ID_CV_GRAPHICAL_VIEW, popupClassItemsEnable);

}

/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Reparse the file and redraw the view.
 *
 * Parameters:
 *   aFile        The file to reparse.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh( const char *aFile)
{
}

/*---------------------------------------------- CClassView::refresh()
 * refresh()
 *   Reparse and redraw all classes in the view.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refresh()
{
  QString str;
  QListViewItem *item;
  QString treeStr;
  QList<CParsedMethod> *methodList;
  QList<CParsedAttribute> *attributeList;
  QList<CParsedStruct> *structList;

  debug( "CClassView::refresh()" );

  // Try to fetch a stored classview tree.
  treeStr = project->getClassViewTree();

  // If there doesn't exists a stored tree we build one.
  if( treeStr.isEmpty() )
    buildInitalClassTree();
  else // Build the tree using the stored string.
    buildTree( treeStr );

  // Add the globals folder.
  str = i18n( GLOBALROOTNAME );
  globalsItem = treeH->addRoot( str, THFOLDER );

  // Add all global items.
  item = treeH->addItem( i18n( "Structures" ), THFOLDER, globalsItem );
  structList = store->globalContainer.getSortedStructList();
  ((CClassTreeHandler *)treeH)->addGlobalStructs( structList, item );
  delete structList;

  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Functions" ), THFOLDER, globalsItem );
  methodList = store->globalContainer.getSortedMethodList();
  ((CClassTreeHandler *)treeH)->addGlobalFunctions( methodList, item );
  delete methodList;

  treeH->setLastItem( item );
  item = treeH->addItem( i18n( "Variables" ), THFOLDER, globalsItem );
  attributeList = store->globalContainer.getSortedAttributeList();
  ((CClassTreeHandler *)treeH)->addGlobalVariables( attributeList, item );
  delete attributeList;

  treeH->setLastItem( item );

  // Open the classes and globals folder.
  setOpen( classesItem, true );
  setOpen( globalsItem, true );
}

/*---------------------------------------------- CClassView::addFile()
 * addFile()
 *   Add a source file, parse it and rebuild the tree.
 *
 * Parameters:
 *   aName          The absolute filename.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::addFile( const char *aName )
{
  // Reset the tree.
  ((CClassTreeHandler *)treeH)->clear();

  debug( "Adding file %s", aName );

  // Parse the file.
  cp.parse( aName );

  // Build the new classtree.
  refresh();
}

/*---------------------------------- CClassView::refreshClassByName()
 * refreshClassByName()
 *   Reparse and redraw a class by using its' name.
 *
 * Parameters:
 *   aName          The classname
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::refreshClassByName( const char *aName )
{
  QListViewItem *classItem;
  CParsedClass *aClass;

  classItem = firstChild();
  while( classItem != NULL && strcmp( classItem->text(0), aName ) != 0 )
    classItem = classItem->nextSibling();

  // If the item was found we reparse and update.
  if( classItem )
  {
    // cp->reparseClass( aName );
    aClass = store->getClassByName( aName );
    ((CClassTreeHandler *)treeH)->updateClass( aClass , classItem );
  }
}

/*------------------------------------- CClassView::viewGraphicalTree()
 * viewGraphicalTree()
 *   View graphical classtree.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::viewGraphicalTree()
{
  QList<CClassTreeNode> *forest = store->asForest();
  CGfxClassTreeWindow *cb = new CGfxClassTreeWindow(NULL);
  cb->setCaption(i18n("Graphical classview"));
  cb->InitializeTree(forest);
  cb->show();
}

/*------------------------------------------------- CClassView::tip()
 * tip()
 *   Check and get a tooltip for a point.
 *
 * Parameters:
 *   p          Point to check if we should get a tooltip for.
 *   r          Rectangle of the tooltip item.
 *   str        String that should contain the tooltip.
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::tip( const QPoint &p, QRect &r, QString &str )
{
  QListViewItem *i;

  i = itemAt( p );
  r = itemRect( i );

  if( treeH->itemType( i ) != THFOLDER && i != NULL && r.isValid() )
    str = i->text( 0 );
  else
    str = "";
}

/*------------------------------------- CClassView::viewDefinition()
 * viewDefinition()
 *   Views a definition of an item.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::slotViewDefinition( const char *className, 
                                     const char *declName, 
                                     THType type )
{
  if( validClassDecl( className, declName, type ) )
    emit selectedViewDefinition( className, declName, type );
}

/*------------------------------------- CClassView::viewDefinition()
 * viewDefinition()
 *   Views a declaration of an item.
 *
 * Parameters:
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::slotViewDeclaration( const char *className, 
                                      const char *declName, 
                                      THType type )
{
  if( validClassDecl( className, declName, type ) )
    emit selectedViewDeclaration( className, declName, type );
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*********************************************************************
 *                                                                   *
 *                        PROTECTED QUERIES                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassView::getCurrentClass()
 * getCurrentClass()
 *   Fetches the class currently selected in the tree.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass *CClassView::getCurrentClass()
{
  return store->getClassByName( currentItem()->text(0) );
}


/*--------------------------------- CClassView::getCurrentPopup()
 * getCurrentPopup()
 *   Get the current popupmenu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
KPopupMenu *CClassView::getCurrentPopup()
{
  KPopupMenu *popup = NULL;

  switch( treeH->itemType() )
  {
    case THFOLDER:
      if( strcmp( currentItem()->text(0), i18n( CLASSROOTNAME ) ) == 0 )
        popup = &projectPopup;
      else
        popup = &folderPopup;
      break;
    case THCLASS:
      popup = &classPopup;
      break;
    case THSTRUCT:
      popup = &structPopup;
      break;
    case THPUBLIC_METHOD:
    case THPROTECTED_METHOD:
    case THPRIVATE_METHOD:
    case THGLOBAL_FUNCTION:
      popup = &methodPopup;
      break;
    case THPUBLIC_ATTR:
    case THPROTECTED_ATTR:
    case THPRIVATE_ATTR:
    case THGLOBAL_VARIABLE:
      popup = &attributePopup;
      break;
    case THPUBLIC_SLOT:
    case THPROTECTED_SLOT:
    case THPRIVATE_SLOT:
      popup = &slotPopup;
      break;
    case THSIGNAL:
      popup = &signalPopup;
      break;
    default:
      break;
  }

  return popup;
}

/*--------------------------------- CClassView::getTreeStrItem()
 * getTreeStrItem()
 *   Fetch one node from a tree string.
 *
 * Parameters:
 *   str        String containing the tree.
 *   pos        Current position.
 *   buf        Resulting string.
 *
 * Returns:
 *   int        The new position.
 *-----------------------------------------------------------------*/
int CClassView::getTreeStrItem( const char *str, int pos, char *buf )
{
  int idx = 0;

  // Skip first '.
  pos++;
  while( str[pos] != '\'' )
  {
    buf[idx]=str[pos];
    idx++;
    pos++;
  }

  // Add a null termination.
  buf[ idx ] = '\0';

  // Skip to next ' or to ')'.
  pos++;
  
  return pos;
}

/*------------------------------------------- CClassView::buildTree()
 * buildTree()
 *   Make the classtree from a treestring.
 *
 * Parameters:
 *   str            The string holding the classtree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildTree( const char *str )
{
  uint pos=0;
  QListViewItem *root=NULL;
  QListViewItem *parent=NULL;
  QListViewItem *ci;
  CParsedClass *aPC;
  char buf[50];

  debug( "CClassView::buildtree( treeStr )" );

  while( pos < strlen( str ) )
  {
    if( str[ pos ] == '(' )
    {
      pos++;
      pos = getTreeStrItem( str, pos, buf );

      if( parent == NULL )
      {
        parent = ((CClassTreeHandler *)treeH)->addRoot( buf, THFOLDER );
        root = parent;
      }
      else
        parent = ((CClassTreeHandler *)treeH)->addItem( buf, THFOLDER, parent );
    }

    while( str[ pos ] == '\'' )
    {
      pos = getTreeStrItem( str, pos, buf );
      aPC = store->getClassByName( buf );
      if( aPC )
      {
        ci = ((CClassTreeHandler *)treeH)->addClass( aPC, parent );
        ((CClassTreeHandler *)treeH)->updateClass( aPC, ci );
        treeH->setLastItem( ci );
      }
      else
        ((CClassTreeHandler *)treeH)->addItem( buf, THCLASS, parent );
    }

    if( str[ pos ] == ')' )
    {
      pos++;
      parent = parent->parent();

      if( parent != NULL && parent != root )
        treeH->setLastItem( parent );
    }
  }
}

/*----------------------------------------- CClassView::buildTreeStr()
 * buildTreeStr()
 *   Make a tree(as a string).
 *
 * Parameters:
 *   item           The root item.
 *   str            The string to store the result in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildTreeStr( QListViewItem *item, QString &str )
{
  THType type;

  if( item != NULL )
  {
    while( item != NULL )
    {
      type = treeH->itemType( item );
      if( item != NULL && type == THFOLDER )
      {
        str += "('";
        str += item->text( 0 );
        str += "'";
        str += ( item->isOpen() ? "1" : "0" );
        
        buildTreeStr( item->firstChild(), str );
        str += ")";
      }
      else if( type == THCLASS )
      {
        str += "'";
        str += item->text( 0 );
        str += "'";
        str += ( item->isOpen() ? "1" : "0" );
      }

      // Ignore globals folder.
      if( item->parent() == NULL )
        item = NULL;
      else
        item = item->nextSibling();
    }
  }
}

/*-------------------------------------------- CClassView::asTreeStr()
 * asTreeStr()
 *   Return this view as a treestring.
 *
 * Parameters:
 *   -
 * Returns:
 *   QString        The tree as a string.
 *-----------------------------------------------------------------*/
const char *CClassView::asTreeStr()
{
  QString str="";

  buildTreeStr( classesItem, str );

  return str;
}

/*-------------------------------- CClassView::buildInitalClassTree()
 * buildInitalClassTree()
 *   Build the classtree without using a treestring.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassView::buildInitalClassTree()
{
  QString str;
  CParsedClass *aPC;
  QListViewItem *folder;
  QList<CParsedClass> *list;
  QList<CParsedClass> *iterlist;
  QString projDir;
  QDict< QList<CParsedClass> > dict;
  QDictIterator< QList<CParsedClass> > dictI( dict );
  QList<CParsedClass> rootList;

  debug( "buildInitalClassTree" );

  dict.setAutoDelete( true );

  // Insert root item
  str = i18n( CLASSROOTNAME );
  classesItem = treeH->addRoot( str, THFOLDER );

  list = store->getSortedClassList();
  projDir = project->getProjectDir();

  // Add all parsed classes to the correct list;
  for( aPC = list->first();
       aPC != NULL;
       aPC = list->next() )
  {
    // Try to determine if this is a subdirectory.
    str = aPC->definedInFile;
    str = str.remove( 0, projDir.length() );
    str = str.remove( 0, str.find( '/', 1 ) );
    str = str.remove( str.findRev( '/' ), 10000 );

    if( str.isEmpty() )
      rootList.append( aPC );
    else
    {
      // Remove heading /
      str = str.remove( 0, 1 );

      iterlist = dict.find( str );

      if( iterlist == NULL )
      {
        iterlist = new QList<CParsedClass>();
        dict.insert( str, iterlist );
      }

      iterlist->append( aPC );
    }
  }

  delete list;

  // Add all classes with a folder.
  for( dictI.toFirst();
       dictI.current();
       ++dictI )
  {
    // Add folder.
    folder = treeH->addItem( dictI.currentKey(), THFOLDER, classesItem );
    ((CClassTreeHandler *)treeH)->addClasses( dictI.current(), folder );
    treeH->setLastItem( folder );
  }

  // Add all classes without a folder.
  ((CClassTreeHandler *)treeH)->addClasses( &rootList, classesItem );

  // Save the tree. FOR SOME REASON ONLY 1974 CHARACTERS ARE SAVED. :-(
  //  str = asTreeStr();
  //  project->setClassViewTree( str );
}

/*-------------------------------- CClassView::createCTDlg()
 * createCTDlg()
 *   Create a new ClassTool dialog and setup its' attributes.
 *
 * Parameters:
 *   -
 * Returns:
 *   A newly allocated classtool dialog.
 *-----------------------------------------------------------------*/
CClassToolDlg *CClassView::createCTDlg()
{
  CClassToolDlg *ctDlg = new CClassToolDlg( NULL );

  connect( ctDlg, 
           SIGNAL( signalViewDeclaration(const char *, const char *, THType ) ),
           SLOT(slotViewDeclaration(const char *, const char *, THType ) ) );
                   
  connect( ctDlg, 
           SIGNAL( signalViewDefinition(const char *, const char *, THType ) ),
           SLOT(slotViewDefinition(const char *, const char *, THType ) ) );

  ctDlg->setStore( store );
  ctDlg->setClass( getCurrentClass() );

  return ctDlg;
}

/*-------------------------------------- CClassView::validClassDecl()
 * validClassDecl()
 *   Create a new ClassTool dialog and setup its' attributes.
 *
 * Parameters:
 *   -
 * Returns:
 *   A newly allocated classtool dialog.
 *-----------------------------------------------------------------*/
bool CClassView::validClassDecl( const char *className, 
                                 const char *declName, 
                                 THType type )
{
  bool retVal = false;
  QString str = i18n( "No item selected." );

  retVal = !( className == NULL && declName == NULL );

  if( retVal && className != NULL )
  {
    str = i18n("The class '%1' cound not be found.").arg(className == NULL ? "" : className);
    retVal = store->hasClass( className );

    if( !retVal )
    {
        str = i18n("The struct '%1' cound not be found.").arg(className == NULL ? "" : className);
        retVal = store->hasStruct( className );
    }
  }

  if( !retVal )
    QMessageBox::warning( this, i18n( "Not found" ), str );

  return retVal;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CClassView::slotProjectOptions()
{
  emit selectedProjectOptions();
}

void CClassView::slotGraphicalView()
{
  viewGraphicalTree();
}

void CClassView::slotFileNew()
{
  emit selectedFileNew();
}

void CClassView::slotClassNew()
{
  emit selectedClassNew();
}

void CClassView::slotClassDelete()
{
  if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete this class?"))
      == KMessageBox::Yes)
  {
    KMessageBox::sorry( this, "This function isn't implemented yet." );
  }
                      
}

void CClassView::slotClassViewSelected()
{
  THType type;

  type = treeH->itemType();

  // Take care of left-button clicks.
  if( mouseBtn == LeftButton && type != THFOLDER )
  {
    if( type == THCLASS || type == THSTRUCT || type == THGLOBAL_VARIABLE ||
        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR || 
        type == THPRIVATE_ATTR || type == THSIGNAL )
      slotViewDeclaration();
    else
      slotViewDefinition();
  }
  else if( mouseBtn == MidButton && type != THFOLDER ) // Middle button clicks
  {
    if( type == THCLASS || type == THSTRUCT || type == THGLOBAL_VARIABLE ||
        type == THPUBLIC_ATTR || type == THPROTECTED_ATTR || 
        type == THPRIVATE_ATTR  || type == THSIGNAL )
      slotViewDefinition();
    else
      slotViewDeclaration();
  }

  // Set it back, so next time only if user clicks again we react.
  mouseBtn = RightButton; 
}

void CClassView::slotMethodNew()
{
  emit signalAddMethod( currentItem()->text( 0 ) );
}

void CClassView::slotMethodDelete()
{
  const char *className;
  const char *otherName;
  THType idxType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( &className, &otherName, &idxType );

  emit signalMethodDelete( className, otherName );
}

void CClassView::slotAttributeNew()
{
  emit signalAddAttribute( currentItem()->text( 0 ) );
}

void CClassView::slotAttributeDelete()
{
  if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete this attribute?"))
      == KMessageBox::Yes)
  {
    KMessageBox::sorry(this, i18n("This function isn't implemented yet."));
  }
}

void CClassView::slotAddSlotSignal()
{
}

void CClassView::slotImplementVirtual()
{
}

void CClassView::slotFolderNew() 
{
  CCVAddFolderDlg dlg;
  QListViewItem *item;

  if( dlg.exec() )
  {
    item = ((CClassTreeHandler *)treeH)->addItem( dlg.folderEdit.text(),
                                                  THFOLDER,
                                                  currentItem() );
    setOpen( item, true );
  }
}

void CClassView::slotFolderDelete() 
{
  QListViewItem *parent;

  if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete this folder?"))
      == KMessageBox::Yes)
  {
    parent = currentItem()->parent();
    parent->removeItem( currentItem() );
  }
}

void CClassView::slotClassBaseClasses()
{
  CClassToolDlg *ctDlg = createCTDlg();

  ctDlg->viewParents();
  ctDlg->show();
}

void CClassView::slotClassDerivedClasses() 
{
  CClassToolDlg *ctDlg = createCTDlg();

  ctDlg->viewChildren();
  ctDlg->show();
}

void CClassView::slotClassTool()
{
  CClassToolDlg *ctDlg = createCTDlg();

  ctDlg->show();
}

void CClassView::slotViewDefinition() 
{
  const char *className;
  const char *otherName;
  THType idxType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( &className, &otherName, &idxType );

  slotViewDefinition( className, otherName, idxType );
}

void CClassView::slotViewDeclaration()
{
  const char *className;
  const char *otherName;
  THType idxType;

  // Fetch the current data for classname etc..
  ((CClassTreeHandler *)treeH)->getCurrentNames( &className, &otherName, &idxType );

  slotViewDeclaration( className, otherName, idxType );
}

void CClassView::slotClassWizard()
{
  CClassWizardDlg dlg;

  dlg.setStore( store );
  dlg.exec();
}




