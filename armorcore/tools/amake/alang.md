# alang

alang is a TypeScript to C transpiler. A small subset of TypeScript is supported which maps well with C in order to achieve the best performance.

```ts
// TS
type point_t = {
	x: f32;
	y: f32;
};

function add(a: point_t, b: point_t) {
	a.x += b.x;
    a.y += b.y;
}

function main() {
	let a: point_t = { x: 1.0, y: 2.0 };
	let b: point_t = { x: 3.0, y: 4.0 };
	add(a, b);
}
```

```c
// C
#include <iron.h>

typedef struct point {
	f32 x;
	f32 y;
} point_t;

void add(point_t *a, point_t *b) {
	a->x += b->x;
	a->y += b->y;
}

void main() {
	point_t *a = GC_ALLOC_INIT(point_t, { x: 1.0, y: 2.0 });
	point_t *b = GC_ALLOC_INIT(point_t, { x: 3.0, y: 4.0 });
	add(a, b);
}
```
