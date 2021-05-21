#include "Serialization.hpp"
#include <tuple>
using namespace rttr;

enum class color
{
    red,
    green,
    blue
};

struct point2d
{
    point2d() {}
    point2d(int x_, int y_) : x(x_), y(y_) {}
    int x = 0;
    int y = 0;
};

struct shape
{
public:
    shape(std::string n) : name(n) {}

    void set_visible(bool v) { visible = v; }
    bool get_visible() const { return visible; }

    color color_ = color::blue;
    std::string name = "";
    point2d position;
    std::map<color, point2d> dictionary;

    RTTR_ENABLE()
private:
    bool visible = false;
};

struct circle : shape
{
    circle(std::string n) : shape(n) {}

    double radius = 5.2;
    std::vector<point2d> points;
    std::vector<int> clown;
    std::tuple<int, float> dick{ 1, 2.f };

    int no_serialize = 100;

    RTTR_ENABLE(shape)
};

// Property = variables
// Method = functions
RTTR_REGISTRATION
{
    registration::class_<shape>("shape")
        .property("visible", &shape::get_visible, &shape::set_visible)
        .property("color", &shape::color_)
        .property("name", &shape::name)
        .property("position", &shape::position)
        .property("dictionary", &shape::dictionary);

    registration::class_<circle>("circle")
        .property("radius", &circle::radius)
        .property("points", &circle::points)
        .property("clown", &circle::clown)
        .property("dick", &circle::dick)
        .property("no_serialize", &circle::no_serialize)
        (
            metadata("NO_SERIALIZE", true)
        );

    registration::class_<point2d>("point2d")
        .constructor()(policy::ctor::as_object)
        .property("x", &point2d::x)
        .property("y", &point2d::y);

    registration::enumeration<color>("color")
        (
            value("red", color::red),
            value("blue", color::blue),
            value("green", color::green)
        );
}

int main()
{
    //JSON::Serialization serializer;

    circle c_1("Circle #1");
    shape& my_shape = c_1;

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

    circle c_2{ "Circle: #2" };

    JSON_SERIALIZE("json\\testing.json", c_1)
    JSON_DESRIALIZE("json\\testing.json", c_2)

    for (auto& itr : c_2.points)
        {
            std::cout << itr.x << ", " << itr.y << std::endl;
        }


    //serializer.SerializeToFile("json\\testing.json");
    //serializer.DeserializeFromFile("json\\testing.json");
}