#include <cassert>

#include <core/Memory/StackMemory.hpp>

using namespace core;

StackMemory::StackMemory(size_t size) :
    m_topOffset(0),
    m_capacity(size)
{
    m_memory = new unsigned char[size];
}

StackMemory::~StackMemory()
{
    assert(m_memory != nullptr);
    delete[] m_memory;
}

void* StackMemory::Alloc(size_t size)
{
    // 매우 큰 사이즈는 따로 new 하자
    assert(size < DEFAULT_SIZE);
    assert(m_topOffset < m_capacity);
    assert(m_topOffset >= 0);

    const int32_t remainSize = this->GetRemainSize();
    if(remainSize < size)
        return nullptr;

    void* returnMemory = static_cast<void*>(&m_memory[m_topOffset]);
    m_topOffset += size;

    return returnMemory;
}

void StackMemory::Free(size_t size)
{
    assert(this->GetUsedSize() >= size);
    m_topOffset -= size;
}
