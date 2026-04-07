#pragma once

#include "common.h"

template <typename T>
struct Array {
	T* data = nullptr;
	int size = 0;

	Array() {}
	Array(T* data, int size) : data(data), size(size) {}

	T& operator[](int index) {
		return data[index];
	}

	T get(int index) const {
		if (index >= size) panic("Out of bounds array access");
		return data[index];
	}

	T get_or_default(int index) const {
		if (index >= size) return T();
		return data[index];
	}

	Find_Result find(T& elem) const {
		for (int i = 0; i < size; i++)
		{
			if (data[i] == elem)
			{
				return Find_Result {i, true};
			}
		}

		return Find_Result {0, false};
	}

	bool operator==(const Array<T>& other) const {
		if (size != other.size) {
			return false;
		}

		for (int i = 0; i < size; i++) {
			if (data[i] != other.data[i]) {
				return false;
			}
		}

		return true;
	}

	void free_data()
	{
		delete[] data;
		size = 0;
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + size;
	}

	const T* begin() const {
		return data;
	}

	const T* end() const {
		return data + size;
	}
};

template<typename T, int N>
Array<T> make_array(T (&arr)[N]) {
	return Array<T>(arr, N);
}

template<typename T, int N>
Array<const T> make_const_array(const T (&arr)[N]) {
	return Array<const T>(arr, N);
}

template <typename T>
struct DArray {
private:
	T* m_data = NULL;
	int m_size = 0;
	int m_cap = 0;

public:
	T* data() { return m_data; }
	int size() const { return m_size; }

	DArray() {}
	DArray(int cap) {
		m_data = new T[cap];
		m_cap = cap;
	}

	void discard_data() {
		m_size = 0;
	}

	// to use this as an allocator
	Array<T> get_array(int size)
	{
		ensure_size(size);
		return Array<T>(m_data, size);
	}

	Array<T> to_array() const
	{
		return Array<T> (m_data, m_size);
	}

	void reset() {
		if (m_data)
		{
			delete[](m_data);
			m_data = nullptr;

			m_size = 0;
			m_cap = 0;
		}
	}

	bool in_bounds(int index) {
		return index < m_size && index >= 0;
	}

	T get(int index) const {
		if (index >= m_size) panic("Out of bounds array access");
		return m_data[index];
	}

	T get_or_default(int index) const {
		if (index >= m_size) return T();
		return m_data[index];
	}

	T& get_ref(int index) const {
		if (index >= m_size) panic("Out of bounds array access");
		return m_data[index];
	}

	T* get_ptr(int index) const {
		if (index >= m_size) panic("Out of bounds array access");
		return &m_data[index];
	}

	T& operator[](int index) const
	{
		return get_ref(index);
	}

	int add(const T& elem)	{
		int ret_index = m_size;
		if (m_size + 1 > m_cap)
		{
			grow();
		}

		m_data[m_size] = elem;
		m_size += 1;
		return ret_index;
	}

	int add(T&& elem)	{
		int ret_index = m_size;
		if (m_size + 1 > m_cap)
		{
			grow();
		}

		m_data[m_size] = std::move(elem);
		m_size += 1;
		return ret_index;
	}

	void remove_shift(int index) {
		if (!in_bounds(index)) {
			panic("Out of bounds array access");
		}

		for (int i = index; i < m_size-1; i++) {
			m_data[i] = m_data[i+1];
		}

		m_size -= 1;
	}

	void remove(int index) {
		if (!in_bounds(index)) {
			panic("Out of bounds array access");
		}

		m_size -= 1;

		m_data[index] = m_data[m_size-1];
	}

	void replace(T elem, int index)
	{
		if (!in_bounds(index)) {
			panic("Out of bounds array access");
		}

		m_data[index] = elem;
	}

	void insert(T elem, int index)
	{
		if (m_size + 1 >= m_cap)
		{
			grow();
		}

		for (int i = m_size; i > index; i -= 1)
		{
			m_data[i] = m_data[i - 1];
		}

		m_data[index] = elem;

		m_size += 1;
	}

	int add_unique(T elem) {
		Find_Result find_result = find(elem);
		if (find_result.found)
		{
			return find_result.index;
		}

		return add(elem);
	}

	Find_Result find(T& elem) const {
		for (int i = 0; i < m_size; i++)
		{
			if (m_data[i] == elem)
			{
				return Find_Result {i, true};
			}
		}

		return Find_Result {0, false};
	}

	bool is_empty()	const {
		return m_size == 0;
	}

	T pop()	{
		if (is_empty())
		{
			panic("Trying to pop from empty array");
		}

		m_size -= 1;
		return m_data[m_size];
	}

	void free()	{
		reset();
	}

	void ensure_size(int size) {
		if (m_size < size) {
			resize(size);
		}
	}

