#include <cmath>
#include <iostream>
#include <type_traits> // std::is_same

#include <boost/pfr.hpp> // reflect
// #include <boost/pfr/core.hpp>
// #include <boost/pfr/core_name.hpp>

#include "common/db2_settings.h"
#include "common/db2_hardware_difference.h"
#include "common/db2_nullval.h"

#include "containers/db2_dynarray.h"
#include "containers/db2_chunk.h"
#include "containers/db2_cson.h"

#include "data/db2_structure.h"

#include "decoders/db2_decoder.h"

#include "dotBox2d.h"

auto test_c_array() -> void
{
    char type[4]{'N', 'U', 'L', 'L'};
    // type = {'a', 'b', 'c', 'd'}; // no
    // type = "abc"; // no
    printf("type = %s\n", type);
}

auto test_size() -> void
{
    union UN
    {
        char name[4];
        int32_t id{0};
    } un;
    printf("sizeof(un) = %d\n", sizeof(un));
    printf("sizeof(un.name) = %d\n", sizeof(un.name));

    printf("&un.name = %d\n", &un.name);
    printf("un.name + 1 = %d\n", un.name + 1);
}

auto test_cast_reference() -> void
{
    int i = 0;
    auto &cast_i_f = reinterpret_cast<float &>(i);
    // auto &force_i_f = (float)i; // initial value of reference to non-const must be an lvalueC/C++(461)
    auto &force_i_f = (float &)i;
    auto &force_i_f_p = *(float *)&i;
    printf((void *)&i == (void *)&force_i_f ? "true\n" : "false\n"); // true
    printf(&cast_i_f == &force_i_f ? "true\n" : "false\n");          // true
    printf(&cast_i_f == &force_i_f_p ? "true\n" : "false\n");        // true
}

auto test_cast_pointer() -> void
{
    int *data{nullptr};
    printf("data = %p\n", data); // 0000000000000000
    *(void **)(&data) = malloc(8);
    printf("data = %p\n", data); // 00000245044613a0
    // printf("*(void **)(&data) + 1 = %p\n", *(void **)(&data) + 1); // 00000245044613a1
    printf("*(char **)(&data) + 1 = %p\n", *(char **)(&data) + 1); // 00000245044613a1
    printf("*(int  **)(&data) + 1 = %p\n", *(int **)(&data) + 1);  // 00000245044613a4

    char type[4];
    printf("type = %p\n", type);         // 00000016aafffab4
    printf("type + 1 = %p\n", type + 1); // 00000016aafffab5
}

auto test_cast_value() -> void
{
    float value_f = 3.1415926f;
    printf("value_f : %f\n", value_f);                                                  // 3.141593
    printf("(int)value_f : %d\n", (int)value_f);                                        // 3
    printf("static_cast<int>(value_f) : %d\n", static_cast<int>(value_f));              // 3
    printf("reinterpret_cast<int&>(value_f) : %d\n", reinterpret_cast<int &>(value_f)); // 1078530010

    int value_i = 1078530010;
    printf("value_i : %d\n", value_i);                                                      // 1078530010
    printf("(float)value_i : %f\n", (float)value_i);                                        // 1078530048.000000
    printf("static_cast<float>(value_i) : %f\n", static_cast<float>(value_i));              // 1078530048.000000
    printf("reinterpret_cast<float&>(value_i) : %f\n", reinterpret_cast<float &>(value_i)); // 3.141593
}

