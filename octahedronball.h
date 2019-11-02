#ifndef OCTAHEDRONBALL_H
#define OCTAHEDRONBALL_H

#include "visualobject.h"

class OctahedronBall : public VisualObject
{
private:
    int m_rekursjoner;
    int m_indeks;               // brukes i rekursjon, til å bygge m_vertices
    void lagTriangel(const gsl::Vector3D& v1, const gsl::Vector3D& v2, const gsl::Vector3D& v3);
    void subDivide(const gsl::Vector3D& a, const gsl::Vector3D& b, const gsl::Vector3D& c, int n);
    void oktaederUnitBall();
public:
    OctahedronBall(int n=0);

    void init() override;
    void draw() override;

    ~OctahedronBall() override;
};

#endif // OCTAHEDRONBALL_H
