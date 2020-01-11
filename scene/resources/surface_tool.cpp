/*************************************************************************/
/*  surface_tool.cpp                                                     */
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

#include "surface_tool.h"
#include "core/method_bind.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh_enum_casters.h"

constexpr float _VERTEX_SNAP = 0.0001f;
constexpr float EQ_VERTEX_DIST = 0.00001f;

IMPL_GDCLASS(SurfaceTool)
//TODO: SEGS: Mesh::PrimitiveType is used here

template<>
struct Hasher<SurfaceTool::Vertex> {
    uint32_t operator()(const SurfaceTool::Vertex &p_vtx) {
        uint32_t h = hash_djb2_buffer((const uint8_t *)&p_vtx.vertex, sizeof(real_t) * 3);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.normal, sizeof(real_t) * 3, h);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.binormal, sizeof(real_t) * 3, h);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.tangent, sizeof(real_t) * 3, h);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.uv, sizeof(real_t) * 2, h);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.uv2, sizeof(real_t) * 2, h);
        h = hash_djb2_buffer((const uint8_t *)&p_vtx.color, sizeof(real_t) * 4, h);
        h = hash_djb2_buffer((const uint8_t *)p_vtx.bones.data(), p_vtx.bones.size() * sizeof(int), h);
        h = hash_djb2_buffer((const uint8_t *)p_vtx.weights.data(), p_vtx.weights.size() * sizeof(float), h);
        return h;
    }
};

bool SurfaceTool::Vertex::operator==(const Vertex &p_vertex) const {

    if (vertex != p_vertex.vertex)
        return false;

    if (uv != p_vertex.uv)
        return false;

    if (uv2 != p_vertex.uv2)
        return false;

    if (normal != p_vertex.normal)
        return false;

    if (binormal != p_vertex.binormal)
        return false;

    if (color != p_vertex.color)
        return false;

    if (bones.size() != p_vertex.bones.size())
        return false;

    for (int i = 0; i < bones.size(); i++) {
        if (bones[i] != p_vertex.bones[i])
            return false;
    }

    for (int i = 0; i < weights.size(); i++) {
        if (weights[i] != p_vertex.weights[i])
            return false;
    }

    return true;
}

void SurfaceTool::begin(Mesh::PrimitiveType p_primitive) {

    clear();

    primitive = p_primitive;
    begun = true;
    first = true;
}

void SurfaceTool::add_vertex(const Vector3 &p_vertex) {

    ERR_FAIL_COND(!begun)

    Vertex vtx;
    vtx.vertex = p_vertex;
    vtx.color = last_color;
    vtx.normal = last_normal;
    vtx.uv = last_uv;
    vtx.uv2 = last_uv2;
    vtx.weights = last_weights;
    vtx.bones = last_bones;
    vtx.tangent = last_tangent.normal;
    vtx.binormal = last_normal.cross(last_tangent.normal).normalized() * last_tangent.d;

    const int expected_vertices = 4;

    if ((format & Mesh::ARRAY_FORMAT_WEIGHTS || format & Mesh::ARRAY_FORMAT_BONES) && (vtx.weights.size() != expected_vertices || vtx.bones.size() != expected_vertices)) {
        //ensure vertices are the expected amount
        ERR_FAIL_COND(vtx.weights.size() != vtx.bones.size())
        if (vtx.weights.size() < expected_vertices) {
            //less than required, fill
            for (int i = vtx.weights.size(); i < expected_vertices; i++) {
                vtx.weights.push_back(0);
                vtx.bones.push_back(0);
            }
        } else if (vtx.weights.size() > expected_vertices) {
            //more than required, sort, cap and normalize.
            Vector<WeightSort> weights;
            for (int i = 0; i < vtx.weights.size(); i++) {
                WeightSort ws;
                ws.index = vtx.bones[i];
                ws.weight = vtx.weights[i];
                weights.push_back(ws);
            }

            //sort
            weights.sort();
            //cap
            weights.resize(expected_vertices);
            //renormalize
            float total = 0;
            for (int i = 0; i < expected_vertices; i++) {
                total += weights[i].weight;
            }

            vtx.weights.resize(expected_vertices);
            vtx.bones.resize(expected_vertices);

            for (int i = 0; i < expected_vertices; i++) {
                if (total > 0) {
                    vtx.weights[i] = weights[i].weight / total;
                } else {
                    vtx.weights[i] = 0;
                }
                vtx.bones[i] = weights[i].index;
            }
        }
    }

    vertex_array.push_back(vtx);
    first = false;

    format |= Mesh::ARRAY_FORMAT_VERTEX;
}
void SurfaceTool::add_color(Color p_color) {

    ERR_FAIL_COND(!begun)

    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_COLOR))

    format |= Mesh::ARRAY_FORMAT_COLOR;
    last_color = p_color;
}
void SurfaceTool::add_normal(const Vector3 &p_normal) {

    ERR_FAIL_COND(!begun)

    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_NORMAL))

    format |= Mesh::ARRAY_FORMAT_NORMAL;
    last_normal = p_normal;
}