auto test_equality() -> void
{
    char sc = 0b11111111;
    unsigned char uc = 0b11111111;

    printf("char c = 0b11111111;\nunsigned char uc = 0b11111111;\n");
    printf("sc == uc; //%s\n", sc == uc ? "true" : "false");                                   // false
    printf("std::equal(&sc, &sc, &uc); //%s\n", std::equal(&sc, &sc, &uc) ? "true" : "false"); // true
    printf("\n");

    int8_t si8 = -1;
    uint8_t ui8 = -1;
    printf("signed char si8 = -1; //%d\nunsigned char ui8 = -1; //%d\n", si8, ui8);
    printf("si8 == ui8; //%s\n", si8 == ui8 ? "true" : "false");                                     // false
    printf("std::equal(&si8, &si8, &ui8); //%s\n", std::equal(&si8, &si8, &ui8) ? "true" : "false"); // true
    printf("\n");
    /*
    For the comparisons between signed char and unsigned char, si8 is assigned the value -1, while
    ui8 is also assigned the value -1. However, they have different types, with si8 being a signed
    char and ui8 being an unsigned char.
    When comparing them, the compiler converts them both to a common type for comparison, which
    in this case is int:
        signed      0b11111111 -> 0b11111111 11111111 11111111 11111111
        unsigned    0b11111111 -> 0b00000000 00000000 00000000 11111111
    */

    int16_t si16 = -1;
    uint16_t ui16 = -1;
    printf("signed char si16 = -1;\nunsigned char ui16 = -1;\n");
    printf("si16 == ui16; //%s\n", si16 == ui16 ? "true" : "false");                                       // false
    printf("std::equal(&si16, &si16, &ui16); //%s\n", std::equal(&si16, &si16, &ui16) ? "true" : "false"); // true
    printf("\n");

    int32_t si32 = -1;
    uint32_t ui32 = -1;
    printf("signed int si32 = -1; //%d\nunsigned int ui32 = -1; //%d\n", si32, ui32);
    printf("si32 == ui32; //%s\n", si32 == ui32 ? "true" : "false");                                       // true?!
    printf("std::equal(&si32, &si32, &ui32); //%s\n", std::equal(&si32, &si32, &ui32) ? "true" : "false"); // true
    printf("\n");

    int64_t si64 = -1;
    uint64_t ui64 = -1;
    printf("int64_t si64 = -1;\nuint64_t ui64 = -1;\n");
    printf("si64 == ui64; //%s\n", si64 == ui64 ? "true" : "false");                                       // true?!
    printf("std::equal(&si64, &si64, &ui64); //%s\n", std::equal(&si64, &si64, &ui64) ? "true" : "false"); // true
    printf("\n");

    printf("-1 == UINT8_MAX //%s\n", -1 == UINT8_MAX ? "true" : "false");   // false
    printf("-1 == UINT16_MAX //%s\n", -1 == UINT16_MAX ? "true" : "false"); // false
    printf("-1 == UINT32_MAX //%s\n", -1 == UINT32_MAX ? "true" : "false"); // true
    printf("-1 == UINT64_MAX //%s\n", -1 == UINT64_MAX ? "true" : "false"); // true
    printf("\n");
}

auto test_derived_init() -> void
{
    class Base
    {
    public:
        Base()
        {
            std::cout << "Base constructor0 called." << std::endl;
        }

        Base(bool b)
        {
            std::cout << "Base constructor1 called." << std::endl;
        }
    };

    class Derived : public Base
    {
    public:
        Derived()
        {
            std::cout << "Derived constructor0 called." << std::endl;
        }
        Derived(bool b)
            : Base(b) // call base default constructor if not specified
        {
            std::cout << "Derived constructor1 called." << std::endl;
        }
    };

    Derived d0;
    Derived d1(true);
}

auto test_derived_func() -> void
{
    class empty_class
    {
    };

    class Base
    {
    public:
        void func_t()
        {
            std::cout << "Type of this: " << typeid(decltype(*this)).name() << std::endl;
        }
        void func1()
        {
            std::cout << "Base func1() called." << std::endl;
            this->func2();
            this->func3();
        }
        void func2()
        {
            std::cout << "Base func2() called." << std::endl;
        }
        virtual void func3()
        {
            std::cout << "Base func3() called." << std::endl;
        }

        void sizeof_()
        {
            std::cout << "sizeof(*this) = " << sizeof(*this) << std::endl;
        }
    };

    class Derived : public Base
    {
    public:
        int32_t i = 0;

    public:
        void func2()
        {
            std::cout << "Derived func2() called." << std::endl;
        }
        void func3() override // key word "override" is not nessary
        {
            std::cout << "Derived func3() called." << std::endl;
        }
    };

    Base b;
    b.func_t();
    Derived d;
    d.func_t();

    d.func1();

    /*
    output:
    Type of this: Z17test_derived_funcvE4Base
    Type of this: Z17test_derived_funcvE4Base
    Base func1() called.
    Base func2() called.
    Derived func3() called.
    */

    sizeof(empty_class); // 1 ,not 0
    sizeof(Base);        // 8, not 1, virtual funtion takes 8
    sizeof(Derived);     // 16, not 12, due to data structure alignment
    b.sizeof_();         // 8
    d.sizeof_();         // 8 // not 16
}

