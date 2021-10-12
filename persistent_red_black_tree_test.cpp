#include "persistent_red_black_tree_test.hpp"

#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif
#include <catch/catch.hpp>

typedef PersistentRedBlackTreeTest<int, char> Tree;
typedef Tree::ConstIterator CIterator;
typedef Tree::Version* VersionPtr;
typedef std::pair<CIterator, bool> InsertResult;
typedef std::pair<VersionPtr, bool> DeleteResult;
typedef typename std::pair<int, char> NonConstValueType;

TEST_CASE("Simple Case", "")
{
    Tree tree;
    InsertResult insert_result, insert_result2;
    DeleteResult delete_result, delete_result2;
    std::vector<NonConstValueType> require_values;
    VersionPtr version;

    require_values.reserve(32);

    insert_result = tree.Insert({10, 'a'});
    version = insert_result.first.version();
    require_values.push_back({10, 'a'});
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    insert_result2 = insert_result;

    insert_result = tree.Insert({20, 'a'});
    version = insert_result.first.version();
    require_values.push_back({20, 'a'});
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    insert_result2 = insert_result;

    insert_result = tree.Insert({30, 'a'});
    version = insert_result.first.version();
    require_values.push_back({30, 'a'});
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    insert_result2 = insert_result;

    insert_result = tree.Insert({40, 'a'});
    require_values.push_back({40, 'a'});
    version = insert_result.first.version();
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    insert_result2 = insert_result;

    delete_result = tree.Delete(10);
    require_values.erase(require_values.begin());
    version = delete_result.first;
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    delete_result2 = delete_result;

    delete_result = tree.Delete(10);
    version = delete_result.first;
    REQUIRE_FALSE(delete_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    delete_result2 = delete_result;

    insert_result = tree.Insert({10, 'a'});
    require_values.insert(require_values.begin(), {10, 'a'});
    version = insert_result.first.version();
    REQUIRE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    insert_result2 = insert_result;

    insert_result = tree.Insert({20, 'a'});
    version = insert_result.first.version();
    REQUIRE_FALSE(insert_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));

    delete_result = tree.Delete(40);
    require_values.erase(require_values.begin() + 3);
    version = delete_result.first;
    REQUIRE(delete_result.second);
    REQUIRE(tree.CheckTreeValid(version, require_values));
    delete_result2 = delete_result;

    REQUIRE(tree.CheckTreeValidAllVersion());
}

