/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "VMapManager2.h"
#include "VMapFactory.h"

namespace VMAP
{

    IVMapManager *gVMapManager = 0;

    //===============================================
    // just return the instance
    IVMapManager* VMapFactory::createOrGetVMapManager()
    {
        if(gVMapManager == 0)
            gVMapManager= new VMapManager2();                // should be taken from config ... Please change if you like :-)
        return gVMapManager;
    }

    //===============================================
    // delete all internal data structures
    void VMapFactory::clear()
    {
        if (gVMapManager)
            delete gVMapManager;
        gVMapManager = NULL;
    }
}
