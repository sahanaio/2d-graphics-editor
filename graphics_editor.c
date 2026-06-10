/*
 * 2D Graphics Editor in C
 * Uses a 2D character array as a drawing canvas
 * Canvas: '_' (background), '*' (drawn objects)
 *
 * Supports: Circle, Rectangle, Line, Triangle
 * Operations: Add, Delete, Modify
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Canvas Configuration ─────────────────────────────────────── */
#define ROWS 25
#define COLS 60
#define EMPTY '_'
#define DRAW  '*'
#define MAX_OBJECTS 50

/* ─── Object Types ──────────────────────────────────────────────── */
#define TYPE_CIRCLE    1
#define TYPE_RECT      2
#define TYPE_LINE      3
#define TYPE_TRIANGLE  4

/* ─── Data Structures ───────────────────────────────────────────── */
typedef struct {
    int type;       /* TYPE_CIRCLE, TYPE_RECT, TYPE_LINE, TYPE_TRIANGLE */
    int active;     /* 1 = exists, 0 = deleted                          */

    /* Circle  : cx, cy, r                    */
    /* Rect    : x1, y1 (top-left), x2, y2   */
    /* Line    : x1, y1, x2, y2              */
    /* Triangle: x1,y1, x2,y2, x3,y3        */
    int x1, y1, x2, y2, x3, y3;
    int r;          /* radius for circle */
} Object;

/* Global canvas and object list */
char canvas[ROWS][COLS];
Object objects[MAX_OBJECTS];
int obj_count = 0;

/* ─── Canvas Utilities ──────────────────────────────────────────── */
void init_canvas() {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            canvas[i][j] = EMPTY;
}

void display_canvas() {
    printf("\n");
    /* Column ruler */
    printf("   ");
    for (int j = 0; j < COLS; j += 5) printf("%-5d", j);
    printf("\n   ");
    for (int j = 0; j < COLS; j++) printf("-");
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        printf("%2d|", i);
        for (int j = 0; j < COLS; j++)
            putchar(canvas[i][j]);
        printf("|\n");
    }

    printf("   ");
    for (int j = 0; j < COLS; j++) printf("-");
    printf("\n\n");
}

/* Safe pixel set (bounds check) */
void set_pixel(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = DRAW;
}

/* Clear a pixel back to background */
void clear_pixel(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = EMPTY;
}

/* Rebuild the entire canvas from active objects */
void rebuild_canvas() {
    init_canvas();
    for (int i = 0; i < obj_count; i++)
        if (objects[i].active)
            ; /* handled by draw_object() calls below */
}

/* ─── Drawing Primitives ────────────────────────────────────────── */

