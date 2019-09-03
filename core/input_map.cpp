/*************************************************************************/
/*  input_map.cpp                                                        */
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

#include "input_map.h"

#include "core/os/keyboard.h"
#include "core/project_settings.h"
#include "core/method_bind.h"


IMPL_GDCLASS(InputMap)

InputMap *InputMap::singleton = nullptr;

int InputMap::ALL_DEVICES = -1;

void InputMap::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("has_action", "action"), &InputMap::has_action);
    MethodBinder::bind_method(D_METHOD("get_actions"), &InputMap::_get_actions);
    MethodBinder::bind_method(D_METHOD("add_action", "action", "deadzone"), &InputMap::add_action, {DEFVAL(0.5f)});
    MethodBinder::bind_method(D_METHOD("erase_action", "action"), &InputMap::erase_action);

    MethodBinder::bind_method(D_METHOD("action_set_deadzone", "action", "deadzone"), &InputMap::action_set_deadzone);
    MethodBinder::bind_method(D_METHOD("action_add_event", "action", "event"), &InputMap::action_add_event);
    MethodBinder::bind_method(D_METHOD("action_has_event", "action", "event"), &InputMap::action_has_event);
    MethodBinder::bind_method(D_METHOD("action_erase_event", "action", "event"), &InputMap::action_erase_event);
    MethodBinder::bind_method(D_METHOD("action_erase_events", "action"), &InputMap::action_erase_events);
    MethodBinder::bind_method(D_METHOD("get_action_list", "action"), &InputMap::_get_action_list);
    MethodBinder::bind_method(D_METHOD("event_is_action", "event", "action"), &InputMap::event_is_action);
    MethodBinder::bind_method(D_METHOD("load_from_globals"), &InputMap::load_from_globals);
}

void InputMap::add_action(const StringName &p_action, float p_deadzone) {

    ERR_FAIL_COND(input_map.has(p_action));
    input_map[p_action] = Action();
    static int last_id = 1;
    input_map[p_action].id = last_id;
    input_map[p_action].deadzone = p_deadzone;
    last_id++;
}

void InputMap::erase_action(const StringName &p_action) {

    ERR_FAIL_COND(!input_map.has(p_action));
    input_map.erase(p_action);
}

Array InputMap::_get_actions() {

    Array ret;
    List<StringName> actions = get_actions();
    if (actions.empty())
        return ret;

    for (const List<StringName>::Element *E = actions.front(); E; E = E->next()) {

        ret.push_back(E->get());
    }

    return ret;
}

List<StringName> InputMap::get_actions() const {

    List<StringName> actions = List<StringName>();
    if (input_map.empty()) {
        return actions;
    }

    for (Map<StringName, Action>::Element *E = input_map.front(); E; E = E->next()) {
        actions.push_back(E->key());
    }

    return actions;
}

List<Ref<InputEvent> >::Element *InputMap::_find_event(Action &p_action, const Ref<InputEvent> &p_event, bool *p_pressed, float *p_strength) const {

    for (List<Ref<InputEvent> >::Element *E = p_action.inputs.front(); E; E = E->next()) {

        const Ref<InputEvent> e = E->get();

        //if (e.type != Ref<InputEvent>::KEY && e.device != p_event.device) -- unsure about the KEY comparison, why is this here?
        //	continue;

        int device = e->get_device();
        if (device == ALL_DEVICES || device == p_event->get_device()) {
            if (e->action_match(p_event, p_pressed, p_strength, p_action.deadzone)) {
                return E;
            }
        }
    }

    return nullptr;
}

bool InputMap::has_action(const StringName &p_action) const {

    return input_map.has(p_action);
}

void InputMap::action_set_deadzone(const StringName &p_action, float p_deadzone) {

    ERR_FAIL_COND(!input_map.has(p_action));

    input_map[p_action].deadzone = p_deadzone;
}

void InputMap::action_add_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

    ERR_FAIL_COND(p_event.is_null());
    ERR_FAIL_COND(!input_map.has(p_action));
    if (_find_event(input_map[p_action], p_event))
        return; //already gots

    input_map[p_action].inputs.push_back(p_event);
}

bool InputMap::action_has_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

    ERR_FAIL_COND_V(!input_map.has(p_action), false);
    return (_find_event(input_map[p_action], p_event) != nullptr);
}

void InputMap::action_erase_event(const StringName &p_action, const Ref<InputEvent> &p_event) {

    ERR_FAIL_COND(!input_map.has(p_action));

    List<Ref<InputEvent> >::Element *E = _find_event(input_map[p_action], p_event);
    if (E)
        input_map[p_action].inputs.erase(E);
}

void InputMap::action_erase_events(const StringName &p_action) {

    ERR_FAIL_COND(!input_map.has(p_action));

    input_map[p_action].inputs.clear();
}

