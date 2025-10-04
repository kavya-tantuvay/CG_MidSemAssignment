#include <GL/glut.h>
#include <cmath>
#include <string>
#include <vector>

const int WIDTH = 1600;
const int HEIGHT = 900;

int animationStep = 0;
int maxSteps = 150;
bool isPaused = false;

// Structure to store points
struct Point {
    int x, y;
    float r, g, b;
};

std::vector<Point> ddaPoints;
std::vector<Point> bresenhamLinePoints;
std::vector<Point> bresenhamCirclePoints;
std::vector<Point> midpointCirclePoints;

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_9_BY_15) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawBigText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Compute DDA points
void computeDDA(int x1, int y1, int x2, int y2) {
    ddaPoints.clear();
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = x1, y = y1;

    for (int i = 0; i <= steps; i++) {
        Point p;
        p.x = round(x);
        p.y = round(y);
        p.r = 0.2f; p.g = 0.6f; p.b = 1.0f;
        ddaPoints.push_back(p);
        x += xInc;
        y += yInc;
    }
}

// Compute Bresenham Line points
void computeBresenhamLine(int x1, int y1, int x2, int y2) {
    bresenhamLinePoints.clear();
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        Point p;
        p.x = x1;
        p.y = y1;
        p.r = 0.1f; p.g = 0.8f; p.b = 0.2f;
        bresenhamLinePoints.push_back(p);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Compute Bresenham Circle points
void computeBresenhamCircle(int xc, int yc, int r) {
    bresenhamCirclePoints.clear();
    int x = 0, y = r;
    int d = 3 - 2 * r;

    auto addPoints = [&](int x, int y) {
        Point p;
        p.r = 1.0f; p.g = 0.2f; p.b = 0.6f;

        p.x = xc + x; p.y = yc + y; bresenhamCirclePoints.push_back(p);
        p.x = xc - x; p.y = yc + y; bresenhamCirclePoints.push_back(p);
        p.x = xc + x; p.y = yc - y; bresenhamCirclePoints.push_back(p);
        p.x = xc - x; p.y = yc - y; bresenhamCirclePoints.push_back(p);
        p.x = xc + y; p.y = yc + x; bresenhamCirclePoints.push_back(p);
        p.x = xc - y; p.y = yc + x; bresenhamCirclePoints.push_back(p);
        p.x = xc + y; p.y = yc - x; bresenhamCirclePoints.push_back(p);
        p.x = xc - y; p.y = yc - x; bresenhamCirclePoints.push_back(p);
    };

    addPoints(x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        addPoints(x, y);
    }
}

// Compute Midpoint Circle points
void computeMidpointCircle(int xc, int yc, int r) {
    midpointCirclePoints.clear();
    int x = 0, y = r;
    int p = 1 - r;

    auto addPoints = [&](int x, int y) {
        Point pt;
        pt.r = 0.2f; pt.g = 0.6f; pt.b = 1.0f;

        pt.x = xc + x; pt.y = yc + y; midpointCirclePoints.push_back(pt);
        pt.x = xc - x; pt.y = yc + y; midpointCirclePoints.push_back(pt);
        pt.x = xc + x; pt.y = yc - y; midpointCirclePoints.push_back(pt);
        pt.x = xc - x; pt.y = yc - y; midpointCirclePoints.push_back(pt);
        pt.x = xc + y; pt.y = yc + x; midpointCirclePoints.push_back(pt);
        pt.x = xc - y; pt.y = yc + x; midpointCirclePoints.push_back(pt);
        pt.x = xc + y; pt.y = yc - x; midpointCirclePoints.push_back(pt);
        pt.x = xc - y; pt.y = yc - x; midpointCirclePoints.push_back(pt);
    };

    addPoints(x, y);
    while (x < y) {
        x++;
        if (p < 0) {
            p = p + 2 * x + 1;
        } else {
            y--;
            p = p + 2 * (x - y) + 1;
        }
        addPoints(x, y);
    }
}

void drawGridBox(float x, float y, float w, float h, const char* title, float r, float g, float b) {
    // Background
    glColor3f(0.95f, 0.95f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    // Grid lines
    glColor3f(0.88f, 0.88f, 0.9f);
    glLineWidth(1);
    for (int i = 0; i <= 20; i++) {
        glBegin(GL_LINES);
        glVertex2f(x + i * w / 20, y);
        glVertex2f(x + i * w / 20, y + h);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x, y + i * h / 20);
        glVertex2f(x + w, y + i * h / 20);
        glEnd();
    }

    // Border
    glColor3f(r, g, b);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    // Title background
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x + 5, y + h - 35);
    glVertex2f(x + 250, y + h - 35);
    glVertex2f(x + 250, y + h - 5);
    glVertex2f(x + 5, y + h - 5);
    glEnd();

    // Title text
    glColor3f(1, 1, 1);
    drawBigText(x + 15, y + h - 23, title);
}

