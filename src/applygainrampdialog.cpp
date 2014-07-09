/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "applygainrampdialog.h"
#include "ui_applygainrampdialog.h"


//==================================================================================================
// Public:

ApplyGainRampDialog::ApplyGainRampDialog( QWidget* parent ) :
    QDialog( parent ),
    mUI( new Ui::ApplyGainRampDialog )
{
    mUI->setupUi( this );
}



ApplyGainRampDialog::~ApplyGainRampDialog()
{
    delete mUI;
}



qreal ApplyGainRampDialog::getStartGainValue() const
{
    return mUI->doubleSpinBox_StartGain->value();
}



qreal ApplyGainRampDialog::getEndGainValue() const
{
    return mUI->doubleSpinBox_EndGain->value();
}



//==================================================================================================
// Protected:

void ApplyGainRampDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        mUI->retranslateUi(this);
        break;
    default:
        break;
    }
}



//==================================================================================================
// Private Slots:

void ApplyGainRampDialog::on_dial_StartGain_valueChanged( const int value )
{
    mUI->doubleSpinBox_StartGain->setValue( value / 10.0 );
}



void ApplyGainRampDialog::on_doubleSpinBox_StartGain_valueChanged( const double value )
{
    mUI->dial_StartGain->setValue( value * 10 );
}



void ApplyGainRampDialog::on_dial_EndGain_valueChanged( const int value )
{
    mUI->doubleSpinBox_EndGain->setValue( value / 10.0 );
}



void ApplyGainRampDialog::on_doubleSpinBox_EndGain_valueChanged( const double value )
{
    mUI->dial_EndGain->setValue( value * 10 );
}
