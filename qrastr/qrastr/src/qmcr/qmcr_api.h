#pragma once
#include <QtCore/qglobal.h>

///@note qmcr подключается в качестве dll, поэтому для использования
///современного connect требуется qmcr_EXPORTS
///
#ifdef qmcr_EXPORTS
#  define QMCR_API Q_DECL_EXPORT
#else
#  define QMCR_API Q_DECL_IMPORT
#endif
