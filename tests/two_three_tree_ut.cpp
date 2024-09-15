#include "gtest/gtest.h"

#include "src/public.h"
#include "src/two_three_tree.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <set>

namespace NVis {

namespace {
enum class QueryType {
    Insert,
    Erase,
    Check,
};
struct Query {
    Key key;
    QueryType type;
};
} // namespace

TEST(TreeSimple, InsertsAndErases) {
    TwoThreeTree tree;
    tree.Insert(5);
    tree.Insert(2);
    tree.Insert(7);
    tree.Insert(-221);
    tree.Insert(122334);
    EXPECT_TRUE(tree.Contains(5));
    EXPECT_FALSE(tree.Contains(1));
    tree.Erase(5);
    EXPECT_FALSE(tree.Contains(5));
    EXPECT_TRUE(tree.Contains(-221));
}

TEST(TreeSimple, RepeatInsert) {
    TwoThreeTree tree;
    for (int x = 0; x < 10; ++x) {
        tree.Insert(x);
    }
    EXPECT_FALSE(tree.Insert(5));
    EXPECT_FALSE(tree.Insert(5));

    EXPECT_TRUE(tree.Contains(5));
    EXPECT_TRUE(tree.Erase(5));
    EXPECT_FALSE(tree.Contains(5));
    EXPECT_TRUE(tree.Insert(5));
}

TEST(TreeSimple, RepeatErase) {
    TwoThreeTree tree;
    for (int x = 0; x < 10; ++x) {
        tree.Insert(x);
    }
    EXPECT_TRUE(tree.Erase(6));
    EXPECT_FALSE(tree.Erase(6));
    EXPECT_FALSE(tree.Erase(6));
    for (int x = 0; x < 10; ++x) {
        if (x != 6) {
            EXPECT_TRUE(tree.Contains(x));
        } else {
            EXPECT_FALSE(tree.Contains(x));
        }
    }
}

TEST(TreeBig, LinearPass) {
    constexpr int kC = 1000;
    TwoThreeTree tree;
    for (int x = -kC; x < kC; ++x) {
        EXPECT_TRUE(tree.Insert(x));
    }
    for (int x = -kC; x < kC; ++x) {
        EXPECT_FALSE(tree.Insert(x));
    }
    for (int x = -kC; x < kC; ++x) {
        EXPECT_TRUE(tree.Contains(x));
    }
    for (int x = -kC; x < kC; ++x) {
        EXPECT_TRUE(tree.Erase(x));
    }
    for (int x = -kC; x < kC; ++x) {
        EXPECT_FALSE(tree.Erase(x));
    }
}

TEST(TreeBig, RandomPreInserts) {
    // Firstly, insert all keys. Secondly, call `Erase` and `Contains` in random order.
    constexpr int kSeed = 22;
    constexpr int kIters = 1000;
    constexpr Key kNumberLimit = 1'000'000;
    TwoThreeTree tree;
    std::set<Key> set;
    std::vector<Query> actions;
    std::mt19937 mt(kSeed);
    std::uniform_int_distribution<Key> rng(0, kNumberLimit);
    for (int i = 0; i < kIters; ++i) {
        Key key = rng(mt);
        tree.Insert(key);
        set.emplace(key);
        actions.emplace_back(Query{.key = key, .type = QueryType::Check});
        actions.emplace_back(Query{.key = key, .type = QueryType::Erase});
    }
    std::shuffle(actions.begin(), actions.end(), mt);
    for (auto& [key, type] : actions) {
        switch (type) {
        case QueryType::Erase:
            tree.Erase(key);
            set.erase(key);
            break;
        case QueryType::Check:
            EXPECT_EQ(tree.Contains(key), set.contains(key));
            break;
        case QueryType::Insert:
            FAIL();
            break;
        }
    }
}

TEST(TreeBig, RandomLifetimes) {
    // One insert, one erase, one check. Random order.
    constexpr int kSeed = 22;
    constexpr int kIters = 1000;
    constexpr Key kNumberLimit = 1'000'000;
    std::vector<Query> actions;
    std::mt19937 mt(kSeed);
    std::uniform_int_distribution<Key> rng(0, kNumberLimit);
    for (int i = 0; i < kIters; ++i) {
        Key key = rng(mt);
        actions.emplace_back(Query{.key = key, .type = QueryType::Insert});
        actions.emplace_back(Query{.key = key, .type = QueryType::Check});
        actions.emplace_back(Query{.key = key, .type = QueryType::Erase});
    }
    std::shuffle(actions.begin(), actions.end(), mt);

    TwoThreeTree tree;
    std::set<Key> set;
    for (auto& [key, type] : actions) {
        switch (type) {
        case QueryType::Erase:
            tree.Erase(key);
            set.erase(key);
            break;
        case QueryType::Check:
            EXPECT_EQ(tree.Contains(key), set.contains(key));
            break;
        case QueryType::Insert:
            tree.Insert(key);
            set.insert(key);
            break;
        }
    }
}

} // namespace NVis