	void resize(int size) {
		T* new_buffer = new T[size];

		int new_size = (size < m_size) ? size : m_size;

		for (int i = 0; i < new_size; i++) {
			new_buffer[i] = m_data[i];
		}

		if (m_data) {
			delete[] m_data;
		}

		m_data = new_buffer;
		m_cap = size;
		m_size = new_size;
	}

	T* begin() {
		return m_data;
	}

	T* end() {
		return m_data + m_size;
	}

	const T* begin() const {
		return m_data;
	}

	const T* end() const {
		return m_data + m_size;
	}

private:
	void grow() {
		int ncap = m_cap ? (m_cap * 2) : 8;
		T* ndata = new T[ncap];
		for (int i = 0; i < m_size; i++)
		{
			ndata[i] = std::move(m_data[i]);
		}
		if (m_data)
			delete[](m_data);
		m_data = ndata;
		m_cap = ncap;
	}
};

template<typename T>
struct BucketList {
	#define BUCKET_SIZE 16
	#define BUCKET_FLAGS_FULL 0xFFFF  // match the bucket size in the number of bits
	struct Bucket {
		T elements[BUCKET_SIZE];
		uint16_t occupied_flags;  // at least as many bits as bucket size
	};

	// due to the fact that the dynamic array can relocate, the pointers are not stable but slot ids are
	DArray<Bucket> buckets;

	int count() const
	{
		int total = 0;
		for (const auto& bucket : buckets)
		{
			total += pop_count(bucket.occupied_flags);
		}

		return total;
	}

	int add(const T& elem)
	{
		int bucket_index = 0;
		for (auto& bucket : buckets)
		{
			auto flags = bucket.occupied_flags;
			if (flags == BUCKET_FLAGS_FULL)
			{
				bucket_index += 1;
				continue;
			}

			u16 inverse_flags = ~flags;
			int index = lsb_index(inverse_flags);
			bucket.elements[index] = elem;
			bucket.occupied_flags |= BIT(index);

			return bucket_index * BUCKET_SIZE + index;
		}

		int new_bucket_index = buckets.add(Bucket());
		buckets.get_ref(new_bucket_index).elements[0] = elem;
		buckets.get_ref(new_bucket_index).occupied_flags |= BIT(0);
		return new_bucket_index * BUCKET_SIZE + 0;
	}

	void remove(int elem_index)
	{
		int bucket_index = elem_index / BUCKET_SIZE;
		int index = elem_index % BUCKET_SIZE;

		buckets.get_ref(bucket_index).occupied_flags &= ~BIT(index);
	}

	T& get(int elem_index)
	{
		int bucket_index = elem_index / BUCKET_SIZE;
		int index = elem_index % BUCKET_SIZE;

		return buckets.get_ref(bucket_index).elements[index];
	}

	struct Iterator {
		BucketList<T>* list = {};
		int bucket_index = 0;
		int slot_index = 0;

		void next()
		{
			for (int i = bucket_index; i < list->buckets.size(); i++)
			{
				Bucket& bucket = list->buckets.get_ref(i);
				auto flags = bucket.occupied_flags;
				flags &= BUCKET_FLAGS_FULL << (slot_index + 1);  // ignore flags before the point we are looking
				if (flags != 0)
				{
					int index = lsb_index(flags);

					bucket_index = i;
					slot_index = index;
					return;
				}
				else
				{
					slot_index = 0;
				}
			}

			bucket_index = list->buckets.size();
		}

		Iterator& operator++() {
			next();
			return *this;
		}

		T& operator*() {
			return list->buckets.get_ref(bucket_index).elements[slot_index];
		}

		bool operator!=(const Iterator& other) const {
			return bucket_index != other.bucket_index || slot_index != other.slot_index;
		}
	};

	Iterator begin()
	{
		Iterator it = { this, 0, -1 };
		it.next();
		return it;
	}

	Iterator end()
	{
		return { this, buckets.size(), 0 };
	}

	struct ConstIterator {
		const BucketList<T>* list = {};
		int bucket_index = 0;
		int slot_index = 0;

		void next()
		{
			for (int i = bucket_index; i < list->buckets.size(); i++)
			{
				const Bucket& bucket = list->buckets.get_ref(i);
				auto flags = bucket.occupied_flags;
				flags &= BUCKET_FLAGS_FULL << (slot_index + 1);  // ignore flags before the point we are looking
				if (flags != 0)
				{
					int index = lsb_index(flags);

					bucket_index = i;
					slot_index = index;
					return;
				}
				else
				{
					slot_index = 0;
				}
			}

			bucket_index = list->buckets.size();
		}

		ConstIterator& operator++() {
			next();
			return *this;
		}

		const T& operator*() const {
			return list->buckets.get_ref(bucket_index).elements[slot_index];
		}

		bool operator!=(const ConstIterator& other) const {
			return bucket_index != other.bucket_index || slot_index != other.slot_index;
		}
	};

	ConstIterator begin() const
	{
		ConstIterator it = { this, 0, -1 };
		it.next();
		return it;
	}

	ConstIterator end() const
	{
		return { this, buckets.size(), 0 };
	}
};