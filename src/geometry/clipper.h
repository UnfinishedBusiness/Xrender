/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  6.1.3 (float) - Experimental                                    *
* Date      :  16 January 2014                                                 *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2014                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
* Computer graphics and geometric modeling: implementation and algorithms      *
* By Max K. Agoston                                                            *
* Springer; 1 edition (January 4, 2005)                                        *
* http://books.google.com/books?q=vatti+clipping+agoston                       *
*                                                                              *
* See also:                                                                    *
* "Polygon Offsetting by Computing Winding Numbers"                            *
* Paper no. DETC2005-85513 pp. 565-575                                         *
* ASME 2005 International Design Engineering Technical Conferences             *
* and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
* September 24-28, 2005 , Long Beach, California, USA                          *
* http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
*                                                                              *
*******************************************************************************/

#ifndef clipper_hpp
#define clipper_hpp

#define CLIPPER_VERSION "6.1.3"

//use_xyz: adds a Z member to FPoint. Adds a minor cost to perfomance.
//#define use_xyz

//use_lines: Enables line clipping. Adds a very minor cost to performance.
//#define use_lines
  
//use_deprecated: Enables support for the obsolete OffsetPaths() function
//which has been replace with the ClipperOffset class.
#define use_deprecated  

#include <vector>
#include <set>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <ostream>
#include <functional>

namespace ClipperLib {

enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
enum PolyType { ptSubject, ptClip };
//By far the most widely used winding rules for polygon filling are
//EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
//Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
//see http://glprogramming.com/red/chapter11.html
enum PolyFillType { pftEvenOdd, pftNonZero, pftPositive, pftNegative };

struct FPoint {
  double X;
  double Y;
#ifdef use_xyz
  long long Z;
  FPoint(double x = 0, double y = 0, long long z = 0): X(x), Y(y), Z(z) {};
#else
  FPoint(double x = 0, double y = 0): X(x), Y(y) {};
#endif

