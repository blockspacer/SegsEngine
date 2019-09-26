/*************************************************************************/
/*  base_button.cpp                                                      */
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

#include "base_button.h"

#include "core/method_bind.h"
#include "core/os/keyboard.h"
#include "scene/main/viewport.h"
#include "scene/scene_string_names.h"

IMPL_GDCLASS(BaseButton)
IMPL_GDCLASS(ButtonGroup)

void BaseButton::_unpress_group() {

    if (not button_group)
        return;

    if (toggle_mode) {
        status.pressed = true;
    }

    for (BaseButton * E : button_group->buttons) {
        if (E == this)
            continue;

        E->set_pressed(false);
    }
}

void BaseButton::_gui_input(Ref<InputEvent> p_event) {

    if (status.disabled) // no interaction with disabled button
        return;

    Ref<InputEventMouseButton> mouse_button = dynamic_ref_cast<InputEventMouseButton>(p_event);
    bool ui_accept = p_event->is_action("ui_accept") && !p_event->is_echo();

    bool button_masked = mouse_button && ((1 << (mouse_button->get_button_index() - 1)) & button_mask) > 0;
    if (button_masked || ui_accept) {
        on_action_event(p_event);
        return;
    }

    Ref<InputEventMouseMotion> mouse_motion = dynamic_ref_cast<InputEventMouseMotion>(p_event);
    if (mouse_motion) {
        if (status.press_attempt) {
            bool last_press_inside = status.pressing_inside;
            status.pressing_inside = has_point(mouse_motion->get_position());
            if (last_press_inside != status.pressing_inside) {
                update();
            }
        }
    }
}

void BaseButton::_notification(int p_what) {

    if (p_what == NOTIFICATION_MOUSE_ENTER) {

        status.hovering = true;
        update();
    }

    if (p_what == NOTIFICATION_MOUSE_EXIT) {
        status.hovering = false;
        update();
    }
    if (p_what == NOTIFICATION_DRAG_BEGIN || p_what == NOTIFICATION_SCROLL_BEGIN) {

        if (status.press_attempt) {
            status.press_attempt = false;
            update();
        }
    }

    if (p_what == NOTIFICATION_FOCUS_ENTER) {

        status.hovering = true;
        update();
    }

    if (p_what == NOTIFICATION_FOCUS_EXIT) {

        if (status.press_attempt) {
            status.press_attempt = false;
            status.hovering = false;
            update();
        } else if (status.hovering) {
            status.hovering = false;
            update();
        }
    }

    if (p_what == NOTIFICATION_EXIT_TREE || (p_what == NOTIFICATION_VISIBILITY_CHANGED && !is_visible_in_tree())) {

        if (!toggle_mode) {
            status.pressed = false;
        }
        status.hovering = false;
        status.press_attempt = false;
        status.pressing_inside = false;
    }
}

void BaseButton::_pressed() {

    if (get_script_instance()) {
        get_script_instance()->call(SceneStringNames::get_singleton()->_pressed);
    }
    pressed();
    emit_signal("pressed");
}

void BaseButton::_toggled(bool p_pressed) {

    if (get_script_instance()) {
        get_script_instance()->call(SceneStringNames::get_singleton()->_toggled, p_pressed);
    }
    toggled(p_pressed);
    emit_signal("toggled", p_pressed);
}

void BaseButton::on_action_event(Ref<InputEvent> p_event) {

    if (p_event->is_pressed()) {
        status.press_attempt = true;
        status.pressing_inside = true;
        emit_signal("button_down");
    }

    if (status.press_attempt && status.pressing_inside) {
        if (toggle_mode) {
            if ((p_event->is_pressed() && action_mode == ACTION_MODE_BUTTON_PRESS) || (!p_event->is_pressed() && action_mode == ACTION_MODE_BUTTON_RELEASE)) {
                if (action_mode == ACTION_MODE_BUTTON_PRESS) {
                    status.press_attempt = false;
                    status.pressing_inside = false;
                }
                status.pressed = !status.pressed;
                _unpress_group();
                _toggled(status.pressed);
                _pressed();
            }
        } else {
            if (!p_event->is_pressed()) {
                _pressed();
            }
        }
    }

    if (!p_event->is_pressed()) { // pressed state should be correct with button_up signal
        emit_signal("button_up");
        status.press_attempt = false;
    }

    update();
}