/* Bresenham's line algorithm */
void draw_line_pixels(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        set_pixel(y1, x1);   /* note: row=y, col=x */
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

/* Midpoint circle algorithm */
void draw_circle_pixels(int cx, int cy, int r) {
    int x = 0, y = r;
    int d = 1 - r;

    while (x <= y) {
        /* 8 symmetric points */
        set_pixel(cy + y, cx + x);
        set_pixel(cy - y, cx + x);
        set_pixel(cy + y, cx - x);
        set_pixel(cy - y, cx - x);
        set_pixel(cy + x, cx + y);
        set_pixel(cy - x, cx + y);
        set_pixel(cy + x, cx - y);
        set_pixel(cy - x, cx - y);

        if (d < 0)        d += 2 * x + 3;
        else            { d += 2 * (x - y) + 5; y--; }
        x++;
    }
}

void draw_rect_pixels(int x1, int y1, int x2, int y2) {
    /* Ensure x1<=x2 and y1<=y2 */
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

    draw_line_pixels(x1, y1, x2, y1); /* top    */
    draw_line_pixels(x1, y2, x2, y2); /* bottom */
    draw_line_pixels(x1, y1, x1, y2); /* left   */
    draw_line_pixels(x2, y1, x2, y2); /* right  */
}

void draw_triangle_pixels(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line_pixels(x1, y1, x2, y2);
    draw_line_pixels(x2, y2, x3, y3);
    draw_line_pixels(x3, y3, x1, y1);
}

/* Draw a single object onto the canvas */
void draw_object(Object *o) {
    if (!o->active) return;
    switch (o->type) {
        case TYPE_CIRCLE:   draw_circle_pixels(o->x1, o->y1, o->r);                      break;
        case TYPE_RECT:     draw_rect_pixels(o->x1, o->y1, o->x2, o->y2);                break;
        case TYPE_LINE:     draw_line_pixels(o->x1, o->y1, o->x2, o->y2);                break;
        case TYPE_TRIANGLE: draw_triangle_pixels(o->x1, o->y1, o->x2, o->y2, o->x3, o->y3); break;
    }
}

/* Redraw all active objects */
void redraw_all() {
    init_canvas();
    for (int i = 0; i < obj_count; i++)
        draw_object(&objects[i]);
}

/* ─── Input Helpers ─────────────────────────────────────────────── */
int read_int(const char *prompt) {
    int v;
    printf("%s", prompt);
    while (scanf("%d", &v) != 1) {
        printf("Invalid. %s", prompt);
        while (getchar() != '\n');
    }
    return v;
}

void flush_stdin() { while (getchar() != '\n'); }

/* ─── Object Operations ─────────────────────────────────────────── */

/* Print a human-readable description of one object */
void print_object(int idx) {
    Object *o = &objects[idx];
    if (!o->active) { printf("  [%d] (deleted)\n", idx + 1); return; }

    printf("  [%d] ", idx + 1);
    switch (o->type) {
        case TYPE_CIRCLE:
            printf("Circle  — center (%d,%d)  radius %d\n", o->x1, o->y1, o->r);
            break;
        case TYPE_RECT:
            printf("Rectangle — (%d,%d) to (%d,%d)\n", o->x1, o->y1, o->x2, o->y2);
            break;
        case TYPE_LINE:
            printf("Line    — (%d,%d) to (%d,%d)\n", o->x1, o->y1, o->x2, o->y2);
            break;
        case TYPE_TRIANGLE:
            printf("Triangle  — (%d,%d) (%d,%d) (%d,%d)\n",
                   o->x1, o->y1, o->x2, o->y2, o->x3, o->y3);
            break;
    }
}

/* List all objects */
void list_objects() {
    if (obj_count == 0) { printf("  No objects added yet.\n"); return; }
    for (int i = 0; i < obj_count; i++)
        print_object(i);
}

/* ─── Add Functions ─────────────────────────────────────────────── */
void add_circle() {
    if (obj_count >= MAX_OBJECTS) { printf("Canvas full!\n"); return; }
    Object *o = &objects[obj_count];
    o->type   = TYPE_CIRCLE;
    o->active = 1;

    printf("\n  [Canvas: cols 0-%d, rows 0-%d]\n", COLS-1, ROWS-1);
    o->x1 = read_int("  Center X (col): ");
    o->y1 = read_int("  Center Y (row): ");
    o->r  = read_int("  Radius        : ");

    draw_object(o);
    obj_count++;
    printf("  Circle added (object #%d).\n", obj_count);
}

void add_rectangle() {
    if (obj_count >= MAX_OBJECTS) { printf("Canvas full!\n"); return; }
    Object *o = &objects[obj_count];
    o->type   = TYPE_RECT;
    o->active = 1;

    printf("\n  [Canvas: cols 0-%d, rows 0-%d]\n", COLS-1, ROWS-1);
    o->x1 = read_int("  Top-left  X (col): ");
    o->y1 = read_int("  Top-left  Y (row): ");
    o->x2 = read_int("  Bot-right X (col): ");
    o->y2 = read_int("  Bot-right Y (row): ");

    draw_object(o);
    obj_count++;
    printf("  Rectangle added (object #%d).\n", obj_count);
}

void add_line() {
    if (obj_count >= MAX_OBJECTS) { printf("Canvas full!\n"); return; }
    Object *o = &objects[obj_count];
    o->type   = TYPE_LINE;
    o->active = 1;

    printf("\n  [Canvas: cols 0-%d, rows 0-%d]\n", COLS-1, ROWS-1);
    o->x1 = read_int("  Start X (col): ");
    o->y1 = read_int("  Start Y (row): ");
    o->x2 = read_int("  End   X (col): ");
    o->y2 = read_int("  End   Y (row): ");

    draw_object(o);
    obj_count++;
    printf("  Line added (object #%d).\n", obj_count);
}

void add_triangle() {
    if (obj_count >= MAX_OBJECTS) { printf("Canvas full!\n"); return; }
    Object *o = &objects[obj_count];
    o->type   = TYPE_TRIANGLE;
    o->active = 1;

    printf("\n  [Canvas: cols 0-%d, rows 0-%d]\n", COLS-1, ROWS-1);
    o->x1 = read_int("  Vertex 1 X: "); o->y1 = read_int("  Vertex 1 Y: ");
    o->x2 = read_int("  Vertex 2 X: "); o->y2 = read_int("  Vertex 2 Y: ");
    o->x3 = read_int("  Vertex 3 X: "); o->y3 = read_int("  Vertex 3 Y: ");

    draw_object(o);
    obj_count++;
    printf("  Triangle added (object #%d).\n", obj_count);
}

/* ─── Delete ────────────────────────────────────────────────────── */
void delete_object() {
    if (obj_count == 0) { printf("  No objects to delete.\n"); return; }
    list_objects();
    int idx = read_int("\n  Enter object number to delete (0 to cancel): ");
    if (idx == 0) return;
    if (idx < 1 || idx > obj_count || !objects[idx-1].active) {
        printf("  Invalid selection.\n"); return;
    }
    objects[idx-1].active = 0;
    redraw_all();
    printf("  Object #%d deleted.\n", idx);
}

/* ─── Modify ────────────────────────────────────────────────────── */
void modify_object() {
    if (obj_count == 0) { printf("  No objects to modify.\n"); return; }
    list_objects();
    int idx = read_int("\n  Enter object number to modify (0 to cancel): ");
    if (idx == 0) return;
    if (idx < 1 || idx > obj_count || !objects[idx-1].active) {
        printf("  Invalid selection.\n"); return;
    }

    Object *o = &objects[idx-1];
    printf("\n  Modifying object #%d. Enter new values:\n", idx);
    printf("  [Canvas: cols 0-%d, rows 0-%d]\n", COLS-1, ROWS-1);

    switch (o->type) {
        case TYPE_CIRCLE:
            o->x1 = read_int("  Center X (col): ");
            o->y1 = read_int("  Center Y (row): ");
            o->r  = read_int("  Radius        : ");
            break;
        case TYPE_RECT:
            o->x1 = read_int("  Top-left  X: ");
            o->y1 = read_int("  Top-left  Y: ");
            o->x2 = read_int("  Bot-right X: ");
            o->y2 = read_int("  Bot-right Y: ");
            break;
        case TYPE_LINE:
            o->x1 = read_int("  Start X: "); o->y1 = read_int("  Start Y: ");
            o->x2 = read_int("  End   X: "); o->y2 = read_int("  End   Y: ");
            break;
        case TYPE_TRIANGLE:
            o->x1 = read_int("  V1 X: "); o->y1 = read_int("  V1 Y: ");
            o->x2 = read_int("  V2 X: "); o->y2 = read_int("  V2 Y: ");
            o->x3 = read_int("  V3 X: "); o->y3 = read_int("  V3 Y: ");
            break;
    }

    redraw_all();
    printf("  Object #%d updated.\n", idx);
}

/* ─── Clear Canvas ──────────────────────────────────────────────── */
void clear_all() {
    for (int i = 0; i < obj_count; i++) objects[i].active = 0;
    obj_count = 0;
    init_canvas();
    printf("  Canvas cleared.\n");
}

/* ─── Sub-menus ─────────────────────────────────────────────────── */
void add_menu() {
    printf("\n  ┌─ Add Object ─────────────────┐\n");
    printf("  │  1. Circle                   │\n");
    printf("  │  2. Rectangle                │\n");
    printf("  │  3. Line                     │\n");
    printf("  │  4. Triangle                 │\n");
    printf("  │  0. Back                     │\n");
    printf("  └──────────────────────────────┘\n");
    int ch = read_int("  Choice: ");
    switch (ch) {
        case 1: add_circle();    break;
        case 2: add_rectangle(); break;
        case 3: add_line();      break;
        case 4: add_triangle();  break;
        case 0: break;
        default: printf("  Invalid choice.\n");
    }
}

/* ─── Main Menu ─────────────────────────────────────────────────── */
void main_menu() {
    init_canvas();

    int running = 1;
    while (running) {
        printf("\n  ╔══════════════════════════════╗\n");
        printf("  ║   2D CHARACTER GRAPHICS EDITOR  ║\n");
        printf("  ╚══════════════════════════════╝\n");
        printf("  1. Display Canvas\n");
        printf("  2. Add Object\n");
        printf("  3. Delete Object\n");
        printf("  4. Modify Object\n");
        printf("  5. List Objects\n");
        printf("  6. Clear All\n");
        printf("  0. Exit\n");
        printf("  ──────────────────────────────\n");

        int ch = read_int("  Choice: ");
        switch (ch) {
            case 1: display_canvas();  break;
            case 2: add_menu();        break;
            case 3: delete_object();   break;
            case 4: modify_object();   break;
            case 5: list_objects();    break;
            case 6: clear_all();       break;
            case 0: running = 0; printf("\n  Goodbye!\n\n"); break;
            default: printf("  Invalid choice. Try again.\n");
        }
    }
}

/* ─── Entry Point ───────────────────────────────────────────────── */
int main() {
    main_menu();
    return 0;
}
