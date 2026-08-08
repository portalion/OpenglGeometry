# The `Algebra` library

A small self-contained math library built as a **static library target**
(`Algebra/CMakeLists.txt`) and linked into `OpenglGeometry`. Everything lives in
`namespace Algebra`.

```cpp
#include "Algebra.h"     // pulls in Vector4, Matrix4, Utils, Quaternion
```

| File | Contents |
| --- | --- |
| `Algebra.h` | Umbrella header |
| `Vector4.{h,cpp}` | 4-component vector |
| `Matrix4.{h,cpp}` | 4×4 **row-major** matrix, transform factories, tridiagonal solver |
| `Quaternion.{h,cpp}` | Rotation quaternion |
| `Utils.{h,cpp}` | Degree/radian conversion |

There is no `Vector3`. Directions are `Vector4` with `w = 0`, positions with `w = 1`.

---

## `Vector4`

```cpp
class Vector4
{
public:
    float x, y, z, w;

    Vector4();                                        // (0,0,0,0)
    Vector4(float x, float y, float z, float w = 0.f);

    float Length() const;
    float Sum() const;
    const Vector4 Scale(const Vector4& scalingVector) const;   // component-wise
    Vector4 Normalize() const;
    Vector4 Cross(Vector4 anotherVector);
    std::vector<float> ToVector() const;

    float& operator[](std::size_t index);              // 0..3 → x,y,z,w
    const Vector4 operator+(const Vector4&) const;
    const Vector4 operator-() const;
    const Vector4 operator-(const Vector4&) const;
    const float   operator*(const Vector4&) const;     // ← DOT PRODUCT
    const bool    operator==(const Vector4&) const;
    Vector4& operator+=(const Vector4&);

    friend Vector4 operator*(const Vector4&, const Matrix4&);
    friend Vector4 operator*(const Matrix4&, const Vector4&);
    friend Vector4 operator*(const Vector4&, const float&);
    friend Vector4 operator*(const float&,   const Vector4&);
    friend Vector4 operator/(const Vector4&, const float&);
    friend std::ostream& operator<<(std::ostream&, const Vector4&);
};
```

**`operator*` between two vectors is the dot product**, returning a `float` — not a
component-wise product. Use `Scale()` for component-wise multiplication.

The default constructor gives `w = 0` (a direction). The 3-argument constructor also defaults
`w = 0`, so `Vector4(1, 2, 3)` is a **direction**, not a point. Code that needs a position
must pass `1.f` explicitly, which is why you see `position.w = 1.f;` fix-ups in
`MeshGeneratingSystem::CopyValidPointsToVector` and
`InterpolatedBezierCurve::GenerateVertices`.

`operator!=` is not declared, but `Observable<Algebra::Vector4>` uses `!=` — C++20 synthesises
it from `operator==`.

Both `vector * matrix` and `matrix * vector` exist (row-vector and column-vector
conventions). `DragCamera::HandleTranslation` uses the row-vector form:

```cpp
position += direction * 10.f * dt * GetRotationMatrix() * GetZoomMatrix();
```

---

## `Matrix4`

```cpp
class Matrix4
{
private:
    Vector4 rows[4];        // ← ROW-major storage
public:
    Matrix4();                                              // all zeros
    Matrix4(const Vector4& diagonal);                       // diagonal matrix
    Matrix4(float d1, float d2, float d3, float d4);        // diagonal matrix
    Matrix4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3);

    float   Sum() const;
    Matrix4 Transpose();
    Matrix4 Inverse();
    Vector4 Column(std::size_t index) const;

    Vector4& operator[](std::size_t index);   // row access: m[row][col]
    ...
};
```

**Storage is row-major**, and `m[i]` is row *i*. This is why `Shader::SetUniformMat4f` passes
`GL_TRUE` for the `transpose` parameter — GLSL expects column-major.

The default constructor is the **zero** matrix, not the identity. Use `Matrix4::Identity()`.

### Transform factories

```cpp
static Matrix4 Identity();
static Matrix4 Translation(float x, float y, float z);
static Matrix4 Translation(Algebra::Vector4 translation);
static Matrix4 Rotation(float x, float y, float z);          // combined XYZ
static Matrix4 RotationX(float angle);                       // radians
static Matrix4 RotationY(float angle);
static Matrix4 RotationZ(float angle);
static Matrix4 RotationDegree(float x, float y, float z);     // degree variants
static Matrix4 RotationXDegree(float angle);
static Matrix4 RotationYDegree(float angle);
static Matrix4 RotationZDegree(float angle);
static Matrix4 DiagonalScaling(float x, float y, float z, float w = 1);
static Matrix4 Projection(float aspect, float lastZ, float firstZ, float fov);
```

Translation lives in **column 3** (`result[i][3]`), consistent with row-major storage and the
`matrix * vector` convention used by the renderer. `RenderingSystem` reads the camera position
back out as `cameraUniforms.Position[3]` — row 3 of the translation matrix.