void SurfaceTool::add_tangent(const Plane &p_tangent) {

    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_TANGENT))

    format |= Mesh::ARRAY_FORMAT_TANGENT;
    last_tangent = p_tangent;
}

void SurfaceTool::add_uv(const Vector2 &p_uv) {

    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_TEX_UV))

    format |= Mesh::ARRAY_FORMAT_TEX_UV;
    last_uv = p_uv;
}

void SurfaceTool::add_uv2(const Vector2 &p_uv2) {

    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_TEX_UV2))

    format |= Mesh::ARRAY_FORMAT_TEX_UV2;
    last_uv2 = p_uv2;
}

void SurfaceTool::add_bones(Span<const int> p_bones) {

    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_BONES))

    format |= Mesh::ARRAY_FORMAT_BONES;
    last_bones.assign(p_bones.begin(),p_bones.end());
}

void SurfaceTool::add_weights(Span<const float> p_weights) {

    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(!first && !(format & Mesh::ARRAY_FORMAT_WEIGHTS))

    format |= Mesh::ARRAY_FORMAT_WEIGHTS;
    last_weights.assign(p_weights.begin(), p_weights.end());
}

void SurfaceTool::add_smooth_group(bool p_smooth) {

    ERR_FAIL_COND(!begun)
    if (!index_array.empty()) {
        smooth_groups[index_array.size()] = p_smooth;
    } else {

        smooth_groups[vertex_array.size()] = p_smooth;
    }
}

void SurfaceTool::add_triangle_fan(const Vector<Vector3> &p_vertices, const Vector<Vector2> &p_uvs, const Vector<Color> &p_colors, const Vector<Vector2> &p_uv2s, const Vector<Vector3> &p_normals, const PODVector<Plane> &p_tangents) {
    ERR_FAIL_COND(!begun)
    ERR_FAIL_COND(primitive != Mesh::PRIMITIVE_TRIANGLES)
    ERR_FAIL_COND(p_vertices.size() < 3)

#define ADD_POINT(n)                    \
    {                                   \
        if (p_colors.size() > n)        \
            add_color(p_colors[n]);     \
        if (p_uvs.size() > n)           \
            add_uv(p_uvs[n]);           \
        if (p_uv2s.size() > n)          \
            add_uv2(p_uv2s[n]);         \
        if (p_normals.size() > n)       \
            add_normal(p_normals[n]);   \
        if (p_tangents.size() > n)      \
            add_tangent(p_tangents[n]); \
        add_vertex(p_vertices[n]);      \
    }

    for (int i = 0; i < p_vertices.size() - 2; i++) {
        ADD_POINT(0);
        ADD_POINT(i + 1);
        ADD_POINT(i + 2);
    }

#undef ADD_POINT
}

void SurfaceTool::add_index(int p_index) {

    ERR_FAIL_COND(!begun)

    format |= Mesh::ARRAY_FORMAT_INDEX;
    index_array.push_back(p_index);
}