void BaseButton::pressed() {
}

void BaseButton::toggled(bool p_pressed) {
}

void BaseButton::set_disabled(bool p_disabled) {
    if (status.disabled == p_disabled)
        return;

    status.disabled = p_disabled;
    if (p_disabled) {
        if (!toggle_mode) {
            status.pressed = false;
        }
        status.press_attempt = false;
        status.pressing_inside = false;
    }
    update();
    _change_notify("disabled");
}

bool BaseButton::is_disabled() const {

    return status.disabled;
}

void BaseButton::set_pressed(bool p_pressed) {

    if (!toggle_mode)
        return;
    if (status.pressed == p_pressed)
        return;
    _change_notify("pressed");
    status.pressed = p_pressed;

    if (p_pressed) {
        _unpress_group();
    }
    _toggled(status.pressed);

    update();
}

bool BaseButton::is_pressing() const {

    return status.press_attempt;
}

bool BaseButton::is_pressed() const {

    return toggle_mode ? status.pressed : status.press_attempt;
}

bool BaseButton::is_hovered() const {

    return status.hovering;
}

BaseButton::DrawMode BaseButton::get_draw_mode() const {

    if (status.disabled) {
        return DRAW_DISABLED;
    }

    if (!status.press_attempt && status.hovering) {
        if (status.pressed)
            return DRAW_HOVER_PRESSED;

        return DRAW_HOVER;
    } else {
        /* determine if pressed or not */

        bool pressing;
        if (status.press_attempt) {

            pressing = (status.pressing_inside || keep_pressed_outside);
            if (status.pressed)
                pressing = !pressing;
        } else {

            pressing = status.pressed;
        }

        if (pressing)
            return DRAW_PRESSED;
        else
            return DRAW_NORMAL;
    }

    return DRAW_NORMAL;
}

void BaseButton::set_toggle_mode(bool p_on) {

    toggle_mode = p_on;
}

bool BaseButton::is_toggle_mode() const {

    return toggle_mode;
}

void BaseButton::set_shortcut_in_tooltip(bool p_on) {

    shortcut_in_tooltip = p_on;
}

bool BaseButton::is_shortcut_in_tooltip_enabled() const {

    return shortcut_in_tooltip;
}

void BaseButton::set_action_mode(ActionMode p_mode) {

    action_mode = p_mode;
}

BaseButton::ActionMode BaseButton::get_action_mode() const {

    return action_mode;
}

void BaseButton::set_button_mask(int p_mask) {

    button_mask = p_mask;
}

int BaseButton::get_button_mask() const {

    return button_mask;
}

void BaseButton::set_enabled_focus_mode(FocusMode p_mode) {

    enabled_focus_mode = p_mode;
    if (!status.disabled) {
        set_focus_mode(p_mode);
    }
}

Control::FocusMode BaseButton::get_enabled_focus_mode() const {

    return enabled_focus_mode;
}

void BaseButton::set_keep_pressed_outside(bool p_on) {

    keep_pressed_outside = p_on;
}

bool BaseButton::is_keep_pressed_outside() const {

    return keep_pressed_outside;
}

void BaseButton::set_shortcut(const Ref<ShortCut> &p_shortcut) {

    shortcut = p_shortcut;
    set_process_unhandled_input(shortcut);
}

Ref<ShortCut> BaseButton::get_shortcut() const {
    return shortcut;
}

void BaseButton::_unhandled_input(Ref<InputEvent> p_event) {

    if (!is_disabled() && is_visible_in_tree() && !p_event->is_echo() && shortcut && shortcut->is_shortcut(p_event)) {

        if (get_viewport()->get_modal_stack_top() && !get_viewport()->get_modal_stack_top()->is_a_parent_of(this))
            return; //ignore because of modal window

        on_action_event(p_event);
    }
}

