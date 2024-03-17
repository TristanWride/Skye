#include "ecsmanager.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <format>
#include <vector>
#include <type_traits>
#include <string>
#include <utility>
#include <concepts>
#include <set>
#include <numeric>

TEST(ECS, DefaultConstruction) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>
    >{};

    ASSERT_EQ(ecs.NumComponentSlots(), 3u);
    ASSERT_EQ(ecs.NumEntitySlots(), 0u);
}

TEST(ECS, EntityCreation) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>
    >{};

    auto eid = ecs.NewEntity();
    ASSERT_TRUE(eid);
}

TEST(ECS, BitsetCreation) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>
    >{};

    ASSERT_EQ((decltype(ecs)::MakeComponentBitset<int, float>().to_ulong()), 0b101);
    ASSERT_EQ((decltype(ecs)::MakeComponentBitset<int, double>().to_ulong()), 0b011);
    ASSERT_EQ((decltype(ecs)::MakeComponentBitset<float>().to_ulong()), 0b100);
}

TEST(ECS, StaticComponentCheck) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>,
        BasicCompManager<std::vector<bool>>
    >{};

    ASSERT_TRUE(decltype(ecs)::HasComponentManager<int>());
    ASSERT_TRUE(decltype(ecs)::HasComponentManager<float>());
    ASSERT_TRUE(decltype(ecs)::HasComponentManager<double>());
    ASSERT_TRUE(decltype(ecs)::HasComponentManager<std::vector<bool>>());

    ASSERT_FALSE(decltype(ecs)::HasComponentManager<bool>());
    ASSERT_FALSE(decltype(ecs)::HasComponentManager<std::vector<int>>());
    ASSERT_FALSE(decltype(ecs)::HasComponentManager<std::string>());
}

TEST(ECS, StaticComponentIndices) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>,
        BasicCompManager<std::vector<bool>>
    >{};

    ASSERT_EQ(decltype(ecs)::ComponentIndex<int>(), 0u);
    ASSERT_EQ(decltype(ecs)::ComponentIndex<double>(), 1u);
    ASSERT_EQ(decltype(ecs)::ComponentIndex<float>(), 2u);
    ASSERT_EQ(decltype(ecs)::ComponentIndex<std::vector<bool>>(), 3u);
}

TEST(ECS, RuntimeComponentCheck) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>,
        BasicCompManager<std::vector<int>>
    >{};

    std::vector<EntityId> eids;

    for (auto i = 0u; i < 100u; ++i) {
        eids.emplace_back(ecs.NewEntity().value());
    }

    for (auto i = 0u; i < 100u; ++i) {
        ecs.NewComponent<int>(eids[i], i);
        if (i % 2 == 0) ecs.NewComponent<double>(eids[i], static_cast<double>(i) / 2.0);
        if (i % 3 == 0) ecs.NewComponent<float>(eids[i], static_cast<float>(i) / 7.0f);
        if (i % 5 == 0) ecs.NewComponent<std::vector<int>>(eids[i], i, i * 13);
    }

    for (auto i = 0u; i < 100u; ++i) {
        auto message = std::format("Checking entity {}", i);
        ASSERT_TRUE(ecs.HasComponents<int>(i)) << message;
        if (i %  2 == 0) ASSERT_TRUE((ecs.HasComponents<int, double>(i))) << message;
        if (i %  3 == 0) ASSERT_TRUE((ecs.HasComponents<int, float>(i))) << message;
        if (i %  5 == 0) ASSERT_TRUE((ecs.HasComponents<int, std::vector<int>>(i))) << message;
        if (i %  6 == 0) ASSERT_TRUE((ecs.HasComponents<int, double, float>(i))) << message;
        if (i % 10 == 0) ASSERT_TRUE((ecs.HasComponents<int, double, std::vector<int>>(i))) << message;
        if (i % 15 == 0) ASSERT_TRUE((ecs.HasComponents<int, float, std::vector<int>>(i))) << message;
        if (i % 30 == 0) ASSERT_TRUE((ecs.HasComponents<float, int, std::vector<int>, double>(i))) << message;

        if (i %  2 != 0) ASSERT_FALSE((ecs.HasComponents<double>(i))) << message;
        if (i %  2 != 0) ASSERT_FALSE((ecs.HasComponents<int, double>(i))) << message;
        if (i %  2 != 0) ASSERT_FALSE((ecs.HasComponents<int, double, float>(i))) << message;
        if (i %  2 != 0) ASSERT_FALSE((ecs.HasComponents<int, double, float, std::vector<int>>(i))) << message;
    }
}