Array SurfaceTool::commit_to_arrays() {

    int varr_len = vertex_array.size();

    Array a;
    a.resize(Mesh::ARRAY_MAX);

    for (int i = 0; i < Mesh::ARRAY_MAX; i++) {

        if (!(format & (1 << i)))
            continue; //not in format

        switch (i) {

            case Mesh::ARRAY_VERTEX:
            case Mesh::ARRAY_NORMAL: {

                PoolVector<Vector3> array;
                array.resize(varr_len);
                PoolVector<Vector3>::Write w = array.write();

                int idx = 0;
                for (const Vertex &v : vertex_array) {
                    switch (i) {
                        case Mesh::ARRAY_VERTEX: {
                            w[idx] = v.vertex;
                        } break;
                        case Mesh::ARRAY_NORMAL: {
                            w[idx] = v.normal;
                        } break;
                    }
                    idx++;
                }

                w.release();
                a[i] = array;

            } break;

            case Mesh::ARRAY_TEX_UV:
            case Mesh::ARRAY_TEX_UV2: {

                PoolVector<Vector2> array;
                array.resize(varr_len);
                PoolVector<Vector2>::Write w = array.write();

                int idx = 0;
                for (const Vertex &v : vertex_array) {
                    switch (i) {

                        case Mesh::ARRAY_TEX_UV: {
                            w[idx] = v.uv;
                        } break;
                        case Mesh::ARRAY_TEX_UV2: {
                            w[idx] = v.uv2;
                        } break;
                    }
                    idx++;
                }

                w.release();
                a[i] = Variant(array);
            } break;
            case Mesh::ARRAY_TANGENT: {

                PoolVector<float> array;
                array.resize(varr_len * 4);
                PoolVector<float>::Write w = array.write();

                int idx = 0;
                for (const Vertex &v : vertex_array) {

                    w[idx + 0] = v.tangent.x;
                    w[idx + 1] = v.tangent.y;
                    w[idx + 2] = v.tangent.z;

                    //float d = v.tangent.dot(v.binormal,v.normal);
                    float d = v.binormal.dot(v.normal.cross(v.tangent));
                    w[idx + 3] = d < 0 ? -1 : 1;
                    idx += 4;
                }

                w.release();
                a[i] = array;

            } break;
            case Mesh::ARRAY_COLOR: {

                PoolVector<Color> array;
                array.resize(varr_len);
                PoolVector<Color>::Write w = array.write();

                int idx = 0;
                for (const Vertex &v : vertex_array) {
                    w[idx++] = v.color;
                }

                w.release();
                a[i] = array;
            } break;
            case Mesh::ARRAY_BONES: {

                PoolVector<int> array;
                array.resize(varr_len * 4);
                PoolVector<int>::Write w = array.write();

                int idx = -4;
                for (const Vertex &v : vertex_array) {

                    idx += 4;
                    ERR_CONTINUE(v.bones.size() != 4)

                    for (int j = 0; j < 4; j++) {
                        w[idx + j] = v.bones[j];
                    }
                }

                w.release();
                a[i] = array;

            } break;
            case Mesh::ARRAY_WEIGHTS: {

                PoolVector<float> array;
                array.resize(varr_len * 4);
                PoolVector<float>::Write w = array.write();

                int idx = -4;
                for (const Vertex &v : vertex_array) {
                    idx += 4;
                    ERR_CONTINUE(v.weights.size() != 4)

                    for (int j = 0; j < 4; j++) {

                        w[idx + j] = v.weights[j];
                    }
                }

                w.release();
                a[i] = array;

            } break;
            case Mesh::ARRAY_INDEX: {

                ERR_CONTINUE(index_array.empty())

                PoolVector<int> array;
                array.resize(index_array.size());
                PoolVector<int>::Write w = array.write();

                int idx = 0;
                for (int v : index_array) {

                    w[idx++] = v;
                }

                w.release();

                a[i] = array;
            } break;

            default: {
            }
        }
    }

    return a;
}

Ref<ArrayMesh> SurfaceTool::commit(const Ref<ArrayMesh> &p_existing, uint32_t p_flags) {

    Ref<ArrayMesh> mesh;
    if (p_existing)
        mesh = p_existing;
    else
        mesh = make_ref_counted<ArrayMesh>();

    int varr_len = vertex_array.size();

    if (varr_len == 0)
        return mesh;

    int surface = mesh->get_surface_count();

    Array a = commit_to_arrays();

    mesh->add_surface_from_arrays(primitive, a, Array(), p_flags);

    if (material)
        mesh->surface_set_material(surface, material);

    return mesh;
}

void SurfaceTool::index() {

    if (!index_array.empty())
        return; //already indexed

    HashMap<Vertex, int> indices;
    PODVector<Vertex> new_vertices;

    for (const Vertex &E : vertex_array) {

        int *idxptr = indices.getptr(E);
        int idx;
        if (!idxptr) {
            idx = indices.size();
            new_vertices.push_back(E);
            indices[E] = idx;
        } else {
            idx = *idxptr;
        }

        index_array.push_back(idx);
    }

    vertex_array.clear();
    vertex_array = new_vertices;

    format |= Mesh::ARRAY_FORMAT_INDEX;
}

void SurfaceTool::deindex() {

    if (index_array.empty())
        return; //nothing to deindex
    Vector<Vertex> varr;
    varr.resize(vertex_array.size());
    int idx = 0;
    for (const Vertex &E : vertex_array) {

        varr.write[idx++] = E;
    }
    vertex_array.clear();
    for (int E : index_array) {

        ERR_FAIL_INDEX(E, varr.size())
        vertex_array.push_back(varr[E]);
    }
    format &= ~Mesh::ARRAY_FORMAT_INDEX;
    index_array.clear();
}

