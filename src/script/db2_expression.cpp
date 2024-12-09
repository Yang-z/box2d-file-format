#include "db2_expression.h"

#include "db2_math.h"

db2Expression::~db2Expression()
{
    for (auto i = 0; i < this->args.size(); ++i)
        delete this->args[i];
}

auto db2Expression::operator()() -> void
{
    this->run();
}

auto db2Expression::bind(int32_t type) -> void
{
    this->type = type;

    switch (this->type)
    {
    case db2Key::Sequence:
        this->run = [this]() -> void
        {
            for (auto i = 0; i < this->args.size(); ++i)
                this->args[i]->run();
            this->result = this->args.back()->result;
        };
        break;

    case db2Key::If:
        this->args.resize(3);
        this->run = [this]() -> void
        {
            this->args[0]->run();
            if (static_cast<bool>(this->args[0]->result))
                this->args[1]->run(), this->result = this->args[1]->result;

            else
                this->args[2]->run(), this->result = this->args[2]->result;
        };
        break;

    case db2Key::LITERAL:
        this->run = []() {};
        break;

        /************************************/

    case db2Key::Add:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = this->args[0]->result + this->args[1]->result; };
        break;

    case db2Key::Sub:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = this->args[0]->result - this->args[1]->result; };
        break;

    case db2Key::Mul:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = this->args[0]->result * this->args[1]->result; };
        break;

    case db2Key::Div:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = this->args[0]->result / this->args[1]->result; };
        break;

    case db2Key::Mod:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = this->args[0]->result % this->args[1]->result; };
        break;

    case db2Key::Sin:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = sin(this->args[0]->result); };
        break;

    case db2Key::Cos:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = cos(this->args[0]->result); };
        break;

    case db2Key::Tan:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = tan(this->args[0]->result); };
        break;

    case db2Key::ASin:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = asin(this->args[0]->result); };
        break;

    case db2Key::ACos:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = acos(this->args[0]->result); };
        break;

    case db2Key::ATan:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = atan(this->args[0]->result); };
        break;

    case db2Key::Pow:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = pow(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::Log:
        this->args.resize(2);
        this->run = [this]() -> void
        { this->args[0]->run(), this->args[1]->run(), this->result = log(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::Abs:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = abs(this->args[0]->result); };
        break;

    case db2Key::Ceil:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = ceil(this->args[0]->result); };
        break;

    case db2Key::Round:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = round(this->args[0]->result); };
        break;

    case db2Key::Floor:
        this->args.resize(1);
        this->run = [this]() -> void
        { this->args[0]->run(), this->result = floor(this->args[0]->result); };
        break;

    case db2Key::Equal:
        this->args.resize(2);
        this->run = [this]()
        { this->args[0]->run(), this->args[1]->run(), this->result = equal(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::Greater:
        this->args.resize(2);
        this->run = [this]()
        { this->args[0]->run(), this->args[1]->run(), this->result = greater(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::GreaterEqual:
        this->args.resize(2);
        this->run = [this]()
        { this->args[0]->run(), this->args[1]->run(), this->result = greater_equal(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::Less:
        this->args.resize(2);
        this->run = [this]()
        { this->args[0]->run(), this->args[1]->run(), this->result = less(this->args[0]->result, this->args[1]->result); };
        break;

    case db2Key::LessEqual:
        this->args.resize(2);
        this->run = [this]()
        { this->args[0]->run(), this->args[1]->run(), this->result = less_equal(this->args[0]->result, this->args[1]->result); };
        break;

    default:
        assert(false);
        break;
    }
}