TEST(ECS, ValidEntity) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>
    >{};

    EXPECT_FALSE(ecs.IsValidEntity(15u));

    std::vector<EntityId> eids;

    for (auto i = 0u; i < 20u; ++i) {
        eids.emplace_back(ecs.NewEntity().value());
        EXPECT_TRUE(ecs.IsValidEntity(eids[i]));
    }

    for (auto i = 0u; i < 20u; ++i) {
        ecs.DeleteEntity(eids[i]);
        EXPECT_FALSE(ecs.IsValidEntity(eids[i]));
    }
}

template <typename Comp>
struct MockCompManager {
    using ComponentType = Comp;

    MOCK_METHOD(void, New, (EntityId, const Comp&));
    MOCK_METHOD(bool, Delete, (EntityId));
};

TEST(ECS, EntityDeletion) {
    using ::testing::StrictMock;
    using ::testing::Return;

    auto ecs = ECSManager<
        StrictMock<MockCompManager<int>>,
        StrictMock<MockCompManager<double>>
    >{};

    std::vector<EntityId> eids;

    for (auto i = 0u; i < 10u; ++i) {
        eids.emplace_back(ecs.NewEntity().value());
    }

    for (auto i = 0u; i < 10u; ++i) {
        EXPECT_CALL(ecs.GetComponentManager<int>(), New(eids[i], i)).Times(1);
        ecs.NewComponent<int>(eids[i], i);
        if (i % 2 == 0) {
            auto val = static_cast<double>(i) / 2.0;
            EXPECT_CALL(ecs.GetComponentManager<double>(), New(eids[i], val));
            ecs.NewComponent<double>(eids[i], val);
        }
    }

    for (auto i = 0u; i < 10u; ++i) {
        EXPECT_CALL(ecs.GetComponentManager<int>(), Delete(eids[i]));
        if (i % 2 == 0) {
            EXPECT_CALL(ecs.GetComponentManager<double>(), Delete(eids[i]))
                .Times(1)
                .WillOnce(Return(true));
        }
        ecs.DeleteEntity(i);
    }

    for (auto i = 0u; i < 10u; ++i) {
        auto message = std::format("Checking entity {}", i);
        EXPECT_FALSE(ecs.IsValidEntity(i));
    }
}

template <typename Comp>
struct RefCheckCompManagerMock {
    using ComponentType = Comp;

    MOCK_METHOD(void, New, (EntityId, const Comp&));
    MOCK_METHOD(Comp&, LValRefGet, (EntityId));
    MOCK_METHOD(Comp&&, RValRefGet, (EntityId));
    MOCK_METHOD(const Comp&, ConstLValRefGet, (EntityId), (const));

    Comp& Get(EntityId id)& { return LValRefGet(id); }
    const Comp& Get(EntityId id) const& { return ConstLValRefGet(id); }
    Comp&& Get(EntityId id)&& { return RValRefGet(id); }
};

TEST(ECS, ComponentAccess) {
    using ::testing::StrictMock;
    using ::testing::Return;
    using ::testing::ReturnRef;
    using ::testing::Return;

    auto ecs = ECSManager<
        StrictMock<RefCheckCompManagerMock<int>>,
        StrictMock<RefCheckCompManagerMock<double>>
    >{};

    auto eid = ecs.NewEntity().value();

    EXPECT_CALL(ecs.GetComponentManager<int>(), New(eid, 4)).Times(1);
    ecs.NewComponent<int>(eid, 4);

    int tmp = 0;
    EXPECT_CALL(ecs.GetComponentManager<int>(), LValRefGet(eid)).Times(1).WillOnce(ReturnRef(tmp));
    auto&& comp = ecs.GetComponent<int>(eid);
    EXPECT_TRUE((std::same_as<decltype(comp), int&>));

    comp = 17;
    EXPECT_EQ(tmp, comp);

    EXPECT_CALL(ecs.GetComponentManager<int>(), ConstLValRefGet(eid)).Times(1).WillOnce(ReturnRef(tmp));
    auto&& constcomp = std::as_const(ecs).GetComponent<int>(eid);
    EXPECT_TRUE((std::same_as<decltype(constcomp), const int&>));

    EXPECT_EQ(constcomp, comp);
    tmp = 31;
    EXPECT_EQ(constcomp, tmp);
    EXPECT_EQ(comp, tmp);

    {
        auto ecs0 = ECSManager<
            StrictMock<RefCheckCompManagerMock<int>>,
            StrictMock<RefCheckCompManagerMock<double>>
        >{};

        auto eid0 = ecs0.NewEntity().value();

        using ::testing::ByMove;

        EXPECT_CALL(ecs0.GetComponentManager<int>(), RValRefGet(eid0)).Times(1).WillOnce(Return(ByMove(2)));
        auto&& comprval = std::move(ecs0).GetComponent<int>(eid0);

        EXPECT_TRUE((std::same_as<decltype(comprval), int&&>));
    }
}

