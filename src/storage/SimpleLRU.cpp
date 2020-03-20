#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h

void SimpleLRU::append(const std::string &key,const std::string &value) {
    std::unique_ptr<lru_node> node(new lru_node());
    node->value = value;
    node->key = key;
    _cur_size += key.size() + value.size();
    node->next = nullptr;
    if (_lru_head.get() != nullptr) {
        node->prev=_lru_head->prev;
        _lru_head->prev->next.reset(node.release());
    }
    else {
        node->next = nullptr;
        node->prev = node.get();
        _lru_head.reset(node.release());
    }
}
void SimpleLRU::del_node(lru_node *node) {
    _cur_size -= node->key.size() + node->value.size();
    if (_cur_size == 0) {
        _lru_head.reset();
    }
    else {
        std::unique_ptr<lru_node> tmp(new lru_node());
        if (_lru_head.get() == node) {
            tmp.reset(_lru_head.release());
            _lru_head.reset(tmp->next.release());
            _lru_head->prev = tmp->prev;
        }
        else {
            lru_node *prev = node->prev;
            tmp.reset(prev->next.release());
            prev->next.reset(tmp->next.release());
            if (_lru_head->prev == tmp.get()) {
                _lru_head->prev = prev;
            }
            else prev->next->prev = prev;
        }
    }
}
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size) return false;

	if (_cur_size == 0) {
		append(key, value);
        _lru_index.insert({key,*(_lru_head.get())});

		return true;
	}

	auto it = _lru_index.find(key);

	if (it != _lru_index.end()) {
        _lru_index.erase(it);
        del_node(&(it->second.get()));
	}

    while (_cur_size + value.size() + key.size() > _max_size) {
        _lru_index.erase(_lru_head->key);
        del_node(_lru_head.get());
    }

    _cur_size += value.size() + key.size();
    append(key, value);
    _lru_index.insert({key,*(_lru_head.get())});
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it != _lru_index.end()) return false;
    return Put(key, value); 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;
    return Put(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;
    _lru_index.erase(it);
    del_node(&(it->second.get()));
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto it = _lru_index.find(key);
    if (it == _lru_index.end()) return false;
    value = it->second.get().value;
    _lru_index.erase(it);
    del_node(&(it->second.get()));
    append(key, value);
    return true;
}

} // namespace Backend
} // namespace Afina