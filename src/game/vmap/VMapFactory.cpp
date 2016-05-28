/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
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

#include <sys/types.h>
#include "VMapFactory.h"
#include "VMapManager2.h"

using namespace G3D;

namespace VMAP
{

    IVMapManager *gVMapManager = 0;
    std::list<std::pair<uint32, uint32>> disabledModels;

    void VMapFactory::disableCertainModels(std::string models)
    {
        size_t space = models.find(' ');
        size_t delim = models.find(',');
        while (space != models.npos)
        {
            uint32 map = atoi(models.substr(0, space).c_str());
            uint32 model = atoi(models.substr(space).c_str());
            disabledModels.push_back(std::pair<uint32, uint32>(map, model));
            if (delim != models.npos)
                models = models.substr(delim + 1);
            size_t space = models.find(' ');
            size_t delim = models.find(',');
        }
    }

    bool VMapFactory::isValidModel(uint32 map, uint32 model)
    {
        // will require some optimization someday
        for (std::list<std::pair<uint32, uint32>>::iterator i = disabledModels.begin(); i != disabledModels.end(); i++)
        {
            if (i->first == map && i->first == model)
                return false;
        }
        return true;
    }

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
        disabledModels.clear();

        if (gVMapManager)
            delete gVMapManager;
        gVMapManager = NULL;
    }
}
