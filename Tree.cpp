/*
** L-System �ɂ��؂̐���
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
#include "Matrix.h"
#include "Tree.h"

/*
** �~���� M_PI �� Visual Studio �� cmath �ł͒�`����Ă��Ȃ�
*/
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

/*
** ��̎}�̍����̈ʒu�i���_�j
*/
const double Tree::base[] = { 0.0, 0.0, 0.0, 1.0 };

/*
** �^�[�g���̏���
*/
void Tree::turtle(const char *p)
{
  for (; *p != '\0'; ++p) {
 
    switch (*p){

    case 'F': // �O�i
      m.translate(top);
      m.projection(base, spine[nspine++]);
      break;
      
    case '+': // ���E��]
      m.rotate( rotate, top);
      break;
    case '-': // ������]
      m.rotate(-rotate, top);
      break;
      
    case '>': // �E����
      m.rotate( bend, 0.0, 0.0, 1.0);
      break;
    case '<': // ������
      m.rotate(-bend, 0.0, 0.0, 1.0);
      break;
      
    case '[': // ���݈ʒu�ۑ�
      m.push();
      break;
    case ']': // �ۑ��ʒu���A
      m.pop();
      m.projection(base, spine[branch[nbranch++] = nspine++]);
      break;
      
    default:
      break;
    }
  }
}

/*
** �ߓ_���̃J�E���g
*/
void Tree::count(const char *p)
{
  for (; *p != '\0'; ++p) {
    switch (*p){
    case ']': // �ۑ��ʒu���A
      ++nbranch;
    case 'F': // �O�i
      ++nspine;
    default:
      break;
    }
  }
}

/*
** �������@�̏���
*/
void Tree::production(const char *istr, const char * const *rstr, int iter)
{
  if (--iter >= 0) {
    for (const char *p = istr; *p; ++p) {   // ���������񂩂�P�������o��
      const char * const *q;

      for (q = rstr; *q; ++q) {             // �K�����ЂƂ��o��
        if (*p == **q) {                    // ����������̕����ƋK���̂P�����ڂ��r
          production(*q + 2, rstr, iter);   // ��v�����珉������������̋K���ŏ���������
          break;
        }
      }
      
      if (*q == 0) {                        // ��v����K����������Ȃ�������
        const char r[2] = { *p, 0 };        // ����������̕����̏��������s����

        if (flag)
          count(r);
        else
          turtle(r);
      }
    }
  }
  else {
    if (flag)
      count(istr);
    else
      turtle(istr);
  }
}

/*
** �R���X�g���N�^�i�؂̐����j
*/
Tree::Tree(
           const char *initial,       // ����������
           const char * const *rule,  // ���������K��
           int level,                 // �ċA���x��
           const double *direction,   // �؂��L�т����
           double rstep,              // �����S�̉�]�̊p�x�X�e�b�v
           double bstep,              // �Ȃ������̊p�x�X�e�b�v
           double r,                  // �؂̍����̔��a
           int n                      // �؂̑��ʐ�
           )
{
  // �|�C���^�̏�����
  spine = 0;
  branch = 0;
  cs = 0;

  // �؂��L�т����
  if (direction != 0) {
    top[0] = direction[0];
    top[1] = direction[1];
    top[2] = direction[2];
  }
  else {
    top[0] = 0.0;
    top[1] = 1.0;
    top[2] = 0.0;
  }
  top[3] = 1.0;

  // �����S�̉�]�̊p�x�X�e�b�v
  rotate = rstep * M_PI / 180.0;

  // �Ȃ������̊p�x�X�e�b�v
  bend = bstep * M_PI / 180.0;

  // �����̔��a
  radius = r;

  // �f�ʌ`��𐶐�����
  ncs = (n <= EXTRUSION_CS_LIMIT) ? n : EXTRUSION_CS_LIMIT;
  cs = new double[ncs][2];
  for (int i = 0; i < ncs; ++i) {
    double t = 2.0 * M_PI * (double)i / (double)ncs;
    
    cs[i][0] = radius * cos(t);
    cs[i][1] = radius * sin(t);
  }
  
  // �K�v�ȃ��������m�ۂ���
  flag = nspine = nbranch = 1;
  production(initial, rule, level);
  spine = new double[nspine][3];
  branch = new int[nbranch];

  // �ŏ��̐ߓ_�ɖ؂̍����̈ʒu��ݒ肷��
  flag = nspine = nbranch = 0;
  spine[nspine][0] = base[0] / base[3];
  spine[nspine][1] = base[1] / base[3];
  spine[nspine][2] = base[2] / base[3];
  nspine++;

  // �؂𐶐�����
  production(initial, rule, level);

  // �Ō�̕���ɍŌ�̐ߓ_�ԍ���o�^����
  branch[nbranch++] = nspine;
}

/*
** �f�X�g���N�^�i����������j
*/
Tree::~Tree()
{
  delete[] spine;
  spine = 0;
  
  delete[] branch;
  branch = 0;
  
  delete[] cs;
  cs = 0;
}

/*
** �؂̕`��
*/
void Tree::draw()
{
  for (int i = 0, j = 0; i < nbranch; ++i) {
#if 1
    extrusion(cs, ncs, spine + j, branch[i] - j);
    j = branch[i];
#else
    glBegin(GL_LINE_STRIP);
    while (j < branch[i]) glVertex4dv(spine[j++]);
    glEnd();
#endif
  }
}
