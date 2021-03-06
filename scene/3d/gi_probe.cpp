/*************************************************************************/
/*  gi_probe.cpp                                                         */
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

#include "gi_probe.h"

#include "core/os/os.h"

#include "mesh_instance.h"
#include "voxel_light_baker.h"
#include "core/method_bind.h"
#include "core/object_tooling.h"
#include "core/translation_helpers.h"
#include "scene/main/scene_tree.h"

IMPL_GDCLASS(GIProbeData)
IMPL_GDCLASS(GIProbe)
VARIANT_ENUM_CAST(GIProbe::Subdiv)

void GIProbeData::set_bounds(const AABB &p_bounds) {

    VisualServer::get_singleton()->gi_probe_set_bounds(probe, p_bounds);
}

AABB GIProbeData::get_bounds() const {

    return VisualServer::get_singleton()->gi_probe_get_bounds(probe);
}

void GIProbeData::set_cell_size(float p_size) {

    VisualServer::get_singleton()->gi_probe_set_cell_size(probe, p_size);
}

float GIProbeData::get_cell_size() const {

    return VisualServer::get_singleton()->gi_probe_get_cell_size(probe);
}

void GIProbeData::set_to_cell_xform(const Transform &p_xform) {

    VisualServer::get_singleton()->gi_probe_set_to_cell_xform(probe, p_xform);
}

Transform GIProbeData::get_to_cell_xform() const {

    return VisualServer::get_singleton()->gi_probe_get_to_cell_xform(probe);
}

void GIProbeData::set_dynamic_data(const PoolVector<int> &p_data) {

    VisualServer::get_singleton()->gi_probe_set_dynamic_data(probe, p_data);
}
PoolVector<int> GIProbeData::get_dynamic_data() const {

    return VisualServer::get_singleton()->gi_probe_get_dynamic_data(probe);
}

void GIProbeData::set_dynamic_range(int p_range) {

    VisualServer::get_singleton()->gi_probe_set_dynamic_range(probe, p_range);
}

void GIProbeData::set_energy(float p_range) {

    VisualServer::get_singleton()->gi_probe_set_energy(probe, p_range);
}

float GIProbeData::get_energy() const {

    return VisualServer::get_singleton()->gi_probe_get_energy(probe);
}

void GIProbeData::set_bias(float p_range) {

    VisualServer::get_singleton()->gi_probe_set_bias(probe, p_range);
}

float GIProbeData::get_bias() const {

    return VisualServer::get_singleton()->gi_probe_get_bias(probe);
}

void GIProbeData::set_normal_bias(float p_range) {

    VisualServer::get_singleton()->gi_probe_set_normal_bias(probe, p_range);
}

float GIProbeData::get_normal_bias() const {

    return VisualServer::get_singleton()->gi_probe_get_normal_bias(probe);
}

void GIProbeData::set_propagation(float p_range) {

    VisualServer::get_singleton()->gi_probe_set_propagation(probe, p_range);
}

float GIProbeData::get_propagation() const {

    return VisualServer::get_singleton()->gi_probe_get_propagation(probe);
}

void GIProbeData::set_interior(bool p_enable) {

    VisualServer::get_singleton()->gi_probe_set_interior(probe, p_enable);
}

bool GIProbeData::is_interior() const {

    return VisualServer::get_singleton()->gi_probe_is_interior(probe);
}

bool GIProbeData::is_compressed() const {

    return VisualServer::get_singleton()->gi_probe_is_compressed(probe);
}

void GIProbeData::set_compress(bool p_enable) {

    VisualServer::get_singleton()->gi_probe_set_compress(probe, p_enable);
}

int GIProbeData::get_dynamic_range() const {

    return VisualServer::get_singleton()->gi_probe_get_dynamic_range(probe);
}

RID GIProbeData::get_rid() const {

    return probe;
}