void SurfaceTool::_create_list(const Ref<Mesh> &p_existing, int p_surface, PODVector<Vertex> *r_vertex, PODVector<int> *r_index, int &lformat) {

    Array arr = p_existing->surface_get_arrays(p_surface);
    ERR_FAIL_COND(arr.size() != VS::ARRAY_MAX)
    _create_list_from_arrays(arr, r_vertex, r_index, lformat);
}

Vector<SurfaceTool::Vertex> SurfaceTool::create_vertex_array_from_triangle_arrays(const Array &p_arrays) {

    Vector<SurfaceTool::Vertex> ret;

    PoolVector<Vector3> varr = p_arrays[VS::ARRAY_VERTEX];
    PoolVector<Vector3> narr = p_arrays[VS::ARRAY_NORMAL];
    PoolVector<float> tarr = p_arrays[VS::ARRAY_TANGENT];
    PoolVector<Color> carr = p_arrays[VS::ARRAY_COLOR];
    PoolVector<Vector2> uvarr = p_arrays[VS::ARRAY_TEX_UV];
    PoolVector<Vector2> uv2arr = p_arrays[VS::ARRAY_TEX_UV2];
    PoolVector<int> barr = p_arrays[VS::ARRAY_BONES];
    PoolVector<float> warr = p_arrays[VS::ARRAY_WEIGHTS];

    int vc = varr.size();

    if (vc == 0)
        return ret;
    int lformat = 0;

    PoolVector<Vector3>::Read rv;
    if (!varr.empty()) {
        lformat |= VS::ARRAY_FORMAT_VERTEX;
        rv = varr.read();
    }
    PoolVector<Vector3>::Read rn;
    if (!narr.empty()) {
        lformat |= VS::ARRAY_FORMAT_NORMAL;
        rn = narr.read();
    }
    PoolVector<float>::Read rt;
    if (!tarr.empty()) {
        lformat |= VS::ARRAY_FORMAT_TANGENT;
        rt = tarr.read();
    }
    PoolVector<Color>::Read rc;
    if (carr.size()) {
        lformat |= VS::ARRAY_FORMAT_COLOR;
        rc = carr.read();
    }

    PoolVector<Vector2>::Read ruv;
    if (!uvarr.empty()) {
        lformat |= VS::ARRAY_FORMAT_TEX_UV;
        ruv = uvarr.read();
    }

    PoolVector<Vector2>::Read ruv2;
    if (uv2arr.size()) {
        lformat |= VS::ARRAY_FORMAT_TEX_UV2;
        ruv2 = uv2arr.read();
    }

    PoolVector<int>::Read rb;
    if (barr.size()) {
        lformat |= VS::ARRAY_FORMAT_BONES;
        rb = barr.read();
    }

    PoolVector<float>::Read rw;
    if (warr.size()) {
        lformat |= VS::ARRAY_FORMAT_WEIGHTS;
        rw = warr.read();
    }
    ret.resize(vc);
    Vertex *tgt = ret.ptrw();
    if (lformat & VS::ARRAY_FORMAT_VERTEX)
        for (int i = 0; i < vc; i++) {
            tgt[i].vertex = varr[i];
        }
    if (lformat & VS::ARRAY_FORMAT_NORMAL)
        for (int i = 0; i < vc; i++) {
            tgt[i].normal = narr[i];
        }
    if (lformat & VS::ARRAY_FORMAT_TANGENT) {
        for (int i = 0; i < vc; i++) {
            Plane p(tarr[i * 4 + 0], tarr[i * 4 + 1], tarr[i * 4 + 2], tarr[i * 4 + 3]);
            tgt[i].tangent = p.normal;
            tgt[i].binormal = p.normal.cross(p.normal).normalized() * p.d;
        }
    }
    if (lformat & VS::ARRAY_FORMAT_COLOR)
        for (int i = 0; i < vc; i++) {
            tgt[i].color = carr[i];
        }
    if (lformat & VS::ARRAY_FORMAT_TEX_UV)
        for (int i = 0; i < vc; i++) {
            tgt[i].uv = uvarr[i];
        }
    if (lformat & VS::ARRAY_FORMAT_TEX_UV2)
        for (int i = 0; i < vc; i++) {
            tgt[i].uv2 = uv2arr[i];
        }

    if (lformat & VS::ARRAY_FORMAT_BONES) {
        for (int i = 0; i < vc; i++) {
            int b[4] = {
                barr[i * 4 + 0],
                barr[i * 4 + 1],
                barr[i * 4 + 2],
                barr[i * 4 + 3]
            };
            tgt[i].bones.assign(eastl::begin(b), eastl::end(b));
        }
    }
    if (lformat & VS::ARRAY_FORMAT_WEIGHTS) {
        for (int i = 0; i < vc; i++) {
            float w[4]{
                warr[i * 4 + 0],
                warr[i * 4 + 1],
                warr[i * 4 + 2],
                warr[i * 4 + 3],
            };
            tgt[i].weights.assign(eastl::begin(w), eastl::end(w));
        }
    }
    return ret;
}

