#include "CppUTest/TestHarness.h"
#include "data/s_list.h"
#include "log/log.h"
#include <stddef.h>

TEST_GROUP(base)
{
};

TEST(base, node_init)
{
    S_List_Node node;
    node.next = (S_List_Node*) -5;

    DATA_S_List_Node_init(&node);

    POINTERS_EQUAL(NULL, node.next);
}

TEST(base, list_init)
{
    S_List list;
    list.head = (S_List_Node *) -1;
    list.tail = (S_List_Node *) -2;
    list.len  = 45;

    DATA_S_List_init(&list);

    POINTERS_EQUAL(NULL, list.head);
    POINTERS_EQUAL(NULL, list.tail);
    CHECK_EQUAL(0, list.len);
}

TEST_GROUP(nominal)
{
    S_List list;
    S_List_Node node;

    void setup()
    {
        DATA_S_List_init(&list);
        CHECK_EQUAL(0, DATA_S_List_Node_init(&node));
    }

    void teardown()
    {
        DATA_S_List_deinit(&list);
    }
};

TEST(nominal, append_onto_empty)
{
    POINTERS_EQUAL(NULL, list.head);
    POINTERS_EQUAL(NULL, list.tail);
    CHECK_EQUAL (0, list.len);

    DATA_S_List_append(&list, &node);

    POINTERS_EQUAL(NULL,  node.next);
    POINTERS_EQUAL(&node, list.head);
    POINTERS_EQUAL(&node, list.tail);
    CHECK_EQUAL (1, list.len);
}

TEST(nominal, insert_into_empty)
{
    POINTERS_EQUAL(NULL, list.head);
    POINTERS_EQUAL(NULL, list.tail);
    CHECK_EQUAL (0, list.len);

    DATA_S_List_insert(&list, &node, 0);

    POINTERS_EQUAL(NULL,  node.next);
    POINTERS_EQUAL(&node, list.head);
    POINTERS_EQUAL(&node, list.tail);
    CHECK_EQUAL (1, list.len);
}

TEST(nominal, prepend_onto_empty)
{
    POINTERS_EQUAL(NULL, list.head);
    POINTERS_EQUAL(NULL, list.tail);
    CHECK_EQUAL (0, list.len);

    CHECK_EQUAL(0, DATA_S_List_prepend(&list, &node));

    POINTERS_EQUAL(NULL,  node.next);
    POINTERS_EQUAL(&node, list.head);
    POINTERS_EQUAL(&node, list.tail);
    CHECK_EQUAL (1, list.len);
}

TEST(nominal, pop_last)
{
    DATA_S_List_append(&list, &node);

    POINTERS_EQUAL(NULL,  node.next);
    POINTERS_EQUAL(&node, list.head);
    POINTERS_EQUAL(&node, list.tail);
    CHECK_EQUAL (1, list.len);

    POINTERS_EQUAL(&node,  DATA_S_List_pop(&list));

    POINTERS_EQUAL(NULL, list.head);
    POINTERS_EQUAL(NULL, list.tail);
    CHECK_EQUAL (0, list.len);
}


TEST_GROUP(nominal_large)
{
    S_List list;

    typedef struct _test_struct
    {
        uint8_t id;
        uint8_t dummy[15];
        S_List_Node node;
    } Test_Struct;

    Test_Struct structs[5];
    const uint8_t structs_len = 5;

    void setup()
    {
        LOG_init();
        DATA_S_List_init(&list);
        for (uint8_t n_iter = 0; n_iter < structs_len; n_iter++)
        {
            structs[n_iter].id = n_iter;
            memset(&structs[n_iter].dummy[0], 0, sizeof(structs[n_iter].dummy));
            CHECK_EQUAL(0, DATA_S_List_Node_init(&structs[n_iter].node));
        }
    }

    void teardown()
    {
        DATA_S_List_deinit(&list);
        LOG_destory();
    }
};

