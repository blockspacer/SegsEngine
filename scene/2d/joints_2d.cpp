/*************************************************************************/
/*  joints_2d.cpp                                                        */
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

#include "joints_2d.h"

#include "core/engine.h"
#include "physics_body_2d.h"
#include "servers/physics_2d_server.h"
#include "core/method_bind.h"

IMPL_GDCLASS(Joint2D)
IMPL_GDCLASS(PinJoint2D)
IMPL_GDCLASS(GrooveJoint2D)
IMPL_GDCLASS(DampedSpringJoint2D)

void Joint2D::_update_joint(bool p_only_free) {

    if (joint.is_valid()) {
        if (ba.is_valid() && bb.is_valid())
            Physics2DServer::get_singleton()->body_remove_collision_exception(ba, bb);

        Physics2DServer::get_singleton()->free(joint);
        joint = RID();
        ba = RID();
        bb = RID();
    }

    if (p_only_free || !is_inside_tree())
        return;

    Node *node_a = has_node(get_node_a()) ? get_node(get_node_a()) : (Node *)nullptr;
    Node *node_b = has_node(get_node_b()) ? get_node(get_node_b()) : (Node *)nullptr;

    if (!node_a || !node_b)
        return;

    PhysicsBody2D *body_a = Object::cast_to<PhysicsBody2D>(node_a);
    PhysicsBody2D *body_b = Object::cast_to<PhysicsBody2D>(node_b);

    if (!body_a || !body_b)
        return;

    SWAP(body_a, body_b);

    joint = _configure_joint(body_a, body_b);

    if (!joint.is_valid())
        return;

    Physics2DServer::get_singleton()->get_singleton()->joint_set_param(joint, Physics2DServer::JOINT_PARAM_BIAS, bias);

    ba = body_a->get_rid();
    bb = body_b->get_rid();

    Physics2DServer::get_singleton()->joint_disable_collisions_between_bodies(joint, exclude_from_collision);
}

void Joint2D::set_node_a(const NodePath &p_node_a) {

    if (a == p_node_a)
        return;

    a = p_node_a;
    _update_joint();
}

NodePath Joint2D::get_node_a() const {

    return a;
}

void Joint2D::set_node_b(const NodePath &p_node_b) {

    if (b == p_node_b)
        return;
    b = p_node_b;
    _update_joint();
}
NodePath Joint2D::get_node_b() const {

    return b;
}

void Joint2D::_notification(int p_what) {

    switch (p_what) {

        case NOTIFICATION_READY: {
            _update_joint();
        } break;
        case NOTIFICATION_EXIT_TREE: {
            if (joint.is_valid()) {
                _update_joint(true);
            }
        } break;
    }
}

void Joint2D::set_bias(real_t p_bias) {

    bias = p_bias;
    if (joint.is_valid())
        Physics2DServer::get_singleton()->get_singleton()->joint_set_param(joint, Physics2DServer::JOINT_PARAM_BIAS, bias);
}

real_t Joint2D::get_bias() const {

    return bias;
}

void Joint2D::set_exclude_nodes_from_collision(bool p_enable) {

    if (exclude_from_collision == p_enable)
        return;
    exclude_from_collision = p_enable;
    _update_joint();
}

bool Joint2D::get_exclude_nodes_from_collision() const {

    return exclude_from_collision;
}

void Joint2D::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_node_a", {"node"}), &Joint2D::set_node_a);
    MethodBinder::bind_method(D_METHOD("get_node_a"), &Joint2D::get_node_a);

    MethodBinder::bind_method(D_METHOD("set_node_b", {"node"}), &Joint2D::set_node_b);
    MethodBinder::bind_method(D_METHOD("get_node_b"), &Joint2D::get_node_b);

    MethodBinder::bind_method(D_METHOD("set_bias", {"bias"}), &Joint2D::set_bias);
    MethodBinder::bind_method(D_METHOD("get_bias"), &Joint2D::get_bias);

    MethodBinder::bind_method(D_METHOD("set_exclude_nodes_from_collision", {"enable"}), &Joint2D::set_exclude_nodes_from_collision);
    MethodBinder::bind_method(D_METHOD("get_exclude_nodes_from_collision"), &Joint2D::get_exclude_nodes_from_collision);

    ADD_PROPERTY(PropertyInfo(VariantType::NODE_PATH, "node_a", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "CollisionObject2D"), "set_node_a", "get_node_a");
    ADD_PROPERTY(PropertyInfo(VariantType::NODE_PATH, "node_b", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "CollisionObject2D"), "set_node_b", "get_node_b");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "bias", PROPERTY_HINT_RANGE, "0,0.9,0.001"), "set_bias", "get_bias");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "disable_collision"), "set_exclude_nodes_from_collision", "get_exclude_nodes_from_collision");
}

