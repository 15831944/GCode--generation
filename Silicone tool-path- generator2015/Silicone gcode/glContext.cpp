// glContext1.cpp: implementation of the glContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "glContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCamera::CCamera()
{

}

CCamera::~CCamera()
{

}

void CCamera::Init()                                   // 初始化
{
	m_eye=CPoint3D(0,0,1000);  // 眼睛相对于世界坐标系原点的坐标值
	m_ref=CPoint3D(0,0,0);
	m_vecUp=CVector3D(0,1,0);

	m_width=1000;
	m_height=1000;
	m_near=0.1;   // 相对于人眼的距离
	m_far=10000;

	m_screenX=400;
	m_screenY=400;

	m_renderMode=GL_RENDER;
}

void CCamera::SetEye(CPoint3D pt)
{
	m_eye=pt;
}

void CCamera::SetRef(CPoint3D pt)
{
	m_ref=pt;
}

void CCamera::SetVecUp(CVector3D vec)
{
	m_vecUp=vec;
}                

void CCamera::SetViewVolume(double width,double height,double dNear, double dFar)
{
	m_width=width;
	m_height=height;
	double aspect=m_screenX/m_screenY;
	m_width=m_height*aspect;
	m_near=dNear;
	m_far=dFar;
}  

void CCamera::GetViewRect(double& width, double& height)
{
	width=m_width;
	height=m_height;
}

void CCamera::SetViewType(int type) 
{
	CVector3D vec;
	double r;
	vec=m_ref-m_eye;
	r=vec.GetLength();
	switch(type)
	{
	case VIEW_FRONT:
		m_eye=m_ref+CVector3D(r,0,0);
		m_vecUp=CVector3D(0,0,1);
		break;
	case VIEW_BACK:
		m_eye=m_ref+CVector3D(-r,0,0);
		m_vecUp=CVector3D(0,0,1);
		break;
	case VIEW_RIGHT:
		m_eye=m_ref+CVector3D(0,r,0);
		m_vecUp=CVector3D(0,0,1);
		break;
	case VIEW_LEFT:
		m_eye=m_ref+CVector3D(0,-r,0);
		m_vecUp=CVector3D(0,0,1);
		break;
	case VIEW_TOP:
		m_eye=m_ref+CVector3D(0,0,r);
		m_vecUp=CVector3D(0,1,0);
		break;
	case VIEW_BOTTOM:
		m_eye=m_ref+CVector3D(0,0,-r);
		m_vecUp=CVector3D(0,1,0);
		break;
	case VIEW_SE:
		m_eye=m_ref+CVector3D(1,1,1).GetNormal()*r;
		UpdateVecUp();
		break;
	case VIEW_SW:
		m_eye=m_ref+CVector3D(1,-1,1).GetNormal()*r;
		UpdateVecUp();
		break;
	case VIEW_NE:
		m_eye=m_ref+CVector3D(-1,1,1).GetNormal()*r;
		UpdateVecUp();
		break;
	case VIEW_NW:
		m_eye=m_ref+CVector3D(-1,-1,1).GetNormal()*r;
		UpdateVecUp();
		break;
	}
} 
                  
void CCamera::MoveView(double dpx,double dpy)
{
	CVector3D vec=m_ref-m_eye;

	CVector3D xUp,yUp;
	vec.Normalize();
	xUp=vec*m_vecUp;
	yUp=xUp*vec;
	
	m_eye-=xUp*m_width*dpx+yUp*m_height*dpy;
	m_ref-=xUp*m_width*dpx+yUp*m_height*dpy;
}   

void CCamera::MoveView(CPoint ptFrom,CPoint ptTo)
{
	int cx, cy;
	double ratioX, ratioY;
	cx=ptTo.x-ptFrom.x;
	cy=ptTo.y- ptFrom.y;
	ratioX=(double)cx/m_screenX;
	ratioY=-(double)cy/m_screenY;
	MoveView(ratioX,ratioY);
}