TEST(nominal_large, append_and_pop)
{   
    for (uint8_t n_iter = 0; n_iter < structs_len; n_iter++)
    {
        CHECK_EQUAL(n_iter, list.len);
        CHECK_EQUAL(0, DATA_S_List_append(&list, &structs[n_iter].node));
    }

    CHECK_EQUAL(structs_len, list.len);

    S_List_Node *iter = list.head;
    uint8_t n_iter = 0;
    for (; n_iter < structs_len - 1; n_iter++)
    {
        POINTERS_EQUAL(&structs[n_iter].node, iter);
        POINTERS_EQUAL(&structs[n_iter+1].node, iter->next);
        iter = iter->next;
    }
    POINTERS_EQUAL(&structs[n_iter].node, iter);
    POINTERS_EQUAL(NULL, iter->next);

    iter = list.head;
    n_iter = 0;
    S_List_Node *pop;
    for(; n_iter < structs_len; n_iter++)
    {
        POINTERS_EQUAL(list.head, iter);
        S_List_Node *next = iter->next;
        pop = DATA_S_List_pop(&list);
        POINTERS_EQUAL(iter, pop);
        iter = next;
    }
    POINTERS_EQUAL(NULL, iter);
    POINTERS_EQUAL(NULL, DATA_S_List_pop(&list));
}


TEST(nominal_large, append_and_insert)
{   
    //Append 0,1,3,4
    for (uint8_t n_iter = 0; n_iter < 2; n_iter++)
    {
        CHECK_EQUAL(n_iter, list.len);
        CHECK_EQUAL(0, DATA_S_List_append(&list, &structs[n_iter].node));
        POINTERS_EQUAL(&structs[n_iter], DATA_LIST_GET_OBJ(list.tail, Test_Struct, node));
    }
    for (uint8_t n_iter = 3; n_iter < structs_len; n_iter++)
    {
        CHECK_EQUAL(n_iter - 1, list.len);
        CHECK_EQUAL(0, DATA_S_List_append(&list, &structs[n_iter].node));
        POINTERS_EQUAL(&structs[n_iter], DATA_LIST_GET_OBJ(list.tail, Test_Struct, node));
    }

    // Confirm order
    S_List_Node *iter = list.head;
    for (uint8_t n_iter = 0; n_iter < 2; n_iter++)
    {
        Test_Struct *ts = DATA_LIST_GET_OBJ(iter, Test_Struct, node);
        POINTERS_EQUAL(n_iter, ts->id);
        iter = iter->next;
    }
    for (uint8_t n_iter = 3; n_iter < structs_len; n_iter++)
    {
        Test_Struct *ts = DATA_LIST_GET_OBJ(iter, Test_Struct, node);
        POINTERS_EQUAL(n_iter, ts->id);
        iter = iter->next;
    }

    // Add 2 in the right spot
    DATA_S_List_insert (&list, &structs[2].node, 2);

    // Confirm order
    iter = list.head;
    for (uint8_t n_iter = 0; n_iter < structs_len; n_iter++)
    {
        Test_Struct *ts = DATA_LIST_GET_OBJ(iter, Test_Struct, node);
        POINTERS_EQUAL(n_iter, ts->id);
        iter = iter->next;
    }
}

TEST(nominal_large, append_and_prepend)
{
    //Append 1,2,3,4
    for (uint8_t n_iter = 1; n_iter < structs_len; n_iter++)
    {
        CHECK_EQUAL(n_iter - 1, list.len);
        CHECK_EQUAL(0, DATA_S_List_append(&list, &structs[n_iter].node));
        POINTERS_EQUAL(&structs[n_iter], DATA_LIST_GET_OBJ(list.tail, Test_Struct, node));
    }

    // Confirm order
    S_List_Node *iter = list.head;
    for (uint8_t n_iter = 1; n_iter < structs_len; n_iter++)
    {
        Test_Struct *ts = DATA_LIST_GET_OBJ(iter, Test_Struct, node);
        POINTERS_EQUAL(n_iter, ts->id);
        iter = iter->next;
    }

    // Add 2 in the right spot
    DATA_S_List_prepend (&list, &structs[0].node);

    // Confirm order
    iter = list.head;
    for (uint8_t n_iter = 0; n_iter < structs_len; n_iter++)
    {
        Test_Struct *ts = DATA_LIST_GET_OBJ(iter, Test_Struct, node);
        POINTERS_EQUAL(n_iter, ts->id);
        iter = iter->next;
    }
}

TEST_GROUP(bad)
{
};