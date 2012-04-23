/************************************************************************
*
* Copyright 2010 Jakob Leben (jakob.leben@gmail.com)
*
* This file is part of SuperCollider Qt GUI.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
************************************************************************/

#include "BasicWidgets.h"
#include "../QcWidgetFactory.h"
#include "../Common.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#ifdef Q_WS_MAC
# include <QMacStyle>
#endif

QC_DECLARE_QWIDGET_FACTORY( QcDefaultWidget );
QC_DECLARE_QWIDGET_FACTORY( QcHLayoutWidget );
QC_DECLARE_QWIDGET_FACTORY( QcVLayoutWidget );
QC_DECLARE_QWIDGET_FACTORY( QLabel );
QC_DECLARE_QWIDGET_FACTORY( QcTextField );
QC_DECLARE_QWIDGET_FACTORY( QcCheckBox );

//////////////////////////// QcListWidget //////////////////////////////////////

class QcListWidgetFactory : public QcWidgetFactory<QcListWidget>
{
  void initialize( QWidgetProxy *p, QcListWidget *l ) {
    p->setMouseEventWidget( l->viewport() );
  }
};

QC_DECLARE_FACTORY( QcListWidget, QcListWidgetFactory );

QcListWidget::QcListWidget() : _emitAction(true)
{
  connect( this, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ),
           this, SLOT( onCurrentItemChanged() ) );
}

void QcListWidget::setItems( const VariantList & items )
{
  _emitAction = false;
  clear();
  Q_FOREACH( QVariant item, items.data )
    addItem( item.toString() );
  setCurrentRow( 0 );
  _emitAction = true;
}

void QcListWidget::setColors( const VariantList & colors ) const
{
  int cc = colors.data.count();
  int ic = count();
  for( int i=0; i<cc && i < ic; ++i ) {
    QListWidgetItem *it = item(i);
    QColor color( colors.data[i].value<QColor>() );
    if( color.isValid() ) it->setBackground( color );
  }
}

void QcListWidget::setCurrentRowWithoutAction( int row )
{
  bool b = _emitAction;
  _emitAction = false;
  setCurrentRow( row );
  _emitAction = b;
}

void QcListWidget::onCurrentItemChanged()
{
  if( _emitAction ) Q_EMIT( action() );
}

void QcListWidget::keyPressEvent( QKeyEvent *e )
{
  QListWidget::keyPressEvent( e );

  switch (e->key())
  {
  case Qt::Key_Return:
  case Qt::Key_Enter:
    Q_EMIT( returnPressed() );
    e->accept();
    break;
  case Qt::Key_Up:
  case Qt::Key_Down:
  case Qt::Key_Left:
  case Qt::Key_Right:
  case Qt::Key_PageUp:
  case Qt::Key_PageDown:
  case Qt::Key_Home:
  case Qt::Key_End:
    // Prevent propagating to parent when scroller reaches minimum or maximum:
    e->accept();
  default: break;
  }
}

////////////////////////// QcPopUpMenu /////////////////////////////////////////

QC_DECLARE_QWIDGET_FACTORY(QcPopUpMenu);

QcPopUpMenu::QcPopUpMenu() :
  _changed(false),
  _reactivation(false)
{
  connect( this, SIGNAL(currentIndexChanged(int)),
           this, SLOT(setChanged()) );
  connect( this, SIGNAL(currentIndexChanged(int)),
           this, SLOT(clearChanged()), Qt::QueuedConnection );

  connect( this, SIGNAL(activated(int)), this, SLOT(doAction(int)) );
}

void QcPopUpMenu::setItems( const VariantList & items )
{
  clear();
  Q_FOREACH( QVariant item, items.data )
      addItem( item.toString() );
}

void QcPopUpMenu::doAction( int choice )
{
  if( _changed || _reactivation )
    Q_EMIT( action() );
}

/////////////////////////////// QcTextField ////////////////////////////////////

void QcTextField::keyPressEvent( QKeyEvent *e )
{
  // NOTE: We could use the returnPressed() signal, but that would still
  // propagate the event to parent, which we want to avoid.
  if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    Q_EMIT(action());
  else
    QLineEdit::keyPressEvent(e);
}

/////////////////////////////// QcButton ///////////////////////////////////////

QC_DECLARE_QWIDGET_FACTORY(QcButton);

QcButton::QcButton()
: currentState(0), defaultPalette( palette() )
{
  connect( this, SIGNAL(clicked()), this, SLOT(doAction()) );
}

#ifdef Q_WS_MAC
bool QcButton::hitButton( const QPoint & pos ) const
{
  // FIXME: This is a workaround for Qt Bug 15936:
  // incorrect QPushButton hit area.

  QMacStyle *macStyle = qobject_cast<QMacStyle *>(style());

  if( !macStyle || isFlat() )
    return QAbstractButton::hitButton(pos);
  else
    return QPushButton::hitButton(pos);
}
#endif

void QcButton::setStates( const VariantList & statesArg )
{
  if( !statesArg.data.count() ) return;

  states.clear();

  Q_FOREACH( QVariant var, statesArg.data ) {
    VariantList stateArg = var.value<VariantList>();
    int count = stateArg.data.size();
    State state;
    if( count >= 1 )
      state.text = stateArg.data[0].toString();
    if( count >= 2 )
      state.textColor = stateArg.data[1].value<QColor>();
    if( count >= 3 )
      state.buttonColor = stateArg.data[2].value<QColor>();
    states.append( state );
  }
  setState( 0 );
}

void QcButton::setState( int i )
{
  int c = states.count();
  if( !c ) return;

  i = qBound( 0, i, c-1 );

  currentState = i;

  State state = states[i];

  setText( state.text );

  QPalette p ( defaultPalette );

  if( state.textColor.isValid() )
    p.setColor( QPalette::ButtonText, state.textColor );
  if( state.buttonColor.isValid() )
    p.setColor( QPalette::Button, state.buttonColor );

  setPalette( p );
}

void QcButton::cycleStates()
{
  if( states.size() < 2 ) return;
  int i = currentState + 1;
  if( i >= states.size() ) i = 0;
  setState( i );
}

void QcButton::doAction()
{
  cycleStates();
  Q_EMIT( action((int)QApplication::keyboardModifiers()) );
}

/////////////////////////////////////////////////////////////////////////

class QcCustomPaintedFactory : public QcWidgetFactory<QcCustomPainted>
{
protected:
  virtual void initialize( QWidgetProxy *p, QcCustomPainted *w )
  {
    QObject::connect( w, SIGNAL(painting(QPainter*)),
                      p, SLOT(customPaint(QPainter*)) );
  }
};

QC_DECLARE_FACTORY( QcCustomPainted, QcCustomPaintedFactory );
