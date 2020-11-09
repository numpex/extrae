/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Extrae                                  *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

#include "common.h"
#include "utils.h"

#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
#ifdef HAVE_MATH_H
# include <math.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "wrapper.h"
#include "clock.h"
#include "hwc.h"
#include "extrae_user_events.h"
#include "misc_wrapper.h"
#include "common_hwc.h"
#include "threadinfo.h"
#include "sampling-common.h"
#include "xalloc.h"

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

void Extrae_shutdown_Wrapper (void)
{
	TRACE_MISCEVENTANDCOUNTERS (LAST_READ_TIME, TRACING_EV, EVT_END, EMPTY);
	tracejant = FALSE;
}

void Extrae_restart_Wrapper (void)
{
	tracejant = TRUE;
	TRACE_MISCEVENTANDCOUNTERS (LAST_READ_TIME, TRACING_EV, EVT_BEGIN, EMPTY);

	/* Force bursty tracing to consider only from this point */
	last_mpi_exit_time = LAST_READ_TIME;
}

void Extrae_N_Event_Wrapper (unsigned *count, extrae_type_t *types, extrae_value_t *values)
{
	unsigned i;
	int events_id[*count];

	if (*count > 0)
	{
		for (i = 0; i < *count; i++)
			events_id[i] = USER_EV;
		TRACE_N_MISCEVENT(LAST_READ_TIME, *count, events_id, types, values);
	}
}

void Extrae_N_Eventsandcounters_Wrapper (unsigned *count, extrae_type_t *types, extrae_value_t *values)
{
	unsigned i;
	int events_id[*count];

	if (*count > 0)
	{
		for (i = 0; i < *count; i++)
			events_id[i] = USER_EV;
		TRACE_N_MISCEVENTANDCOUNTERS(LAST_READ_TIME, *count, events_id, types, values);
	}
}

void Extrae_counters_Wrapper (void)
{
#if USE_HARDWARE_COUNTERS
	TRACE_EVENTANDCOUNTERS (LAST_READ_TIME, HWC_EV, 0, TRUE);
#endif
}

void Extrae_counters_at_Time_Wrapper (UINT64 time)
{
#if USE_HARDWARE_COUNTERS
	TRACE_EVENTANDCOUNTERS (time, HWC_EV, 0, TRUE);
#else
	UNREFERENCED_PARAMETER(time);
#endif
}

void Extrae_next_hwc_set_Wrapper (void)
{
#if USE_HARDWARE_COUNTERS
	HWC_Start_Next_Set (COUNT_GLOBAL_OPS, LAST_READ_TIME, THREADID);
#endif
}

void Extrae_previous_hwc_set_Wrapper (void)
{
#if USE_HARDWARE_COUNTERS
	HWC_Start_Previous_Set (COUNT_GLOBAL_OPS, LAST_READ_TIME, THREADID);
#endif
}

void Extrae_set_options_Wrapper (int options)
{
	Trace_Caller_Enabled[CALLER_MPI] = options & EXTRAE_CALLER_OPTION;
	Trace_HWC_Enabled = options & EXTRAE_HWC_OPTION;
	tracejant_mpi     = options & EXTRAE_MPI_OPTION;   
	tracejant_omp     = options & EXTRAE_OMP_OPTION;   
	Extrae_set_pthread_tracing (options & EXTRAE_PTHREAD_OPTION);
	tracejant_hwc_mpi = options & EXTRAE_MPI_HWC_OPTION;
	tracejant_hwc_omp = options & EXTRAE_OMP_HWC_OPTION;   
	Extrae_set_pthread_hwc_tracing (options & EXTRAE_PTHREAD_HWC_OPTION);
	tracejant_hwc_uf  = options & EXTRAE_UF_HWC_OPTION;
	Extrae_setSamplingEnabled (options & EXTRAE_SAMPLING_OPTION);
}

UINT64 Extrae_user_function_Wrapper (unsigned enter)
{
	UINT64 ip = (enter)?Extrae_get_caller(4):EMPTY;
	TRACE_EVENTANDCOUNTERS (LAST_READ_TIME, USRFUNC_EV, ip, tracejant_hwc_uf);
	return ip;
}

void Extrae_function_from_address_Wrapper (extrae_type_t type, void *address)
{
	if (type == USRFUNC_EV || type == OMPFUNC_EV)
	{
#if USE_HARDWARE_COUNTERS
		int filter = (type==USRFUNC_EV)?tracejant_hwc_uf:tracejant_hwc_omp;
		TRACE_EVENTANDCOUNTERS (LAST_READ_TIME, type, (UINT64) address, filter);
#else
		TRACE_EVENT(LAST_READ_TIME, type, (UINT64) address);
#endif
	}
}