void CCamera::RotateView(CPoint ptFrom,CPoint ptTo)
{
	double factor=5;

	CVector3D vec_ref_to_eye=m_eye-m_ref;
	double r=vec_ref_to_eye.GetLength();
	double len_x=(double)ptTo.x-(double)ptFrom.x; // x 方向上
	double theta=factor*len_x/r;
	CVector3D vec1=vec_ref_to_eye*cos(theta);
	CVector3D vec2=m_vecUp*vec_ref_to_eye;
	CVector3D vec_n=vec2;  // 记住v2方向，下面计算有用
	vec2.Normalize();
	vec2=vec2*(r*sin(theta));
	m_eye=m_ref+vec1+vec2;

	vec_ref_to_eye=m_eye-m_ref;
	r=vec_ref_to_eye.GetLength();   
    double len_y=(double)ptTo.y-(double)ptFrom.y; // y 方向上
	theta=factor*len_y/r;
	vec1=vec_ref_to_eye*cos(theta);
	vec2=m_vecUp;
	vec2=vec2*(r*sin(theta));
	CVector3D vec=vec1+vec2;
	m_eye=m_ref+vec; 
	m_vecUp=vec*vec_n;
	m_vecUp.Normalize();
}

void CCamera::Zoom(double scale)  
{
	m_height*=scale;
	m_width*=scale;
} 

void CCamera::Projection()
{
	// For GL_SELECT mode popurse
	GLint viewport[4];
	if (GL_SELECT==m_renderMode)
	{
		glGetIntegerv(GL_VIEWPORT, viewport);     //获得视口, 左下角点 到 右上角点
		glSelectBuffer(SEL_BUFSIZE, m_selBuf);    //指定存储点击记录的数组
		glRenderMode(GL_SELECT);                  //进入选择模式
		glInitNames();							  //初始化名字堆栈并压入初始元素
		glPushName(0);
	}

	glMatrixMode(GL_PROJECTION);

	if (GL_SELECT==m_renderMode)
		glPushMatrix();

	glLoadIdentity();

   // For GL_SELECT mode popurse
	if (GL_SELECT==m_renderMode)
	{
		double x=(double)(m_hitPt1.x+m_hitPt2.x)/2;
		double y=(double)(viewport[3]-(m_hitPt1.y+m_hitPt2.y)/2);
		double w=abs(m_hitPt2.x-m_hitPt1.x);
		double h=abs(m_hitPt2.y-m_hitPt1.y);
		if (w<5) w=5;
		if (h<5) h=5;

		gluPickMatrix(x,y,w,h,viewport);
	}

	glOrtho(-m_width/2,m_width/2,-m_height/2,m_height/2,m_near,m_far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_eye.x,m_eye.y,m_eye.z,m_ref.x,m_ref.y,m_ref.z,m_vecUp.dx,m_vecUp.dy,m_vecUp.dz);
}                              

void CCamera::SetViewport(int cx,int cy)           //放在COpenGLDC的Resize函数中放在View类的OnSize中；；；；
{
	glViewport(0,0,cx,cy);
	
	double ratio=(double)cx/(double)cy;
	m_width*=(double)cx/m_screenX;
	m_height*=(double)cy/m_screenY;
	m_width=m_height*ratio;
	m_screenX=(double)cx;
	m_screenY=(double)cy;
}      

void CCamera::UpdateVecUp()
{
	CVector3D vec=m_ref-m_eye;
	CVector3D zVec(0,0,1);
	CVector3D vec0;

	vec.Normalize();
	vec0=vec*zVec;
	m_vecUp=vec0*vec;
}

CPoint3D CCamera::ScreenToCLClient(CPoint ptScreen)
{
	double x,y;
	double ration=m_height/m_screenY;
	x=m_eye.x-m_width*0.5+ration*(double)(ptScreen.x);
	y=m_eye.y+m_height*0.5-ration*(double)ptScreen.y;

	return CPoint3D(x,y,0);
}

CPoint CCamera::GLClientToScreen(CPoint3D ptGLClient)
{
	int x,y;
	double ration=m_height/m_screenY;
	x=(int)(ptGLClient.x-m_eye.x+m_width*0.5)/ration;
	y=(int)(-(ptGLClient.y-m_eye.y-m_height*0.5)/ration);

	return CPoint(x,y);
}


//////////////////////////////////////////////////////////////////////////
// COpenGLDC class
//////////////////////////////////////////////////////////////////////////

COpenGLDC::COpenGLDC()
{
	m_pDC=NULL;
}

COpenGLDC::~COpenGLDC()
{
	if (NULL!=m_pDC)
	{
	    delete m_pDC;
	}
}

