#ifndef OKSSYSTEM_EXCEPTIONS_H
#define OKSSYSTEM_EXCEPTIONS_H

/*
 *  exceptions.h
 *  OksSystem
 *
 *  Created by Serguei Kolos on 03.12.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */
 
#include "ers/Issue.hpp"
#include <errno.h>

#include <string>


ERS_DECLARE_ISSUE(OksSystem, Exception, ERS_EMPTY, ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(      OksSystem, // namespace
			     NotFoundIssue, // issue class name
			     OksSystem::Exception,
			     "Object \"" << name << "\" not found", // no message
			     ERS_EMPTY,
			     ((const char *)name ) // single attribute
		      )

ERS_DECLARE_ISSUE_BASE(      OksSystem, // namespace
			     PosixIssue, // issue class name
			     OksSystem::Exception,
			     " (Error code = " << error << ")", // no message
			     ERS_EMPTY,
			     ((int)error ) // single attribute
		      )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			AllocIssue, // issue class name
                        PosixIssue, // base class name
                        "Request for allocating " << size << " bytes of memory fails", // message
                        ((int)error ), // base class attribute
                        ((int)size ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			ExecutionIssue, // issue class name
                        PosixIssue, // base class name
                        "Execution of the \"" << command << "\" command fails with " << status <<  " status", // message
                        ((int)error ), // base class attribute
                        ((const char *)command ) // first attribute
                        ((int)status ) // second attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			TerminationIssue, // issue class name
                        PosixIssue, // base class name
                        "Process terminated with the " << status <<  " status", // message
                        ((int)error ), // base class attribute
                        ((int)status ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			SignalIssue, // issue class name
                        PosixIssue, // base class name
                        "Process has been terminated by the " << signal << " signal", // message
                        ((int)error ), // base class attribute
                        ((int)signal ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			NoUserIssue, // issue class name
                        PosixIssue, // base class name
                        "User with (name:id)=(" << name << ":" << id << ") does not exist", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // first attribute
                        ((int)id ) // second attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			OksSystemCallIssue, // issue class name
                        PosixIssue, // base class name
                        "OksSystem call \"" << name << "\" fails " << action, // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // first attribute
                        ((const char *)action ) // second attribute (short description of what the okssystem call was doing)
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			OpenFileIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not open file \"" << name << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			ReadIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not read from file \"" << name << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			WriteIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not write to file \"" << name << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			CloseFileIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not close file \"" << name << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			RemoveFileIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not remove file \"" << name << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)name ) // single attribute
                 )

ERS_DECLARE_ISSUE_BASE(	OksSystem, // namespace
			RenameFileIssue, // issue class name
                        PosixIssue, // base class name
                        "Can not rename file \"" << source << "\" to file \"" << dest << "\"", // message
                        ((int)error ), // base class attribute
                        ((const char *)source ) // single attribute
                        ((const char *)dest ) // single attribute
                 )

#define OKSSYSTEM_ALLOC_CHECK( p, size ) \
{ if(0==p) throw OksSystem::AllocIssue( ERS_HERE, errno, size ); }

#endif
