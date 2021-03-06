/*************************************************************************/
/*  editor_vcs_interface.cpp                                             */
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

#include "editor_vcs_interface.h"
#include "core/method_bind.h"
#include "core/class_db.h"

EditorVCSInterface *EditorVCSInterface::singleton = nullptr;

IMPL_GDCLASS(EditorVCSInterface)

void EditorVCSInterface::_bind_methods() {

    // Proxy end points that act as fallbacks to unavailability of a function in the VCS addon
    MethodBinder::bind_method(D_METHOD("_initialize", {"project_root_path"}), &EditorVCSInterface::_initialize);
    MethodBinder::bind_method(D_METHOD("_is_vcs_initialized"), &EditorVCSInterface::_is_vcs_initialized);
    MethodBinder::bind_method(D_METHOD("_get_vcs_name"), &EditorVCSInterface::_get_vcs_name);
    MethodBinder::bind_method(D_METHOD("_shut_down"), &EditorVCSInterface::_shut_down);
    MethodBinder::bind_method(D_METHOD("_get_project_name"), &EditorVCSInterface::_get_project_name);
    MethodBinder::bind_method(D_METHOD("_get_modified_files_data"), &EditorVCSInterface::_get_modified_files_data);
    MethodBinder::bind_method(D_METHOD("_commit", {"msg"}), &EditorVCSInterface::_commit);
    MethodBinder::bind_method(D_METHOD("_get_file_diff", {"file_path"}), &EditorVCSInterface::_get_file_diff);
    MethodBinder::bind_method(D_METHOD("_stage_file", {"file_path"}), &EditorVCSInterface::_stage_file);
    MethodBinder::bind_method(D_METHOD("_unstage_file", {"file_path"}), &EditorVCSInterface::_unstage_file);

    MethodBinder::bind_method(D_METHOD("is_addon_ready"), &EditorVCSInterface::is_addon_ready);

    // API methods that redirect calls to the proxy end points
    MethodBinder::bind_method(D_METHOD("initialize", {"project_root_path"}), &EditorVCSInterface::initialize);
    MethodBinder::bind_method(D_METHOD("is_vcs_initialized"), &EditorVCSInterface::is_vcs_initialized);
    MethodBinder::bind_method(D_METHOD("get_modified_files_data"), &EditorVCSInterface::get_modified_files_data);
    MethodBinder::bind_method(D_METHOD("stage_file", {"file_path"}), &EditorVCSInterface::stage_file);
    MethodBinder::bind_method(D_METHOD("unstage_file", {"file_path"}), &EditorVCSInterface::unstage_file);
    MethodBinder::bind_method(D_METHOD("commit", {"msg"}), &EditorVCSInterface::commit);
    MethodBinder::bind_method(D_METHOD("get_file_diff", {"file_path"}), &EditorVCSInterface::get_file_diff);
    MethodBinder::bind_method(D_METHOD("shut_down"), &EditorVCSInterface::shut_down);
    MethodBinder::bind_method(D_METHOD("get_project_name"), &EditorVCSInterface::get_project_name);
    MethodBinder::bind_method(D_METHOD("get_vcs_name"), &EditorVCSInterface::get_vcs_name);
}

bool EditorVCSInterface::_initialize(se_string_view p_project_root_path) {

    WARN_PRINT("Selected VCS addon does not implement an initialization function. This warning will be suppressed.");
    return true;
}

bool EditorVCSInterface::_is_vcs_initialized() {

    return false;
}

Dictionary EditorVCSInterface::_get_modified_files_data() {

    return Dictionary();
}

void EditorVCSInterface::_stage_file(se_string_view p_file_path) {
}

void EditorVCSInterface::_unstage_file(se_string_view p_file_path) {
}

void EditorVCSInterface::_commit(se_string_view p_msg) {
}

Array EditorVCSInterface::_get_file_diff(se_string_view p_file_path) {

    return Array();
}

bool EditorVCSInterface::_shut_down() {

    return false;
}

String EditorVCSInterface::_get_project_name() {

    return String();
}

String EditorVCSInterface::_get_vcs_name() {

    return String();
}

bool EditorVCSInterface::initialize(se_string_view p_project_root_path) {

    is_initialized = call_va("_initialize", p_project_root_path);
    return is_initialized;
}

bool EditorVCSInterface::is_vcs_initialized() {

    return call_va("_is_vcs_initialized");
}

Dictionary EditorVCSInterface::get_modified_files_data() {

    return call_va("_get_modified_files_data");
}

void EditorVCSInterface::stage_file(se_string_view p_file_path) {

    if (is_addon_ready()) {

        call_va("_stage_file", p_file_path);
    }
}

void EditorVCSInterface::unstage_file(se_string_view p_file_path) {

    if (is_addon_ready()) {

        call_va("_unstage_file", p_file_path);
    }
}

bool EditorVCSInterface::is_addon_ready() {

    return is_initialized;
}

void EditorVCSInterface::commit(se_string_view p_msg) {

    if (is_addon_ready()) {

        call_va("_commit", p_msg);
    }
}

Array EditorVCSInterface::get_file_diff(se_string_view p_file_path) {

    if (is_addon_ready()) {

        return call_va("_get_file_diff", p_file_path);
    }
    return Array();
}

bool EditorVCSInterface::shut_down() {

    return call_va("_shut_down");
}

String EditorVCSInterface::get_project_name() {

    return call_va("_get_project_name");
}

String EditorVCSInterface::get_vcs_name() {

    return call_va("_get_vcs_name");
}

EditorVCSInterface::EditorVCSInterface() {

    is_initialized = false;
}

EditorVCSInterface::~EditorVCSInterface() {
}

EditorVCSInterface *EditorVCSInterface::get_singleton() {

    return singleton;
}

void EditorVCSInterface::set_singleton(EditorVCSInterface *p_singleton) {

    singleton = p_singleton;
}