COpenGLDC::COpenGLDC(CWnd* pWnd)
{
	m_camera.Init();

	m_pWnd=pWnd;
	m_pDC=new CClientDC(m_pWnd);
	m_hDC=m_pDC->m_hDC;

	if(FALSE==SetupPixelFormat())
	{
		AfxMessageBox(_T("SetPixelFormat Failed"));
		return;
	}

	m_hRC=wglCreateContext(m_hDC);
	if(NULL==m_hRC)
	{
		AfxMessageBox(_T("RC Create Failed"));
		return;
	}

	wglMakeCurrent(m_hDC,m_hRC);

	GLSetupRC();
 
	wglMakeCurrent(NULL,NULL);
}


void COpenGLDC::GLResize(int cx,int cy)
{
	if(0>=cx ||0>=cy)
		return;

    wglMakeCurrent(m_hDC,m_hRC);

	m_camera.SetViewport(cx,cy);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	wglMakeCurrent(NULL,NULL);
}


void COpenGLDC::GLRelease()
{
	wglDeleteContext(m_hRC);
	::ReleaseDC(m_pWnd->m_hWnd,m_hDC);
}


BOOL COpenGLDC::SetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
			1,                     // version number 
			PFD_DRAW_TO_WINDOW |   // support window 
			PFD_SUPPORT_OPENGL |   // support OpenGL 
			PFD_DOUBLEBUFFER,      // double buffered 
			PFD_TYPE_RGBA,         // RGBA type 
			24,                    // 24-bit color depth 
			0, 0, 0, 0, 0, 0,      // color bits ignored 
			0,                     // no alpha buffer 
			0,                     // shift bit ignored 
			0,                     // no accumulation buffer 
			0, 0, 0, 0,            // accum bits ignored 
			32,                    // 32-bit z-buffer     
			0,                     // no stencil buffer 
			0,                     // no auxiliary buffer 
			PFD_MAIN_PLANE,        // main layer 
			0,                     // reserved 
			0, 0, 0 
	};
	
	int iPixelFormat;
	iPixelFormat=ChoosePixelFormat(m_hDC,&pfd);
	
	if(0==iPixelFormat)
	{
		AfxMessageBox(_T("0=iPixelFormat"));
		return FALSE;
	}
	
    if(FALSE==SetPixelFormat(m_hDC,iPixelFormat,&pfd))
	{
		AfxMessageBox(_T("FALSE=SetPixelFormat"));
		return FALSE;
	}
	
    return TRUE;
}

void COpenGLDC::Ready()
{
	wglMakeCurrent(m_hDC,m_hRC);
	if (GL_RENDER==m_camera.m_renderMode)  // 加这个条件可以解决选择时产生闪烁的问题
		ClearBkGround();
	OnShading();
	m_camera.Projection();
}

void COpenGLDC::Finish()
{
	if (GL_SELECT==m_camera.m_renderMode)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	glFlush();

	if (GL_SELECT==m_camera.m_renderMode)
	{
		m_hits = glRenderMode(GL_RENDER); //获取记录下的点击的个数
		SetRenderMode(GL_RENDER);
	}

	SwapBuffers(m_hDC);
	wglMakeCurrent(NULL,NULL);
}

void COpenGLDC::GLSetupRC()
{
	m_bShading=TRUE;

	glClearDepth(1.0f);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_CCW);

	glEnable(GL_LIGHTING);

	GLfloat ambientLight[]={0.75f,0.75f,0.75f,1.0f};
	GLfloat diffuseLight[]={1.0f,1.0f,1.0f,1.0f};   // 光源颜色练习最密切
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
	SetLightDirection(1,1,1);
	glEnable(GL_LIGHT0);

	SetBkColor(RGB(255,255,255));
	SetMaterialColor(RGB(122,122,122));
	SetColor(RGB(255,255,255));
	SetHighLightColor(RGB(255,0,0));

	glPointSize(3.0);
}

void COpenGLDC::SetRenderMode(int nRenderMode/* =GL_RENDER */)
{
	m_camera.m_renderMode=nRenderMode;
}

int COpenGLDC::GetRenderMode()
{
	return m_camera.m_renderMode;
}

void COpenGLDC::SetSelRect(CPoint pt1, CPoint pt2)
{
	m_camera.m_hitPt1=pt1;
	m_camera.m_hitPt2=pt2;
}

