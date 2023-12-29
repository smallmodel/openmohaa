/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cl_ui.h"

class PMPickerItem : public UIListCtrlItem
{
    str m_string;

public:
    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

CLASS_DECLARATION(USignal, PlayerModelPickerClass, NULL) {
    {&EV_UIListBase_ItemSelected,       &PlayerModelPickerClass::FileSelected },
    {&EV_UIListBase_ItemDoubleClicked,  &PlayerModelPickerClass::FileChosen   },
    {&UIFloatingWindow::W_ClosePressed, &PlayerModelPickerClass::OnDeactivated},
    {&W_Deactivated,                    &PlayerModelPickerClass::OnDeactivated},
    {NULL,                              NULL                                  }
};

PlayerModelPickerClass::PlayerModelPickerClass()
{
    // FIXME: stub
}

PlayerModelPickerClass::~PlayerModelPickerClass()
{
    // FIXME: stub
}

void PlayerModelPickerClass::Setup(const char *root_directory, const char *current_directory, qboolean bGermanModels)
{
    // FIXME: stub
}

void PlayerModelPickerClass::Initialize(
    const char *root_directory, const char *current_directory, qboolean bGermanModels
)
{
    // FIXME: stub
}

void PlayerModelPickerClass::SetupFiles(void)
{
    // FIXME: stub
}

void PlayerModelPickerClass::FileSelected(str& currentDirectory, str& partialName, str& fullname)
{
    // FIXME: stub
}

void PlayerModelPickerClass::FileChosen(str& currentDirectory, str& partialName, str& fullname)
{
    // FIXME: stub
}

void PlayerModelPickerClass::FileSelected(Event *ev)
{
    // FIXME: stub
}

void PlayerModelPickerClass::FileChosen(Event *ev)
{
    // FIXME: stub
}

void PlayerModelPickerClass::CloseWindow(void)
{
    // FIXME: stub
}

void PlayerModelPickerClass::OnDeactivated(Event *ev)
{
    // FIXME: stub
}

int PMPickerItem::getListItemValue(int which) const
{
    // FIXME: stub
    return 0;
}

griditemtype_t PMPickerItem::getListItemType(int which) const
{
    // FIXME: stub
    return griditemtype_t::TYPE_STRING;
}

str PMPickerItem::getListItemString(int which) const
{
    // FIXME: stub
    return str();
}

void PMPickerItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont)
{
    // FIXME: stub
}

qboolean PMPickerItem::IsHeaderEntry() const
{
    // FIXME: stub
    return qfalse;
}