Joint2D::Joint2D() {

    bias = 0;
    exclude_from_collision = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PinJoint2D::_notification(int p_what) {

    switch (p_what) {
        case NOTIFICATION_DRAW: {

            if (!is_inside_tree())
                break;

            if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
                break;
            }

            draw_line(Point2(-10, 0), Point2(+10, 0), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(0, -10), Point2(0, +10), Color(0.7, 0.6, 0.0, 0.5), 3);
        } break;
    }
}

RID PinJoint2D::_configure_joint(PhysicsBody2D *body_a, PhysicsBody2D *body_b) {

    RID pj = Physics2DServer::get_singleton()->pin_joint_create(get_global_transform().get_origin(), body_a->get_rid(), body_b ? body_b->get_rid() : RID());
    Physics2DServer::get_singleton()->pin_joint_set_param(pj, Physics2DServer::PIN_JOINT_SOFTNESS, softness);
    return pj;
}

void PinJoint2D::set_softness(real_t p_softness) {

    softness = p_softness;
    update();
    if (get_joint().is_valid())
        Physics2DServer::get_singleton()->pin_joint_set_param(get_joint(), Physics2DServer::PIN_JOINT_SOFTNESS, p_softness);
}

real_t PinJoint2D::get_softness() const {

    return softness;
}

void PinJoint2D::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_softness", {"softness"}), &PinJoint2D::set_softness);
    MethodBinder::bind_method(D_METHOD("get_softness"), &PinJoint2D::get_softness);

    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "softness", PROPERTY_HINT_EXP_RANGE, "0.00,16,0.01"), "set_softness", "get_softness");
}

PinJoint2D::PinJoint2D() {

    softness = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void GrooveJoint2D::_notification(int p_what) {

    switch (p_what) {
        case NOTIFICATION_DRAW: {
            if (!is_inside_tree())
                break;

            if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
                break;
            }

            draw_line(Point2(-10, 0), Point2(+10, 0), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(-10, length), Point2(+10, length), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(0, 0), Point2(0, length), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(-10, initial_offset), Point2(+10, initial_offset), Color(0.8, 0.8, 0.9, 0.5), 5);
        } break;
    }
}

RID GrooveJoint2D::_configure_joint(PhysicsBody2D *body_a, PhysicsBody2D *body_b) {

    Transform2D gt = get_global_transform();
    Vector2 groove_A1 = gt.get_origin();
    Vector2 groove_A2 = gt.xform(Vector2(0, length));
    Vector2 anchor_B = gt.xform(Vector2(0, initial_offset));

    return Physics2DServer::get_singleton()->groove_joint_create(groove_A1, groove_A2, anchor_B, body_a->get_rid(), body_b->get_rid());
}

void GrooveJoint2D::set_length(real_t p_length) {

    length = p_length;
    update();
}

real_t GrooveJoint2D::get_length() const {

    return length;
}

void GrooveJoint2D::set_initial_offset(real_t p_initial_offset) {

    initial_offset = p_initial_offset;
    update();
}

real_t GrooveJoint2D::get_initial_offset() const {

    return initial_offset;
}

void GrooveJoint2D::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_length", {"length"}), &GrooveJoint2D::set_length);
    MethodBinder::bind_method(D_METHOD("get_length"), &GrooveJoint2D::get_length);
    MethodBinder::bind_method(D_METHOD("set_initial_offset", {"offset"}), &GrooveJoint2D::set_initial_offset);
    MethodBinder::bind_method(D_METHOD("get_initial_offset"), &GrooveJoint2D::get_initial_offset);

    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "length", PROPERTY_HINT_EXP_RANGE, "1,65535,1"), "set_length", "get_length");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "initial_offset", PROPERTY_HINT_EXP_RANGE, "1,65535,1"), "set_initial_offset", "get_initial_offset");
}