### `Projection`

```cpp
Matrix4 Matrix4::Projection(float aspect, float lastZ, float firstZ, float fov)
{
    float ctgFov = cosf(fov / 2.f) / sinf(fov / 2.f);
    auto result = Matrix4(ctgFov / aspect, ctgFov, (lastZ + firstZ) / (lastZ - firstZ), 0.f);
    result[3][2] = 1.f;
    result[2][3] = (-2.f * firstZ * lastZ) / (lastZ - firstZ);
    return result;
}
```

Called once in `BaseScene`:

```cpp
Algebra::Matrix4::Projection(aspect, 0.1f, 10000.0f, 3.14f / 2.f);
```

Note the parameter names: the *second* argument (`lastZ`) receives the near plane and the
*third* (`firstZ`) the far plane. Read the call site, not the names. `fov` is in radians.

### `SolveTrilinealEquation`

```cpp
static std::vector<Vector4> SolveTrilinealEquation(std::vector<float> alpha,
                                                   std::vector<float> beta,
                                                   std::vector<Vector4> r);
```

The **Thomas algorithm** for a tridiagonal system whose main diagonal is a constant 2:

```
| 2      β0                  |   | c0 |   | r0 |
| α0     2      β1           |   | c1 |   | r1 |
|        α1     2      β2    | · | c2 | = | r2 |
|               …            |   | …  |   | …  |
```

Forward sweep, then back substitution, operating on `Vector4` right-hand sides (so all three
spatial components are solved at once). `w` is forced to 0 at every step, keeping the results
as directions.

Used only by
[`InterpolatedBezierCurve::GenerateVertices`](geometry/bezier-curves.md#interpolated-bézier-natural-cubic-spline)
to find the natural cubic spline's second-derivative coefficients. The `m == 1` special case
handles a spline with a single interior knot.

There is no pivoting or singularity check; a degenerate input (zero denominator) produces
NaNs, which is why the caller scrubs NaNs from `alpha`/`beta` before calling.

---

## `Quaternion`

```cpp
class Quaternion
{
public:
    float w, x, y, z;                                  // ← w first

    Quaternion() : w(1), x(0), y(0), z(0) {}           // identity
    Quaternion(float w, float x, float y, float z);    // ← w first here too

    static Quaternion Identity();
    static Quaternion CreateFromAxisAngle(const Vector4& axis, float angle);
    static Quaternion CreateFromEulerAngles(float yaw, float pitch, float roll);

    float      Length() const;
    Quaternion Normalize() const;
    Quaternion Conjugate() const;
    Quaternion Inverse() const;
    float      Dot(const Quaternion& other) const;
    Matrix4    ToMatrix() const;
    Vector4    Rotate(const Vector4& vector) const;

    Quaternion operator*(const Quaternion& other) const;   // composition
    Quaternion operator+(const Quaternion& other) const;
    Quaternion operator*(float scalar) const;
    Quaternion& operator*=(float scalar);
    bool operator==(const Quaternion& other) const;
};
```

**Component order is `(w, x, y, z)`** — the scalar comes first, both in the layout and in the
constructor. `Quaternion(1, 0, 0, 0)` is the identity.

Unlike `Matrix4`, the default constructor *is* the identity.

Multiplication composes rotations, applied right to left: `a * b` means "apply `b`, then
`a`". `DragCamera` relies on this:

```cpp
rotation = (yawQuat * rotation).Normalize();       // yaw applied after the existing rotation
```

`ToMatrix()` is what `RotationComponent` feeds into the model matrix, via
`UniformApplier::RotationApplier`.

Normalise after composing — repeated multiplication accumulates floating-point drift.
`DragCamera` and `RotationComponent::Rotate` both do.

---

## `Utils`

```cpp
float DegreeToRadians(float degree);
float RadiansToDegree(float radians);
```

Note that most of the codebase writes angles in radians directly, often with a hard-coded
`3.14f` rather than `std::numbers::pi_v<float>` (`BaseScene`'s FOV,
`MeshGenerator::Torus`). `BezierSurfaceArchetypeCreation.h` does use
`std::numbers::pi_v<float>`. Prefer that in new code.

---

## Working on `Algebra`

It is a separate CMake target. New `.cpp` files must be added to the `add_library` list in
`Algebra/CMakeLists.txt`; add new headers to `Algebra.h` so the umbrella include keeps
working. The
target exports `Algebra/src` as a PUBLIC include directory, so new headers are reachable as
`#include "MyHeader.h"` from anywhere in the app.

There is no test project. If you change the math — particularly `SolveTrilinealEquation`, the
projection matrix, or quaternion composition order — verify visually with an interpolated
Bézier curve and the camera, since those are the only consumers that would show a regression.
