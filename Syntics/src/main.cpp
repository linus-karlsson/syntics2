#include "syntic_app.h"
#include "math/transforms.h"
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char* argv[])
{
    synt::run_app(argc, argv);

    //    using namespace synt;
    //    auto test = synt::view(Vec3(1.0f, 2.0f, 3.0f), Vec3(1.0f, 2.0f, 3.0f),
    //                           Vec3(0.0f, 0.0f, 1.0f));
    //
    //    auto test_in = inverse(test);
    //
    //    auto mat1 = glm::lookAt(glm::vec3{ 1.0f, 2.0f, 3.0f },
    //    glm::vec3{ 1.0f, 2.0f, 3.0f },
    //                            glm::vec3{ 0.0f, 0.0f, 1.0f });
    //
    //    auto m_invers = glm::inverse(mat1);
    //    auto dd       = m_invers * glm::vec4(7.0f, 3.9f, 1, 1);
    //
    //    printf("Pos: (x: %f, y: %f, z: %f, w: %f)\n", dd.x, dd.y, dd.z, dd.w);
    //
    //    for (int r = 0; r < 4; r++)
    //    {
    //        printf("Row: %d [ ", r + 1);
    //        for (int c = 0; c < 4; c++)
    //        {
    //            printf("%f ", m_invers[c][r]);
    //        }
    //        printf("]\n");
    //    }

    //    Vec4 first(1.3f, 2.11f, 3, 4);
    //    Vec4 second(2, 3, 4, 6.32f);
    //
    //    Vec4 res1 = first * second;
    //
    //    auto v1   = glm::vec4(1.3f, 2.11f, 3, 4);
    //    auto v2   = glm::vec4(2, 3, 4, 6.32f);
    //    auto res2 = v1 * v2;
    //
    //    printf("Pos: (x: %f, y: %f, z: %f, w: %f)\n", res1.x, res1.y, res1.z, res1.w);
    //    printf("Pos: (x: %f, y: %f, z: %f, w: %f)\n", res2.x, res2.y, res2.z, res2.w);

    return 0;
}
