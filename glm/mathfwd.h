#ifndef MATHFWD_H
#define MATHFWD_H

/** Forward declarations file for math library
 * Include so that you don't need to include
 * all the separate classes that is included
 * in the math.h header.
 * @author andesyv
 */

namespace glm
{
    class Vector2D;
    class Vector3D;
    class Vector4D;

    class Matrix2x2;
    class Matrix3x3;
    class Matrix4x4;

    // Typedefs
#ifndef MATH_TYPEDEF
#define MATH_TYPEDEF
    typedef Vector2D vec2;
    typedef Vector3D vec3;
    typedef Vector4D vec4;
    typedef Vector4D vec;
    typedef Matrix2x2 mat2;
    typedef Matrix3x3 mat3;
    typedef Matrix4x4 mat4;
    typedef Matrix4x4 mat;
#endif
}

#endif // MATHFWD_H