struct test_l_r_value_t
{
    template <typename T>
    void test(T &t) { std::cout << "left value" << std::endl; }

    template <typename T>
    void test(T &&t) { std::cout << "right value" << std::endl; }
};

struct test_forwarding_t
{
    // Perfect forwarding = Universal references(T &&t) + std::forward
    template <typename T>
    auto forward(T &&t)
    {
        test_l_r_value_t l_r;

        l_r.test(t);
        l_r.test(std::forward<T>(t));
        l_r.test(std::forward<decltype(t)>(t));
        std::cout << "typeid(T) == typeid(decltype(t)) ? " << (typeid(T) == typeid(decltype(t)) ? "true" : "false") << std::endl;
        std::cout << "std::is_same_v<T, decltype(t)> ? " << (std::is_same_v<T, decltype(t)> ? "true" : "false") << std::endl;
        l_r.test(std::move(t));
        std::cout << "======================" << std::endl;
    }
};

auto test_forwarding() -> void
{
    test_forwarding_t test{};

    test.forward(0);
    /*
    left value
    right value
    right value
    typeid(T) == typeid(decltype(t)) ? true
    std::is_same_v<T, decltype(t)> ? false
    right value
    */

    int i = 0;
    test.forward(i);
    /*
    left value
    left value
    left value
    typeid(T) == typeid(decltype(t)) ? true
    std::is_same_v<T, decltype(t)> ? true
    right value
    */
}

auto test_func_return_type() -> void
{
    struct test
    {
        auto get_()
        {
            return 1;
        }

        auto get() -> int
        {
            return 1;
        }

        auto get_lr() -> int &
        {
            static int x = 1;
            return x;
        }

        auto get_rr() -> int &&
        {
            return 1;
        }

        auto get_xr() -> int &
        {
            int x = 1;
            return x;
        }

        auto get_rr_as_lr() -> const int &
        {
            return 1;
        }

        auto get_lr_as_rr() -> int &&
        {
            static int x = 1;
            return std::move(x);
        }
    };

    test t;
    // t.get_() = 2; // no
    // t.get() = 2; // no
    t.get_lr() = 2;
    // t.get_rr() = 2; // no
    t.get_xr() = 2;
    // t.get_rr_as_lr() = 2; // no, because of const
    // t.get_lr_as_rr() = 2; // no

    test_l_r_value_t t_lr;

    int i_ = t.get_();                        //
    t_lr.test(i_);                            // left value
    int i = t.get();                          //
    t_lr.test(i);                             // left value
    int &i_lr = t.get_lr();                   //
    t_lr.test(i_lr);                          // left value
    int &&i_rr = t.get_rr();                  //
    t_lr.test(i_rr);                          // left value
    int &i_xr = t.get_xr();                   //
    t_lr.test(i_xr);                          // left value
    const int &i_rr_as_lr = t.get_rr_as_lr(); //
    t_lr.test(i_rr_as_lr);                    // left value
    int &&i_lr_as_rr = t.get_lr_as_rr();      //
    t_lr.test(i_lr_as_rr);                    // left value

    std::cout << "==================" << std::endl;

    t_lr.test(t.get_());         // right value
    t_lr.test(t.get());          // right value
    t_lr.test(t.get_lr());       // left value
    t_lr.test(t.get_rr());       // right value
    t_lr.test(t.get_xr());       // left value
    t_lr.test(t.get_rr_as_lr()); // left value
    t_lr.test(t.get_lr_as_rr()); // right value

    std::cout << "==================" << std::endl;

    /*
    Funny.
    A function() says it is a right value reference, and it acutually is right value.
    While, a variable says it is a right value reference, but it acutually is left value.

    Function() is able to return right value, and that's an additional reason why std::forward could work.
    */
}

