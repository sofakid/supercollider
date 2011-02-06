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

#ifndef QC_CANVAS_H
#define QC_CANVAS_H

#include <QWidget>
#include <QPixmap>

class QcCanvas : public QWidget
{
  Q_OBJECT
public:
  QcCanvas( QWidget *parent = 0 );
  void setBackground( const QColor & c ) { _bkgColor = c; update(); }
  void setPaint( bool b ) { paint = b; }
public Q_SLOTS:
  void refresh();
Q_SIGNALS:
  void painting(QPainter*);
protected:
  virtual void customEvent( QEvent * );
  virtual void resizeEvent( QResizeEvent * );
  virtual void paintEvent( QPaintEvent * );

private:
  QPixmap _pixmap;
  QColor _bkgColor;
  bool paint;
  bool repaintNeeded;
};

#endif
