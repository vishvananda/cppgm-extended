#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace assembly {

// Immutable AVL paths make assembler environment snapshots constant-sized.
// Definitions and key storage are shared; a change replaces only its path.
template<typename Value>
class PersistentMap
{
	struct Node;
	using Link = std::shared_ptr<const Node>;
	using Key = std::shared_ptr<const std::string>;
	struct Node
	{
		Key key;
		Value value;
		Link left, right;
		unsigned height;
		Node(Key key, Value value, Link left, Link right)
			: key(std::move(key)), value(std::move(value)), left(std::move(left)), right(std::move(right)),
			  height(1 + std::max(this->left ? this->left->height : 0, this->right ? this->right->height : 0))
		{}
	};
	static Link node(Key key, Value value, Link left, Link right)
	{
		return std::make_shared<Node>(std::move(key), std::move(value), std::move(left), std::move(right));
	}
	static int height(const Link& value) { return value ? static_cast<int>(value->height) : 0; }
	static int balance(const Link& value) { return height(value->left) - height(value->right); }
	static Link left(const Link& value)
	{
		const Link& root = value->right;
		return node(root->key, root->value, node(value->key, value->value, value->left, root->left), root->right);
	}
	static Link right(const Link& value)
	{
		const Link& root = value->left;
		return node(root->key, root->value, root->left, node(value->key, value->value, root->right, value->right));
	}
	static Link balanced(Link value)
	{
		if (balance(value) > 1)
		{
			if (balance(value->left) < 0)
				value = node(value->key, value->value, left(value->left), value->right);
			return right(value);
		}
		if (balance(value) < -1)
		{
			if (balance(value->right) > 0)
				value = node(value->key, value->value, value->left, right(value->right));
			return left(value);
		}
		return value;
	}
	static Link insert(const Link& root, const Key& key, const Value& value)
	{
		if (!root)
			return node(key, value, {}, {});
		const int order = key->compare(*root->key);
		if (order == 0)
			return node(root->key, value, root->left, root->right);
		return balanced(node(root->key, root->value, order < 0 ? insert(root->left, key, value) : root->left,
			order > 0 ? insert(root->right, key, value) : root->right));
	}
	static Link remove(const Link& root, const std::string& key)
	{
		if (!root)
			return {};
		const int order = key.compare(*root->key);
		if (order != 0)
			return balanced(node(root->key, root->value, order < 0 ? remove(root->left, key) : root->left,
				order > 0 ? remove(root->right, key) : root->right));
		if (!root->left)
			return root->right;
		if (!root->right)
			return root->left;
		Link successor = root->right;
		while (successor->left)
			successor = successor->left;
		return balanced(node(successor->key, successor->value, root->left, remove(root->right, *successor->key)));
	}
	Link root_;

public:
	Value find(const std::string& key) const
	{
		Link current = root_;
		while (current)
		{
			const int order = key.compare(*current->key);
			if (order == 0)
				return current->value;
			current = order < 0 ? current->left : current->right;
		}
		return {};
	}
	void set(const std::string& key, const Value& value)
	{
		root_ = insert(root_, std::make_shared<const std::string>(key), value);
	}
	bool erase(const std::string& key)
	{
		if (!find(key))
			return false;
		root_ = remove(root_, key);
		return true;
	}
};
} // namespace assembly
