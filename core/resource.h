/*************************************************************************/
/*  resource.h                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "core/reference.h"

namespace eastl {
template <typename Key, typename T, typename Compare, typename Allocator>
class map;
}
template <class T>
struct Comparator;
class wrap_allocator;

template <class K,class V>
using DefMap = eastl::map<K,V,Comparator<K>,wrap_allocator>;

#define RES_BASE_EXTENSION_IMPL(m_class,m_ext)                                                                      \
                                                                                                                    \
void m_class::register_custom_data_to_otdb() {                                                                      \
    ClassDB::add_resource_base_extension(StringName(m_ext), get_class_static_name());                               \
}

#define RES_BASE_EXTENSION(m_ext)                                                                                   \
public:                                                                                                             \
    StringName get_base_extension() const override { return StringName(m_ext); }                                    \
                                                                                                                    \
    static void register_custom_data_to_otdb();                                                                     \
private:

class GODOT_EXPORT Resource : public Reference {

    GDCLASS(Resource,Reference)
    OBJ_CATEGORY("Resources")
public:
    virtual StringName get_base_extension() const { return StringName("res"); }
    static void register_custom_data_to_otdb();
private:
    friend class ResBase;
    friend class ResourceCache;
    friend class SceneState;
    struct Data;
    Data *impl_data;

#ifdef TOOLS_ENABLED
    uint64_t last_modified_time;
    uint64_t import_last_modified_time;
#endif
    virtual bool _use_builtin_script() const { return true; }

protected:
    void emit_changed();

    void notify_change_to_owners();

    virtual void _resource_path_changed();
    static void _bind_methods();

    void _set_path(const String &p_path);
    void _take_over_path(const String &p_path);
public:
    static Node *(*_get_local_scene_func)(); //used by editor

    virtual bool editor_can_reload_from_file();
    virtual void reload_from_file();

    void register_owner(Object *p_owner);
    void unregister_owner(Object *p_owner);

    void set_name(const String &p_name);
    String get_name() const;

    virtual void set_path(const String &p_path, bool p_take_over = false);
    String get_path() const;

    void set_subindex(int p_sub_index);
    int get_subindex() const;

    virtual Ref<Resource> duplicate(bool p_subresources = false) const;
    Ref<Resource> duplicate_for_local_scene(Node *p_for_scene, DefMap<Ref<Resource>, Ref<Resource> > &remap_cache);
    void configure_for_local_scene(Node *p_for_scene, DefMap<Ref<Resource>, Ref<Resource> > &remap_cache);

    void set_local_to_scene(bool p_enable);
    bool is_local_to_scene() const;
    virtual void setup_local_to_scene();

    Node *get_local_scene() const;

#ifdef TOOLS_ENABLED

    uint32_t hash_edited_version() const;

    virtual void set_last_modified_time(uint64_t p_time) { last_modified_time = p_time; }
    uint64_t get_last_modified_time() const { return last_modified_time; }

    virtual void set_import_last_modified_time(uint64_t p_time) { import_last_modified_time = p_time; }
    uint64_t get_import_last_modified_time() const { return import_last_modified_time; }

    void set_import_path(const String &p_path);
    String get_import_path() const;

#endif

    void set_as_translation_remapped(bool p_remapped);
    bool is_translation_remapped() const;

    virtual RID get_rid() const; // some resources may offer conversion to RID

#ifdef TOOLS_ENABLED
    //helps keep IDs same number when loading/saving scenes. -1 clears ID and it Returns -1 when no id stored
    void set_id_for_path(const String &p_path, int p_id);
    int get_id_for_path(const String &p_path) const;
#endif

    Resource();
    ~Resource() override;
};

using RES = Ref<Resource>;

class ResourceCache {
    friend class Resource;
    friend class ResourceLoader; //need the lock
    static RWLock *lock;
    friend void unregister_core_types();
    static void clear();
    friend void register_core_types();
    static void setup();
    static Resource *get_unguarded(const String &p_path);
public:
    static void reload_externals();
    static bool has(const String &p_path);
    static Resource *get(const String &p_path);
    static void dump(const char *p_file = nullptr, bool p_short = false);
    static void get_cached_resources(DefList<Ref<Resource> > *p_resources);
    static int get_cached_resource_count();
};
