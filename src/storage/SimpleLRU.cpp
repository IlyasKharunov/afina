#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (key.size() + value.size() > _max_size) return false;
	if (_cur_size == 0) {
		std::unique_ptr<lru_node> node(new lru_node);
		node->key.assign(key);
		node->value.assign(value);
		node->next = nullptr;
		node->prev = node.get();

		_lru_index.emplace(key,value);
		_lru_head = node;
		_cur_size += (key.size() + value.size());

		return true;
	}
	auto it = _lru_index.find(key);
	if (it != _lru_index.end()) {
		if (_cur_size + value.size() - it->second.value.size() <= _max_size) {
				it->second.value.assign(value);
			if (it->second.next.get() != nullptr) {
				head->prev->next = it->second.prev->next;
				it->second.next->prev = it->second.prev;
				it->second.prev->next = it->second.next;
			}
			return true;
		}
		return false;
	}
	else if (_cur_size + value.size() + key.size() <= _max_size) {
		std::unique_ptr<lru_node> node(new lru_node);
		node->key.assign(key);
		node->value.assign(value);
		node->prev = head->prev;
		node->next = nullptr;

		_lru_index.emplace(key,value);
		head->prev->next = node;
	}
	else {
		while(_cur_size + value.size() + key.size() > _max_size) {
			_cur_size -= (head->value.size() + head->key.size());
			head->next->prev = head->prev;
			
		}
	}
	return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) { return false; }

void SimpleLRU::front_del() {
	if (head->next.get() == nullptr)
}

} // namespace Backend
} // namespace Afina
