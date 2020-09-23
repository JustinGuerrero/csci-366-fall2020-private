#include "gtest/gtest.h"



char * print_binary_representation(unsigned int i, char *buffer){

    //start i at 31 make it unsigned
    // decrement to 0
    // start with the value one
    // shift that value into the position that we're looking at
    // i have one i shift it i inpect one bit then i mask using the & all the other bits
    // fill out remaining 32 bits, 1 or 0 depending on the value in the number i
    buffer[0] = '0';
    buffer[1] = 'b';
    for(int j = 32; j >= 0; j--) {
        int math = 3 +(31-j);
        unsigned int mask;
        unsigned int result = i & mask;
        mask = 30u << j;

        if (result) {
            buffer[math] = '1';
            printf("yep that bit was 1\n");
        } else {
            buffer[math] = '0';
            printf("that was not a 1\n");
        }

    }

    return buffer;
}

/* PROBLEM 1: Implement a print_binary_representation function that takes an
 * unsigned integer and created as string representation of the binary values
 * of that number.
 *
 * The test below show what the expected values are for given inputs
 */
TEST(print_binary_representation, works) {
    // row 1
    char buffer[50] = {0}; // init to 0
    EXPECT_STREQ("0b00000000000000000000000000000000", print_binary_representation(0, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000001", print_binary_representation(1, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000010", print_binary_representation(2, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000011", print_binary_representation(3, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000100", print_binary_representation(4, buffer));
    EXPECT_STREQ("0b00000001110111111001101001000010", print_binary_representation(31431234, buffer));
    EXPECT_STREQ("0b00011011111000100100001101011101", print_binary_representation(467813213, buffer));
    EXPECT_STREQ("0b11111111111111111111111111111111", print_binary_representation(UINT32_MAX, buffer));
}

/* PROBLEM 2: The test below fails.  Change the signature of set_my_age and the
 * call of the function in get_my_age so that the expected value is returned.
 *
 * HINT: C is pass by value
 */

struct Person {
    char * name;
    int age;
};

int set_my_age(struct Person *p) {
    struct Person age;
    (*p).age = 30;
    return (*p).age;
}

int get_my_age() {
    struct Person me;
    me.name = "Justin";
    set_my_age(&me);
    return me.age;
}

TEST(set_my_age, works) {
    EXPECT_EQ(30, get_my_age());
}