void SurfaceTool::_create_list_from_arrays(Array arr, PODVector<Vertex> *r_vertex, PODVector<int> *r_index, int &lformat) {

    PoolVector<Vector3> varr = arr[VS::ARRAY_VERTEX];
    PoolVector<Vector3> narr = arr[VS::ARRAY_NORMAL];
    PoolVector<float> tarr = arr[VS::ARRAY_TANGENT];
    PoolVector<Color> carr = arr[VS::ARRAY_COLOR];
    PoolVector<Vector2> uvarr = arr[VS::ARRAY_TEX_UV];
    PoolVector<Vector2> uv2arr = arr[VS::ARRAY_TEX_UV2];
    PoolVector<int> barr = arr[VS::ARRAY_BONES];
    PoolVector<float> warr = arr[VS::ARRAY_WEIGHTS];

    int vc = varr.size();

    if (vc == 0)
        return;
    lformat = 0;

    PoolVector<Vector3>::Read rv;
    if (varr.size()) {
        lformat |= VS::ARRAY_FORMAT_VERTEX;
        rv = varr.read();
    }
    PoolVector<Vector3>::Read rn;
    if (narr.size()) {
        lformat |= VS::ARRAY_FORMAT_NORMAL;
        rn = narr.read();
    }
    PoolVector<float>::Read rt;
    if (tarr.size()) {
        lformat |= VS::ARRAY_FORMAT_TANGENT;
        rt = tarr.read();
    }
    PoolVector<Color>::Read rc;
    if (carr.size()) {
        lformat |= VS::ARRAY_FORMAT_COLOR;
        rc = carr.read();
    }

    PoolVector<Vector2>::Read ruv;
    if (uvarr.size()) {
        lformat |= VS::ARRAY_FORMAT_TEX_UV;
        ruv = uvarr.read();
    }

    PoolVector<Vector2>::Read ruv2;
    if (uv2arr.size()) {
        lformat |= VS::ARRAY_FORMAT_TEX_UV2;
        ruv2 = uv2arr.read();
    }

    PoolVector<int>::Read rb;
    if (barr.size()) {
        lformat |= VS::ARRAY_FORMAT_BONES;
        rb = barr.read();
    }

    PoolVector<float>::Read rw;
    if (warr.size()) {
        lformat |= VS::ARRAY_FORMAT_WEIGHTS;
        rw = warr.read();
    }

    for (int i = 0; i < vc; i++) {

        Vertex v;
        if (lformat & VS::ARRAY_FORMAT_VERTEX)
            v.vertex = varr[i];
        if (lformat & VS::ARRAY_FORMAT_NORMAL)
            v.normal = narr[i];
        if (lformat & VS::ARRAY_FORMAT_TANGENT) {
            Plane p(tarr[i * 4 + 0], tarr[i * 4 + 1], tarr[i * 4 + 2], tarr[i * 4 + 3]);
            v.tangent = p.normal;
            v.binormal = p.normal.cross(v.tangent).normalized() * p.d;
        }
        if (lformat & VS::ARRAY_FORMAT_COLOR)
            v.color = carr[i];
        if (lformat & VS::ARRAY_FORMAT_TEX_UV)
            v.uv = uvarr[i];
        if (lformat & VS::ARRAY_FORMAT_TEX_UV2)
            v.uv2 = uv2arr[i];
        if (lformat & VS::ARRAY_FORMAT_BONES) {
            int b[4] = {
                barr[i * 4 + 0],
                barr[i * 4 + 1],
                barr[i * 4 + 2],
                barr[i * 4 + 3]
            };
            v.bones.assign(eastl::begin(b),eastl::end(b));
        }
        if (lformat & VS::ARRAY_FORMAT_WEIGHTS) {
            float w[4] {
                warr[i * 4 + 0],
                warr[i * 4 + 1],
                warr[i * 4 + 2],
                warr[i * 4 + 3],
            };
            v.weights.assign(eastl::begin(w),eastl::end(w));
        }

        r_vertex->push_back(v);
    }

    //indices

    PoolVector<int> idx = arr[VS::ARRAY_INDEX];
    int is = idx.size();
    if (is) {

        lformat |= VS::ARRAY_FORMAT_INDEX;
        PoolVector<int>::Read iarr = idx.read();
        for (int i = 0; i < is; i++) {
            r_index->push_back(iarr[i]);
        }
    }
}

