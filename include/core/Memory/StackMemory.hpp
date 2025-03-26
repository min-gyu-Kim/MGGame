#pragma once

#include <stddef.h>
#include <cstdint>

namespace core
{
    /**
     * @brief 스택 구조의 메모리, 힙이 아닌 스택에서 큰 사이즈를 잠시 쓰고 반납 하는 용도로 사용
     * Thread Safe 하지 않음, TLS로 쓰레드 로컬로 사용 권장
     */
    class StackMemory
    {
    public:
        enum
        {
            DEFAULT_SIZE = 100000
        };
    public:
        StackMemory(size_t size = DEFAULT_SIZE);
        ~StackMemory();

        void* Alloc(size_t size);
        void Free(size_t size);

        int32_t GetCapacity() const { return m_capacity; }
        int32_t GetRemainSize() const { return m_capacity - m_topOffset; }
        int32_t GetUsedSize() const { return m_topOffset; }

    private:
        unsigned char* m_memory;
        int32_t m_capacity;
        int32_t m_topOffset;
    };
}
