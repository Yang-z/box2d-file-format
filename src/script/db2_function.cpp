#include "db2_function.h"

auto db2Function::operator()() -> void
{

    // for (auto i = 0; i < this->args.size(); ++i)
    //     this->args[i]();

    this->func();
}

auto db2Function::bind(int32_t type) -> void
{
    this->type = type;

    switch (this->type)
    {
    case db2Key::Sequence:
        this->func = [this]() -> void
        {
            for (auto i = 0; i < this->args.size(); ++i)
                this->args[i]();
            this->result = this->args.back().result;
        };
        break;

    case db2Key::If:
        this->args.resize(3);
        this->func = [this]() -> void
        {
            this->args[0]();
            if (static_cast<bool>(this->args[0].result))
                this->args[1](), this->result = this->args[1].result;

            else
                this->args[2](), this->result = this->args[2].result;
        };
        break;

    case db2Key::Literal:
        this->func = []() {};
        break;

    case db2Key::Add:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = this->args[0].result + this->args[1].result; };
        break;

    case db2Key::Sub:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = this->args[0].result - this->args[1].result; };
        break;

    case db2Key::Mul:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = this->args[0].result * this->args[1].result; };
        break;

    case db2Key::Div:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = this->args[0].result / this->args[1].result; };
        break;

    case db2Key::Mod:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = this->args[0].result % this->args[1].result; };
        break;

    case db2Key::Sin:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = sin(this->args[0].result); };
        break;

    case db2Key::Cos:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = cos(this->args[0].result); };
        break;

    case db2Key::Tan:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = tan(this->args[0].result); };
        break;

    case db2Key::ASin:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = asin(this->args[0].result); };
        break;

    case db2Key::ACos:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = acos(this->args[0].result); };
        break;

    case db2Key::ATan:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = atan(this->args[0].result); };
        break;

    case db2Key::Pow:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = pow(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::Log:
        this->args.resize(2);
        this->func = [this]() -> void
        { this->args[0](), this->args[1](), this->result = log(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::Abs:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = abs(this->args[0].result); };
        break;

    case db2Key::Ceil:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = ceil(this->args[0].result); };
        break;

    case db2Key::Round:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = round(this->args[0].result); };
        break;

    case db2Key::Floor:
        this->args.resize(1);
        this->func = [this]() -> void
        { this->args[0](), this->result = floor(this->args[0].result); };
        break;

    case db2Key::Equal:
        this->args.resize(2);
        this->func = [this]()
        { this->args[0](), this->args[1](), this->result = equal(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::Greater:
        this->args.resize(2);
        this->func = [this]()
        { this->args[0](), this->args[1](), this->result = greater(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::GreaterEqual:
        this->args.resize(2);
        this->func = [this]()
        { this->args[0](), this->args[1](), this->result = greater_equal(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::Less:
        this->args.resize(2);
        this->func = [this]()
        { this->args[0](), this->args[1](), this->result = less(this->args[0].result, this->args[1].result); };
        break;

    case db2Key::LessEqual:
        this->args.resize(2);
        this->func = [this]()
        { this->args[0](), this->args[1](), this->result = less_equal(this->args[0].result, this->args[1].result); };
        break;

    default:
        assert(false);
        break;
    }
}