void SurfaceTool::create_from_triangle_arrays(const Array &p_arrays) {

    clear();
    primitive = Mesh::PRIMITIVE_TRIANGLES;
    _create_list_from_arrays(p_arrays, &vertex_array, &index_array, format);
}

void SurfaceTool::create_from(const Ref<Mesh> &p_existing, int p_surface) {

    clear();
    primitive = p_existing->surface_get_primitive_type(p_surface);
    _create_list(p_existing, p_surface, &vertex_array, &index_array, format);
    material = p_existing->surface_get_material(p_surface);
}

void SurfaceTool::create_from_blend_shape(const Ref<Mesh> &p_existing, int p_surface, StringName p_blend_shape_name) {
    clear();
    primitive = p_existing->surface_get_primitive_type(p_surface);
    Array arr = p_existing->surface_get_blend_shape_arrays(p_surface);
    Array blend_shape_names;
    int32_t shape_idx = -1;
    for (int32_t i = 0; i < p_existing->get_blend_shape_count(); i++) {
        StringName name = p_existing->get_blend_shape_name(i);
        if (name == p_blend_shape_name) {
            shape_idx = i;
            break;
        }
    }
    ERR_FAIL_COND(shape_idx == -1)
    ERR_FAIL_COND(shape_idx >= arr.size())
    Array mesh = arr[shape_idx];
    ERR_FAIL_COND(mesh.size() != VS::ARRAY_MAX)
    _create_list_from_arrays(arr[shape_idx], &vertex_array, &index_array, format);
}

void SurfaceTool::append_from(const Ref<Mesh> &p_existing, int p_surface, const Transform &p_xform) {

    if (vertex_array.empty()) {
        primitive = p_existing->surface_get_primitive_type(p_surface);
        format = 0;
    }

    int nformat;
    PODVector<Vertex> nvertices;
    PODVector<int> nindices;
    _create_list(p_existing, p_surface, &nvertices, &nindices, nformat);
    format |= nformat;
    int vfrom = vertex_array.size();

    for (Vertex v : nvertices) {

        v.vertex = p_xform.xform(v.vertex);
        if (nformat & VS::ARRAY_FORMAT_NORMAL) {
            v.normal = p_xform.basis.xform(v.normal);
        }
        if (nformat & VS::ARRAY_FORMAT_TANGENT) {
            v.tangent = p_xform.basis.xform(v.tangent);
            v.binormal = p_xform.basis.xform(v.binormal);
        }

        vertex_array.push_back(v);
    }

    for (int E : nindices) {

        int dst_index = E + vfrom;
        index_array.push_back(dst_index);
    }
    if (index_array.size() % 3) {
        WARN_PRINT("SurfaceTool: Index array not a multiple of 3.")
    }
}

//mikktspace callbacks
namespace {
struct TangentGenerationContextUserData {
    PODVector<SurfaceTool::Vertex> &vertices;
    PODVector<int> &indices;
};
} // namespace

int SurfaceTool::mikktGetNumFaces(const SMikkTSpaceContext *pContext) {

    TangentGenerationContextUserData &triangle_data = *reinterpret_cast<TangentGenerationContextUserData *>(pContext->m_pUserData);

    if (!triangle_data.indices.empty()) {
        return triangle_data.indices.size() / 3;
    } else {
        return triangle_data.vertices.size() / 3;
    }
}
int SurfaceTool::mikktGetNumVerticesOfFace(const SMikkTSpaceContext *pContext, const int iFace) {

    return 3; //always 3
}
void SurfaceTool::mikktGetPosition(const SMikkTSpaceContext *pContext, float fvPosOut[], const int iFace, const int iVert) {

    TangentGenerationContextUserData &triangle_data = *reinterpret_cast<TangentGenerationContextUserData *>(pContext->m_pUserData);
    Vector3 v;
    if (!triangle_data.indices.empty()) {
        size_t index = triangle_data.indices[iFace * 3 + iVert];
        if (index < triangle_data.vertices.size()) {
            v = triangle_data.vertices[index].vertex;
        }
    } else {
        v = triangle_data.vertices[iFace * 3 + iVert].vertex;
    }

    fvPosOut[0] = v.x;
    fvPosOut[1] = v.y;
    fvPosOut[2] = v.z;
}