auto test_reflection() -> void
{
    struct S
    {
        int i0{22};
        float f0{33.33f};
        bool b0{true};
        // int ai[3]{1, 2, 3}; // array is not supported
        char a0{'c'}, a1{'h'}, a2{'a'}, a3{'r'};
        struct
        {
            int i1{33};
            float b2{44.0f};
        } s1;

        union U
        {
            int i{1};
            float32_t f;
        } u0;

    } s0;

    float s1 = 3.14;

    int count = 0;
    boost::pfr::for_each_field(
        s0,
        [&s0, &count](auto &field)
        {
            ++count;
            printf("%s: %d: %d\n", typeid(field).name(), sizeof(field), field);
            // printf("%d\n", boost::pfr::get_name<count, s0>());
        } //
    );

    printf("%s count: %d\n", typeid(s0).name(), count);

    auto size = boost::pfr::tuple_size<S>::value;
    for (std::size_t i = 0; i < size; ++i)
    {
        // auto value = boost::pfr::get<i>(s0); // no
    }
}

auto test_typeid() -> void
{
    printf("sizeof(size_t) = %d\n", sizeof(size_t)); // sizeof(size_t) = 8

    printf("typeid(bool) : %s %zu\n", typeid(bool).name(), typeid(bool).hash_code());    // typeid(bool) : b 10838281452030117757
    printf("typeid(int) : %s %zu\n", typeid(int).name(), typeid(int).hash_code());       // typeid(int) : i 6253375586064260614
    printf("typeid(float) : %s %zu\n", typeid(float).name(), typeid(float).hash_code()); // typeid(float) : f 8968846175329310707

    printf("typeid(db2Chunk<int32_t>) : %s %zu\n", typeid(db2Chunk<int32_t>).name(), typeid(db2Chunk<int32_t>).hash_code());                               // typeid(db2Chunk<int32_t>) : 8db2ChunkIiE 8939727042291170200
    printf("typeid(db2Chunk<float32_t>) : %s %zu\n", typeid(db2Chunk<float32_t>).name(), typeid(db2Chunk<float32_t>).hash_code());                         // typeid(db2Chunk<float32_t>) : 8db2ChunkIfE 18096401828085413589
    printf("typeid(db2Chunk<db2Chunk<int32_t>>) : %s %zu\n", typeid(db2Chunk<db2Chunk<int32_t>>).name(), typeid(db2Chunk<db2Chunk<int32_t>>).hash_code()); // typeid(db2Chunk<db2Chunk<int32_t>>) : 8db2ChunkIS_IiEE 1831950841763916697

    class CK_int32 : public db2Chunk<int32_t>
    {
    };
    printf("typeid(CK_int32>) : %s %zu\n", typeid(CK_int32).name(), typeid(CK_int32).hash_code()); // typeid(CK_int32>) : Z11test_typeidvE8CK_int32 221492753444786947
}

auto test_inline_static() -> void
{
    printf("db2Reflector::reflectors.size(): %d\n", db2Reflector::reflectors.size());
    // if db2Reflector::reflectors is inline static, the size is zero;
    // else if db2Reflector::reflectors is not inline, the size is not zero.

    // db2Reflector::LocalStaticTester();
    // // output 1 rather than 2
}

/* ================================ */

auto test_CRC() -> void
{
    auto data = "test";
    auto length = 4;

    // boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> crc;
    boost::crc_32_type crc;
    crc.process_bytes(data, length);

    printf("%X\n", crc.checksum());
}

