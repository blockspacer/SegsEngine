/*************************************************************************/
/*  dictionary.cpp                                                       */
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

#include "dictionary.h"

#include "core/ordered_hash_map.h"
#include "core/safe_refcount.h"
#include "core/hashfuncs.h"
#include "core/variant.h"
#include "core/list.h"

struct DictionaryPrivate {

    SafeRefCount refcount;
    OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator> variant_map;
};

Vector<Variant> Dictionary::get_key_list() const {
    Vector<Variant> res;
    if (_p->variant_map.empty())
        return {};
    res.reserve(_p->variant_map.size());

    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        res.emplace_back(E.key());
    }
    return res;
}

Variant Dictionary::get_key_at_index(int p_index) const {

    int index = 0;
    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        if (index == p_index) {
            return E.key();
        }
        index++;
    }

    return Variant();
}

Variant Dictionary::get_value_at_index(int p_index) const {

    int index = 0;
    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        if (index == p_index) {
            return E.value();
        }
        index++;
    }

    return Variant();
}

Variant &Dictionary::operator[](const Variant &p_key) {

    return _p->variant_map[p_key];
}

const Variant &Dictionary::operator[](const Variant &p_key) const {

    return _p->variant_map[p_key];
}
const Variant *Dictionary::getptr(const Variant &p_key) const {

    OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::ConstElement E = ((const OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator> *)&_p->variant_map)->find(p_key);

    if (!E)
        return nullptr;
    return &E.get();
}

Variant *Dictionary::getptr(const Variant &p_key) {

    OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.find(p_key);

    if (!E)
        return nullptr;
    return &E.get();
}

Variant Dictionary::get_valid(const Variant &p_key) const {

    OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::ConstElement E = ((const OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator> *)&_p->variant_map)->find(p_key);

    if (!E)
        return Variant();
    return E.get();
}

Variant Dictionary::get(const Variant &p_key, const Variant &p_default) const {
    const Variant *result = getptr(p_key);
    if (!result) {
        return p_default;
    }

    return *result;
}

int Dictionary::size() const {

    return _p->variant_map.size();
}
bool Dictionary::empty() const {

    return _p->variant_map.empty();
}

bool Dictionary::has(const Variant &p_key) const {

    return _p->variant_map.has(p_key);
}

bool Dictionary::has_all(const Array &p_keys) const {
    for (int i = 0; i < p_keys.size(); i++) {
        if (!has(p_keys[i])) {
            return false;
        }
    }
    return true;
}

bool Dictionary::erase(const Variant &p_key) {

    return _p->variant_map.erase(p_key);
}

bool Dictionary::operator==(const Dictionary &p_dictionary) const {

    return _p == p_dictionary._p;
}

bool Dictionary::operator!=(const Dictionary &p_dictionary) const {

    return _p != p_dictionary._p;
}

void Dictionary::_ref(const Dictionary &p_from) const {

    //make a copy first (thread safe)
    if (!p_from._p->refcount.ref())
        return; // couldn't copy

    //if this is the same, unreference the other one
    if (p_from._p == _p) {
        _p->refcount.unref();
        return;
    }
    if (_p)
        _unref();
    _p = p_from._p;
}

void Dictionary::clear() {

    _p->variant_map.clear();
}

void Dictionary::_unref() const {

    ERR_FAIL_COND(!_p);
    if (_p->refcount.unref()) {
        memdelete(_p);
    }
    _p = nullptr;
}
uint32_t Dictionary::hash() const {

    uint32_t h = hash_djb2_one_32(int(VariantType::DICTIONARY));

    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        h = hash_djb2_one_32(E.key().hash(), h);
        h = hash_djb2_one_32(E.value().hash(), h);
    }

    return h;
}

Array Dictionary::keys() const {

    Array varr;
    if (_p->variant_map.empty())
        return varr;

    varr.resize(size());
    int i = 0;
    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        varr[i] = E.key();
        i++;
    }

    return varr;
}

Array Dictionary::values() const {

    Array varr;
    if (_p->variant_map.empty())
        return varr;

    varr.resize(size());
    int i = 0;
    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        varr[i] = E.get();
        i++;
    }

    return varr;
}

const Variant *Dictionary::next(const Variant *p_key) const {

    if (p_key == nullptr) {
        // caller wants to get the first element
        if (_p->variant_map.front())
            return &_p->variant_map.front().key();
        return nullptr;
    }
    OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.find(*p_key);

    if (E && E.next())
        return &E.next().key();
    return nullptr;
}

Dictionary Dictionary::duplicate(bool p_deep) const {

    Dictionary n;

    for (OrderedHashMap<Variant, Variant, Hasher<Variant>, VariantComparator>::Element E = _p->variant_map.front(); E; E = E.next()) {
        n[E.key()] = p_deep ? E.value().duplicate(true) : E.value();
    }

    return n;
}

Dictionary &Dictionary::operator=(const Dictionary &p_dictionary) {
    if(this==&p_dictionary)
        return *this;
    _ref(p_dictionary);
    return *this;
}

const void *Dictionary::id() const {
    return _p->variant_map.id();
}

Dictionary::Dictionary(const Dictionary &p_from) {
    _p = nullptr;
    _ref(p_from);
}

Dictionary::Dictionary() {

    _p = memnew(DictionaryPrivate);
    _p->refcount.init();
}
Dictionary::~Dictionary() {
    if(_p)
        _unref();
}