TEST(ECS, ComponentCreation) {
    auto ecs = ECSManager<
        BasicCompManager<int>,
        BasicCompManager<double>,
        BasicCompManager<float>,
        BasicCompManager<std::vector<bool>>
    >{};

    auto eid = ecs.NewEntity().value();
    ecs.NewComponent<double>(eid, 654.321);

    ASSERT_EQ(ecs.GetComponent<double>(eid), 654.321);

    ecs.NewComponent<std::vector<bool>>(eid);

    ASSERT_EQ(ecs.GetComponent<double>(eid), 654.321);
    ASSERT_EQ(ecs.GetComponent<std::vector<bool>>(eid).size(), 0u);
}

TEST(ECS, RangedComponentAccess) {
    using ::testing::StrictMock;
    using ::testing::Return;
    using ::testing::ReturnRef;
    using ::testing::_;

    auto ecs = ECSManager<
        StrictMock<RefCheckCompManagerMock<int>>,
        StrictMock<RefCheckCompManagerMock<double>>
    >{};

    std::vector<EntityId> eids;

    for (auto i = 0u; i < 10u; ++i) {
        eids.emplace_back(ecs.NewEntity().value());
    }

    for (auto i = 0u; i < 10u; ++i) {
        EXPECT_CALL(ecs.GetComponentManager<int>(), New(eids[i], i)).Times(1);
        ecs.NewComponent<int>(eids[i], i);

        if (i % 2 == 0) {
            auto val = static_cast<double>(i) / 2.0;
            EXPECT_CALL(ecs.GetComponentManager<double>(), New(eids[i], val)).Times(1);
            ecs.NewComponent<double>(eids[i], val);
        }
    }

    int tmpint = 5135918203;
    double tmpdouble = 3246.34623476;
    EXPECT_CALL(ecs.GetComponentManager<int>(), LValRefGet(_))
        .Times(5)
        .WillRepeatedly(ReturnRef(tmpint));

    EXPECT_CALL(ecs.GetComponentManager<double>(), LValRefGet(_))
        .Times(5)
        .WillRepeatedly(ReturnRef(tmpdouble));

    {
        auto expected = std::set<EntityId>{0, 2, 4, 6, 8};
        auto actual = std::set<EntityId>{};

        for (auto [id, intVal, doubleVal] : ecs.GetAll<int, double>()) {
            EXPECT_TRUE((std::convertible_to<decltype(id), EntityId>));
            EXPECT_TRUE((std::same_as<decltype(intVal), int&>));
            EXPECT_TRUE((std::same_as<decltype(doubleVal), double&>));
            actual.emplace(id);

            tmpint = (id * 17) + 51;
            EXPECT_EQ(intVal, tmpint) << "Can read component as reference";

            doubleVal = static_cast<double>(id) * 2.512 + 8.9325098;
            EXPECT_EQ(doubleVal, tmpdouble) << "Can write to component as reference";
        }

        EXPECT_EQ(expected, actual);
    }

    EXPECT_CALL(ecs.GetComponentManager<int>(), ConstLValRefGet(_))
        .Times(10)
        .WillRepeatedly(ReturnRef(tmpint));

    {
        auto expected = std::set<EntityId>{};
        auto actual = std::set<EntityId>{};

        std::generate_n(std::inserter(expected, expected.end()), 10, [&] { return expected.size(); });

        for (auto [id, intVal] : std::as_const(ecs).GetAll<int>()) {
            EXPECT_TRUE((std::convertible_to<decltype(id), EntityId>));
            EXPECT_TRUE((std::same_as<decltype(intVal), const int&>));
            actual.emplace(id);

            tmpint = (id * 331) -  43;
            EXPECT_EQ(intVal, tmpint) << "Can read component as reference";
        }

        EXPECT_EQ(expected, actual);
    }
}

