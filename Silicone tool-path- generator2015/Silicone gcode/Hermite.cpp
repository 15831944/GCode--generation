#include "stdafx.h"
#include "Hermite.h"


CHermite::CHermite(void)
{

}
CHermite::~CHermite(void)
{

}
CHermite::CHermite(CPoint3D beforePt,CPoint3D pts, CPoint3D pte, CPoint3D afterPt)
{
	m_ptStart = pts;
	m_ptEnd = pte;
	m_BeforePt = beforePt;
	m_AfterPt = afterPt;
}

void CHermite::Interpolate()
{
	vec1 =  m_ptStart - m_BeforePt;
	vec2 = m_AfterPt- m_ptEnd;
	vec1.Normalize();
	vec2.Normalize();
	double width = fabs(m_ptEnd.y - m_ptStart.y);
	vec1 *=width/2;
	vec2 *=width/2;
	float coe[4];//中间变量
	//Hermite Matrix
	float hemite[4][4]={ 
	{1, 0, -3, 2},
	{0, 0,  3,-2},
	{0, 1, -2, 1},
	{0, 0, -1, 1}};

	float t = 0.0000;    
	float delta = 1.0 / (COUNT - 1);  //设置等分的数量，变化值
	for(int i = 0; i < COUNT ; i++)  //利用矩阵求出各点P= C*T
	{
		
		for(int j = 0; j < 4; j++)
		{
			coe[j] = 1 * hemite[j][0] + t * hemite[j][1] + t * t * hemite[j][2] + t * t * t * hemite[j][3];
		}
		m_vD[i].x = coe[0] * m_ptStart.x +coe[1] * m_ptEnd.x + 3 * coe[2] * vec1.dx  + 3 * coe[3] * vec2.dx;
		m_vD[i].y = coe[0] * m_ptStart.y +coe[1] * m_ptEnd.y + 3 * coe[2] * vec1.dy  + 3 * coe[3] * vec2.dy;
		m_vD[i].z = 0;

		t += delta;
	}
	
}

double CHermite::Getlength()
{
	double len = 0;
	for (int i = 0; i < COUNT -1 ;i++)
	{
		CVector3D *vec = new CVector3D(m_vD[i + 1] - m_vD[i]);
		len += vec->GetLength();
	}
	return len;

}
void CHermite::Draw(COpenGLDC *pDC, COLORREF clr)
{
	for (int i = 0; i < COUNT -1; i++)
	{
		pDC->DrawLine(m_vD[i],m_vD[i+1],LS_SOLID,2, clr);
	}
}