GrooveJoint2D::GrooveJoint2D() {

    length = 50;
    initial_offset = 25;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DampedSpringJoint2D::_notification(int p_what) {

    switch (p_what) {
        case NOTIFICATION_DRAW: {

            if (!is_inside_tree())
                break;

            if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
                break;
            }

            draw_line(Point2(-10, 0), Point2(+10, 0), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(-10, length), Point2(+10, length), Color(0.7, 0.6, 0.0, 0.5), 3);
            draw_line(Point2(0, 0), Point2(0, length), Color(0.7, 0.6, 0.0, 0.5), 3);
        } break;
    }
}

RID DampedSpringJoint2D::_configure_joint(PhysicsBody2D *body_a, PhysicsBody2D *body_b) {

    Transform2D gt = get_global_transform();
    Vector2 anchor_A = gt.get_origin();
    Vector2 anchor_B = gt.xform(Vector2(0, length));

    RID dsj = Physics2DServer::get_singleton()->damped_spring_joint_create(anchor_A, anchor_B, body_a->get_rid(), body_b->get_rid());
    if (rest_length)
        Physics2DServer::get_singleton()->damped_string_joint_set_param(dsj, Physics2DServer::DAMPED_STRING_REST_LENGTH, rest_length);
    Physics2DServer::get_singleton()->damped_string_joint_set_param(dsj, Physics2DServer::DAMPED_STRING_STIFFNESS, stiffness);
    Physics2DServer::get_singleton()->damped_string_joint_set_param(dsj, Physics2DServer::DAMPED_STRING_DAMPING, damping);

    return dsj;
}

void DampedSpringJoint2D::set_length(real_t p_length) {

    length = p_length;
    update();
}

real_t DampedSpringJoint2D::get_length() const {

    return length;
}

void DampedSpringJoint2D::set_rest_length(real_t p_rest_length) {

    rest_length = p_rest_length;
    update();
    if (get_joint().is_valid())
        Physics2DServer::get_singleton()->damped_string_joint_set_param(get_joint(), Physics2DServer::DAMPED_STRING_REST_LENGTH, p_rest_length ? p_rest_length : length);
}

real_t DampedSpringJoint2D::get_rest_length() const {

    return rest_length;
}

void DampedSpringJoint2D::set_stiffness(real_t p_stiffness) {

    stiffness = p_stiffness;
    update();
    if (get_joint().is_valid())
        Physics2DServer::get_singleton()->damped_string_joint_set_param(get_joint(), Physics2DServer::DAMPED_STRING_STIFFNESS, p_stiffness);
}

real_t DampedSpringJoint2D::get_stiffness() const {

    return stiffness;
}

void DampedSpringJoint2D::set_damping(real_t p_damping) {

    damping = p_damping;
    update();
    if (get_joint().is_valid())
        Physics2DServer::get_singleton()->damped_string_joint_set_param(get_joint(), Physics2DServer::DAMPED_STRING_DAMPING, p_damping);
}

real_t DampedSpringJoint2D::get_damping() const {

    return damping;
}

void DampedSpringJoint2D::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_length", {"length"}), &DampedSpringJoint2D::set_length);
    MethodBinder::bind_method(D_METHOD("get_length"), &DampedSpringJoint2D::get_length);
    MethodBinder::bind_method(D_METHOD("set_rest_length", {"rest_length"}), &DampedSpringJoint2D::set_rest_length);
    MethodBinder::bind_method(D_METHOD("get_rest_length"), &DampedSpringJoint2D::get_rest_length);
    MethodBinder::bind_method(D_METHOD("set_stiffness", {"stiffness"}), &DampedSpringJoint2D::set_stiffness);
    MethodBinder::bind_method(D_METHOD("get_stiffness"), &DampedSpringJoint2D::get_stiffness);
    MethodBinder::bind_method(D_METHOD("set_damping", {"damping"}), &DampedSpringJoint2D::set_damping);
    MethodBinder::bind_method(D_METHOD("get_damping"), &DampedSpringJoint2D::get_damping);

    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "length", PROPERTY_HINT_EXP_RANGE, "1,65535,1"), "set_length", "get_length");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "rest_length", PROPERTY_HINT_EXP_RANGE, "0,65535,1"), "set_rest_length", "get_rest_length");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "stiffness", PROPERTY_HINT_EXP_RANGE, "0.1,64,0.1"), "set_stiffness", "get_stiffness");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "damping", PROPERTY_HINT_EXP_RANGE, "0.01,16,0.01"), "set_damping", "get_damping");
}

DampedSpringJoint2D::DampedSpringJoint2D() {

    length = 50;
    rest_length = 0;
    stiffness = 20;
    damping = 1;
}
