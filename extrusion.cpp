/*
** �����o���`��̍쐬
*/
#include <cmath>
#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  undef GL_GLEXT_PROTOTYPES
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include "extrusion.h"

#define TESSELLATION 0 // gluTess*() ���g���Ȃ� 1 �ɂ���

/*
** �s��̐�
**   �R�~�R�s��� m1 �� m2 ���|����
*/
static void multiply(const double m1[], const double m2[], double m3[])
{
  for (int j = 0; j < 9; j += 3) {
    double t[3];
    
    for (int i = 0; i < 3; ++i) {
      t[i] = m1[j + 0] * m2[0 + i]
           + m1[j + 1] * m2[3 + i]
           + m1[j + 2] * m2[6 + i];
    }
    m3[j + 0] = t[0];
    m3[j + 1] = t[1];
    m3[j + 2] = t[2];
  }
}

/*
** �x�N�g���̕����]��`
**   �x�N�g�� u ���x�N�g�� v �����Ɍ�����ϊ��s��� r �Ɋi�[����
*/
static void turn(const double u[], const double v[], double r[])
{
  /* ���ʂ̎��i��]���jn = u�~v */
  const double n[] = {
    u[1] * v[2] - u[2] * v[1],
    u[2] * v[0] - u[0] * v[2],
    u[0] * v[1] - u[1] * v[0],
  };
  
  /* n �̒��� */
  double d = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
  
  if (d == 0.0) {
    /* ��]���Ȃ� */
    r[1] = r[2] = r[3] = r[5] = r[6] = r[7] = 0.0;
    r[0] = r[4] = r[8] = 1.0;
  }
  else {
    /*
    ** r : (u, n, u�~n) �� (x, y, z) �̕ϊ��s��
    ** s : (x, y, z) �� (v, n, v�~n) �̕ϊ��s��
    */
    double s[9], a = sqrt(d);
    
    /* t �� y���̐����Ƃ��� */
    r[1] = s[3] = n[0] / a;
    r[4] = s[4] = n[1] / a;
    r[7] = s[5] = n[2] / a;
    
    /* u����x�� u */
    a = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    r[0] = u[0] / a;
    r[3] = u[1] / a;
    r[6] = u[2] / a;
    
    /* v����x�� v */
    a = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    s[0] = v[0] / a;
    s[1] = v[1] / a;
    s[2] = v[2] / a;
    
    /* u����z�� l = u�~n */
    r[2] = u[1] * n[2] - u[2] * n[1];
    r[5] = u[2] * n[0] - u[0] * n[2];
    r[8] = u[0] * n[1] - u[1] * n[0];
    a = sqrt(r[2] * r[2] + r[5] * r[5] + r[8] * r[8]);
    r[2] /= a;
    r[5] /= a;
    r[8] /= a;
    
    /* v����z�� m = v�~n */
    s[6] = v[1] * n[2] - v[2] * n[1];
    s[7] = v[2] * n[0] - v[0] * n[2];
    s[8] = v[0] * n[1] - v[1] * n[0];
    a = sqrt(s[6] * s[6] + s[7] * s[7] + s[8] * s[8]);
    s[6] /= a;
    s[7] /= a;
    s[8] /= a;
    
    /* r �� r s */
    multiply(r, s, r);
  }
}

/*
** �f�ʂ̂���f�ό`
**   �@���x�N�g���� v * inverse(m) �̕��ʂɉ�����
**   z �������ɂ���f�ό`����ϊ��s��� r �Ɋi�[����
*/
static void shear(const double v[], const double m[], double r[])
{
  for (int i = 0; i < 9; ++i) r[i] = m[i];
  
  const double z = v[0] * m[6] + v[1] * m[7] + v[2] * m[8];
  
  if (z != 0.0) {
    const double x = (v[0] * m[0] + v[1] * m[1] + v[2] * m[2]) / z;
    const double y = (v[0] * m[3] + v[1] * m[4] + v[2] * m[5]) / z;
    
    r[0] -= r[6] * x;
    r[1] -= r[7] * x;
    r[2] -= r[8] * x;
    
    r[3] -= r[6] * y;
    r[4] -= r[7] * y;
    r[5] -= r[8] * y;
  }
}

