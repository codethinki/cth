#include "cth/utility.hpp"

#include "cth/test.hpp"

#include <vector>


namespace cth {
namespace {
    /**
     * owns an id, the destructor logs it
     */
    struct Resource {
        explicit constexpr Resource(int id, std::vector<int>* destroyed) : id{id}, destroyed{destroyed} {}
        constexpr ~Resource() { if(id != 0) destroyed->push_back(id); }

        int id;
        std::vector<int>* destroyed;

        constexpr Resource(Resource&& other) noexcept : id{std::exchange(other.id, 0)}, destroyed{other.destroyed} {}
        constexpr Resource& operator=(Resource&& other) noexcept { return reconstruct_at(*this, std::move(other)); }
    };

    /**
     * const member -> default move assignment is deleted
     */
    struct ConstMember {
        int const id;
    };

    struct Polymorphic {
        virtual ~Polymorphic() = default;
    };

    struct ThrowingMove {
        ThrowingMove() = default;
        ThrowingMove(ThrowingMove&&) {}
    };

    template<class T>
    concept reconstructible = requires(T& object, T&& other) { reconstruct_at(object, std::move(other)); };
}

CTH_TEST(reconstruct_at, destroys_and_moves_other) {
    std::vector<int> destroyed{};
    Resource resource{1, &destroyed};
    Resource other{2, &destroyed};

    auto const& result = reconstruct_at(resource, std::move(other));

    EXPECT_EQ(&result, &resource);
    EXPECT_EQ(resource.id, 2);
    EXPECT_EQ(other.id, 0);
    EXPECT_EQ(destroyed, std::vector{1});
}

CTH_TEST(reconstruct_at, self_is_no_op) {
    std::vector<int> destroyed{};
    Resource resource{1, &destroyed};

    auto const& result = reconstruct_at(resource, std::move(resource));

    EXPECT_EQ(&result, &resource);
    EXPECT_EQ(resource.id, 1);
    EXPECT_TRUE(destroyed.empty());
}

CTH_TEST(reconstruct_at, as_move_assignment) {
    std::vector<int> destroyed{};
    {
        std::vector<Resource> resources{};
        resources.emplace_back(1, &destroyed);
        resources.emplace_back(2, &destroyed);
        resources.emplace_back(3, &destroyed);

        resources.erase(resources.begin());

        EXPECT_EQ(destroyed, std::vector{1});
        ASSERT_EQ(resources.size(), 2);
        EXPECT_EQ(resources[0].id, 2);
        EXPECT_EQ(resources[1].id, 3);
    }
    EXPECT_EQ(destroyed, (std::vector{1, 2, 3}));
}

CTH_TEST(reconstruct_at, const_member) {
    ConstMember constMember{1};
    ConstMember other{2};

    reconstruct_at(constMember, std::move(other));

    EXPECT_EQ(constMember.id, 2);
}

CTH_TEST(reconstruct_at, constexpr) {
    constexpr auto result = [] {
        std::vector<int> destroyed{};
        Resource resource{1, &destroyed};
        Resource other{2, &destroyed};

        resource = std::move(other);

        return resource.id * 10 + destroyed[0];
    }();

    static_assert(result == 21);
}

CTH_TEST(reconstruct_at, constraints) {
    static_assert(reconstructible<Resource>);
    static_assert(reconstructible<ConstMember>);
    static_assert(!reconstructible<Polymorphic>);
    static_assert(!reconstructible<ThrowingMove>);
    static_assert(!reconstructible<Resource const>);
}
}
