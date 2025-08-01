/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#pragma once

class UIListItem
{
public:
    str itemname;
    str itemalias;
};

class UIList : public UIWidget
{
protected:
    Container<UIListItem *> m_itemlist;
    int                     m_currentItem;
    float                   m_arrow_width;
    UIRect2D               *m_next_arrow_region;
    UIRect2D               *m_prev_arrow_region;
    bool                    m_depressed;
    bool                    m_held;
    UIReggedMaterial       *m_prev_arrow;
    UIReggedMaterial       *m_next_arrow;
    bool                    m_prev_arrow_depressed;
    bool                    m_next_arrow_depressed;

public:
    CLASS_PROTOTYPE(UIList);

protected:
    void     Draw(void) override;
    qboolean KeyEvent(int key, unsigned int time) override;
    void     CharEvent(int ch) override;
    void     Pressed(Event *ev);
    void     Released(Event *ev);
    void     ScrollNext(void);
    void     ScrollPrev(void);
    void     FrameInitialized(void) override;
    void     LayoutAddListItem(Event *ev);
    void     AddItem(str item, str alias);

public:
    UIList();
    ~UIList();

    void UpdateUIElement(void) override;
    void UpdateData(void) override;
};

class UIListIndex : public UIList
{
public:
    CLASS_PROTOTYPE(UIListIndex);

    qboolean KeyEvent(int key, unsigned int time) override;
};
