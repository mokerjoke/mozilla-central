/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Created by: Paul Sandoz   <paul.sandoz@sun.com>
 *   Chris Waterson <waterson@netscape.com>
 *   Csaba Borbola  <csaba.borbola@sun.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsIIOService.h"
#include "nsNetCID.h"
#include "nsMemory.h"
#include "nsString.h"
#include "nsCRT.h"
#include "plstr.h"

#include "nsAbDirFactoryService.h"
#include "nsIAbDirFactory.h"

NS_IMPL_ISUPPORTS1(nsAbDirFactoryService, nsIAbDirFactoryService)

nsAbDirFactoryService::nsAbDirFactoryService()
{
}

nsAbDirFactoryService::~nsAbDirFactoryService()
{
}

/* nsIAbDirFactory getDirFactory (in string uri); */
NS_IMETHODIMP nsAbDirFactoryService::GetDirFactory(const char* aURI,
        nsIAbDirFactory** aDirFactory)
{
    nsresult rv;

    NS_PRECONDITION(aURI != nsnull, "null ptr");
    if (!aURI)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(aDirFactory != nsnull, "null ptr");
    if (!aDirFactory)
        return NS_ERROR_NULL_POINTER;

    // Obtain the network IO service
    nsCOMPtr<nsIIOService> nsService = do_GetService (NS_IOSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv,rv);
    
    // Extract the scheme
	nsCAutoString scheme;
    rv = nsService->ExtractScheme (nsDependentCString(aURI), scheme);
    NS_ENSURE_SUCCESS(rv,rv);

    // TODO 
    // Change to use string classes

    // Try to find a factory using the component manager.
    static const char kAbDirFactoryContractIDPrefix[]
        = NS_AB_DIRECTORY_FACTORY_CONTRACTID_PREFIX;

    PRInt32 pos = scheme.Length();
    PRInt32 len = pos + sizeof(kAbDirFactoryContractIDPrefix) - 1;

    // Safely convert to a C-string for the XPCOM APIs
    char buf[128];
    char* contractID = buf;
    if (len >= PRInt32(sizeof buf))
        contractID = NS_STATIC_CAST(char *,nsMemory::Alloc(len + 1));

    if (contractID == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

    PL_strcpy(contractID, kAbDirFactoryContractIDPrefix);
    PL_strncpy(contractID + sizeof(kAbDirFactoryContractIDPrefix) - 1, aURI, pos);
    contractID[len] = '\0';

    nsCID cid;
    rv = nsComponentManager::ContractIDToClassID(contractID, &cid);
    NS_ENSURE_SUCCESS(rv,rv);

    if (contractID != buf)
        nsCRT::free(contractID);

    nsCOMPtr<nsIFactory> factory;
    rv = nsComponentManager::FindFactory(cid, getter_AddRefs(factory));
    NS_ASSERTION(NS_SUCCEEDED(rv), "factory registered, but couldn't load");
    NS_ENSURE_SUCCESS(rv,rv);

    rv = factory->CreateInstance(nsnull, NS_GET_IID(nsIAbDirFactory), NS_REINTERPRET_CAST(void**, aDirFactory));
    return rv;
}

/* nsIAbDirFactory getDirFactory (in string uri); */
/*
NS_IMETHODIMP nsAbDirFactoryService::GetDirFactory(const char* aURI,
        nsIAbDirFactory** aDirFactory)
{
    NS_PRECONDITION(aURI != nsnull, "null ptr");
    if (!aURI)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(aDirFactory != nsnull, "null ptr");
    if (!aDirFactory)
        return NS_ERROR_NULL_POINTER;

    // Compute the scheme of the URI. Scan forward until we either:
    //
    // 1. Reach the end of the string
    // 2. Encounter a non-alpha character
    // 3. Encouter a colon.
    //
    // If we encounter a colon _before_ encountering a non-alpha
    // character, then assume it's the scheme.
    //
    // XXX Although it's really not correct, we'll allow underscore
    // characters ('_'), too.
    const char* p = aURI;
    while (IsLegalSchemeCharacter(*p))
        ++p;
    
    if (*p != ':')
        return NS_ERROR_FAILURE;

    nsresult rv;
    nsCOMPtr<nsIFactory> factory;
    PRUint32 prefixlen = 0;

    prefixlen = (p - aURI);

    // Try to find a factory using the component manager.
    static const char kAbDirFactoryContractIDPrefix[]
        = NS_AB_DIRECTORY_FACTORY_CONTRACTID_PREFIX;

    PRInt32 pos = p - aURI;
    PRInt32 len = pos + sizeof(kAbDirFactoryContractIDPrefix) - 1;

    // Safely convert to a C-string for the XPCOM APIs
    char buf[128];
    char* contractID = buf;
    if (len >= PRInt32(sizeof buf))
        contractID = NS_STATIC_CAST(char *,nsMemory::Alloc(len + 1));

    if (contractID == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

    PL_strcpy(contractID, kAbDirFactoryContractIDPrefix);
    PL_strncpy(contractID + sizeof(kAbDirFactoryContractIDPrefix) - 1, aURI, pos);
    contractID[len] = '\0';

    nsCID cid;
    rv = nsComponentManager::ContractIDToClassID(contractID, &cid);
    NS_ENSURE_SUCCESS(rv,rv);

    if (contractID != buf)
        nsCRT::free(contractID);


    rv = nsComponentManager::FindFactory(cid, getter_AddRefs(factory));
    NS_ASSERTION(NS_SUCCEEDED(rv), "factory registered, but couldn't load");
    NS_ENSURE_SUCCESS(rv,rv);

    rv = factory->CreateInstance(nsnull, NS_GET_IID(nsIAbDirFactory), NS_REINTERPRET_CAST(void**, aDirFactory));
    return rv;
}
*/