TEST_CASE("figure 13.8 based (mainly for delete and remove version)", "")
{
    Tree tree;
    std::vector<InsertResult> insert_results;
    std::vector<NonConstValueType> require_values;
    VersionPtr version;
    std::vector<DeleteResult> delete_results;
    bool removed;

    insert_results.push_back(tree.Insert({40, 'a'}));// i 0
    insert_results.push_back(tree.Insert({30, 'a'}));
    insert_results.push_back(tree.Insert({80, 'a'}));
    insert_results.push_back(tree.Insert({20, 'a'}));
    insert_results.push_back(tree.Insert({70, 'a'}));
    insert_results.push_back(tree.Insert({100, 'a'}));// i 5
    insert_results.push_back(tree.Insert({18, 'a'}));
    insert_results.push_back(tree.Insert({22, 'a'}));
    insert_results.push_back(tree.Insert({65, 'a'}));
    insert_results.push_back(tree.Insert({75, 'a'}));
    insert_results.push_back(tree.Insert({98, 'a'}));// i 10
    insert_results.push_back(tree.Insert({110, 'a'}));
    insert_results.push_back(tree.Insert({26, 'a'}));
    insert_results.push_back(tree.Insert({93, 'a'}));
    insert_results.push_back(tree.Insert({25, 'a'}));
    insert_results.push_back(tree.Insert({94, 'a'}));// i 15
    insert_results.push_back(tree.Insert({24, 'a'}));
    insert_results.push_back(tree.Insert({96, 'a'}));
    delete_results.push_back(tree.Delete(30));// d 0
    delete_results.push_back(tree.Delete(80));
    delete_results.push_back(tree.Delete(40));
    insert_results.push_back(tree.Insert({69, 'a'}));
    insert_results.push_back(tree.Insert({130, 'a'}));
    insert_results.push_back(tree.Insert({69, 'b'}, 
        insert_results[17].first.version()));// i 20
    insert_results.push_back(tree.Insert({130, 'b'}));
    insert_results.push_back(tree.InsertOrAssign({75, 'c'}));
    insert_results.push_back(tree.InsertOrAssign({75, 'd'}, insert_results[19].first.version()));
    insert_results.push_back(tree.InsertOrAssign({150, 'd'}));

    removed = false;
    while (true)
    {
        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {30, 'a'}, {40, 'a'},
            {65, 'a'}, {70, 'a'}, {75, 'a'}, {80, 'a'}, {100, 'a'},
        };
        REQUIRE(tree.CheckTreeValid(insert_results[9].first.version(), require_values));
        
        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {30, 'a'}, {40, 'a'}, {65, 'a'}, {70, 'a'},
            {75, 'a'}, {80, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}
        };
        REQUIRE(tree.CheckTreeValid(insert_results[17].first.version(), require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {40, 'a'}, {65, 'a'}, {70, 'a'},
            {75, 'a'}, {80, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}
        };
        REQUIRE(tree.CheckTreeValid(delete_results[0].first, require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {40, 'a'}, {65, 'a'}, {70, 'a'},
            {75, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}
        };
        REQUIRE(tree.CheckTreeValid(delete_results[1].first, require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {65, 'a'}, {70, 'a'},
            {75, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}
        };
        REQUIRE(tree.CheckTreeValid(delete_results[2].first, require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {65, 'a'}, {69, 'a'}, {70, 'a'},
            {75, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}, {130, 'a'}
        };
        REQUIRE(tree.CheckTreeValid(insert_results[19].first.version(), require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {30, 'a'}, {40, 'a'}, {65, 'a'}, {69, 'b'}, {70, 'a'},
            {75, 'a'}, {80, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}, {130, 'b'}
        };
        REQUIRE(tree.CheckTreeValid(insert_results[21].first.version(), require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {30, 'a'}, {40, 'a'}, {65, 'a'}, {69, 'b'}, {70, 'a'},
            {75, 'c'}, {80, 'a'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}, {130, 'b'}
        };
        REQUIRE(tree.CheckTreeValid(insert_results[22].first.version(), require_values));

        require_values = {
            {18, 'a'}, {20, 'a'}, {22, 'a'}, {24, 'a'}, {25, 'a'},
            {26, 'a'}, {65, 'a'}, {69, 'a'}, {70, 'a'},
            {75, 'd'}, {93, 'a'}, {94, 'a'}, {96, 'a'},
            {98, 'a'}, {100, 'a'}, {110, 'a'}, {130, 'a'}, {150, 'd'}
        };
        REQUIRE(tree.CheckTreeValid(insert_results[24].first.version(), require_values));

        REQUIRE(tree.At(80, insert_results[10].first.version()) == 'a');
        REQUIRE(tree.At(80, insert_results[15].first.version()) == 'a');
        REQUIRE(tree.At(130, insert_results[19].first.version()) == 'a');
        REQUIRE(tree.At(130, insert_results[21].first.version()) == 'b');
        REQUIRE(tree.At(75, insert_results[22].first.version()) == 'c');
        REQUIRE(tree.At(75, insert_results[23].first.version()) == 'd');
        REQUIRE(tree.At(130, insert_results[22].first.version()) == 'b');
        REQUIRE(tree.At(130, insert_results[23].first.version()) == 'a');

        REQUIRE(tree.Find(130, insert_results[21].first.version()) 
            != tree.Find(130, insert_results[19].first.version()));
        REQUIRE(tree.Find(130, insert_results[22].first.version()) 
            != tree.Find(130, insert_results[23].first.version()));
        REQUIRE(tree.Find(130, insert_results[19].first.version())->first == 130);
        REQUIRE(tree.Find(130, insert_results[21].first.version())->first == 130);
        REQUIRE(tree.Find(130, insert_results[19].first.version())->second == 'a');
        REQUIRE(tree.Find(130, insert_results[21].first.version())->second == 'b');
        REQUIRE(tree.Find(130, insert_results[15].first.version()) == tree.CEnd());

        REQUIRE(tree.CheckTreeValidAllVersion());

        if (removed == false)
        {
            tree.RemoveVersion(insert_results[0].first.version());
            tree.RemoveVersion(insert_results[1].first.version());
            tree.RemoveVersion(insert_results[11].first.version());

            REQUIRE(tree.CheckTreeValidAllVersion());

            removed = true;
        }
        else
        {
            break;
        }
    }

}
