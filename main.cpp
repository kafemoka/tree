#include <cstdlib>
#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  undef GL_GLEXT_PROTOTYPES
#  include "glut.h"
#elif defined(X11)
#  include <GL/glut.h>
#elif defined(__APPLE__)
#  include <GLUT/glut.h>
#endif

/*
**�g���b�N�{�[������
*/
#include "Trackball.h"
static Trackball *tb = 0;               // �g���b�N�{�[���̃I�u�W�F�N�g
static int btn;                         // ������Ă���}�E�X�{�^��

/*
** L-System �ɂ��؂̐���
*/
#include "Tree.h"
static Tree *tree = 0;

/*
** �؂̐������@
*/
static const char initial[] = "X";      // ����������
static const char * const rule[] = {    // ���������K��
  //"X:F[+<X]-<X",
  "X:F[+<X]F[++>X][+++<X]FX[++++<X]",
  //"X:F[>X]+[>X]+>X",
  //"X:F[>X]<X",
  //"X:F>X",
  //"X:F+X",
  //"X:F>X[[+>Y]->Y]<X",
  //"Y:F<Y[[-<X]-<X]>Y",
  0
};
static const int level = 6;             // �ċA���x���i�[���j

/*
** �؂��L�т����
*/
static const double dir[] = { 0.0, 0.2, 0.0, 1.0 };

/*
** ����] (+/-) �̊p�x�̃X�e�b�v
*/
static const double rotate = 120.0;

/*
** �܂�Ȃ� (>/<) �p�x�̃X�e�b�v
*/
static const double bend = 30.0;

/*
** ���̍����̔��a
*/
static const double radius = 0.02;

/*
** �؂̑��ʐ�
*/
static const int side = 8;

/*
** �؂̐F
*/
static const GLfloat wood[] = { 0.5f, 0.3f, 0.1f, 1.0f };

/*
** �����̈ʒu�i�w�b�h���C�g�j
*/
static const GLfloat light[] = { 0.0f, 0.0f, 1.0f, 0.0f };

/*
** ��ʕ\��
*/
static void display(void)
{
  // ��ʃN���A
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // �g���b�N�{�[������
  glPushMatrix();
  glMultMatrixd(tb->rotation());
  
  // �ގ��̐ݒ�
  glMaterialfv(GL_FRONT, GL_DIFFUSE, wood);

  // �؂̕`��
  tree->draw();
  
  // ���f���r���[�ϊ��s��̕��A
  glPopMatrix();
  
  // �_�u���o�b�t�@�����O
  glutSwapBuffers();
}


/*
** �E�B���h�E�̃��T�C�Y
*/
static void resize(int w, int h)
{
  // �g���b�N�{�[�������͈̔�
  tb->region(w, h);
  
  // �r���[�|�[�g�̐ݒ�
  glViewport(0, 0, w, h);
  
  // �����ϊ��s��̐ݒ�
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLdouble)w / (GLdouble)h, 1.0, 20.0);

  // ���f���r���[�ϊ��s��̐ݒ�
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 5.0, 10.0, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0);

  // �����̈ʒu�̐ݒ�i���_�ʒu��j
  glLightfv(GL_LIGHT0, GL_POSITION, light);
}

/*
** ��ʕ\���̍X�V
*/
static void idle(void)
{
  glutPostRedisplay();
}

/*
** �}�E�X�{�^������
*/
static void mouse(int button, int state, int x, int y)
{
  // �����ꂽ�{�^�����L�����Ă���
  btn = button;
  
  switch (btn) {
      
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
      // �g���b�N�{�[���J�n
      tb->start(x, y);
      glutIdleFunc(idle);
    }
    else {
      // �g���b�N�{�[����~
      tb->stop(x, y);
      glutIdleFunc(0);
    }
    break;
      
  case GLUT_MIDDLE_BUTTON:
    break;
      
  case GLUT_RIGHT_BUTTON:
    break;
      
  default:
    break;
  }
}

/*
** �}�E�X�h���b�O����
*/
static void motion(int x, int y)
{
  switch (btn) {
      
  case GLUT_LEFT_BUTTON:
    // �g���b�N�{�[���ړ�
    tb->motion(x, y);
    break;
      
  case GLUT_MIDDLE_BUTTON:
    break;

  case GLUT_RIGHT_BUTTON:
    break;

  default:
    break;
  }
}

/*
** �L�[�{�[�h����
*/
static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
      
  case 'q':
  case 'Q':
  case '\033':
    // �I��
    exit(0);
      
  default:
    break;
  }
}

/*
** ��n��
*/
static void cleanup(void)
{
  delete tb;
  delete tree;
}

/*
** ������
*/
static void init(void)
{
  // �w�i�F
  glClearColor(1.0, 1.0, 1.0, 1.0);
  
  // �B�ʏ��������̐ݒ�
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  
  // �����̐ݒ�
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

/*
** ���C��
*/
int main(int argc, char *argv[])
{
  // �I�u�W�F�N�g����
  tb = new Trackball;
  tree = new Tree(initial, rule, level, dir, rotate, bend, radius, side);
  atexit(cleanup);

  // ��ʕ\���̐ݒ�
  glutInit(&argc, argv);
  glutInitWindowSize(500, 500);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();

  return 0;
}