void COpenGLDC::GetSelected(CArray<int,int>& selAry)
{
	if (m_hits>0)
	{
		GLuint* ptr=m_camera.m_selBuf;
		for (int i=0;i<m_hits;i++)
		{
			ptr+=3;
			selAry.Add(*ptr);
			ptr++;
		}
	}
}

CDC* COpenGLDC::GetCDC()
{
	return m_pDC;
}

CPoint3D COpenGLDC::ScreenToGLClient(CPoint ptScreen)
{
	return m_camera.ScreenToCLClient(ptScreen);
}

CPoint COpenGLDC::GLClientToScreen(CPoint3D ptGLClient)
{
	return m_camera.GLClientToScreen(ptGLClient);
}

void COpenGLDC::ClearBkGround()                        // 设置背景色
{
	GLclampf r,g,b;
	r=(GLclampf)GetRValue(m_clrBk)/255;
	g=(GLclampf)GetGValue(m_clrBk)/255;
	b=(GLclampf)GetBValue(m_clrBk)/255;
	glClearColor(r,g,b,0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);  // 可以马上生效
}

void COpenGLDC::OnShading()
{
	if(m_bShading)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
	else
	{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
}

void COpenGLDC::Shading(BOOL bShading)
{
	m_bShading=bShading;
}

BOOL COpenGLDC::IsShading()
{
	return m_bShading;
}

void COpenGLDC::Lighting(BOOL bLighting)
{
	if(bLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}

BOOL COpenGLDC::IsLighting()
{
	GLboolean bLighting;
	glGetBooleanv(GL_LIGHTING,&bLighting);
	return bLighting;
}

void COpenGLDC::SetLightDirection(float dx,float dy,float dz)
{
	m_vecLight[0]=dx;
	m_vecLight[1]=dy;
	m_vecLight[2]=dz;

	GLfloat lightPos[]={dx,dy,dz,0.0f};  // 无穷远处
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
}

void COpenGLDC::GetLightDirection(float& dx,float& dy,float& dz)
{
	dx=m_vecLight[0];
    dy=m_vecLight[1];
    dz=m_vecLight[2];
}

COLORREF COpenGLDC::SetMaterialColor(COLORREF clr)
{
	COLORREF oldClr=m_clrMaterial;
	m_clrMaterial=clr;
	BYTE r,g,b;
	r=GetRValue(clr);
	g=GetGValue(clr);
	b=GetBValue(clr);
	GLfloat mat_amb_diff[]={(GLfloat)r/255,(GLfloat)g/255,(GLfloat)b/255};
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,mat_amb_diff);

	//
	GLfloat mat_specular[]={1.0,1.0,1.0,1.0};
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);  // 镜面反射强度的RGBA值
	GLfloat mat_shininess[]={50};
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);// 镜面反射指数
	//

	return oldClr;
}

void COpenGLDC::GetMaterialColor(COLORREF& clr)
{
	clr=m_clrMaterial;
}

void COpenGLDC::SetBkColor(COLORREF clr)      // 设置背景色
{
	m_clrBk=clr;
}

void COpenGLDC::GetBkColor(COLORREF& clr)      // 获取背景色
{
	clr=m_clrBk;
}

void COpenGLDC::SetColor(COLORREF clr)         // 设置模型框架颜色，没有材料时的。用函数glColor3ub()
{                                             // 函数用于绘制模型前调用。即在RenderScene()里调用。
	m_clr=clr;
	BYTE r,g,b;
    r=GetRValue(clr);
	g=GetGValue(clr);
	b=GetBValue(clr);
	glColor3ub(r,g,b);
}

void COpenGLDC::GetColor(COLORREF& clr)        // 获取模型框架颜色，没有材料时的
{
	clr=m_clr; 
}

void COpenGLDC::SetHighLightColor(COLORREF clr)
{
	m_clrHighLight=clr;
}

void COpenGLDC::GetHighLightColor(COLORREF& clr)
{
	clr=m_clrHighLight;
}

