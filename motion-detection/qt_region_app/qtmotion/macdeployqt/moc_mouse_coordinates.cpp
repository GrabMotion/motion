/****************************************************************************
** Meta object code from reading C++ file 'mouse_coordinates.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qtmotion/drawing/mouse_coordinates.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mouse_coordinates.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_mouse_coordinates_t {
    QByteArrayData data[6];
    char stringdata0[73];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_mouse_coordinates_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_mouse_coordinates_t qt_meta_stringdata_mouse_coordinates = {
    {
QT_MOC_LITERAL(0, 0, 17), // "mouse_coordinates"
QT_MOC_LITERAL(1, 18, 9), // "Mouse_Pos"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 17), // "sendMousePosition"
QT_MOC_LITERAL(4, 47, 7), // "QPoint&"
QT_MOC_LITERAL(5, 55, 17) // "savedRegionResutl"

    },
    "mouse_coordinates\0Mouse_Pos\0\0"
    "sendMousePosition\0QPoint&\0savedRegionResutl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_mouse_coordinates[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    1,   30,    2, 0x06 /* Public */,
       5,    1,   33,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    2,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void mouse_coordinates::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        mouse_coordinates *_t = static_cast<mouse_coordinates *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Mouse_Pos(); break;
        case 1: _t->sendMousePosition((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 2: _t->savedRegionResutl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (mouse_coordinates::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mouse_coordinates::Mouse_Pos)) {
                *result = 0;
            }
        }
        {
            typedef void (mouse_coordinates::*_t)(QPoint & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mouse_coordinates::sendMousePosition)) {
                *result = 1;
            }
        }
        {
            typedef void (mouse_coordinates::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mouse_coordinates::savedRegionResutl)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject mouse_coordinates::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_mouse_coordinates.data,
      qt_meta_data_mouse_coordinates,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *mouse_coordinates::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *mouse_coordinates::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_mouse_coordinates.stringdata0))
        return static_cast<void*>(const_cast< mouse_coordinates*>(this));
    return QLabel::qt_metacast(_clname);
}

int mouse_coordinates::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void mouse_coordinates::Mouse_Pos()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void mouse_coordinates::sendMousePosition(QPoint & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void mouse_coordinates::savedRegionResutl(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
