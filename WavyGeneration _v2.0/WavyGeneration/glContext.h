// glContext.h: interface for the glContext class.
//
//////////////////////////////////////////////////////////////////////

// 2012-5-1: 
// 增加了实体选择功能

#pragma once

#include "GeomBase.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <gl/glaux.h>
#pragma comment(lib,"OPENGL32.LIB") // 基本库，gl打头的函数都在这个库里
#pragma comment(lib,"glu32.lib")    // 实用库
//#pragma comment(lib,"glut32.lib")   // 民间做的实用库
#pragma comment(lib,"glut.lib")     // 和上面一样的,包含一个就可以了
#pragma comment(lib,"GlAux.Lib")    // 官方发行的辅助库


#define VIEW_FRONT      0
#define VIEW_BACK       1
#define VIEW_TOP        2
#define VIEW_BOTTOM     3
#define VIEW_RIGHT      4
#define VIEW_LEFT       5
#define VIEW_SW         6
#define VIEW_SE         7
#define VIEW_NE         8
#define VIEW_NW         9

#define ST_RECT         0   // selected type
#define ST_CIRCLE       1
#define ST_TRIANGLE     2

#define LS_SOLID        0   // line style
#define LS_DASH         1
#define LS_CENTER       2

#define SEL_BUFSIZE     65536

#define CLR_SELECTED    RGB(0,0,0)


class AFX_CLASS_EXPORT CCamera  
{
public:
	CPoint3D m_eye;
	CPoint3D m_ref;
	CVector3D m_vecUp;                       
	
	double m_width,m_height,m_near,m_far;    // 视景体
	double m_screenX,m_screenY;              // 屏幕

	// For select purpose
	int    m_renderMode;
	GLuint m_selBuf[SEL_BUFSIZE];
	CPoint m_hitPt1;  // The first hit point
	CPoint m_hitPt2;  // The second hit point, these two points ara for selection purpose
		
public:
	CCamera();
	virtual ~CCamera();
	
	void Init();                                   // 初始化
	
	void SetEye(CPoint3D pt);
	void SetRef(CPoint3D pt);
	void SetVecUp(CVector3D vec);                 // 视点
	
	void SetViewVolume(double width,double height,double near, double far);  //视景体
	void GetViewRect(double& width, double& height);
	
	void SetViewType(int type);                     // 观察视图
	void MoveView(double dpx,double dpy);           // 移动视图
	void MoveView(CPoint ptFrom,CPoint ptTo);       // 重写移动视图函数，ptFrom 记录按下鼠标时的点，ptTo 为移动到的鼠标点
	void RotateView(CPoint ptFrom,CPoint ptTo);     // 绕当前参考点选转视图，也就是屏幕中心点。ptFrom是初始点
	void Zoom(double scale);
	
	void Projection();                              // 投影
	
	void SetViewport(int cx,int cy);         //设置视口尺寸,包含了m_aspect的设置
	
	void UpdateVecUp();

	CPoint3D ScreenToCLClient(CPoint ptScreen);
	CPoint GLClientToScreen(CPoint3D ptGLClient);
};


class AFX_CLASS_EXPORT COpenGLDC  
{
public:
	COpenGLDC();
	virtual ~COpenGLDC();
	
public:
	COpenGLDC(CWnd* pWnd);
	BOOL SetupPixelFormat();
	void GLResize(int cx,int cy);
	void GLRelease();
	void Ready();
	void Finish();
	void GLSetupRC();
	void SetRenderMode(int nRenderMode=GL_RENDER);
	int  GetRenderMode();
	void SetSelRect(CPoint pt1, CPoint pt2);
	void GetSelected(CArray<int,int>& selAry);     // Get the selected objects in GL_SELECT mode
	CDC* GetCDC();

public:
	CPoint3D ScreenToGLClient(CPoint ptScreen);  // 定义了屏幕窗口坐标点到GL坐标系点的转换。平面的
	CPoint   GLClientToScreen(CPoint3D ptGLClient);
	
public:
	CCamera m_camera;
private:
	CWnd* m_pWnd;
	CDC*  m_pDC;
	HDC   m_hDC;
	HGLRC m_hRC;