void COpenGLDC::HighLight(BOOL bHighLight)      // 高亮显示，其实是在材料颜色上进行操作的
{
	BYTE r,g,b;
	if(bHighLight)
	{
		r=GetRValue(m_clrHighLight);
		g=GetGValue(m_clrHighLight);
		b=GetBValue(m_clrHighLight);
	}
	else
	{
		r=GetRValue(m_clrMaterial);
		g=GetGValue(m_clrMaterial);
		b=GetBValue(m_clrMaterial);
	}
	GLfloat mat_amb_diff[]={(GLfloat)r/255,(GLfloat)g/255,(GLfloat)b/255};
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,mat_amb_diff);
}


//////////////////////////////////////////////////////////////////////////
// draw
//////////////////////////////////////////////////////////////////////////

void COpenGLDC::DrawCoord()
{
	BOOL bLighting=IsLighting();
	Lighting(FALSE);

	double len=min(m_camera.m_height,m_camera.m_width);
	len*=0.2;
	double lenArrow=len/12;

	CPoint3D oPt, xPt, yPt, zPt;
	xPt.x=yPt.y=zPt.z=len;
	CVector3D x(1,0,0);
	CVector3D y(0,1,0);
	CVector3D z(0,0,1);

	COLORREF oldClr;
	GetColor(oldClr);

	SetColor(RGB(255,0,0));
	DrawLine(oPt,xPt);
	DrawArrow(xPt,y,z,lenArrow);

	SetColor(RGB(0,255,0));
	DrawLine(oPt,yPt);
	DrawArrow(yPt,z,x,lenArrow);

	SetColor(RGB(0,0,255));
	DrawLine(oPt,zPt);
	DrawArrow(zPt,x,y,lenArrow);

	Lighting(bLighting);

	SetColor(oldClr);
}

void COpenGLDC::DrawArrow(CPoint3D ptCenter,CVector3D vec_i,CVector3D vec_j,double len)
{
	double l=len/2;
	CPoint3D pt1=ptCenter+(vec_i+vec_j)*l;
	CPoint3D pt2=ptCenter+(vec_j-vec_i)*l;
	CPoint3D pt3=ptCenter+(vec_i+vec_j)*(-l);
	CPoint3D pt4=ptCenter+(vec_i-vec_j)*l;
	CPoint3D pt5=ptCenter+(vec_i*vec_j)*l*3;

	DrawLine(pt1,pt2);
	DrawLine(pt2,pt3);
	DrawLine(pt3,pt4);
	DrawLine(pt4,pt1);
	DrawLine(pt1,pt5);
	DrawLine(pt2,pt5);
	DrawLine(pt3,pt5);
	DrawLine(pt4,pt5);
	DrawLine(ptCenter,pt5);
}


void COpenGLDC::DrawSelPoint(const CPoint3D& pt,int ShapeType/* =ST_RECT */,COLORREF clr/* =RGB(0,0,255) */)
{
	BOOL bLighting=IsLighting();
	Lighting(FALSE);

	double len=min(m_camera.m_height,m_camera.m_width);
	len*=0.01;

	COLORREF oldClr;
	GetColor(oldClr);
	SetColor(clr);

	switch(ShapeType)
	{
	case ST_RECT:
		{
			CPoint3D pt1(pt.x-len,pt.y-len,pt.z);
			CPoint3D pt2(pt.x+len,pt.y-len,pt.z);
			CPoint3D pt3(pt.x+len,pt.y+len,pt.z);
			CPoint3D pt4(pt.x-len,pt.y+len,pt.z);

			DrawLine(pt1,pt2);
			DrawLine(pt2,pt3);
			DrawLine(pt3,pt4);
			DrawLine(pt4,pt1);
			break;
		}
	case ST_CIRCLE:
		{
			double step=2*PI/20;
			double r=1.2*len;
			CPoint3D ptFrom(pt.x+r,pt.y,0);
			CPoint3D ptTo;
			for (double theta=step;theta<2*PI;theta=theta+step)
			{
				ptTo=CPoint3D(pt.x+r*cos(theta),pt.y+r*sin(theta),0);
				DrawLine(ptFrom,ptTo);
				ptFrom=ptTo;
			}
			ptTo=CPoint3D(pt.x+r,pt.y,0);
			DrawLine(ptFrom,ptTo);

			break;
		}
	case ST_TRIANGLE:
		{
			double r=1.5*len;
			CPoint3D pt1(pt.x-0.866*r,pt.y-0.5*r,0);
			CPoint3D pt2(pt.x+0.866*r,pt.y-0.5*r,0);
			CPoint3D pt3(pt.x,pt.y+r,0);
			DrawLine(pt1,pt2);
			DrawLine(pt2,pt3);
			DrawLine(pt3,pt1);

			break;
		}

	}
	Lighting(bLighting);
	SetColor(oldClr);
}