void SurfaceTool::mikktGetNormal(const SMikkTSpaceContext *pContext, float fvNormOut[], const int iFace, const int iVert) {

    TangentGenerationContextUserData &triangle_data = *reinterpret_cast<TangentGenerationContextUserData *>(pContext->m_pUserData);
    Vector3 v;
    if (!triangle_data.indices.empty()) {
        int index = triangle_data.indices[iFace * 3 + iVert];
        if (index < triangle_data.vertices.size()) {
            v = triangle_data.vertices[index].normal;
        }
    } else {
        v = triangle_data.vertices[iFace * 3 + iVert].normal;
    }

    fvNormOut[0] = v.x;
    fvNormOut[1] = v.y;
    fvNormOut[2] = v.z;
}
void SurfaceTool::mikktGetTexCoord(const SMikkTSpaceContext *pContext, float fvTexcOut[], const int iFace, const int iVert) {

    TangentGenerationContextUserData &triangle_data = *reinterpret_cast<TangentGenerationContextUserData *>(pContext->m_pUserData);
    Vector2 v;
    if (!triangle_data.indices.empty()) {
        int index = triangle_data.indices[iFace * 3 + iVert];
        if (index < triangle_data.vertices.size()) {
            v = triangle_data.vertices[index].uv;
        }
    } else {
        v = triangle_data.vertices[iFace * 3 + iVert].uv;
    }

    fvTexcOut[0] = v.x;
    fvTexcOut[1] = v.y;
}

void SurfaceTool::mikktSetTSpaceDefault(const SMikkTSpaceContext *pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
        const tbool bIsOrientationPreserving, const int iFace, const int iVert) {

    TangentGenerationContextUserData &triangle_data = *reinterpret_cast<TangentGenerationContextUserData *>(pContext->m_pUserData);
    Vertex *vtx = nullptr;
    if (!triangle_data.indices.empty()) {
        int index = triangle_data.indices[iFace * 3 + iVert];
        if (index < triangle_data.vertices.size()) {
            vtx = &triangle_data.vertices[index];
        }
    } else {
        vtx = &triangle_data.vertices[iFace * 3 + iVert];
    }

    if (vtx != nullptr) {
        vtx->tangent = Vector3(fvTangent[0], fvTangent[1], fvTangent[2]);
        vtx->binormal = Vector3(-fvBiTangent[0], -fvBiTangent[1], -fvBiTangent[2]); // for some reason these are reversed, something with the coordinate system in Godot
    }
}

void SurfaceTool::generate_tangents() {

    ERR_FAIL_COND(!(format & Mesh::ARRAY_FORMAT_TEX_UV))
    ERR_FAIL_COND(!(format & Mesh::ARRAY_FORMAT_NORMAL))

    SMikkTSpaceInterface mkif;
    mkif.m_getNormal = mikktGetNormal;
    mkif.m_getNumFaces = mikktGetNumFaces;
    mkif.m_getNumVerticesOfFace = mikktGetNumVerticesOfFace;
    mkif.m_getPosition = mikktGetPosition;
    mkif.m_getTexCoord = mikktGetTexCoord;
    mkif.m_setTSpace = mikktSetTSpaceDefault;
    mkif.m_setTSpaceBasic = nullptr;

    SMikkTSpaceContext msc;
    msc.m_pInterface = &mkif;

    TangentGenerationContextUserData triangle_data {vertex_array,index_array};
    for (Vertex &E : vertex_array) {
        E.binormal = Vector3();
        E.tangent = Vector3();
    }
    msc.m_pUserData = &triangle_data;

    bool res = genTangSpaceDefault(&msc);

    ERR_FAIL_COND(!res)
    format |= Mesh::ARRAY_FORMAT_TANGENT;
}

void SurfaceTool::generate_normals(bool p_flip) {

    ERR_FAIL_COND(primitive != Mesh::PRIMITIVE_TRIANGLES)

    bool was_indexed = !index_array.empty();

    deindex();

    HashMap<Vertex, Vector3> vertex_hash;

    int count = 0;
    bool smooth = false;
    if (smooth_groups.contains(0))
        smooth = smooth_groups[0];

    PODVector<Vertex>::iterator B = vertex_array.begin();
    for (PODVector<Vertex>::iterator E = B; E!=vertex_array.end();) {

        PODVector<Vertex>::iterator v[3];
        v[0] = E++;
        v[1] = E++;
        ERR_FAIL_COND(v[1]==vertex_array.end())
        v[2] = E++;
        ERR_FAIL_COND(v[2]==vertex_array.end())

        Vector3 normal;
        if (!p_flip)
            normal = Plane(v[0]->vertex, v[1]->vertex, v[2]->vertex).normal;
        else
            normal = Plane(v[2]->vertex, v[1]->vertex, v[0]->vertex).normal;

        if (smooth) {

            for (int i = 0; i < 3; i++) {

                Vector3 *lv = vertex_hash.getptr(*v[i]);
                if (!lv) {
                    vertex_hash.set(*v[i], normal);
                } else {
                    (*lv) += normal;
                }
            }
        } else {

            for (int i = 0; i < 3; i++) {

                v[i]->normal = normal;
            }
        }
        count += 3;

        if (smooth_groups.contains(count) || !E) {

            if (!vertex_hash.empty()) {

                while (B != E) {

                    Vector3 *lv = vertex_hash.getptr(*B);
                    if (lv) {
                        B->normal = lv->normalized();
                    }

                    ++B;
                }

            } else {
                B = E;
            }

            vertex_hash.clear();
            if (E) {
                smooth = smooth_groups[count];
            }
        }
    }

    format |= Mesh::ARRAY_FORMAT_NORMAL;

    if (was_indexed) {
        index();
        smooth_groups.clear();
    }
}

