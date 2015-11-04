/****************************************************************************
** Meta object code from reading C++ file 'streamingthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qtmotion/threads/streamingthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'streamingthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_StreamingThread_t {
    QByteArrayData data[6];
    char stringdata0[63];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StreamingThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StreamingThread_t qt_meta_stringdata_StreamingThread = {
    {
QT_MOC_LITERAL(0, 0, 15), // "StreamingThread"
QT_MOC_LITERAL(1, 16, 25), // "StreamingUpdateLabelImage"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "std::string"
QT_MOC_LITERAL(4, 55, 3), // "Mat"
QT_MOC_LITERAL(5, 59, 3) // "img"

    },
    "StreamingThread\0StreamingUpdateLabelImage\0"
    "\0std::string\0Mat\0img"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StreamingThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 4,    2,    5,

       0        // eod
};

void StreamingThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        StreamingThread *_t = static_cast<StreamingThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->StreamingUpdateLabelImage((*reinterpret_cast< std::string(*)>(_a[1])),(*reinterpret_cast< Mat(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (StreamingThread::*_t)(std::string , Mat );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&StreamingThread::StreamingUpdateLabelImage)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject StreamingThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_StreamingThread.data,
      qt_meta_data_StreamingThread,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *StreamingThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StreamingThread::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_StreamingThread.stringdata0))
        return static_cast<void*>(const_cast< StreamingThread*>(this));
    return QThread::qt_metacast(_clname);
}

int StreamingThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void StreamingThread::StreamingUpdateLabelImage(std::string _t1, Mat _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
