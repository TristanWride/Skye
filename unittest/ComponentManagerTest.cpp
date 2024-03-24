#include "ecsmanager.h"

#include <glm/glm.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <utility>

template <typename CompManagerType>
struct CompManagerFixture : ::testing::Test {};

using CompManagerTypes = ::testing::Types<BasicCompManager<int>, DynamicCompManager<int>>;

TYPED_TEST_SUITE(CompManagerFixture, CompManagerTypes);

TYPED_TEST(CompManagerFixture, ComponentCreation) {
    TypeParam compManager{};
    auto&& component = compManager.New(8u, 154);
    EXPECT_TRUE((std::same_as<decltype(component), typename TypeParam::ComponentType&>)) << "New should return reference to component type";

    component = 133;
    auto&& componentConstRef = std::as_const(compManager).Get(8u);
    EXPECT_TRUE((std::same_as<decltype(componentConstRef), const typename TypeParam::ComponentType&>)) << "New on const manager should return const reference to component type";
    EXPECT_EQ(component, componentConstRef) << "Const ref should be to the correct object";

    component = 67;
    EXPECT_EQ(component, componentConstRef) << "Const ref should be to the correct object";
}

TYPED_TEST(CompManagerFixture, ComponentCreationMulti) {
    TypeParam compManager{};
    auto&& component1 = compManager.New(8u, 154);
    EXPECT_TRUE((std::same_as<decltype(component1), typename TypeParam::ComponentType&>)) << "New should return reference to component type";
    auto&& component2 = compManager.New(10u, 114);
    EXPECT_TRUE((std::same_as<decltype(component1), typename TypeParam::ComponentType&>)) << "New should return reference to component type";

    component1 = 133;
    auto&& componentConstRef1 = std::as_const(compManager).Get(8u);
    EXPECT_TRUE((std::same_as<decltype(componentConstRef1), const typename TypeParam::ComponentType&>)) << "New on const manager should return const reference to component type";
    EXPECT_EQ(component1, componentConstRef1) << "Const ref should be to the correct object";

    component2 = 67;
    auto&& componentConstRef2 = std::as_const(compManager).Get(10u);
    EXPECT_TRUE((std::same_as<decltype(componentConstRef2), const typename TypeParam::ComponentType&>)) << "New on const manager should return const reference to component type";
    EXPECT_EQ(component2, componentConstRef2) << "Const ref should be to the correct object";
}

TYPED_TEST(CompManagerFixture, ComponentDeletion) {
    TypeParam compManager{};
    auto&& component1 = compManager.New(8U, 154);
    EXPECT_TRUE((std::same_as<decltype(component1), typename TypeParam::ComponentType&>)) << "New should return reference to component type";
    auto&& component2 = compManager.New(10U, 114);
    EXPECT_TRUE((std::same_as<decltype(component2), typename TypeParam::ComponentType&>)) << "New should return reference to component type";

    EXPECT_TRUE(compManager.HasEntity(8U)) << "Component should exist";
    EXPECT_TRUE(compManager.HasEntity(10U)) << "Component should exist";

    EXPECT_TRUE(compManager.Delete(8U)) << "Component should be deleted successfully";
    
    EXPECT_FALSE(compManager.HasEntity(8U)) << "Component should not exist after being deleted";
    EXPECT_TRUE(compManager.HasEntity(10U)) << "Component should still exist";

    EXPECT_FALSE(compManager.Delete(8U)) << "Failed deletion should return false";
    
    EXPECT_FALSE(compManager.HasEntity(8U)) << "Component should not exist after failed deletion";
    EXPECT_TRUE(compManager.HasEntity(10U)) << "Component should still exist after failed deletion";

    EXPECT_TRUE(compManager.Delete(10U)) << "Successful deletion should return true";
    
    EXPECT_FALSE(compManager.HasEntity(8U)) << "Component should not exist after deletion";
    EXPECT_FALSE(compManager.HasEntity(10U)) << "Component should not exist after deletion";

    EXPECT_FALSE(compManager.Delete(10U)) << "Failed deletion should return false";
}

TEST(DynamicCompManager, CanDoPolymorphism) {
    struct Base {
        virtual auto IsBase() const noexcept -> bool { return true; }
    };
    struct Derived : public Base {
        auto IsBase() const noexcept -> bool override { return false; }
    };

    DynamicCompManager<Base> compManager{};

    compManager.New(5U, Base{});
    compManager.New(9U, Derived{});
    
    EXPECT_TRUE(compManager.Get(5U).IsBase());
    EXPECT_FALSE(compManager.Get(9U).IsBase());
}