void SurfaceTool::set_material(const Ref<Material> &p_material) {

    material = p_material;
}

void SurfaceTool::clear() {

    begun = false;
    primitive = Mesh::PRIMITIVE_LINES;
    format = 0;
    last_bones.clear();
    last_weights.clear();
    index_array.clear();
    vertex_array.clear();
    smooth_groups.clear();
    material.unref();
}

void SurfaceTool::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("begin", {"primitive"}), &SurfaceTool::begin);

    MethodBinder::bind_method(D_METHOD("add_vertex", {"vertex"}), &SurfaceTool::add_vertex);
    MethodBinder::bind_method(D_METHOD("add_color", {"color"}), &SurfaceTool::add_color);
    MethodBinder::bind_method(D_METHOD("add_normal", {"normal"}), &SurfaceTool::add_normal);
    MethodBinder::bind_method(D_METHOD("add_tangent", {"tangent"}), &SurfaceTool::add_tangent);
    MethodBinder::bind_method(D_METHOD("add_uv", {"uv"}), &SurfaceTool::add_uv);
    MethodBinder::bind_method(D_METHOD("add_uv2", {"uv2"}), &SurfaceTool::add_uv2);
    MethodBinder::bind_method(D_METHOD("add_bones", {"bones"}), &SurfaceTool::add_bones);
    MethodBinder::bind_method(D_METHOD("add_weights", {"weights"}), &SurfaceTool::add_weights);
    MethodBinder::bind_method(D_METHOD("add_smooth_group", {"smooth"}), &SurfaceTool::add_smooth_group);

    MethodBinder::bind_method(D_METHOD("add_triangle_fan", {"vertices", "uvs", "colors", "uv2s", "normals", "tangents"}), &SurfaceTool::add_triangle_fan, {DEFVAL(PODVector<Vector2>()), DEFVAL(PODVector<Color>()), DEFVAL(PODVector<Vector2>()), DEFVAL(PODVector<Vector3>()), DEFVAL(PODVector<Plane>())});

    MethodBinder::bind_method(D_METHOD("add_index", {"index"}), &SurfaceTool::add_index);

    MethodBinder::bind_method(D_METHOD("index"), &SurfaceTool::index);
    MethodBinder::bind_method(D_METHOD("deindex"), &SurfaceTool::deindex);
    MethodBinder::bind_method(D_METHOD("generate_normals", {"flip"}), &SurfaceTool::generate_normals, {DEFVAL(false)});
    MethodBinder::bind_method(D_METHOD("generate_tangents"), &SurfaceTool::generate_tangents);

    MethodBinder::bind_method(D_METHOD("set_material", {"material"}), &SurfaceTool::set_material);

    MethodBinder::bind_method(D_METHOD("clear"), &SurfaceTool::clear);

    MethodBinder::bind_method(D_METHOD("create_from", {"existing", "surface"}), &SurfaceTool::create_from);
    MethodBinder::bind_method(D_METHOD("create_from_blend_shape", {"existing", "surface", "blend_shape"}), &SurfaceTool::create_from_blend_shape);
    MethodBinder::bind_method(D_METHOD("append_from", {"existing", "surface", "transform"}), &SurfaceTool::append_from);
    MethodBinder::bind_method(D_METHOD("commit", {"existing", "flags"}), &SurfaceTool::commit, {DEFVAL(Variant()), DEFVAL(Mesh::ARRAY_COMPRESS_DEFAULT)});
    MethodBinder::bind_method(D_METHOD("commit_to_arrays"), &SurfaceTool::commit_to_arrays);
}

SurfaceTool::SurfaceTool() {

    first = false;
    begun = false;
    primitive = Mesh::PRIMITIVE_LINES;
    format = 0;
}