void GIProbeData::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_bounds", {"bounds"}), &GIProbeData::set_bounds);
    MethodBinder::bind_method(D_METHOD("get_bounds"), &GIProbeData::get_bounds);

    MethodBinder::bind_method(D_METHOD("set_cell_size", {"cell_size"}), &GIProbeData::set_cell_size);
    MethodBinder::bind_method(D_METHOD("get_cell_size"), &GIProbeData::get_cell_size);

    MethodBinder::bind_method(D_METHOD("set_to_cell_xform", {"to_cell_xform"}), &GIProbeData::set_to_cell_xform);
    MethodBinder::bind_method(D_METHOD("get_to_cell_xform"), &GIProbeData::get_to_cell_xform);

    MethodBinder::bind_method(D_METHOD("set_dynamic_data", {"dynamic_data"}), &GIProbeData::set_dynamic_data);
    MethodBinder::bind_method(D_METHOD("get_dynamic_data"), &GIProbeData::get_dynamic_data);

    MethodBinder::bind_method(D_METHOD("set_dynamic_range", {"dynamic_range"}), &GIProbeData::set_dynamic_range);
    MethodBinder::bind_method(D_METHOD("get_dynamic_range"), &GIProbeData::get_dynamic_range);

    MethodBinder::bind_method(D_METHOD("set_energy", {"energy"}), &GIProbeData::set_energy);
    MethodBinder::bind_method(D_METHOD("get_energy"), &GIProbeData::get_energy);

    MethodBinder::bind_method(D_METHOD("set_bias", {"bias"}), &GIProbeData::set_bias);
    MethodBinder::bind_method(D_METHOD("get_bias"), &GIProbeData::get_bias);

    MethodBinder::bind_method(D_METHOD("set_normal_bias", {"bias"}), &GIProbeData::set_normal_bias);
    MethodBinder::bind_method(D_METHOD("get_normal_bias"), &GIProbeData::get_normal_bias);

    MethodBinder::bind_method(D_METHOD("set_propagation", {"propagation"}), &GIProbeData::set_propagation);
    MethodBinder::bind_method(D_METHOD("get_propagation"), &GIProbeData::get_propagation);

    MethodBinder::bind_method(D_METHOD("set_interior", {"interior"}), &GIProbeData::set_interior);
    MethodBinder::bind_method(D_METHOD("is_interior"), &GIProbeData::is_interior);

    MethodBinder::bind_method(D_METHOD("set_compress", {"compress"}), &GIProbeData::set_compress);
    MethodBinder::bind_method(D_METHOD("is_compressed"), &GIProbeData::is_compressed);

    ADD_PROPERTY(PropertyInfo(VariantType::AABB, "bounds", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_bounds", "get_bounds");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "cell_size", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_cell_size", "get_cell_size");
    ADD_PROPERTY(PropertyInfo(VariantType::TRANSFORM, "to_cell_xform", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_to_cell_xform", "get_to_cell_xform");

    ADD_PROPERTY(PropertyInfo(VariantType::POOL_INT_ARRAY, "dynamic_data", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_dynamic_data", "get_dynamic_data");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "dynamic_range", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_dynamic_range", "get_dynamic_range");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "energy", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_energy", "get_energy");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "bias", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_bias", "get_bias");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "normal_bias", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_normal_bias", "get_normal_bias");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "propagation", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_propagation", "get_propagation");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "interior", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_interior", "is_interior");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "compress", PropertyHint::None, "", PROPERTY_USAGE_NOEDITOR), "set_compress", "is_compressed");
}

GIProbeData::GIProbeData() {

    probe = VisualServer::get_singleton()->gi_probe_create();
}

GIProbeData::~GIProbeData() {

    VisualServer::get_singleton()->free_rid(probe);
}

//////////////////////
//////////////////////

void GIProbe::set_probe_data(const Ref<GIProbeData> &p_data) {

    if (p_data) {
        VisualServer::get_singleton()->instance_set_base(get_instance(), p_data->get_rid());
    } else {
        VisualServer::get_singleton()->instance_set_base(get_instance(), RID());
    }

    probe_data = p_data;
}

Ref<GIProbeData> GIProbe::get_probe_data() const {

    return probe_data;
}

void GIProbe::set_subdiv(Subdiv p_subdiv) {

    ERR_FAIL_INDEX(p_subdiv, SUBDIV_MAX);
    subdiv = p_subdiv;
    update_gizmo();
}

GIProbe::Subdiv GIProbe::get_subdiv() const {

    return subdiv;
}

void GIProbe::set_extents(const Vector3 &p_extents) {

    extents = p_extents;
    update_gizmo();
    Object_change_notify(this,"extents");
}

Vector3 GIProbe::get_extents() const {

    return extents;
}

void GIProbe::set_dynamic_range(int p_dynamic_range) {

    dynamic_range = p_dynamic_range;
}
int GIProbe::get_dynamic_range() const {

    return dynamic_range;
}

void GIProbe::set_energy(float p_energy) {

    energy = p_energy;
    if (probe_data) {
        probe_data->set_energy(energy);
    }
}
float GIProbe::get_energy() const {

    return energy;
}

void GIProbe::set_bias(float p_bias) {

    bias = p_bias;
    if (probe_data) {
        probe_data->set_bias(bias);
    }
}
float GIProbe::get_bias() const {

    return bias;
}

void GIProbe::set_normal_bias(float p_normal_bias) {

    normal_bias = p_normal_bias;
    if (probe_data) {
        probe_data->set_normal_bias(normal_bias);
    }
}
float GIProbe::get_normal_bias() const {

    return normal_bias;
}

void GIProbe::set_propagation(float p_propagation) {

    propagation = p_propagation;
    if (probe_data) {
        probe_data->set_propagation(propagation);
    }
}
float GIProbe::get_propagation() const {

    return propagation;
}

void GIProbe::set_interior(bool p_enable) {

    interior = p_enable;
    if (probe_data) {
        probe_data->set_interior(p_enable);
    }
}

bool GIProbe::is_interior() const {

    return interior;
}

void GIProbe::set_compress(bool p_enable) {

    compress = p_enable;
    if (probe_data) {
        probe_data->set_compress(p_enable);
    }
}

bool GIProbe::is_compressed() const {

    return compress;
}

void GIProbe::_find_meshes(Node *p_at_node, Vector<GIProbe::PlotMesh> &plot_meshes) const {

    MeshInstance *mi = object_cast<MeshInstance>(p_at_node);
    if (mi && mi->get_flag(GeometryInstance::FLAG_USE_BAKED_LIGHT) && mi->is_visible_in_tree()) {
        Ref<Mesh> mesh = mi->get_mesh();
        if (mesh) {

            AABB aabb = mesh->get_aabb();

            Transform xf = get_global_transform().affine_inverse() * mi->get_global_transform();

            if (AABB(-extents, extents * 2).intersects(xf.xform(aabb))) {
                PlotMesh pm;
                pm.local_xform = xf;
                pm.mesh = mesh;
                for (int i = 0; i < mesh->get_surface_count(); i++) {
                    pm.instance_materials.push_back(mi->get_surface_material(i));
                }
                pm.override_material = mi->get_material_override();
                plot_meshes.push_back(pm);
            }
        }
    }

    Spatial *s = object_cast<Spatial>(p_at_node);
    if (s) {

        if (s->is_visible_in_tree()) {

            Array meshes = p_at_node->call_va("get_meshes");
            for (int i = 0; i < meshes.size(); i += 2) {

                Transform mxf = meshes[i];
                Ref<Mesh> mesh = refFromRefPtr<Mesh>(meshes[i + 1]);
                if (not mesh)
                    continue;

                AABB aabb = mesh->get_aabb();

                Transform xf = get_global_transform().affine_inverse() * (s->get_global_transform() * mxf);

                if (AABB(-extents, extents * 2).intersects(xf.xform(aabb))) {
                    PlotMesh pm;
                    pm.local_xform = xf;
                    pm.mesh = mesh;
                    plot_meshes.push_back(pm);
                }
            }
        }
    }

    for (int i = 0; i < p_at_node->get_child_count(); i++) {

        Node *child = p_at_node->get_child(i);
        _find_meshes(child, plot_meshes);
    }
}

GIProbe::BakeBeginFunc GIProbe::bake_begin_function = nullptr;
GIProbe::BakeStepFunc GIProbe::bake_step_function = nullptr;
GIProbe::BakeEndFunc GIProbe::bake_end_function = nullptr;

void GIProbe::bake(Node *p_from_node, bool p_create_visual_debug) {

    static const int subdiv_value[SUBDIV_MAX] = { 7, 8, 9, 10 };

    VoxelLightBaker baker;

    baker.begin_bake(subdiv_value[subdiv], AABB(-extents, extents * 2.0));

    Vector<PlotMesh> mesh_list;

    _find_meshes(p_from_node ? p_from_node : get_parent(), mesh_list);

    if (bake_begin_function) {
        bake_begin_function(mesh_list.size() + 1);
    }

    int pmc = 0;

    for (PlotMesh & E : mesh_list) {

        if (bake_step_function) {
            bake_step_function(pmc, RTR_utf8("Plotting Meshes") + " " + itos(pmc) + "/" + itos(mesh_list.size()));
        }

        pmc++;

        baker.plot_mesh(E.local_xform, E.mesh, E.instance_materials, E.override_material);
    }
    if (bake_step_function) {
        bake_step_function(pmc++, RTR_utf8("Finishing Plot"));
    }

    baker.end_bake();

    //create the data for visual server

    PoolVector<int> data = baker.create_gi_probe_data();

    if (p_create_visual_debug) {
        MultiMeshInstance *mmi = memnew(MultiMeshInstance);
        mmi->set_multimesh(baker.create_debug_multimesh());
        add_child(mmi);
#ifdef TOOLS_ENABLED
        if (get_tree()->get_edited_scene_root() == this) {
            mmi->set_owner(this);
        } else {
            mmi->set_owner(get_owner());
        }
#else
        mmi->set_owner(get_owner());
#endif

    } else {

        Ref<GIProbeData> probe_data = get_probe_data();

        if (not probe_data)
            probe_data = make_ref_counted<GIProbeData>();

        probe_data->set_bounds(AABB(-extents, extents * 2.0));
        probe_data->set_cell_size(baker.get_cell_size());
        probe_data->set_dynamic_data(data);
        probe_data->set_dynamic_range(dynamic_range);
        probe_data->set_energy(energy);
        probe_data->set_bias(bias);
        probe_data->set_normal_bias(normal_bias);
        probe_data->set_propagation(propagation);
        probe_data->set_interior(interior);
        probe_data->set_compress(compress);
        probe_data->set_to_cell_xform(baker.get_to_cell_space_xform());

        set_probe_data(probe_data);
    }

    if (bake_end_function) {
        bake_end_function();
    }
}

void GIProbe::_debug_bake() {

    bake(nullptr, true);
}

AABB GIProbe::get_aabb() const {

    return AABB(-extents, extents * 2);
}

Vector<Face3> GIProbe::get_faces(uint32_t p_usage_flags) const {

    return Vector<Face3>();
}

StringName GIProbe::get_configuration_warning() const {
    return StringName();
}

void GIProbe::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_probe_data", {"data"}), &GIProbe::set_probe_data);
    MethodBinder::bind_method(D_METHOD("get_probe_data"), &GIProbe::get_probe_data);

    MethodBinder::bind_method(D_METHOD("set_subdiv", {"subdiv"}), &GIProbe::set_subdiv);
    MethodBinder::bind_method(D_METHOD("get_subdiv"), &GIProbe::get_subdiv);

    MethodBinder::bind_method(D_METHOD("set_extents", {"extents"}), &GIProbe::set_extents);
    MethodBinder::bind_method(D_METHOD("get_extents"), &GIProbe::get_extents);

    MethodBinder::bind_method(D_METHOD("set_dynamic_range", {"max"}), &GIProbe::set_dynamic_range);
    MethodBinder::bind_method(D_METHOD("get_dynamic_range"), &GIProbe::get_dynamic_range);

    MethodBinder::bind_method(D_METHOD("set_energy", {"max"}), &GIProbe::set_energy);
    MethodBinder::bind_method(D_METHOD("get_energy"), &GIProbe::get_energy);

    MethodBinder::bind_method(D_METHOD("set_bias", {"max"}), &GIProbe::set_bias);
    MethodBinder::bind_method(D_METHOD("get_bias"), &GIProbe::get_bias);

    MethodBinder::bind_method(D_METHOD("set_normal_bias", {"max"}), &GIProbe::set_normal_bias);
    MethodBinder::bind_method(D_METHOD("get_normal_bias"), &GIProbe::get_normal_bias);

    MethodBinder::bind_method(D_METHOD("set_propagation", {"max"}), &GIProbe::set_propagation);
    MethodBinder::bind_method(D_METHOD("get_propagation"), &GIProbe::get_propagation);

    MethodBinder::bind_method(D_METHOD("set_interior", {"enable"}), &GIProbe::set_interior);
    MethodBinder::bind_method(D_METHOD("is_interior"), &GIProbe::is_interior);

    MethodBinder::bind_method(D_METHOD("set_compress", {"enable"}), &GIProbe::set_compress);
    MethodBinder::bind_method(D_METHOD("is_compressed"), &GIProbe::is_compressed);

    MethodBinder::bind_method(D_METHOD("bake", {"from_node", "create_visual_debug"}), &GIProbe::bake, {DEFVAL(Variant()), DEFVAL(false)});
    MethodBinder::bind_method(D_METHOD("debug_bake"), &GIProbe::_debug_bake);
    ClassDB::set_method_flags(get_class_static_name(), StringName("debug_bake"), METHOD_FLAGS_DEFAULT | METHOD_FLAG_EDITOR);

    ADD_PROPERTY(PropertyInfo(VariantType::INT, "subdiv", PropertyHint::Enum, "64,128,256,512"), "set_subdiv", "get_subdiv");
    ADD_PROPERTY(PropertyInfo(VariantType::VECTOR3, "extents"), "set_extents", "get_extents");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "dynamic_range", PropertyHint::Range, "1,16,1"), "set_dynamic_range", "get_dynamic_range");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "energy", PropertyHint::Range, "0,16,0.01,or_greater"), "set_energy", "get_energy");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "propagation", PropertyHint::Range, "0,1,0.01"), "set_propagation", "get_propagation");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "bias", PropertyHint::Range, "0,4,0.001"), "set_bias", "get_bias");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "normal_bias", PropertyHint::Range, "0,4,0.001"), "set_normal_bias", "get_normal_bias");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "interior"), "set_interior", "is_interior");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "compress"), "set_compress", "is_compressed");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "data", PropertyHint::ResourceType, "GIProbeData", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_DO_NOT_SHARE_ON_DUPLICATE), "set_probe_data", "get_probe_data");

    BIND_ENUM_CONSTANT(SUBDIV_64)
    BIND_ENUM_CONSTANT(SUBDIV_128)
    BIND_ENUM_CONSTANT(SUBDIV_256)
    BIND_ENUM_CONSTANT(SUBDIV_512)
    BIND_ENUM_CONSTANT(SUBDIV_MAX)
}

GIProbe::GIProbe() {

    subdiv = SUBDIV_128;
    dynamic_range = 4;
    energy = 1.0;
    bias = 1.5;
    normal_bias = 0.0;
    propagation = 0.7;
    extents = Vector3(10, 10, 10);
    interior = false;
    compress = false;

    gi_probe = VisualServer::get_singleton()->gi_probe_create();
    set_disable_scale(true);
}

GIProbe::~GIProbe() {
    VisualServer::get_singleton()->free_rid(gi_probe);
}
