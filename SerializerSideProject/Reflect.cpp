/******************************************************************************/
/*!
\file       Reflect.cpp
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "Reflect.hpp"

namespace Reflect
{
    RTTR_REGISTRATION
    {
        registration::method("sin", sin);

        // Registering Classes
        registration::class_<shape>("shape")
            .property("visible", &shape::get_visible, &shape::set_visible)(metadata("NO_SERIALIZE", true))
            .property("color", &shape::color_)(metadata("NO_SERIALIZE", true))
            .property("name", &shape::name)(metadata("NO_SERIALIZE", true))
            .property("position", &shape::position)//(policy::prop::bind_as_ptr)
            .property("dictionary", &shape::dictionary);//(policy::prop::bind_as_ptr);

        registration::class_<Vector3>("Vector3")
            .property("x", &Vector3::x)
            .property("y", &Vector3::y)
            .property("z", &Vector3::z);

        registration::class_<circle>("circle")
            .property("radius", &circle::radius)
            .property("points", &circle::points)//(policy::prop::bind_as_ptr)
            .property("clown", &circle::clown)//(policy::prop::bind_as_ptr)
            .property("allah", &circle::allah)//(policy::prop::bind_as_ptr)
            .property("no_serialize", &circle::no_serialize)(metadata("NO_SERIALIZE", true));

        registration::class_<point2d>("point2d")
            .constructor()(policy::ctor::as_object)
            .property("x", &point2d::x)
            .property("y", &point2d::y);

        // Registering Enum
        registration::enumeration<color>("color")
            (
                value("red", color::red),
                value("blue", color::blue),
                value("green", color::green)
            );
    }
}