void COpenGLDC::DrawPoint(const CPoint3D& pt, double pointSize)
{
	glPointSize(pointSize);
	glBegin(GL_POINTS);
		glVertex3f(pt.x,pt.y,pt.z);
	glEnd();		
}

//void COpenGLDC::DrawLine(const CPoint3D& sp,const CPoint3D& ep,int lineStyle/* =LS_SOLID */,int lineWidth/* =1 */)
//{
//	glLineWidth(lineWidth);
//
//	glEnable(GL_LINE_STIPPLE);
//
//	switch(lineStyle)
//	{
//	case LS_SOLID:
//		glLineStipple(1,0xFFFF);  
//		break;
//
//	case LS_DASH:
//		glLineStipple(2,0x0F0F);
//		break;
//
//	case LS_CENTER:
//		glLineStipple(1,0xFF18);
//		break;
//	}
//
//	glBegin(GL_LINES);
//	    glVertex3f(sp.x,sp.y,sp.z);
//		glVertex3f(ep.x,ep.y,ep.z);
//	glEnd();
//
//	glDisable(GL_LINE_STIPPLE);
//}
void COpenGLDC::DrawLine(const CPoint3D& sp,const CPoint3D& ep,int lineStyle/* =LS_SOLID */,int lineWidth/* =1 */,COLORREF clr/*=RGB(255,255,0)*/)
{
	COLORREF oldclr;
	GetMaterialColor(oldclr);
	SetMaterialColor(clr);

	glLineWidth(lineWidth);

	glEnable(GL_LINE_STIPPLE);

	switch(lineStyle)
	{
	case LS_SOLID:
		glLineStipple(1,0xFFFF);  
		break;

	case LS_DASH:
		glLineStipple(2,0x0F0F);
		break;

	case LS_CENTER:
		glLineStipple(1,0xFF18);
		break;
	}

	glBegin(GL_LINES);
	    glVertex3f(sp.x,sp.y,sp.z);
		glVertex3f(ep.x,ep.y,ep.z);
	glEnd();

	glDisable(GL_LINE_STIPPLE);
	SetMaterialColor(oldclr);
}


void COpenGLDC::DrawLine(double x1, double y1, double z1, double x2, double y2, double z2)
{
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2,0x0F0F);

	glBegin(GL_LINES);
	glVertex3f(x1,y1,z1);
	glVertex3f(x2,y2,z2);
	glEnd();

	glDisable(GL_LINE_STIPPLE);
}

void COpenGLDC::DrawPlaneRect(const CPoint3D &ptLeftTop, const CPoint3D &ptRightBottom, int lineStyle,int lineWidth)
{
	CPoint3D pt1=CPoint3D(ptLeftTop.x,ptRightBottom.y,0);
	CPoint3D pt2=CPoint3D(ptRightBottom.x,ptLeftTop.y,0);
	DrawLine(ptLeftTop,pt1,lineStyle,lineWidth);
	DrawLine(pt1,ptRightBottom,lineStyle,lineWidth);
	DrawLine(ptRightBottom,pt2,lineStyle,lineWidth);
	DrawLine(pt2,ptLeftTop,lineStyle,lineWidth);
}

void COpenGLDC::DrawPolyline(CPoint3D pt[],int sz, BOOL bClosed/* =TRUE */)
{
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<sz-1;i++)
		glVertex3f(pt[i].x,pt[i].y,pt[i].z);
	if (bClosed && sz>0)
		glVertex3f(pt[0].x,pt[0].y,pt[0].z);
	glEnd();
}

void COpenGLDC::DrawPolygon(CPoint3D pt[], int sz)
{
	glBegin(GL_POLYGON);
	for (int i=0;i<sz;i++)
		glVertex3f(pt[i].x,pt[i].y,pt[i].z);
	glEnd();
}