auto test_hardware_difference() -> void
{

    // std::cout << "Is little endian: " << (HardwareDifference::IsLittleEndian() ? "true" : "false") << std::endl;
    // std::cout << "Is little endian (bit endianness): " << (HardwareDifference::IsLittleEndian_Bit() ? "true" : "false") << std::endl;
    // std::cout << "Date Structure Alignment offset: " << +HardwareDifference::GetDataStructureAlignment() << std::endl;
    // std::cout << "Date Structure Alignment offset (packed): " << +HardwareDifference::GetDataStructureAlignment(true) << std::endl;

    printf("Is little endian: %s\n", HardwareDifference::IsLittleEndian() ? "true" : "false");
    printf("Is little endian (bit endianness): %s\n", HardwareDifference::IsLittleEndian_Bit() ? "true" : "false");
    printf("Date Structure Alignment offset: %d\n", HardwareDifference::GetDataStructureAlignment());              // 8
    printf("Date Structure Alignment offset (packed): %d\n", HardwareDifference::GetDataStructureAlignment(true)); // 1

    int32_t i4r = 8;
    HardwareDifference::ReverseEndian((char *)&i4r, sizeof(i4r));
    printf("int32_t i32 = 8; //reversed = %d\n", i4r);
}

auto test_nullval() -> void
{
    printf("%d\n", &nullval);
    printf("%d\n", &nullval);
    // same address

    int &i = nullval;
    int i1 = 0;
    // int i2 = nullval; // don't do this

    printf("%d\n", i == nullval);  // true
    printf("%d\n", i1 == nullval); // false

    printf("%d\n", i != nullval);  // false
    printf("%d\n", i1 != nullval); // true
}

auto test_data_structure_write() -> void
{
    auto db2 = new dotBox2d();
    auto &infos = db2->chunks.get<db2Chunk<db2Info>>();
    infos.emplace_back();
    infos.emplace_pfx();

    db2->chunks.get<db2Chunk<db2World>>().emplace_back();
    db2->chunks.get<db2Chunk<db2Joint>>().emplace_back();
    for (int i = 0; i < 2; i++)
        db2->chunks.get<db2Chunk<db2Body>>().emplace_back();
    for (int i = 0; i < 3; i++)
        db2->chunks.get<db2Chunk<db2Fixture>>().emplace_back();
    for (int i = 0; i < 5; i++)
        db2->chunks.get<db2Chunk<db2Shape>>().emplace_back();

    // dict
    auto &dict = db2->chunks.get<db2Chunk<db2Dict>>().emplace_back();

    if (true) // test for db2Dynarry::append
    {
        auto &da = reinterpret_cast<db2DynArray<db2DictElement> &>(dict);
        da.emplace_back(0, 'e', 'p', 'b', '0', 0); // good

        // da.append(1, 'a', 'p', 'd', '1', 1, 2, 'a', 'p', 'd', '2', 2, 3); // compilable but ill
        da.append(db2DictElement{1, 'a', 'p', 'd', '1', 1}, db2DictElement{2, 'a', 'p', 'd', '2', 2});

        // da.append_range({1, 'a', 'p', 'r', '1', 1}, {2, 'a', 'p', 'r', '2', 2}, {3, 'a', 'p', 'r', '3', 3}); // not compilable
        // da.append_range({'1', 'a', 'p', 'r', '1', '1'}, {'2', 'a', 'p', 'r', '2', '2'}); // (std::initializer_list<Args>... args_lists) // all args within {} need to be of a same type.
        da.append_range({{1, 'a', 'p', 'r', '1', 1}, {2, 'a', 'p', 'r', '2', 2}}); // (std::initializer_list<U> arg_list)
        // da.append_range({ std::make_tuple(1, 'a', 'p', 'r', '1', 1), {2, 'a', 'p', 'r', '2', 2} }); // (const std::initializer_list<std::tuple<Args...>> &arg_list)
    }

    dict.get(1) = 2;
    dict.get<float32_t>(2) = acos(-1.0); // M_PI

    const auto &i = dict.at<int32_t>(1);
    const auto &f = dict.at<float32_t>(2);
    const auto &null = dict.at<int32_t>(3);

    auto &d = dict.find(*(int32_t *)nullptr, nullptr);
    auto &d2 = dict.find(2);

    dict.link<db2Chunk<db2Shape>>(100) = 2;
    dict.link<db2Chunk<db2Body>>(101, 1);

    // list
    auto &list = db2->chunks.get<db2Chunk<db2List>>().emplace_back();
    list.emplace_back<int32_t>(1);
    list.emplace_back<int32_t>(3);
    // list.append<int32_t>(11, 12, 13);
    // list.append({21},{22}); // no

    auto &list1 = db2->chunks.get<db2Chunk<db2List>>().emplace_back();
    list1.emplace_back<float32_t>(acos(-1.0)); // M_PI
    list1.emplace_back<float32_t>(exp(1.0));   // M_E

    // string
    auto &str = db2->chunks.get<db2Chunk<db2String>>().emplace_back("Hello");
    str.append_range(" ");
    str += "world!";

    // dereference
    dict.emplace<db2Chunk<db2String>>(300, "dereferenced");
    auto &str2 = dict.at<db2Chunk<db2String>>(300);

    db2->save("./test.B2D");
    delete db2;
}

