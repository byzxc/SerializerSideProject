/******************************************************************************/
/*!
\file       SpaceAssert.h
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#ifndef _SPACE_ASSERT_H_
#define _SPACE_ASSERT_H_

#ifdef NDEBUG
#define SPACEASSERT(condition, logMessage) ((void)0)
#else   
#include "Logger.h"
#include <cassert>

/*If Assert fail,*/
#define SPACEASSERT(condition, logMessage)\
            {\
                if (!(condition))\
                {\
                    std::stringstream loggedMessage;\
                    std::cout << logMessage << std::endl;\
                    loggedMessage << "Assert Crashed: " << #logMessage << " at " << __FILE__ << " (" << __LINE__ << ")" ; \
                    Logger log("log\\Assertion.log");\
                    log.logError(loggedMessage.str(), ErrorType::ERROR);\
                    assert(condition);\
                }\
            }

#endif

#ifndef SPACEUNUSED
#define SPACEUNUSED(x) (void)x
#endif

#endif