	int      m_hits;
	COLORREF m_clrBk;
	COLORREF m_clr;
	COLORREF m_clrHighLight;
	COLORREF m_clrMaterial;
	BOOL     m_bShading;
	GLfloat  m_vecLight[3];
	
protected:
	void OnShading();
	
public:	
	void Shading(BOOL bShading);
	BOOL IsShading();
	
	void Lighting(BOOL bLighting);
	BOOL IsLighting();
	
	void SetLightDirection(float dx,float dy,float dz);
	void GetLightDirection(float& dx,float& dy,float& dz);
	
	COLORREF SetMaterialColor(COLORREF clr);  // return old color
	void GetMaterialColor(COLORREF& clr);
	
	void ClearBkGround();
	void SetBkColor(COLORREF clr=0xffffff);
	void GetBkColor(COLORREF& clr);
	
	void SetColor(COLORREF clr);
	void GetColor(COLORREF& clr);
	
	void SetHighLightColor(COLORREF clr);
	void GetHighLightColor(COLORREF& clr);
	void HighLight(BOOL bHighLight=TRUE);
	
public:                       // 绘图
	void DrawCoord();
	void DrawArrow(CPoint3D ptCenter,CVector3D vec_i,CVector3D vec_j,double len);  // 坐标轴的箭头，这里是一个线框
	void DrawSelPoint(const CPoint3D& pt,int ShapeType=ST_RECT,COLORREF clr=RGB(0,0,255));
	void DrawPoint(const CPoint3D& pt, double pointSize=1.0);
	//void DrawLine(const CPoint3D& sp,const CPoint3D& ep,int lineStyle=LS_SOLID,int lineWidth=1);
	void DrawLine(const CPoint3D& sp,const CPoint3D& ep,int lineStyle=LS_SOLID,int lineWidth=1, COLORREF clr=RGB(255,255,0));
	void DrawLine(double x1,double y1,double z1,double x2,double y2,double z2);
	void DrawPlaneRect(const CPoint3D& ptLeftTop, const CPoint3D& ptRightBottom, int lineStyle=LS_SOLID,int lineWidth=1);
	void DrawCircle(const CPoint3D& ptCenter,const double& r, COLORREF clr=RGB(255,255,0), int precision=20);   // 绘制xy平面圆
	void DrawPolyline(CPoint3D pt[],int sz, BOOL bClosed=TRUE);  // 画多变边形,
    void DrawPolygon(CPoint3D pt[], int sz);          // 画多边形面
	void DrawPolygon(Point3DAry &ptList,BOOL bDrawFrame=TRUE);
	void DrawTriChip(double n0, double n1, double n2, double v00,double v01,double v02,
		             double v10,double v11,double v12,double v20,double v21,double v22);
	void DrawTriChip(CVector3D vec,CPoint3D pt1,CPoint3D pt2,CPoint3D pt3);
	void DrawTriFrame(CPoint3D A,CPoint3D B,CPoint3D C);
	void DrawShere(const CPoint3D& center,double r,const CVector3D& vec);
	void DrawCylinder(const CPoint3D& center,double r,const CVector3D& h);
	void DrawTorus(const CPoint3D& center,const CVector3D& axis,double r_in,double r_out);
	void DrawCone(const CPoint3D& center,double r,const CVector3D& h);
	void DrawText(CString str,CPoint3D pt,COLORREF clr=RGB(255,255,255));
	void DrawPolygonFrame(const CTypedPtrArray<CObArray,CPoint3D*>& ptList);                 // 绘制多边形框架
	int Is2PtOn2SideOfLine(const CPoint3D& ptS_line,const CPoint3D& ptE_line,const CPoint3D& pt1,const CPoint3D& pt2);  // 分别是直线的起点和终点，以及需要检测的两个点
    int GetCrossPoint(CPoint3D& pt_out,CPoint3D pt1_line1,CPoint3D pt2_line1,CPoint3D pt1_line2,CPoint3D pt2_line2);
	CVector3D GetNormalVector(CPoint3D pt_start,CPoint3D pt_end);
};
