#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h

void SimpleLRU::append(const std::string &key,const std::string &value) {
    if (_cur_size != 0) {
        std::unique_ptr<lru_node> node(new lru_node(key));
        node->value = value;
        node->next = nullptr;
        node->prev = _lru_head->prev;
        _lru_head->prev = node.get();
        node->prev->next = std::move(node);
    }
    else {
        _lru_head->key = key;
        _lru_head->value = value;
        _lru_head->next = nullptr;
        _lru_head->prev = _lru_head.get();
    }
    _cur_size += key.size() + value.size();
}
void SimpleLRU::del_node(lru_node *node) {
    _cur_size -= node->key.size() + node->value.size();
    if (_cur_size == 0) {
        _lru_head->next = nullptr;
        _lru_head->prev = _lru_head.get();
    }
    else {
        std::unique_ptr<lru_node> tmp(new lru_node);
        if (_lru_head.get() == node) {
            tmp = std::move(_lru_head);
            _lru_head = std::move(tmp->next);
            _lru_head->prev = tmp->prev;
        }
        else {
            lru_node *previ = node->prev;
            tmp = std::move(previ->next);
            if (_lru_head->prev == tmp.get()) {
                _lru_head->prev = previ;
            }
            else {
                previ->next = std::move(tmp->next);
                previ->next->prev = previ;
            }
        }
    }
}
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size) return false;

    //for (auto it = _lru_index.begin();it != _lru_index.end();it++) std::cout << it->first.get() << it->second.get().value << std::endl;
	
    auto it = _lru_index.find(key);

	if (it != _lru_index.end()) {
        auto ptr = &it->second.get(); 
        if (ptr->next == nullptr) {}
        else if (ptr == _lru_head.get()) {
            _lru_head.release();
            _lru_head.reset(ptr->next.release());
            _lru_head->prev = ptr;
            ptr->prev->next.reset(ptr);
        }
        else {
            ptr->next->prev = ptr->prev;
            ptr->prev->next.release();
            ptr->prev->next.reset(ptr->next.release());
            _lru_head->prev->next.reset(ptr);
            ptr->prev = _lru_head->prev;
            _lru_head->prev = ptr;
        }

        while (_cur_size - ptr->value.size() + value.size() > _max_size) {
            auto ptrd = _lru_head.get(); 
            _lru_index.erase(_lru_head->key);
            del_node(ptrd);
        }
        ptr->value = value;
        return true;
	}
    else {
        while (_cur_size + value.size() + key.size() > _max_size) {
            auto ptr = _lru_head.get(); 
            _lru_index.erase(_lru_head->key);
            del_node(ptr); 
        }

        append(key, value);
        _lru_index.insert({_lru_head->prev->key,*(_lru_head->prev)});
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) return false;
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) return false;
    
    while (_cur_size + value.size() + key.size() > _max_size) {
        auto ptr = _lru_head.get(); 
        _lru_index.erase(_lru_head->key);
        del_node(ptr);
    }

    append(key, value);
    _lru_index.insert({_lru_head->prev->key,*(_lru_head->prev)});
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    if (key.size() + value.size() > _max_size) return false;
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;

    auto ptr = &it->second.get(); 
    ///////////
    if (ptr->next == nullptr) {}
    else if (ptr == _lru_head.get()) {
        _lru_head.release();
        _lru_head.reset(ptr->next.release());
        _lru_head->prev = ptr;
        ptr->prev->next.reset(ptr);
    }
    else {
        ptr->next->prev = ptr->prev;
        ptr->prev->next.release();
        ptr->prev->next.reset(ptr->next.release());
        _lru_head->prev->next.reset(ptr);
        ptr->prev = _lru_head->prev;
        _lru_head->prev = ptr;
    }
    while (_cur_size - ptr->value.size() + value.size() > _max_size) {
        auto ptrd = _lru_head.get();
        _lru_index.erase(_lru_head->key);
        del_node(ptrd);
    }
    ptr->value = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;
    auto ptr = &it->second.get();
    _lru_index.erase(it);
    del_node(ptr);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;
    value = it->second.get().value;
    auto ptr = &it->second.get();
    //const std::string vaal = value;
    //_lru_index.erase(it);
    //del_node(ptr);
    //append(key, vaal);
    //_lru_index.insert({_lru_head->prev->key,*(_lru_head->prev)});
    if (ptr->next == nullptr) {}
    else if (ptr == _lru_head.get()) {
        _lru_head.release();
        _lru_head.reset(ptr->next.release());
        _lru_head->prev = ptr;
        ptr->prev->next.reset(ptr);
    }
    else {
        ptr->next->prev = ptr->prev;
        ptr->prev->next.release();
        ptr->prev->next.reset(ptr->next.release());
        _lru_head->prev->next.reset(ptr);
        ptr->prev = _lru_head->prev;
        _lru_head->prev = ptr;
    }
    
    return true;
}

} // namespace Backend
} // namespace Afina