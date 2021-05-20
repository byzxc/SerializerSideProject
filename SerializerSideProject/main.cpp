#include "Serialization.hpp"
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

    int no_serialize = 100;

    RTTR_ENABLE(shape)
};

// Property = variables
// Method = functions
RTTR_REGISTRATION
{
    rttr::registration::class_<shape>("shape")
        .property("visible", &shape::get_visible, &shape::set_visible)
        .property("color", &shape::color_)
        .property("name", &shape::name)
        .property("position", &shape::position)
        .property("dictionary", &shape::dictionary);

    rttr::registration::class_<circle>("circle")
        .property("radius", &circle::radius)
        .property("points", &circle::points)
        .property("no_serialize", &circle::no_serialize)
        (
            metadata("NO_SERIALIZE", true)
        );

    rttr::registration::class_<point2d>("point2d")
        .constructor()(policy::ctor::as_object)
        .property("x", &point2d::x)
        .property("y", &point2d::y);

    rttr::registration::enumeration<color>("color")
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

    // additional braces are needed for a VS 2013 bug
    c_1.dictionary = { { {color::green, {1, 2} }, {color::blue, {3, 4} }, {color::red, {5, 6} } } };
    c_1.radius = 5.123;

    std::vector<point2d> pointzzz;
    pointzzz.emplace_back(2,3);
    pointzzz.emplace_back(4, 5);
    c_1.points = pointzzz;

    c_1.no_serialize = 12345;

    JSON_SERIALIZE("json\\testing.json", my_shape)
    //serializer.SerializeToFile("json\\testing.json");
    //serializer.DeserializeFromFile("json\\testing.json");
}