auto test_data_structure_read() -> void
{
    dotBox2d db2{"./test.B2D"};
    return;
}

auto test_step(dotBox2d &db2) -> void
{
    auto dynamicBody = db2.p_b2w->GetBodyList()->GetNext();

    int t = 0;
    while (true)
    {
        db2.step();
        printf("Py = %f; Vy = %f\n", dynamicBody->GetPosition().y, dynamicBody->GetLinearVelocity().y);
        if (t > 200)
            break;
        t++;
    }
    printf("step count: %d\n", t);
}

auto test_encoding() -> void
{
    b2World *p_b2w;
    db2Decoder der{};
    {
        p_b2w = new b2World{{0.0f, -9.8f}};

        /*body*/
        b2BodyDef dynamicBodyDef{};
        dynamicBodyDef.type = b2_dynamicBody;
        dynamicBodyDef.position.Set(8.0f, 8.0f);

        auto dynamicBody = p_b2w->CreateBody(&dynamicBodyDef);

        /*fixture*/
        b2CircleShape circle{};
        circle.m_radius = 0.5f;

        b2FixtureDef fixturedef{};
        fixturedef.shape = &circle;
        fixturedef.density = 1.0f;
        fixturedef.restitution = 0.8f;

        dynamicBody->CreateFixture(&fixturedef);

        /*body2*/
        b2BodyDef staticBodyDef{};
        staticBodyDef.type = b2_staticBody;
        staticBodyDef.position.Set(0.0f, 0.0f);

        auto staticBody = p_b2w->CreateBody(&staticBodyDef);

        /*fixture2*/
        b2ChainShape chain{};
        // b2Vec2 vs[4]{{0.0f, 0.0f}, {16.0f, 0.0f}, {16.0f, 9.0f}, {0.0f, 9.0f}}; // anti-clockw, ouside
        b2Vec2 vs[4]{{0.0f, 0.0f}, {0.0f, 9.0f}, {16.0f, 9.0f}, {16.0f, 0.0f}}; // inside
        chain.CreateLoop(vs, 4);

        b2FixtureDef fixturedef2{};
        fixturedef2.shape = &chain;

        staticBody->CreateFixture(&fixturedef2);
    }

    dotBox2d db2{};
    db2.p_b2w = p_b2w;
    db2.encode();
    db2.p_db2ContactListener = new db2ContactListener{};
    db2.p_db2OffstepListener = new db2OffstepListener{};

    // db2.save("./test_encode.B2d", true);
    db2.save("./test_encode_BE.B2D", false);

    test_step(db2);
}

auto test_decoding() -> void
{
    // dotBox2d db2{"./test_encode.B2d"};
    dotBox2d db2{"./test_encode_BE.B2D"};
    db2.load();
    db2.decode();

    test_step(db2);
}

auto main() -> int
{
    // test_size();

    // test_cast_reference();
    // test_cast_pointer();
    // test_cast_value();

    // test_equality();

    // test_derived_init();
    // test_derived_func();

    // test_forwarding();
    // test_func_return_type();

    // test_reflection();
    // test_typeid();

    // test_inline_static();

    /* ================================ */

    // test_CRC();

    // test_hardware_difference();

    // test_nullval();

    // test_data_structure_write();
    // test_data_structure_read();

    test_encoding();
    test_decoding();

    return 0;
}
