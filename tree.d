Matrix.o: Matrix.cpp Matrix.h
Trackball.o: Trackball.cpp Trackball.h
Tree.o: Tree.cpp extrusion.h Matrix.h Tree.h
extrusion.o: extrusion.cpp extrusion.h
main.o: main.cpp /usr/X11R6/include/GL/glut.h /usr/X11R6/include/GL/gl.h \
  /usr/X11R6/include/GL/glext.h /usr/X11R6/include/GL/glu.h Trackball.h \
  Tree.h Matrix.h
