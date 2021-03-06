/*************************************************************************/
/*  thread_dummy.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "core/os/mutex.h"
#include "core/os/rw_lock.h"
#include "core/os/semaphore.h"
#include "core/os/thread.h"

class ThreadDummy : public Thread {

	static Thread *create(ThreadCreateCallback p_callback, void *p_user, const Settings &p_settings = Settings());

public:
    ID get_id() const override { return 0; }

    static void make_default();
};

class SemaphoreDummy : public SemaphoreOld {

	static SemaphoreOld *create();

public:
	Error wait() override { return OK; }
	Error post() override { return OK; }
	int get() const override { return 0; } ///< get semaphore value

	static void make_default();
};

class RWLockDummy : public RWLock {

	static RWLock *create();

public:
	void read_lock() override {}
	void read_unlock() override {}
	Error read_try_lock() override { return OK; }

	void write_lock() override {}
	void write_unlock() override {}
	Error write_try_lock() override { return OK; }

	static void make_default();
};

