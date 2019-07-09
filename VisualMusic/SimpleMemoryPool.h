
// SimpleMemoryPool.h
#pragma once

#include <exception>
#include <cstddef>
#include <cassert>

#define INT_BITS = (sizeof(int) * 8);

template <class ElemType>
class CSimpleMemoryPool {
public:
	CSimpleMemoryPool();
	CSimpleMemoryPool(size_t nNumberOfElems);
	~CSimpleMemoryPool();

	void Init(size_t nNumberOfElems);
	void Clear();

	ElemType *New();
	void Delete(ElemType *pElem);

protected:

	ElemType *m_Buffer;

	size_t m_NumberOfElems;

	int *m_bit[INT_BITS/2];
	size_t m_bit_n[INT_BITS/2];
	int m_n;

	CSimpleMemoryPool *m_next_pool;
};


template <class ElemType>
inline CSimpleMemoryPool<ElemType>::CSimpleMemoryPool():m_Buffer(0),m_next_pool(0){
	
}

template <class ElemType>
inline CSimpleMemoryPool<ElemType>::CSimpleMemoryPool(size_t nNumberOfElems):m_Buffer(0),m_next_pool(0){
	Init(nNumberOfElems);
}

template <class ElemType>
inline CSimpleMemoryPool<ElemType>::~CSimpleMemoryPool(){
	Clear();
}


template <class ElemType>
inline void CSimpleMemoryPool<ElemType>::Init(size_t nNumberOfElems){
	assert(nNumberOfElems > 1);

	m_NumberOfElems = nNumberOfElems;

	size_t n = nNumberOfElems;
	size_t int_n = 0;
	m_n = 0;
	while (n > 1)
		int_n += m_bit_n[m_n++] = n = (n + (INT_BITS - 1)) / INT_BITS;

	m_Buffer = (ElemType *)new char [nNumberOfElems * sizeof(ElemType) + int_n * sizeof(int)];
	if (!m_Buffer){
		throw std::bad_alloc();
		return;
	}

	m_bit[0] = (int *)(m_Buffer + nNumberOfElems);
	memset(m_bit[0], 0xff, int_n * sizeof(int));
	int bits = (1<<(nNumberOfElems%INT_BITS)) - 1;
	if (bits) m_bit[0][m_bit_n[0]-1] = bits;
	for (int i = 1; i < m_n; i++){
		m_bit[i] = m_bit[i-1] + m_bit_n[i-1];
		bits = (1<<(m_bit_n[i-1]%INT_BITS)) - 1;
		if (bits) m_bit[i][m_bit_n[i]-1] = bits;
	}
}

template <class ElemType>
inline void CSimpleMemoryPool<ElemType>::Clear(){
	if (m_next_pool) delete m_next_pool; m_next_pool = 0;
	if (m_Buffer) delete m_Buffer; m_Buffer = 0;
}


template <class ElemType>
inline ElemType *CSimpleMemoryPool<ElemType>::New(){
	if (!m_bit[m_n-1][0]){
		if (!m_next_pool){
			m_next_pool = new CSimpleMemoryPool(m_NumberOfElems * 2);
			if (!m_next_pool) return 0;
		}
		return m_next_pool->New();
	}

	size_t k[INT_BITS/2];
	int r[INT_BITS/2];
	size_t lmk = 0;
	for (int i = m_n-1; i >= 0; i--){
		k[i] = lmk;
		int mr = 1;
		int mk = 0;
		while (!(m_bit[i][lmk] & mr)){
			mr <<= 1;
			mk ++;
		}
		r[i] = mr;
		lmk = lmk * INT_BITS + mk;
	}
	for (int i = 0; i < m_n; i++)
		if (m_bit[i][k[i]] -= r[i])
			break;
	return m_Buffer + lmk;
}

template <class ElemType>
inline void CSimpleMemoryPool<ElemType>::Delete(ElemType *pElem){
	size_t mk = pElem - m_Buffer;
	if (mk < 0 || mk > m_NumberOfElems){
		m_next_pool->Delete(pElem);
		return;
	}

	int r;

	r = 1<<(mk % INT_BITS);
	mk = mk / INT_BITS;

	assert(!(m_bit[0][mk] & r));
	for (int i = 0; i < m_n; i++){
		if (m_bit[i][mk] & r)
			break;
		m_bit[i][mk] |= r;

		r = 1<<(mk % INT_BITS);
		mk = mk / INT_BITS;
	}
}