void drawPixel(int x, int y, float r, float g, float b, float size = 4.0f) {
    glColor3f(r, g, b);
    glPointSize(size);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();

    // Glow effect
    glColor4f(r, g, b, 0.3f);
    glPointSize(size * 2);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void drawAnimatedPoints(const std::vector<Point>& points, int maxPoints) {
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < maxPoints && i < points.size(); i++) {
        float alpha = (i == maxPoints - 1) ? 1.0f : 0.7f;
        drawPixel(points[i].x, points[i].y, points[i].r, points[i].g, points[i].b,
                  (i == maxPoints - 1) ? 6.0f : 4.0f);
    }

    glDisable(GL_BLEND);
}

void display() {
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Title
    glColor3f(0.3f, 0.8f, 1.0f);
    drawText(550, 870, "ALGORITHM VISUALIZATION - Computer Graphics Assignment", GLUT_BITMAP_HELVETICA_18);

    // Progress bar
    float progress = (float)animationStep / maxSteps;
    glColor3f(0.2f, 0.25f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(50, 835);
    glVertex2f(1550, 835);
    glVertex2f(1550, 850);
    glVertex2f(50, 850);
    glEnd();

    glColor3f(0.3f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(50, 835);
    glVertex2f(50 + progress * 1500, 835);
    glVertex2f(50 + progress * 1500, 850);
    glVertex2f(50, 850);
    glEnd();

    // Line Drawing Section
    glColor3f(0.8f, 0.9f, 1.0f);
    drawText(350, 800, "LINE DRAWING ALGORITHMS", GLUT_BITMAP_HELVETICA_18);

    // DDA
    drawGridBox(50, 550, 350, 230, "DDA Algorithm", 0.2f, 0.6f, 1.0f);
    int ddaShow = (animationStep * ddaPoints.size()) / maxSteps;
    drawAnimatedPoints(ddaPoints, ddaShow);

    glColor3f(0.2f, 0.6f, 1.0f);
    char buffer[50];
    sprintf(buffer, "Points: %d/%d", ddaShow, (int)ddaPoints.size());
    drawText(60, 560, buffer);

    // Bresenham Line
    drawGridBox(450, 550, 350, 230, "Bresenham Line", 0.1f, 0.8f, 0.2f);
    int bLineShow = (animationStep * bresenhamLinePoints.size()) / maxSteps;
    drawAnimatedPoints(bresenhamLinePoints, bLineShow);

    glColor3f(0.1f, 0.8f, 0.2f);
    sprintf(buffer, "Points: %d/%d", bLineShow, (int)bresenhamLinePoints.size());
    drawText(460, 560, buffer);

    // Info panel
    glColor3f(0.18f, 0.20f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(850, 550);
    glVertex2f(1550, 550);
    glVertex2f(1550, 780);
    glVertex2f(850, 780);
    glEnd();

    glColor3f(0.3f, 0.8f, 1.0f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(850, 550);
    glVertex2f(1550, 550);
    glVertex2f(1550, 780);
    glVertex2f(850, 780);
    glEnd();

    glColor3f(0.3f, 0.8f, 1.0f);
    drawBigText(1030, 750, "LINE ALGORITHMS");

    glColor3f(0.7f, 0.85f, 1.0f);
    drawText(880, 710, "DDA (Digital Differential Analyzer)");
    glColor3f(0.5f, 0.7f, 0.9f);
    drawText(900, 685, "Floating-point calculations");
    drawText(900, 665, "Simple but slower");

    glColor3f(0.5f, 1.0f, 0.6f);
    drawText(880, 630, "Bresenham Line Algorithm");
    glColor3f(0.4f, 0.8f, 0.5f);
    drawText(900, 605, "Integer-only arithmetic");
    drawText(900, 585, "Faster & more efficient");
    drawText(900, 565, "Industry standard");

    // Circle Drawing Section
    glColor3f(1.0f, 0.7f, 0.9f);
    drawText(330, 510, "CIRCLE DRAWING ALGORITHMS", GLUT_BITMAP_HELVETICA_18);

    // Bresenham Circle
    drawGridBox(50, 270, 350, 230, "Bresenham Circle", 1.0f, 0.2f, 0.6f);
    int bCircleShow = (animationStep * bresenhamCirclePoints.size()) / maxSteps;
    drawAnimatedPoints(bresenhamCirclePoints, bCircleShow);

    glColor3f(1.0f, 0.2f, 0.6f);
    sprintf(buffer, "Points: %d/%d", bCircleShow, (int)bresenhamCirclePoints.size());
    drawText(60, 280, buffer);

    // Midpoint Circle
    drawGridBox(450, 270, 350, 230, "Midpoint Circle", 0.2f, 0.6f, 1.0f);
    int mCircleShow = (animationStep * midpointCirclePoints.size()) / maxSteps;
    drawAnimatedPoints(midpointCirclePoints, mCircleShow);

    glColor3f(0.2f, 0.6f, 1.0f);
    sprintf(buffer, "Points: %d/%d", mCircleShow, (int)midpointCirclePoints.size());
    drawText(460, 280, buffer);

    // Circle info panel
    glColor3f(0.18f, 0.20f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(850, 270);
    glVertex2f(1550, 270);
    glVertex2f(1550, 500);
    glVertex2f(850, 500);
    glEnd();

    glColor3f(1.0f, 0.4f, 0.8f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(850, 270);
    glVertex2f(1550, 270);
    glVertex2f(1550, 500);
    glVertex2f(850, 500);
    glEnd();

    glColor3f(1.0f, 0.4f, 0.8f);
    drawBigText(1010, 470, "CIRCLE ALGORITHMS");

    glColor3f(1.0f, 0.6f, 0.9f);
    drawText(880, 430, "Bresenham Circle Algorithm");
    glColor3f(0.9f, 0.5f, 0.8f);
    drawText(900, 405, "Integer decision parameter");
    drawText(900, 385, "8-way symmetry optimization");

    glColor3f(0.6f, 0.8f, 1.0f);
    drawText(880, 350, "Midpoint Circle Algorithm");
    glColor3f(0.5f, 0.7f, 0.9f);
    drawText(900, 325, "Implicit circle equation");
    drawText(900, 305, "Similar efficiency");
    drawText(900, 285, "Simpler decision logic");

    // Key Insights
    glColor3f(0.18f, 0.20f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(50, 50);
    glVertex2f(1550, 50);
    glVertex2f(1550, 240);
    glVertex2f(50, 240);
    glEnd();

    glColor3f(1.0f, 0.8f, 0.2f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(50, 50);
    glVertex2f(1550, 50);
    glVertex2f(1550, 240);
    glVertex2f(50, 240);
    glEnd();

    glColor3f(1.0f, 0.8f, 0.2f);
    drawBigText(650, 210, "KEY OBSERVATIONS");

    glColor3f(0.9f, 0.9f, 0.7f);
    drawText(80, 175, "Integer algorithms avoid rounding errors and are faster");
    drawText(80, 150, "Circle algorithms use 8-way symmetry (plot 8 points per iteration)");
    drawText(80, 125, "Bresenham algorithms are hardware-optimized");
    drawText(80, 100, "All produce pixel-perfect results");

    // Controls
    glColor3f(0.5f, 0.7f, 0.9f);
    drawText(80, 70, "Controls: SPACE = Pause/Resume | R = Reset | ESC = Exit");

    if (isPaused) {
        glColor3f(1.0f, 0.3f, 0.3f);
        drawBigText(720, 440, "PAUSED");
    }

    glutSwapBuffers();
}

void timer(int value) {
    if (!isPaused) {
        animationStep++;
        if (animationStep > maxSteps) {
            animationStep = 0;
        }
        glutPostRedisplay();
    }
    glutTimerFunc(50, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC
            exit(0);
            break;
        case ' ': // SPACE
            isPaused = !isPaused;
            break;
        case 'r':
        case 'R':
            animationStep = 0;
            break;
    }
    glutPostRedisplay();
}

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);

    // Compute all points
    computeDDA(100, 630, 350, 720);
    computeBresenhamLine(500, 630, 750, 720);
    computeBresenhamCircle(225, 385, 80);
    computeMidpointCircle(625, 385, 80);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Algorithm Visualization - Computer Graphics Assignment");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
