/****************************************************************************
** Meta object code from reading C++ file 'broadcastthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qtmotion/threads/broadcastthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'broadcastthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_BroadcastThread_t {
    QByteArrayData data[4];
    char stringdata0[67];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BroadcastThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BroadcastThread_t qt_meta_stringdata_BroadcastThread = {
    {
QT_MOC_LITERAL(0, 0, 15), // "BroadcastThread"
QT_MOC_LITERAL(1, 16, 17), // "BroadcastReceived"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 31) // "BroadcastTimeoutSocketException"

    },
    "BroadcastThread\0BroadcastReceived\0\0"
    "BroadcastTimeoutSocketException"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BroadcastThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       3,    0,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,

       0        // eod
};

void BroadcastThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        BroadcastThread *_t = static_cast<BroadcastThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->BroadcastReceived((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->BroadcastTimeoutSocketException(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (BroadcastThread::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&BroadcastThread::BroadcastReceived)) {
                *result = 0;
            }
        }
        {
            typedef void (BroadcastThread::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&BroadcastThread::BroadcastTimeoutSocketException)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject BroadcastThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_BroadcastThread.data,
      qt_meta_data_BroadcastThread,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *BroadcastThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BroadcastThread::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_BroadcastThread.stringdata0))
        return static_cast<void*>(const_cast< BroadcastThread*>(this));
    return QThread::qt_metacast(_clname);
}

int BroadcastThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void BroadcastThread::BroadcastReceived(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BroadcastThread::BroadcastTimeoutSocketException()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE