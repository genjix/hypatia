//  Copyright Joel de Guzman 2002-2004. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//  Hello World Example from the tutorial
//  [Joel de Guzman 10/9/2002]

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/class.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/list.hpp>
#include <cssysdef.h>
#include <csgeom/matrix3.h>
#include <csgeom/vector3.h>
#include <vector>
#include <algorithm>
#include "trackpar.h"

csVector3 getVert(boost::python::list v)
{
    float z = boost::python::extract<float>(v.pop())(),
        y = boost::python::extract<float>(v.pop())(),
        x = boost::python::extract<float>(v.pop())();
    return csVector3(x,y,z);
}
csMatrix3 getMat(boost::python::list v)
{
    csMatrix3 mat;
    mat.m33 = boost::python::extract<float>(v.pop())();
    mat.m32 = boost::python::extract<float>(v.pop())();
    mat.m31 = boost::python::extract<float>(v.pop())();
    mat.m23 = boost::python::extract<float>(v.pop())();
    mat.m22 = boost::python::extract<float>(v.pop())();
    mat.m21 = boost::python::extract<float>(v.pop())();
    mat.m13 = boost::python::extract<float>(v.pop())();
    mat.m12 = boost::python::extract<float>(v.pop())();
    mat.m11 = boost::python::extract<float>(v.pop())();
    return mat;
}

void Track::load(boost::python::list pyverts, boost::python::list pymatrixs, GLuint text)
{
    texture = text;
    pyverts.reverse();
    int n = boost::python::extract<int>(pyverts.attr("__len__")());
    while(n--)
    {
        csVector3 v =
            getVert(boost::python::extract<boost::python::list>(pyverts.pop()));
        verts.push_back(v);
    }
    pymatrixs.reverse();
    n = boost::python::extract<int>(pymatrixs.attr("__len__")());
    while(n--)
    {
        csMatrix3 v =
            getMat(boost::python::extract<boost::python::list>(pymatrixs.pop()));
        mats.push_back(v);
    }
}

template<typename T>
static T cubicInterpolate(float t, const T p[4])
{
    float t2 = t * t;
    float t3 = t2 * t;

    float b1 = .5 * (  -t3 + 2*t2 - t);
    float b2 = .5 * ( 3*t3 - 5*t2 + 2);
    float b3 = .5 * (-3*t3 + 4*t2 + t);
    float b4 = .5 * (   t3 -   t2    );

    return p[0]*b1 + p[1]*b2 + p[2]*b3 + p[3]*b4;
}

void Track::draw()
{
    if (verts.size() < 2)
        return;
    glPushMatrix();
    glTranslatef( 0, 0, 0 );
    glBindTexture( GL_TEXTURE_2D, texture );
    glBegin( GL_TRIANGLES );
    csVector3 tablpv[4] = {verts[0], verts[0], verts[0], verts[1]};
    csMatrix3 tablpm[4] = {mats[0], mats[0], mats[0], mats[1]};
    for(uint kp = 2; kp <= verts.size(); kp++) {
        tablpv[0] = tablpv[1];
        tablpm[0] = tablpm[1];
        tablpv[1] = tablpv[2];
        tablpm[1] = tablpm[2];
        tablpv[2] = tablpv[3];
        tablpm[2] = tablpm[3];
        if(kp != verts.size()) {
            tablpv[3] = verts[kp];
            tablpm[3] = mats[kp];
        }
        float axdist = (tablpv[2] - tablpv[1]).Norm();
        csVector3 oldp = tablpv[1];
        csMatrix3 oldpm = tablpm[1];
        int steps = int(axdist)*resol;
        steps = steps > 0 ? steps : 1;
        for(int x = 1; x <= steps; x++) {
            float i = x / float(steps);
            csVector3 tweenv = cubicInterpolate(i, tablpv);
            csMatrix3 tweenm = cubicInterpolate(i, tablpm);
            drawSegment(oldp, oldpm, tweenv, tweenm);
            oldp = tweenv;
            oldpm = tweenm;
        }
    }
    glEnd();
    glPopMatrix();
}
void Track::drawSegment(const csVector3 &pp, const csMatrix3 &mp, const csVector3& pc, const csMatrix3& mc)
{
    csVector3 right1 = csVector3(mp.m11, mp.m12, mp.m13)*25;
    csVector3 right2 = csVector3(mc.m11, mc.m12, mc.m13)*25;
    csVector3 corn1 = pp + right1, corn2 = pc + right2;
    csVector3 up(mc.m21, mc.m22, mc.m23);
    glNormal3f(up.x, up.y, up.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(pc.x, pc.y, pc.z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(pp.x, pp.y, pp.z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(corn1.x, corn1.y,  corn1.z);

    glNormal3f(up.x, up.y, up.z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(corn1.x, corn1.y,  corn1.z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(corn2.x, corn2.y,  corn2.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(pc.x, pc.y, pc.z);

    up = csVector3(mc.m21, mc.m22, mc.m23)*3 + pc;
    glNormal3f(right2.x, right2.y, right2.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(pc.x, pc.y, pc.z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(pp.x, pp.y, pp.z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(up.x, up.y,  up.z);

    up = csVector3(mc.m21, mc.m22, mc.m23)*3 + pc + right2;
    glNormal3f(-right2.x, -right2.y, -right2.z);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(corn1.x, corn1.y,  corn1.z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(corn2.x, corn2.y,  corn2.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(up.x, up.y,  up.z);
}
boost::python::tuple Track::attachToTrack(const csVector3 &pp)
{
    int closest_idx[2] = {0, 1};
    float dsqlow[2] = {(pp - verts[0]).SquaredNorm(), (pp - verts[1]).SquaredNorm()};
    for(uint i = 0; i < verts.size(); i++) {
        const float dsqcur = (pp - verts[i]).SquaredNorm();
        if(dsqcur < dsqlow[0]) {
            dsqlow[0] = dsqcur;
            closest_idx[0] = i;
        } else if(dsqcur < dsqlow[1]) {
            dsqlow[1] = dsqcur;
            closest_idx[1] = i;
        }
    }
    if(closest_idx[0] > closest_idx[1])
        std::swap(closest_idx[0], closest_idx[1]);  // always 0->1
    // distance along line segment
    const csVector3 left(verts[closest_idx[0]]), right(verts[closest_idx[1]]),
        edgediff (right - left);
    const float u = (pp - left) * (edgediff / edgediff.SquaredNorm ());
    // u is interpolation value along line from 0->1
    printf("%f\n", u);
    return boost::python::make_tuple(csVector3(0), csVector3(0));
}
