#ifndef TRACKPEOPLE_TRACKPEOPLE_H
#define TRACKPEOPLE_TRACKPEOPLE_H

#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>

struct Track
{
    Track() : resol(1) {}
    void load(boost::python::list pyverts, boost::python::list pymatrixs, GLuint text);
    void drawSegment(const csVector3 &pp, const csMatrix3 &mp,
        const csVector3& pc, const csMatrix3& mc);
    boost::python::tuple attachToTrack(const csVector3 &playerpos);
    void draw();
    float resol;
    std::vector<csVector3> verts;
    std::vector<csMatrix3> mats;
    GLuint texture;
};

#endif