/*
** ���W�ϊ�
**   n �̒��_ v �� m �ŉ�]������ p �ɕ��s�ړ��������̂� t �ɓ���
*/
static void transform(const double v[][2], int n,
                      const double m[], const double p[], double t[][3])
{
  for (int i = 0; i < n; ++i) {
    t[i][0] = v[i][0] * m[0] + v[i][1] * m[3] + p[0];
    t[i][1] = v[i][0] * m[1] + v[i][1] * m[4] + p[1];
    t[i][2] = v[i][0] * m[2] + v[i][1] * m[5] + p[2];
  }
}

/*
** ���ʂ�`��
**   p1: �O�̒f�ʂ̒��_�ʒu
**   p2: ���̒f�ʂ̒��_�ʒu
**   n:  �f�ʂ̖@���x�N�g��
**   nc: �f�ʂ̒��_��
**   m:  �f�ʂ̉�]�̕ϊ��s��
*/
static void side(const double p1[][3], const double p2[][3],
                 const double n[][2], int nc, const double m[])
{
  glBegin(GL_TRIANGLE_STRIP);
  for (int i = 0; i < nc; ++i) {
    glNormal3d(m[0] * n[i][0] + m[3] * n[i][1],
               m[1] * n[i][0] + m[4] * n[i][1],
               m[2] * n[i][0] + m[5] * n[i][1]); 
    glVertex3dv(p1[i]);
    glVertex3dv(p2[i]);
  }
  glNormal3d(m[0] * n[0][0] + m[3] * n[0][1],
             m[1] * n[0][0] + m[4] * n[0][1],
             m[2] * n[0][0] + m[5] * n[0][1]); 
  glVertex3dv(p1[0]);
  glVertex3dv(p2[0]);
  glEnd();
}

/*
** �W��`��
**   p:  �f�ʂ̒��_�ʒu
**   n1: �`����J�n���钸�_�̈ʒu
**   n2: �`����I�����钸�_�̈ʒu
**   n:  �@���x�N�g��
*/
static void cap(const double p[][3], int n1, int n2, const double n[])
{
  if (n1 != n2) {
#if TESSELLATION
    GLUtesselator *tess = gluNewTess();
    
    gluTessCallback(tess, GLU_TESS_BEGIN, (GLvoid (CALLBACK *)())glBegin);
    gluTessCallback(tess, GLU_TESS_VERTEX, (GLvoid (CALLBACK *)())glVertex3dv);
    gluTessCallback(tess, GLU_TESS_END, (GLvoid (CALLBACK *)())glEnd);
    
    gluBeginPolygon(tess);
    gluTessBeginContour(tess);
#else
    glBegin(GL_TRIANGLE_FAN);
#endif
    
    if (n1 < n2) {
      glNormal3dv(n);
      for (int i = n1; i <= n2; ++i) {
#if TESSELLATION
        gluTessVertex(tess, (GLdouble *)p[i], (GLdouble *)p[i]);
#else
        glVertex3dv(p[i]);
#endif
      }
    }
    else {
      glNormal3d(-n[0], -n[1], -n[2]);
      for (int i = n1; i >= n2; --i) {
#if TESSELLATION
        gluTessVertex(tess, (GLdouble *)p[i], (GLdouble *)p[i]);
#else
        glVertex3dv(p[i]);
#endif
      }
    }

#if TESSELLATION
    gluTessEndContour(tess);
    gluEndPolygon(tess);
    
    gluDeleteTess(tess);
#else
    glEnd();
#endif
  }
}