String BaseButton::get_tooltip(const Point2 &p_pos) const {

    String tooltip = Control::get_tooltip(p_pos);
    if (shortcut_in_tooltip && shortcut && shortcut->is_valid()) {
        String text = shortcut->get_name() + " (" + shortcut->get_as_text() + ")";
        if (StringUtils::compare(shortcut->get_name(),tooltip,StringUtils::CaseInsensitive) != 0) {
            text += "\n" + tooltip;
        }
        tooltip = text;
    }
    return tooltip;
}

void BaseButton::set_button_group(const Ref<ButtonGroup> &p_group) {

    if (button_group) {
        button_group->buttons.erase(this);
    }

    button_group = p_group;

    if (button_group) {
        button_group->buttons.insert(this);
    }

    update(); //checkbox changes to radio if set a buttongroup
}

Ref<ButtonGroup> BaseButton::get_button_group() const {

    return button_group;
}

void BaseButton::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("_gui_input"), &BaseButton::_gui_input);
    MethodBinder::bind_method(D_METHOD("_unhandled_input"), &BaseButton::_unhandled_input);
    MethodBinder::bind_method(D_METHOD("set_pressed", {"pressed"}), &BaseButton::set_pressed);
    MethodBinder::bind_method(D_METHOD("is_pressed"), &BaseButton::is_pressed);
    MethodBinder::bind_method(D_METHOD("is_hovered"), &BaseButton::is_hovered);
    MethodBinder::bind_method(D_METHOD("set_toggle_mode", {"enabled"}), &BaseButton::set_toggle_mode);
    MethodBinder::bind_method(D_METHOD("is_toggle_mode"), &BaseButton::is_toggle_mode);
    MethodBinder::bind_method(D_METHOD("set_shortcut_in_tooltip", {"enabled"}), &BaseButton::set_shortcut_in_tooltip);
    MethodBinder::bind_method(D_METHOD("is_shortcut_in_tooltip_enabled"), &BaseButton::is_shortcut_in_tooltip_enabled);
    MethodBinder::bind_method(D_METHOD("set_disabled", {"disabled"}), &BaseButton::set_disabled);
    MethodBinder::bind_method(D_METHOD("is_disabled"), &BaseButton::is_disabled);
    MethodBinder::bind_method(D_METHOD("set_action_mode", {"mode"}), &BaseButton::set_action_mode);
    MethodBinder::bind_method(D_METHOD("get_action_mode"), &BaseButton::get_action_mode);
    MethodBinder::bind_method(D_METHOD("set_button_mask", {"mask"}), &BaseButton::set_button_mask);
    MethodBinder::bind_method(D_METHOD("get_button_mask"), &BaseButton::get_button_mask);
    MethodBinder::bind_method(D_METHOD("get_draw_mode"), &BaseButton::get_draw_mode);
    MethodBinder::bind_method(D_METHOD("set_enabled_focus_mode", {"mode"}), &BaseButton::set_enabled_focus_mode);
    MethodBinder::bind_method(D_METHOD("get_enabled_focus_mode"), &BaseButton::get_enabled_focus_mode);
    MethodBinder::bind_method(D_METHOD("set_keep_pressed_outside", {"enabled"}), &BaseButton::set_keep_pressed_outside);
    MethodBinder::bind_method(D_METHOD("is_keep_pressed_outside"), &BaseButton::is_keep_pressed_outside);

    MethodBinder::bind_method(D_METHOD("set_shortcut", {"shortcut"}), &BaseButton::set_shortcut);
    MethodBinder::bind_method(D_METHOD("get_shortcut"), &BaseButton::get_shortcut);

    MethodBinder::bind_method(D_METHOD("set_button_group", {"button_group"}), &BaseButton::set_button_group);
    MethodBinder::bind_method(D_METHOD("get_button_group"), &BaseButton::get_button_group);

    BIND_VMETHOD(MethodInfo("_pressed"));
    BIND_VMETHOD(MethodInfo("_toggled", PropertyInfo(VariantType::BOOL, "button_pressed")));

    ADD_SIGNAL(MethodInfo("pressed"));
    ADD_SIGNAL(MethodInfo("button_up"));
    ADD_SIGNAL(MethodInfo("button_down"));
    ADD_SIGNAL(MethodInfo("toggled", PropertyInfo(VariantType::BOOL, "button_pressed")));
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "disabled"), "set_disabled", "is_disabled");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "toggle_mode"), "set_toggle_mode", "is_toggle_mode");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "shortcut_in_tooltip"), "set_shortcut_in_tooltip", "is_shortcut_in_tooltip_enabled");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "pressed"), "set_pressed", "is_pressed");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "action_mode", PROPERTY_HINT_ENUM, "Button Press,Button Release"), "set_action_mode", "get_action_mode");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "button_mask", PROPERTY_HINT_FLAGS, "Mouse Left, Mouse Right, Mouse Middle"), "set_button_mask", "get_button_mask");
    ADD_PROPERTY(PropertyInfo(VariantType::INT, "enabled_focus_mode", PROPERTY_HINT_ENUM, "None,Click,All"), "set_enabled_focus_mode", "get_enabled_focus_mode");
    ADD_PROPERTY(PropertyInfo(VariantType::BOOL, "keep_pressed_outside"), "set_keep_pressed_outside", "is_keep_pressed_outside");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "shortcut", PROPERTY_HINT_RESOURCE_TYPE, "ShortCut"), "set_shortcut", "get_shortcut");
    ADD_PROPERTY(PropertyInfo(VariantType::OBJECT, "group", PROPERTY_HINT_RESOURCE_TYPE, "ButtonGroup"), "set_button_group", "get_button_group");

    BIND_ENUM_CONSTANT(DRAW_NORMAL)
    BIND_ENUM_CONSTANT(DRAW_PRESSED)
    BIND_ENUM_CONSTANT(DRAW_HOVER)
    BIND_ENUM_CONSTANT(DRAW_DISABLED)
    BIND_ENUM_CONSTANT(DRAW_HOVER_PRESSED)

    BIND_ENUM_CONSTANT(ACTION_MODE_BUTTON_PRESS)
    BIND_ENUM_CONSTANT(ACTION_MODE_BUTTON_RELEASE)
}