  friend inline bool operator== (const FPoint& a, const FPoint& b);
  friend inline bool operator!= (const FPoint& a, const FPoint& b);
};
//------------------------------------------------------------------------------

typedef std::vector< FPoint > Path;
typedef std::vector< Path > Paths;

inline Path& operator <<(Path& poly, const FPoint& p) {poly.push_back(p); return poly;}
inline Paths& operator <<(Paths& polys, const Path& p) {polys.push_back(p); return polys;}

std::ostream& operator <<(std::ostream &s, const FPoint &p);
std::ostream& operator <<(std::ostream &s, const Path &p);
std::ostream& operator <<(std::ostream &s, const Paths &p);

//------------------------------------------------------------------------------

#ifdef use_xyz
typedef void (*TZFillCallback)(FPoint& z1, FPoint& z2, FPoint& pt);
#endif

enum InitOptions {ioReverseSolution = 1, ioStrictlySimple = 2, ioPreserveCollinear = 4};
enum JoinType {jtSquare, jtRound, jtMiter};
enum EndType {etClosedPolygon, etClosedLine, etOpenButt, etOpenSquare, etOpenRound};
#ifdef use_deprecated
  enum EndType_ {etClosed, etButt = 2, etSquare, etRound};
#endif

class PolyNode;
typedef std::vector< PolyNode* > PolyNodes;

class PolyNode 
{ 
public:
    PolyNode();
    Path Contour;
    PolyNodes Childs;
    PolyNode* Parent;
    PolyNode* GetNext() const;
    bool IsHole() const;
    bool IsOpen() const;
    int ChildCount() const;
private:
    unsigned Index; //node index in Parent.Childs
    bool m_IsOpen;
    JoinType m_jointype;
    EndType m_endtype;
    PolyNode* GetNextSiblingUp() const;
    void AddChild(PolyNode& child);
    friend class Clipper; //to access Index
    friend class ClipperOffset; 
};

class PolyTree: public PolyNode
{ 
public:
    ~PolyTree(){Clear();};
    PolyNode* GetFirst() const;
    void Clear();
    int Total() const;
private:
    PolyNodes AllNodes;
    friend class Clipper; //to access AllNodes
};

bool Orientation(const Path &poly);
double Area(const Path &poly);

#ifdef use_deprecated
  void OffsetPaths(const Paths &in_polys, Paths &out_polys,
    double delta, JoinType jointype, EndType_ endtype, double limit = 0);
#endif

void SimplifyPolygon(const Path &in_poly, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(const Paths &in_polys, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(Paths &polys, PolyFillType fillType = pftEvenOdd);

void CleanPolygon(const Path& in_poly, Path& out_poly, double distance = 1.415);
void CleanPolygon(Path& poly, double distance = 1.415);
void CleanPolygons(const Paths& in_polys, Paths& out_polys, double distance = 1.415);
void CleanPolygons(Paths& polys, double distance = 1.415);

void MinkowskiSum(const Path& poly, const Path& path, Paths& solution, bool isClosed);
void MinkowskiDiff(const Path& poly, const Path& path, Paths& solution, bool isClosed);

void PolyTreeToPaths(const PolyTree& polytree, Paths& paths);
void ClosedPathsFromPolyTree(const PolyTree& polytree, Paths& paths);
void OpenPathsFromPolyTree(PolyTree& polytree, Paths& paths);

void ReversePath(Path& p);
void ReversePaths(Paths& p);

struct FRect { double left; double top; double right; double bottom; };

//enums that are used internally ...
enum EdgeSide { esLeft = 1, esRight = 2};

//forward declarations (for stuff used internally) ...
struct TEdge;
struct IntersectNode;
struct LocalMinima;
struct Scanbeam;
struct OutPt;
struct OutRec;
struct Join;

typedef std::vector < OutRec* > PolyOutList;
typedef std::vector < TEdge* > EdgeList;
typedef std::vector < Join* > JoinList;
typedef std::vector < IntersectNode* > IntersectList;


//------------------------------------------------------------------------------

//ClipperBase is the ancestor to the Clipper class. It should not be
//instantiated directly. This class simply abstracts the conversion of sets of
//polygon coordinates into edge objects that are stored in a LocalMinima list.
class ClipperBase
{
public:
  ClipperBase();
  virtual ~ClipperBase();
  bool AddPath(const Path &pg, PolyType PolyTyp, bool Closed);
  bool AddPaths(const Paths &ppg, PolyType PolyTyp, bool Closed);
  virtual void Clear();
  FRect GetBounds();
  bool PreserveCollinear() {return m_PreserveCollinear;};
  void PreserveCollinear(bool value) {m_PreserveCollinear = value;};
protected:
  void DisposeLocalMinimaList();
  TEdge* AddBoundsToLML(TEdge *e, bool IsClosed);
  void PopLocalMinima();
  virtual void Reset();
  TEdge* ProcessBound(TEdge* E, bool IsClockwise);
  void InsertLocalMinima(LocalMinima *newLm);
  void DoMinimaLML(TEdge* E1, TEdge* E2, bool IsClosed);
  TEdge* DescendToMin(TEdge *&E);
  void AscendToMax(TEdge *&E, bool Appending, bool IsClosed);
  LocalMinima      *m_CurrentLM;
  LocalMinima      *m_MinimaList;
  EdgeList          m_edges;
  bool             m_PreserveCollinear;
  bool             m_HasOpenPaths;
};
//------------------------------------------------------------------------------

class Clipper : public virtual ClipperBase
{
public:
  Clipper(int initOptions = 0);
  ~Clipper();
  bool Execute(ClipType clipType,
    Paths &solution,
    PolyFillType subjFillType = pftEvenOdd,
    PolyFillType clipFillType = pftEvenOdd);
  bool Execute(ClipType clipType,
    PolyTree &polytree,
    PolyFillType subjFillType = pftEvenOdd,
    PolyFillType clipFillType = pftEvenOdd);
  void Clear();
  bool ReverseSolution() {return m_ReverseOutput;};
  void ReverseSolution(bool value) {m_ReverseOutput = value;};
  bool StrictlySimple() {return m_StrictSimple;};
  void StrictlySimple(bool value) {m_StrictSimple = value;};
  //set the callback function for z value filling on intersections (otherwise Z is 0)
#ifdef use_xyz
  void ZFillFunction(TZFillCallback zFillFunc);
#endif
protected:
  void Reset();
  virtual bool ExecuteInternal();
private:
  PolyOutList       m_PolyOuts;
  JoinList          m_Joins;
  JoinList          m_GhostJoins;
  IntersectList     m_IntersectList;
  ClipType          m_ClipType;
  std::set< double, std::greater<double> > m_Scanbeam;
  TEdge           *m_ActiveEdges;
  TEdge           *m_SortedEdges;
  bool             m_ExecuteLocked;
  PolyFillType     m_ClipFillType;
  PolyFillType     m_SubjFillType;
  bool             m_ReverseOutput;
  bool             m_UsingPolyTree; 
  bool             m_StrictSimple;
#ifdef use_xyz
  TZFillCallback   m_ZFill; //custom callback 
#endif
  void SetWindingCount(TEdge& edge);
  bool IsEvenOddFillType(const TEdge& edge) const;
  bool IsEvenOddAltFillType(const TEdge& edge) const;
  void InsertScanbeam(const double Y);
  double PopScanbeam();
  void InsertLocalMinimaIntoAEL(const double botY);
  void InsertEdgeIntoAEL(TEdge *edge, TEdge* startEdge);
  void AddEdgeToSEL(TEdge *edge);
  void CopyAELToSEL();
  void DeleteFromSEL(TEdge *e);
  void DeleteFromAEL(TEdge *e);
  void UpdateEdgeIntoAEL(TEdge *&e);
  void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
  bool IsContributing(const TEdge& edge) const;
  bool IsTopHorz(const double XPos);
  void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
  void DoMaxima(TEdge *e);
  void PrepareHorzJoins(TEdge* horzEdge, bool isTopOfScanbeam);
  void ProcessHorizontals(bool IsTopOfScanbeam);
  void ProcessHorizontal(TEdge *horzEdge, bool isTopOfScanbeam);
  void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const FPoint &pt);
  OutPt* AddLocalMinPoly(TEdge *e1, TEdge *e2, const FPoint &pt);
  OutRec* GetOutRec(int idx);
  void AppendPolygon(TEdge *e1, TEdge *e2);
  void IntersectEdges(TEdge *e1, TEdge *e2,
    const FPoint &pt, bool protect = false);
  OutRec* CreateOutRec();
  OutPt* AddOutPt(TEdge *e, const FPoint &pt);
  void DisposeAllOutRecs();
  void DisposeOutRec(PolyOutList::size_type index);
  bool ProcessIntersections(const double botY, const double topY);
  void BuildIntersectList(const double botY, const double topY);
  void ProcessIntersectList();
  void ProcessEdgesAtTopOfScanbeam(const double topY);
  void BuildResult(Paths& polys);
  void BuildResult2(PolyTree& polytree);
  void SetHoleState(TEdge *e, OutRec *outrec);
  void DisposeIntersectNodes();
  bool FixupIntersectionOrder();
  void FixupOutPolygon(OutRec &outrec);
  bool IsHole(TEdge *e);
  bool FindOwnerFromSplitRecs(OutRec &outRec, OutRec *&currOrfl);
  void FixHoleLinkage(OutRec &outrec);
  void AddJoin(OutPt *op1, OutPt *op2, const FPoint offPt);
  void ClearJoins();
  void ClearGhostJoins();
  void AddGhostJoin(OutPt *op, const FPoint offPt);
  bool JoinPoints(Join *j, OutRec* outRec1, OutRec* outRec2);
  void JoinCommonEdges();
  void DoSimplePolygons();
  void FixupFirstLefts1(OutRec* OldOutRec, OutRec* NewOutRec);
  void FixupFirstLefts2(OutRec* OldOutRec, OutRec* NewOutRec);
#ifdef use_xyz
  void SetZ(FPoint& pt, TEdge& e);
#endif
};
//------------------------------------------------------------------------------

class ClipperOffset 
{
public:
  ClipperOffset(double miterLimit = 2.0, double roundPrecision = 0.25);
  ~ClipperOffset();
  void AddPath(const Path& path, JoinType joinType, EndType endType);
  void AddPaths(const Paths& paths, JoinType joinType, EndType endType);
  void Execute(Paths& solution, double delta);
  void Execute(PolyTree& solution, double delta);
  void Clear();
  double MiterLimit;
  double ArcTolerance;
private:
  Paths m_destPolys;
  Path m_srcPoly;
  Path m_destPoly;
  std::vector<FPoint> m_normals;
  double m_delta, m_sinA, m_sin, m_cos;
  double m_miterLim, m_StepsPerRad;
  FPoint m_lowest;
  PolyNode m_polyNodes;

  void FixOrientations();
  void DoOffset(double delta);
  void OffsetPoint(int j, int& k, JoinType jointype);
  void DoSquare(int j, int k);
  void DoMiter(int j, int k, double r);
  void DoRound(int j, int k);
};
//------------------------------------------------------------------------------

class clipperException : public std::exception
{
  public:
    clipperException(const char* description): m_descr(description) {}
    virtual ~clipperException() throw() {}
    virtual const char* what() const throw() {return m_descr.c_str();}
  private:
    std::string m_descr;
};
//------------------------------------------------------------------------------

} //ClipperLib namespace

#endif //clipper_hpp