void COpenGLDC::DrawPolygon(Point3DAry &ptList,BOOL bDrawFrame/* =TRUE */)
{
	if (bDrawFrame)
		glBegin(GL_LINE_STRIP);
	else
		glBegin(GL_POLYGON);

	int sz=ptList.GetSize();
	for (int i=0;i<sz;i++)
		glVertex3f(ptList[i]->x,ptList[i]->y,ptList[i]->z);

	glEnd();
}

void COpenGLDC::DrawCircle(const CPoint3D& ptCenter,const double& r, COLORREF clr/* =RGB */,int precision)
{
	double step=2*PI/precision;

	COLORREF oldclr;
	GetMaterialColor(oldclr);
	SetMaterialColor(clr);
	CPoint3D ptFrom(ptCenter.x+r,ptCenter.y,0);
	CPoint3D ptTo;
	for (double theta=step;theta<2*PI;theta=theta+step)
	{
		ptTo=CPoint3D(ptCenter.x+r*cos(theta),ptCenter.y+r*sin(theta),0);
		DrawLine(ptFrom,ptTo);
		ptFrom=ptTo;
	}
	ptTo=CPoint3D(ptCenter.x+r,ptCenter.y,0);
	DrawLine(ptFrom,ptTo);
	SetMaterialColor(oldclr);
}

void COpenGLDC::DrawTriChip(double n0, double n1, double n2,
				            double v00,double v01,double v02,
				            double v10,double v11,double v12,
				            double v20,double v21,double v22)
{
	glBegin(GL_TRIANGLES);
		glNormal3d(n0,n1,n2);
	    glVertex3d(v00,v01,v02);
		glVertex3d(v10,v11,v12);
		glVertex3d(v20,v21,v22);
	glEnd();
}

void COpenGLDC::DrawTriChip(CVector3D vec,CPoint3D pt1,CPoint3D pt2,CPoint3D pt3)
{
	glBegin(GL_TRIANGLES);
	    glNormal3d(vec.dx,vec.dy,vec.dz);
	    glVertex3d(pt1.x,pt1.y,pt1.z);
		glVertex3d(pt2.x,pt2.y,pt2.z);
		glVertex3d(pt3.x,pt3.y,pt3.z);
	glEnd();
}

void COpenGLDC::DrawTriFrame(CPoint3D A,CPoint3D B,CPoint3D C)
{
	DrawLine(A,B);
	DrawLine(B,C);
	DrawLine(C,A);
}

void COpenGLDC::DrawShere(const CPoint3D& center,double r,const CVector3D& vec)
{
	glPushMatrix();
	glTranslatef(center.x,center.y,center.z);

	CVector3D vecNY(0,-1,0);
	CVector3D axis=vecNY*vec;
	double angle=GetAngle(vecNY,vec);
	angle=angle*180/PI;
	glRotatef(angle,axis.dx,axis.dy,axis.dz);
	auxWireSphere(r);
	glPopMatrix();
}

void COpenGLDC::DrawCylinder(const CPoint3D& center,double r,const CVector3D& h)
{
	glPushMatrix();
	glTranslatef(center.x,center.y,center.z);
	CVector3D vecNY(0,-1,0);
	CVector3D axis=vecNY*h;
	double angle=GetAngle(vecNY,h);
	angle=angle*180/PI;
	glRotatef(angle,axis.dx,axis.dy,axis.dz);

	auxSolidCylinder(r,h.GetLength());

	glPopMatrix();
}

void COpenGLDC::DrawTorus(const CPoint3D& center,const CVector3D& vecAxis,double r_in,double r_out)
{
	glPushMatrix();
	glTranslatef(center.x,center.y,center.z);
	CVector3D vecNY(0,-1,0);
	CVector3D axis=vecNY*vecAxis;
	double angle=GetAngle(vecNY,vecAxis);
	angle=angle*180/PI;
	glRotatef(angle,axis.dx,axis.dy,axis.dz);
	
	auxSolidTorus(r_in,r_out);
	
	glPopMatrix();
}

