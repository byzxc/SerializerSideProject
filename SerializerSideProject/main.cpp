#include "Serialization.hpp"
using namespace Reflect;

// Property = variables
// Method = functions

int main()
{
    circle c_1("Circle #1");
    shape& my_shape = c_1;

    std::cout << my_shape.name << std::endl;

    c_1.set_visible(true);
    c_1.color_ = color::red;
    c_1.position.x = 12;
    c_1.position.y = 66;

    c_1.dictionary = { {color::green, {1, 2} }, {color::blue, {3, 4} }, {color::red, {5, 6} } };
    c_1.radius = 5.123;
    c_1.clown = { 1,2,3,4,5,6,7,8 };

    std::vector<point2d> pointzzz;
    pointzzz.emplace_back(2, 3);
    pointzzz.emplace_back(4, 5);
    c_1.points = pointzzz;

    c_1.no_serialize = 12345;
    c_1.allah = Vector3{ 2.f, 4.f, 5.f };

    circle c_2{ "Circle: #2" };

    JSON_SERIALIZE("json\\testing.json", c_1)
    JSON_DESRIALIZE("json\\testing.json", c_2)

    auto valuesData = InvokeRegisteredClassFunctionRecursively<circle>("radiusDouble", "circle", {}, { 2.f, 3.f });
    auto valueData = InvokeRegisteredClassFunction<circle>("radiusDoubles", "circle", {}, { 3.f, 2.f });
    std::cout << valueData.get_value<double>();

    for (auto itr : valuesData)
    {
        if (itr.is_valid() && itr.is_type<double>())
        {
            std::cout << itr.get_value<double>() << std::endl;
        }
    }

    //auto valueDatas = InvokeRegisteredClassFunction<circle>("radiusDoubles", "circle", {}, )

    for (auto itr : InvokeGlobalRegisteredFunction("sin", { 1.f, 2.f }))
    {
        std::cout << GetValueFromVariant<float>(itr) << std::endl;
    }
}