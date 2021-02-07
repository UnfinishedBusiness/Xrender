#include <Xrender.h>
#include <string>
#include <dxf/DXFParse_Class.h>
#include <dxflib/dl_dxf.h>
#include <geometry/geometry.h>
#include <dxf/Bezier.h>
#include <json/json.h>

using namespace std;

void Xrender_parse_dxf_file(string filename, void (*callback)(nlohmann::json, int, int))
{
    Geometry g;
    DXFParse_Class* creationClass = new DXFParse_Class();
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(filename, creationClass)) { // if file open failed
        std::cerr << filename << " could not be opened.\n";
        return;
    }
    delete dxf;
    if (creationClass->current_polyline.points.size() > 0)
    {
        creationClass->polylines.push_back(creationClass->current_polyline); //Push last polyline
        creationClass->current_polyline.points.clear();
    }
    if (creationClass->current_spline.points.size() > 0)
    {
        creationClass->splines.push_back(creationClass->current_spline); //Push last spline
        creationClass->current_spline.points.clear();
    }
    //Convert splines to line segments
    for (int x = 0; x < creationClass->splines.size(); x++)
    {
        std::vector<Point> pointList;
        std::vector<Point> out;
        Curve* curve = new Bezier();
	    curve->set_steps(100);
        //printf("Iterating spline(%d)\n", x);
        for (int y = 0; y < creationClass->splines[x].points.size(); y++)
        {
            curve->add_way_point(Vector(creationClass->splines[x].points[y].x, creationClass->splines[x].points[y].y, 0));
        }
        for (int i = 0; i < curve->node_count(); i++)
        {
            pointList.push_back(Point(curve->node(i).x, curve->node(i).y));
        }
        g.RamerDouglasPeucker(pointList, 0.003, out);
        for (int i = 1; i < out.size(); i++)
        {
            //std::cout << "node #" << i << ": X: " << curve->node(i).x << " Y:" << curve->node(i).y << " (length so far: " << curve->length_from_starting_point(i) << ")" << std::endl;
            nlohmann::json line;
            line["layer"] = "spline-" + std::to_string(x);
            line["type"] = "line";
            //line["color"]["r"] = 0;
            //line["color"]["g"] = 1;
            //line["color"]["b"] = 0;
            line["start"]["x"] = (double)out[i-1].first;
            line["start"]["y"] = (double)out[i-1].second;
            line["start"]["z"] = (double)0;
            line["end"]["x"] = (double)out[i].first;
            line["end"]["y"] = (double)out[i].second;
            line["end"]["z"] = (double)0;
            creationClass->dxfJSON.push_back(line);
        }
        if (creationClass->splines[x].isClosed == true)
        {
            nlohmann::json line;
            line["layer"] = "spline-" + std::to_string(x);
            line["type"] = "line";
            //line["color"]["r"] = 0;
            //line["color"]["g"] = 1;
            //line["color"]["b"] = 0;
            line["start"]["x"] = (double)curve->node(0).x;
            line["start"]["y"] = (double)curve->node(0).y;
            line["start"]["z"] = (double)0;
            line["end"]["x"] = (double)curve->node(curve->node_count()-1).x;
            line["end"]["y"] = (double)curve->node(curve->node_count()-1).y;
            line["end"]["z"] = (double)0;
            creationClass->dxfJSON.push_back(line);
        }
        delete curve;
    }
    for (int x = 0; x < creationClass->dxfJSON.size(); x++)
    {
        callback(creationClass->dxfJSON[x], x, creationClass->dxfJSON.size());
    }
    delete creationClass;
}