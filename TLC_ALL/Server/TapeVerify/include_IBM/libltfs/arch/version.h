/*
**  %Z% %I% %W% %G% %U%
**
**  ZZ_Copyright_BEGIN
**
**
**  Licensed Materials - Property of IBM
**
**  IBM Linear Tape File System Single Drive Edition Version 1.3.0.2 for Linux and Mac OS X
**
**  Copyright IBM Corp. 2010, 2013
**
**  This file is part of the IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X
**  (formally known as IBM Linear Tape File System)
**
**  The IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X is free software;
**  you can redistribute it and/or modify it under the terms of the GNU Lesser
**  General Public License as published by the Free Software Foundation,
**  version 2.1 of the License.
**
**  The IBM Linear Tape File System Single Drive Edition for Linux and Mac OS X is distributed in the
**  hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
**  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**  See the GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
**  or download the license from <http://www.gnu.org/licenses/>.
**
**
**  ZZ_Copyright_END
**
*************************************************************************************
**
** COMPONENT NAME:  IBM Linear Tape File System
**
** FILE NAME:       arch/version.h
**
** DESCRIPTION:     Platform-specific version information.
**
** AUTHOR:          Michael A. Richmond
**                  IBM Almaden Research Center
**                  mar@almaden.ibm.com
**
*************************************************************************************
*/

#ifndef VERSION_H_
#define VERSION_H_

/*
 * Creator tag value format:
 *     PackageName Version - Platform - Program
 *     IBM LTFS 0.2.0 - Linux - mkltfs
 */
#define CREATOR_STRING_FORMAT "%s %s - %s - %s"

#ifdef __APPLE__
    #define PLATFORM "Mac OS X"
#elif mingw_PLATFORM
    #define PLATFORM "Windows"
#else
    #define PLATFORM "Linux"
#endif /* __APPLE__ */

#endif /* VERSION_H_ */
