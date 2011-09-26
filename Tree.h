/*
** L-System �ɂ��؂̐���
*/
#ifndef TREE_H
#define TREE_H

#include "Matrix.h"

class Tree {
  double rotate;                  // ����] (+/-) �̊p�x�̃X�e�b�v
  double bend;                    // �܂�Ȃ� (>/<) �p�x�̃X�e�b�v
  double radius;                  // �؂̍����̔��a
  double top[4];                  // ��̎}�̐�[�̈ʒu�i�؂��L�т�����j
  static const double base[4];    // ��̎}�̍����̈ʒu�i���_�j
  double (*spine)[3];             // ���i�̒��_�ʒu
  int nspine;                     // ���i�̒��_��
  int *branch;                    // ����ʒu�̒��_�ԍ�
  int nbranch;                    // ����̐�
  int flag;                       // ���ۂɖ؂𐶐�����Ȃ� 0
  double (*cs)[2];                // �f�ʂ̒��_�ʒu
  int ncs;                        // �f�ʂ̒��_��
  Matrix m;                       // ��Ɨp�̕ϊ��s��
  void turtle(const char *p);
  void count(const char *p);
  void production(const char *istr, const char * const *rstr, int iter);

public:
  Tree(
    const char *initial,          // ����������
    const char * const *rule,     // ���������K��
    int level,                    // �ċA���x��
    const double *direction = 0,  // �؂��L�т����
    double rstep = 120.0,         // �����S�̉�]�̊p�x�X�e�b�v
    double bstep = 30.0,          // �Ȃ������̊p�x�X�e�b�v
    double r = 0.02,              // �؂̍����̔��a
    int n = 8                     // �؂̑��ʐ�
    );
  virtual ~Tree();
  void draw();
};

#endif