BaseButton::BaseButton() {

    toggle_mode = false;
    shortcut_in_tooltip = true;
    keep_pressed_outside = false;
    status.pressed = false;
    status.press_attempt = false;
    status.hovering = false;
    status.pressing_inside = false;
    status.disabled = false;
    set_focus_mode(FOCUS_ALL);
    enabled_focus_mode = FOCUS_ALL;
    action_mode = ACTION_MODE_BUTTON_RELEASE;
    button_mask = BUTTON_MASK_LEFT;
}

BaseButton::~BaseButton() {

    if (button_group) {
        button_group->buttons.erase(this);
    }
}

void ButtonGroup::get_buttons(List<BaseButton *> *r_buttons) {

    for (BaseButton * E : buttons) {
        r_buttons->push_back(E);
    }
}

Array ButtonGroup::_get_buttons() {

    Array btns;
    for (BaseButton * E : buttons) {
        btns.push_back(Variant(E));
    }

    return btns;
}

BaseButton *ButtonGroup::get_pressed_button() {

    for (BaseButton * E : buttons) {
        if (E->is_pressed())
            return E;
    }

    return nullptr;
}

void ButtonGroup::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("get_pressed_button"), &ButtonGroup::get_pressed_button);
    MethodBinder::bind_method(D_METHOD("get_buttons"), &ButtonGroup::_get_buttons);
}

ButtonGroup::ButtonGroup() {

    set_local_to_scene(true);
}