void COpenGLDC::DrawCone(const CPoint3D& center,double r,const CVector3D& h)
{
	glPushMatrix();
	glTranslatef(center.x,center.y,center.z);
	CVector3D vecNY(0,-1,0);
	CVector3D axis=vecNY*h;                                                                                                     
	double angle=GetAngle(vecNY,h);
	angle=angle*180/PI;
	glRotatef(angle,axis.dx,axis.dy,axis.dz);

	GLfloat ang,x,y;
	glBegin(GL_TRIANGLE_FAN);
	    glVertex3f(0,0,h.GetLength());
		for(ang=0.0f;ang<2.0f*PI;ang+=PI/8.0f)
		{
			x=r*sin(ang);
			y=r*cos(ang);
			glVertex2f(x,y);
		}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0f,0.0f);
	for(ang=0.0f;ang<2.0f*PI;ang+=PI/8.0f)
	{
		x=r*sin(ang);
		y=r*cos(ang);
		glVertex2f(x,y);
	}
	glEnd();
	glPopMatrix();
}

void COpenGLDC::DrawText(CString str,CPoint3D pt,COLORREF clr)
{
	wglMakeCurrent(m_hDC,m_hRC);

	CPoint pt1=GLClientToScreen(pt);


	TEXTMETRIC tm;
	m_pDC->GetTextMetrics(&tm);
	int height=tm.tmHeight;
	int width=tm.tmMaxCharWidth;

	int szStr;
	szStr=str.GetLength();
	width*=szStr;
	height/=2;
	width/=2;
	CRect rect(pt1.x-width,pt1.y-height,pt1.x+width,pt1.y+height);

	m_pDC->SetBkMode(TRANSPARENT);

	m_pDC->DrawText(str,&rect,DT_CENTER);

	wglMakeCurrent(NULL,NULL);
}

void COpenGLDC::DrawPolygonFrame(const CTypedPtrArray<CObArray,CPoint3D*>& ptList)
{
	int sz=ptList.GetSize();
	for(int i=0;i<sz-1;i++)
	{
		DrawLine(*ptList[i],*ptList[i+1]);
	}
	DrawLine(*ptList[sz-1],*ptList[0]);
}

int COpenGLDC::Is2PtOn2SideOfLine(const CPoint3D& ptS_line,const CPoint3D& ptE_line,const CPoint3D& pt1,const CPoint3D& pt2)
{
	CVector3D vec1,vec2,vec3,vec4;
	vec1=pt1-ptS_line;
	vec2=ptE_line-pt1;
	vec3=pt2-ptS_line;
	vec4=ptE_line-pt2;

	double flag=((vec1*vec2).dz)*((vec3*vec4).dz);

	if ( flag<0)  // =0说明有一个点在直线上，也就是说直线的延长线刚好过角点
	{
		return 1;     // 相交且交点在直线内
	}
	else if (0==flag)
	{
		return 2;     // 相交交点在直线的起点
	}
	else
	{
		return 0;     // 不相交
	}
}


int COpenGLDC::GetCrossPoint(CPoint3D& pt_out,CPoint3D pt1_line1,
							 CPoint3D pt2_line1,CPoint3D pt1_line2,CPoint3D pt2_line2) // 对于两条相交直线求交点
{
	CVector3D vec_n1=GetNormalVector(pt1_line1,pt2_line1);
	CVector3D vec_n2=GetNormalVector(pt1_line2,pt2_line2);
	double a1=vec_n1.dx;
	double b1=vec_n1.dy;
	double c1=-(a1*pt1_line1.x+b1*pt1_line1.y);
	double a2=vec_n2.dx;
	double b2=vec_n2.dy;
	double c2=-(a2*pt1_line2.x+b2*pt1_line2.y);

	double m=a2*b1-a1*b2;
	double n=a1*c2-a2*c1;

	if (0==m && 0!=n)
	{
		return 0;   // 平行
	}
	if (0==m && 0==n)
	{
		return 1;   // 重合
	}

	pt_out.y=n/m;

	if(0!=a1)
	{
		pt_out.x=-(b1*pt_out.y+c1)/a1;
	}
	else
	{
		pt_out.x=-(b2*pt_out.y+c2)/a2;
	}
	return 2;
}


CVector3D COpenGLDC::GetNormalVector(CPoint3D pt_start,CPoint3D pt_end) 
{
	CVector3D vec=pt_end-pt_start;
	CVector3D vec_r;
	if (0==vec.dx)
	{
		vec_r=CVector3D(1,0,0);
	}
	else
	{
		vec_r=CVector3D(-(vec.dy/vec.dx),1,0);
		vec_r.Normalize();
	}
	if ((vec_r*vec).dz<0)
	{
		vec_r=vec_r*(-1);
	}
	return vec_r;
}