/*
** �����o��
**   cs: �f�ʌ`�� (cross section)
**   nc: �f�ʂ̒��_��
**   sp: �����o���o�H (spine)
**   ns: �o�H�̐ߓ_�̐��i�N�_�ƏI�_���܂ށj
*/
void extrusion(const double cs[][2], int nc, const double sp[][3], int ns)
{
  if (--ns > 0) {
    
    /* ����܂蒸�_���̑����f�ʂ͐؂�l�߂� */
    if (nc > EXTRUSION_CS_LIMIT) nc = EXTRUSION_CS_LIMIT;
    
    /* �f�ʂ̖@���x�N�g�� n �����߂� (z = 0) */
    double n[EXTRUSION_CS_LIMIT][2], x, y, a;
    
    for (int i = 1; i < nc; ++i) {
      x = cs[i][0] - cs[i - 1][0];
      y = cs[i][1] - cs[i - 1][1];
      a = x * x + y * y;
      
      if (a != 0.0) {
        a = sqrt(a);
        n[i][0] = y / a;
        n[i][1] = -x / a;
      }
    }

    x = cs[0][0] - cs[nc - 1][0];
    y = cs[0][1] - cs[nc - 1][1];
    a = x * x + y * y;

    if (a != 0.0) {
      a = sqrt(a);
      n[0][0] = y / a;
      n[0][1] = -x / a;
    }
    
    /* �ړ_�ɐi���E�ޏo��������̕����x�N�g�� v */
    double v[2][3];

    /* �f�ʂ̎��x�N�g���i�f�ʂ�XY���ʏ�Œ�`�j */
    v[0][0] = 0.0;
    v[0][1] = 0.0;
    v[0][2] = 1.0;
    
    /* �N�_�̐i�����̕����x�N�g���͑ޏo���̕����x�N�g���ƈ�v������ */
    v[1][0] = sp[1][0] - sp[0][0];
    v[1][1] = sp[1][1] - sp[0][1];
    v[1][2] = sp[1][2] - sp[0][2];
    a = sqrt(v[1][0] * v[1][0] + v[1][1] * v[1][1] + v[1][2] * v[1][2]);
    v[1][0] /= a;
    v[1][1] /= a;
    v[1][2] /= a;
    
    /* �f�ʂ��N�_�ɂ�����i�����̕����x�N�g���̌����ɉ�]����s�� m */
    double m[9];
    turn(v[0], v[1], m);
    
    /* �ړ_�ɂ�����f�ʂ̒��_�ʒu p �͒f�ʂ̍��W�l�� m �ŕϊ����ċ��߂� */
    double p[2][EXTRUSION_CS_LIMIT][3];
    transform(cs, nc, m, sp[0], p[1]);
    
    /* �N�_�̒f�ʂ�`�� */
    cap(p[1], nc - 1, 0, v[1]);
    
    /* �N�����E�ޏo���̐؂�ւ� */
    int k = 0;
    
    for (int i = 1; i < ns; ++i) {
      
      /* �ޏo���̕����x�N�g�� */
      v[k][0] = sp[i + 1][0] - sp[i][0];
      v[k][1] = sp[i + 1][1] - sp[i][1];
      v[k][2] = sp[i + 1][2] - sp[i][2];
      a = sqrt(v[k][0] * v[k][0] + v[k][1] * v[k][1] + v[k][2] * v[k][2]);
      v[k][0] /= a;
      v[k][1] /= a;
      v[k][2] /= a;
      
      /* ���ԃx�N�g�����f�ʂ̖@���x�N�g�� */
      double h[] = {
        v[k][0] + v[1 - k][0],
        v[k][1] + v[1 - k][1],
        v[k][2] + v[1 - k][2],
      };
      
      /* ���ԃx�N�g�� h �ɒ�������f�ʂ����߂�ϊ� r */
      double r[9];
      shear(h, m, r);

      /* ���ԃx�N�g�� h �ɂ�����f�ʌ`������߂� */
      transform(cs, nc, r, sp[i], p[k]);
      
      /* ���ʂ�`�� */
      side(p[k], p[1 - k], n, nc, m);
      
      /* �i�����̕����x�N�g����ޏo���̕����x�N�g���ɉ�]����s�� r */
      turn(v[1 - k], v[k], r);
      
      /* �f�ʂ�ޏo���̕����x�N�g���̌����ɉ�]����s�� m */
      multiply(m, r, m);
      
      /* �i�����Ƒޏo�������ւ��� */
      k = 1 - k;
    }
    
    /* �I�_�̒f�ʂ͂ЂƂO�̒f�ʂƓ��������ňʒu�݂̂��قȂ� */
    transform(cs, nc, m, sp[ns], p[k]);
    
    /* ���ʂ�`�� */
    side(p[k], p[1 - k], n, nc, m);
    
    /* �I�_�̒f�ʂ�`�� */
    cap(p[k], 0, nc - 1, v[1 - k]);
  }
}