void Generate_Task_File_List (void)
{
	int filedes;
	unsigned thid;
	ssize_t ret;
	char tmpname[1024];
	char hostname[1024];
	char tmp_line[1024];

	sprintf (tmpname, "%s/%s%s", final_dir, appl_name, EXT_MPITS);

	filedes = open (tmpname, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (filedes < 0)
		return;

	if (gethostname (hostname, sizeof(hostname)) != 0)
		sprintf (hostname, "localhost");

	for (thid = 0; thid < Backend_getMaximumOfThreads(); thid++)
	{
		FileName_PTT(tmpname, Get_FinalDir(TASKID), appl_name, hostname,
		  getpid(), TASKID, thid, EXT_MPIT);

		sprintf (tmp_line, "%s named %s\n", tmpname,
		  Extrae_get_thread_name(thid));
		ret = write (filedes, tmp_line, strlen (tmp_line));
		if (ret != (ssize_t) strlen (tmp_line))
		{
			close (filedes);
			return;
		}
	}

	close (filedes);
	return;
}

void Extrae_init_tracing (int forked)
{
	iotimer_t temps_init, temps_fini;
	char * config_file = getenv ("EXTRAE_CONFIG_FILE");
	if (config_file == NULL)
		config_file = getenv ("MPTRACE_CONFIG_FILE");

	Extrae_set_initial_TASKID (TASKID);

	// Initialize the backend
	if (!Backend_preInitialize (TASKID, Extrae_get_num_tasks(), config_file, forked))
		return;

#if !defined(MPI_SUPPORT) // In MPI libraries this is done later at MPI_Init wrapper
	// Generate tentative file list if we don't reuse previous execution through extrae-cmd
	if (!Extrae_getAppendingEventsToGivenPID(NULL))
	{
		Generate_Task_File_List();
	}
#endif

	// Take the time
	temps_init = TIME;

	// Execute a barrier within tasks so they will be synchronized
	Extrae_barrier_tasks();

	// Take the time (a newer one)
	temps_fini = TIME;

	// End initialization of the backend
	if (!Backend_postInitialize (TASKID, Extrae_get_num_tasks(), TRACE_INIT_EV, temps_init, temps_fini, NULL))
		return;

	Extrae_set_is_initialized (EXTRAE_INITIALIZED_EXTRAE_INIT);
}

void Extrae_init_Wrapper (void)
{
	/* Do not initialize if it's already initialized */
	if (Extrae_is_initialized_Wrapper() == EXTRAE_NOT_INITIALIZED)
	{
		/* Actually initialize the tracing */
		Extrae_init_tracing(FALSE);
	}
	else
	{
		char *previous = "Unknown";
		if (Extrae_is_initialized_Wrapper() == EXTRAE_INITIALIZED_EXTRAE_INIT)
			previous = "API";
		else if (Extrae_is_initialized_Wrapper() == EXTRAE_INITIALIZED_MPI_INIT)
			previous = "MPI";
		else if (Extrae_is_initialized_Wrapper() == EXTRAE_INITIALIZED_SHMEM_INIT)
			previous = "SHMEM";

		fprintf (stderr, PACKAGE_NAME": Warning! API tries to initialize more than once\n");
		fprintf (stderr, PACKAGE_NAME":          Previous initialization was done by %s\n", previous);

		/* It is necessary to update the number of threads after an explicit call to
		 * Extrae_init(). In the case of Nanos instrumentation, the
		 * auto-initialization only sees 1 thread, but after the Extrae_init() call
		 * the runtime has created more internal threads that we are not aware of.
		 */
		Backend_ChangeNumberOfThreads( Extrae_get_num_threads() );
	}
}

void Extrae_fini_Wrapper (void)
{
	/* Finalize only if it is initialized */
	if (Extrae_is_initialized_Wrapper() != EXTRAE_NOT_INITIALIZED)
	{
		/* Finalize tracing library */
		Backend_Finalize ();
	}
}

void Extrae_init_UserCommunication_Wrapper (struct extrae_UserCommunication *ptr)
{
	xmemset (ptr, 0, sizeof(struct extrae_UserCommunication));
}

void Extrae_init_CombinedEvents_Wrapper (struct extrae_CombinedEvents *ptr)
{
	xmemset (ptr, 0, sizeof(struct extrae_CombinedEvents));
	ptr->UserFunction = EXTRAE_USER_FUNCTION_NONE;
}

void Extrae_emit_CombinedEvents_Wrapper (struct extrae_CombinedEvents *ptr)
{
	unsigned i;
	int events_id[ptr->nEvents];

	/* Emit events first */
	if (ptr->nEvents > 0)
	{
		for (i = 0; i < ptr->nEvents; i++)
			events_id[i] = USER_EV;
		if (ptr->HardwareCounters)
		{
			TRACE_N_MISCEVENTANDCOUNTERS(LAST_READ_TIME, ptr->nEvents, events_id, ptr->Types, ptr->Values);
		}
		else
		{
			TRACE_N_MISCEVENT(LAST_READ_TIME, ptr->nEvents, events_id, ptr->Types, ptr->Values);
		}
	}

	/* Emit user function. If hwc were emitted before, don't emit now because they
	   will share the same timestamp and Paraver won't handle that well. Otherwise,
	   honor tracejant_hwc_uf
	*/
	if (ptr->UserFunction != EXTRAE_USER_FUNCTION_NONE)
	{
		UINT64 ip = (ptr->UserFunction == EXTRAE_USER_FUNCTION_ENTER)?Extrae_get_caller(4):EMPTY;
#if USE_HARDWARE_COUNTERS
		int EmitHWC = (!ptr->HardwareCounters && tracejant_hwc_uf);
		TRACE_EVENTANDCOUNTERS (LAST_READ_TIME, USRFUNC_EV, ip, EmitHWC);
#else
		TRACE_EVENT (LAST_READ_TIME, USRFUNC_EV, ip);
#endif
	}

	/* Now emit the callers */
	if (ptr->Callers)
	{
		Extrae_trace_callers (LAST_READ_TIME, 4, CALLER_MPI);
	}

	/* Finally emit user communications */
	for (i = 0; i < ptr->nCommunications ; i++)
		TRACE_USER_COMMUNICATION_EVENT(LAST_READ_TIME,
		  (ptr->Communications[i].type==EXTRAE_USER_SEND)?USER_SEND_EV:USER_RECV_EV,
		  ptr->Communications[i].partner, ptr->Communications[i].size,
		  ptr->Communications[i].tag, ptr->Communications[i].id) 
}

/* ***************************************************************************
   These API calls are intended for NANOS instrumentation:
   - Resume_virtual_thread: Used to mark a nanos task that is being executed
   - Suspend_virtual_thread: Used to mark that the current nanos task is being
      suspended. Also, dump stacked types using ->
   - Extrae_register_stacked_type
* ***************************************************************************/

void Extrae_Resume_virtual_thread_Wrapper (unsigned u)
{
	TRACE_EVENTANDCOUNTERS(LAST_READ_TIME, RESUME_VIRTUAL_THREAD_EV, u, FALSE);
}

void Extrae_Suspend_virtual_thread_Wrapper (void)
{
	TRACE_EVENTANDCOUNTERS(LAST_READ_TIME, SUSPEND_VIRTUAL_THREAD_EV, EMPTY, FALSE);
}

void Extrae_register_stacked_type_Wrapper (extrae_type_t type)
{
	TRACE_EVENT(LAST_READ_TIME,REGISTER_STACKED_TYPE_EV,type);
}


/***************************************************************************
  Return the version of Extrae
 **************************************************************************/

void Extrae_get_version_Wrapper (unsigned *major, unsigned *minor,
  unsigned *revision)
{
	int tokens = 0;
	char *version = VERSION;
	char **tokenArray = NULL;
  char *endptr;

	tokens = __Extrae_Utils_explode(version, ".", &tokenArray);

	if (tokens > 0) {
		*major    = strtoul(tokenArray[0], &endptr, 10);
	}
	if (tokens > 1) {
		*minor    = strtoul(tokenArray[1], &endptr, 10);
	}
	if (tokens > 2) {
		*revision = strtoul(tokenArray[2], &endptr, 10);
	}
}

/**************************************************************************
  Registers a type to be treated as a callstack info 
 *************************************************************************/

void Extrae_register_codelocation_type_Wrapper (extrae_type_t type_function,
	extrae_type_t type_file_line, const char *description_function,
	const char *description_file_line)
{
	TRACE_MISCEVENT(LAST_READ_TIME,REGISTER_CODELOCATION_TYPE_EV, type_function,
		type_file_line);

	Extrae_AddTypeValuesEntryToLocalSYM ('C', type_function, (char*)description_function,
		(char)0,  0, NULL, NULL);
	Extrae_AddTypeValuesEntryToLocalSYM ('c', type_file_line, (char*)description_file_line,
		(char)0, 0, NULL, NULL);
}

void Extrae_register_function_address_Wrapper (void *ptr, const char *funcname, 
	const char *modname, unsigned line)
{
	Extrae_AddFunctionDefinitionEntryToLocalSYM ('O', ptr, (char*)funcname, (char*)modname, line);
}

void Extrae_define_event_type_Wrapper (extrae_type_t type, char *description,
	unsigned nvalues, extrae_value_t *values, char **description_values)
{
	Extrae_AddTypeValuesEntryToLocalSYM ('D', type, description, 'd', nvalues, values,
		description_values);
}

/**************************************************************************
 Lets change the number of active threads 
 *************************************************************************/
void Extrae_change_number_of_threads_Wrapper (unsigned nthreads)
{
	Backend_ChangeNumberOfThreads (nthreads);
}

/******************************************************************************
 Called through the API by the user, initiates the flush of the current thread 
 *****************************************************************************/
void Extrae_flush_manual_Wrapper (void)
{
  Flush_Thread( THREADID );
}

