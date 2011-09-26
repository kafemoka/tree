/*
** �ϊ��s��̑���
*/
#include <cmath>
#include <stack>
#include "Matrix.h"

/*
** �X�^�b�N
*/
static std::stack<Matrix> stack;

/*
** �P�ʍs��
*/
const double Matrix::identity[] = {
  1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0,
};

/*
** �z�� src ��z�� dst �ɃR�s�[����
*/
void Matrix::copy(const double *src, double *dst)
{
  for (int i = 0; i < 16; ++i) dst[i] = src[i];
}

/*
** �s��ɔz�� array ��������
*/
Matrix &Matrix::multiply(const double *array)
{
  double t[16];
  
  for (int i = 0; i < 16; i += 4) transform(array + i, t + i);
  
  copy(t, m);
  
  return *this;
}

/*
** (x, y, z) �ɕ��s�ړ�����
*/
Matrix &Matrix::translate(double x, double y, double z)
{
  double t[16];
  
  copy(identity, t);
  
  t[12] = x;
  t[13] = y;
  t[14] = z;

  return multiply(t);
}

/*
** �ʒu�x�N�g�� v �ɕ��s�ړ�����
*/
Matrix &Matrix::translate(const double *v)
{
  if (v[3] != 0.0)
    return translate(v[0] / v[3], v[1] / v[3], v[2] / v[3]);
  else
    return translate(v[0], v[1], v[2]);
}

/*
** �}�`�����_���S�� (x, y, z) �{����
*/
Matrix &Matrix::scale(double x, double y, double z)
{
  double t[16];
  
  copy(identity, t);

  t[ 0] = x;
  t[ 5] = y;
  t[10] = z;
  
  return multiply(t);
}

/*
** �}�`�����_���S�Ƀx�N�g�� v �{����
*/
Matrix &Matrix::scale(const double *v)
{
  if (v[3] != 0.0)
    return scale(v[0] / v[3], v[1] / v[3], v[2] / v[3]);
  else
    return scale(v[0], v[1], v[2]);
}

/*
** (x, y, z) �𒆐S�� a �x��]����i�C�ӎ����S�̉�]�j
*/
Matrix &Matrix::rotate(double a, double x, double y, double z)
{
  double d = x * x + y * y + z * z;

  if (d > 0.0) {
    d = sqrt(d);
    
    double l  = x / d, m  = y / d, n  = z / d;
    double l2 = l * l, m2 = m * m, n2 = n * n;
    double lm = l * m, mn = m * n, nl = n * l;
    
    double s  = sin(a);
    double c  = cos(a);
    double c1 = 1.0 - c;
    
    double t[] = {
      (1.0 - l2) * c + l2, lm * c1 + n * s, nl * c1 - m * s, 0.0,
      lm * c1 - n * s, (1.0 - m2) * c + m2, mn * c1 + l * s, 0.0,
      nl * c1 + m * s, mn * c1 - l * s, (1.0 - n2) * c + n2, 0.0,
      0.0, 0.0, 0.0, 1.0,
    };

    return multiply(t);
  }
  else
    return *this;
}

/*
** ���W�ϊ��i�x�N�g�� v1 �ɍs������������ʂ��x�N�g�� v2 �Ɋi�[����j
*/
void Matrix::transform(const double *v1, double *v2) const
{
  v2[0] = v1[0] * m[ 0] + v1[1] * m[ 4] + v1[2] * m[ 8] + v1[3] * m[12];
  v2[1] = v1[0] * m[ 1] + v1[1] * m[ 5] + v1[2] * m[ 9] + v1[3] * m[13];
  v2[2] = v1[0] * m[ 2] + v1[1] * m[ 6] + v1[2] * m[10] + v1[3] * m[14];
  v2[3] = v1[0] * m[ 3] + v1[1] * m[ 7] + v1[2] * m[11] + v1[3] * m[15];
}

/*
** ���e�i�x�N�g�� v1 �ɍs������������ʂ��l�v�f�Ŋ����� v2 �Ɋi�[����j
*/
void Matrix::projection(const double *v1, double *v2) const
{
  double t[4];
  
  transform(v1, t);
  
  if (t[3] != 0.0) {
    v2[0] = t[0] / t[3];
    v2[1] = t[1] / t[3];
    v2[2] = t[2] / t[3];
  }
  else {
    v2[0] = t[0];
    v2[1] = t[1];
    v2[2] = t[2];
  }
}

/*
** �s���ۑ�����
*/
const Matrix &Matrix::push() const
{
  stack.push(*this);

  return *this;
}

/*
** �ۑ������ϊ��s������o��
*/
Matrix &Matrix::pop()
{
  this->load(stack.top());
  stack.pop();

  return *this;
}
