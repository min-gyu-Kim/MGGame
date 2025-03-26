#include <gtest/gtest.h>

#include <core/Memory/StackMemory.hpp>

class StackMemoryTest : public testing::Test
{
protected:
    StackMemoryTest() :
        m_sm1()
    {}

    core::StackMemory m_sm1;
};

TEST_F(StackMemoryTest, BasicTest)
{
    const std::uint32_t capacity = m_sm1.GetCapacity();

    ASSERT_EQ(capacity, core::StackMemory::DEFAULT_SIZE);

    void* ptr = m_sm1.Alloc(1);
    ASSERT_NE(ptr, nullptr);

    m_sm1.Free(1);
}
