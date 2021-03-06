/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *               (Shared logic for modifications)
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/shared_logic/CClientGUIManager.h
 *  PURPOSE:     GUI entity manager class
 *
 *****************************************************************************/

class CClientGUIManager;
struct SGUIManagerListEntry;

#pragma once

#include <list>
#include "CClientManager.h"
#include "CClientGUIElement.h"
#include <gui/CGUI.h>

class CClientGUIManager
{
    friend class CClientManager;
    friend class CClientGUIElement;

public:
    CClientGUIManager(void);
    ~CClientGUIManager(void);

    void DeleteAll(void);

    bool         Exists(CClientGUIElement* pGUIElement);
    bool         Exists(CGUIElement* pCGUIElement);
    unsigned int Count(void) { return static_cast<unsigned int>(m_Elements.size()); };

    CClientGUIElement* Get(CGUIElement* pCGUIElement);

    void DoPulse(void);
    void QueueGridListUpdate(CClientGUIElement* pGUIElement);

private:
    void Add(CClientGUIElement* pGUIElement);
    void Remove(CClientGUIElement* pGUIElement);
    void FlushQueuedUpdates(void);

private:
    bool                            m_bCanRemoveFromList;
    CMappedList<CClientGUIElement*> m_Elements;
    std::map<ElementID, bool>       m_QueuedGridListUpdates;
};