Array InputMap::_get_action_list(const StringName &p_action) {

    Array ret;
    const List<Ref<InputEvent> > *al = get_action_list(p_action);
    if (al) {
        for (const List<Ref<InputEvent> >::Element *E = al->front(); E; E = E->next()) {

            ret.push_back(E->get());
        }
    }

    return ret;
}

const List<Ref<InputEvent> > *InputMap::get_action_list(const StringName &p_action) {

    const Map<StringName, Action>::Element *E = input_map.find(p_action);
    if (!E)
        return nullptr;

    return &E->get().inputs;
}

bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action) const {
    return event_get_action_status(p_event, p_action);
}

bool InputMap::event_get_action_status(const Ref<InputEvent> &p_event, const StringName &p_action, bool *p_pressed, float *p_strength) const {
    Map<StringName, Action>::Element *E = input_map.find(p_action);
    ERR_FAIL_COND_V_MSG(!E, false, "Request for nonexistent InputMap action: " + String(p_action) + ".");

    Ref<InputEventAction> input_event_action = p_event;
    if (input_event_action.is_valid()) {
        if (p_pressed != nullptr)
            *p_pressed = input_event_action->is_pressed();
        if (p_strength != nullptr)
            *p_strength = (p_pressed != nullptr && *p_pressed) ? input_event_action->get_strength() : 0.0f;
        return input_event_action->get_action() == p_action;
    }

    bool pressed;
    float strength;
    List<Ref<InputEvent> >::Element *event = _find_event(E->get(), p_event, &pressed, &strength);
    if (event != nullptr) {
        if (p_pressed != nullptr)
            *p_pressed = pressed;
        if (p_strength != nullptr)
            *p_strength = strength;
        return true;
    } else {
        return false;
    }
}

const Map<StringName, InputMap::Action> &InputMap::get_action_map() const {
    return input_map;
}

void InputMap::load_from_globals() {
    using namespace StringUtils;
    input_map.clear();

    List<PropertyInfo> pinfo;
    ProjectSettings::get_singleton()->get_property_list(&pinfo);

    for (List<PropertyInfo>::Element *E = pinfo.front(); E; E = E->next()) {
        const PropertyInfo &pi = E->get();

        if (!begins_with(pi.name,"input/"))
            continue;

        String name = substr(pi.name,find(pi.name,"/") + 1, pi.name.length());

        Dictionary action = ProjectSettings::get_singleton()->get(pi.name);
        float deadzone = action.has("deadzone") ? action["deadzone"].as<float>() : 0.5f;
        Array events = action["events"];

        add_action(name, deadzone);
        for (int i = 0; i < events.size(); i++) {
            Ref<InputEvent> event = events[i];
            if (event.is_null())
                continue;
            action_add_event(name, event);
        }
    }
}
namespace {
void addActionKeys(InputMap &im,Ref<InputEventKey> &k,const StringName &n,std::initializer_list<KeyList> action_keys,bool shifted=false) {
	im.add_action(n);
	for(KeyList key : action_keys) {
		k.instance();
		k->set_scancode(key);
		if(shifted)
			k->set_shift(true);
		im.action_add_event(n, k);
	}

}
}
void InputMap::load_default() {

    Ref<InputEventKey> key;
	addActionKeys(*this,key,StaticCString("ui_accept"),{KEY_ENTER,KEY_KP_ENTER,KEY_SPACE});
	addActionKeys(*this,key,StaticCString("ui_select"),{KEY_SPACE});
	addActionKeys(*this,key,StaticCString("ui_cancel"),{KEY_ESCAPE});
	addActionKeys(*this,key,StaticCString("ui_focus_next"),{KEY_TAB});
	addActionKeys(*this,key,StaticCString("ui_focus_prev"),{KEY_TAB},true);
	addActionKeys(*this,key,StaticCString("ui_left"),{KEY_LEFT});
	addActionKeys(*this,key,StaticCString("ui_right"),{KEY_RIGHT});
	addActionKeys(*this,key,StaticCString("ui_up"),{KEY_UP});
	addActionKeys(*this,key,StaticCString("ui_down"),{KEY_DOWN});

	addActionKeys(*this,key,StaticCString("ui_page_up"),{KEY_PAGEUP});
	addActionKeys(*this,key,StaticCString("ui_page_down"),{KEY_PAGEDOWN});
	addActionKeys(*this,key,StaticCString("ui_home"),{KEY_HOME});
	addActionKeys(*this,key,StaticCString("ui_end"),{KEY_END});
    //set("display/window/handheld/orientation", "landscape");
}

InputMap::InputMap() {

    ERR_FAIL_COND(singleton);
    singleton = this;
}
