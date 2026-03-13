#include "graphWorker.h"
#include <QCoreApplication>
#include <QLibrary>
#include <QFile>
#include <QThread>
#include <QDebug>
#include "astra/IPlainRastr.h"
#include "graphServer.h"

GraphWorker::GraphWorker(IPlainRastr* rastr, QLibrary* lib)
        : m_rastr(rastr), m_lib(lib) {}

GraphWorker::~GraphWorker(){
    qInfo() << "GraphWorker was deleted";
}

void GraphWorker::stopFromOutside() {
    if (m_fnInit) {
        qInfo() << ">> Calling m_fnInit(nullptr) directly";
        m_fnInit(nullptr, "", "", 0, nullptr);
        m_fnInit = nullptr;
        qInfo() << ">> m_fnInit(nullptr) returned";
    }
}

void GraphWorker::slot_process() {
    // --- Проверка HTML-ресурсов ---
    auto res = m_rastr->WorkingFolder();
    if (!res || res->Code() != IPlainRastrRetCode::Ok) {
        qWarning() << "Не удалось определить рабочую папку.";
        if (res) res->Destroy();
        return;
    }
    const QString htmlDir =
        QString::fromStdString(std::string(*res) + "/html");
    if (res) res->Destroy();

    const QStringList required = {"grf.css", "grf.js", "grf2.css"};
    QStringList missing;
    for (const QString& f : required)
        if (!QFile::exists(htmlDir + "/" + f))
            missing << f;

    if (!missing.isEmpty()) {
        qWarning() << QString("В папке «%1» не найдены файлы:\n%2")
                                  .arg(htmlDir, missing.join('\n'));
        return;
    }

    const QString graphLibsPath =
        QCoreApplication::applicationDirPath() + "/../Data/graphics/graph2libs.xml";
    if (!QFile::exists(graphLibsPath)) {
        qWarning() << "GraphWorker: graph2libs.xml not found";
        emit sig_finished();
        return;
    }

	if (!m_lib->load()) {
		qCritical() << "GraphWorker: failed to load library:" << m_lib->errorString();
        emit sig_finished();
		return;
	}
	qInfo() << "GraphWorker: library loaded:" << m_lib->fileName();

	loadSymbols();
	if (!m_fnInit || !m_fnPutTextLayer || !m_fnUpdateAll) {
		qWarning() << "GraphWorker: required symbols not found";
        emit sig_finished();
		return;
	}

    qInfo() << ">> slot_process: calling m_fnInit (will block?)";

	m_fnInit(m_rastr,
			 graphLibsPath.toStdString().c_str(),
			 "127.0.0.1", 8081,
             GraphServer::staticCallback);   // подключение к либе

    qInfo() << ">> slot_process: m_fnInit RETURNED <--";
	qInfo() << "GraphWorker: HTTP server ready";
    emit sig_ready();
    qInfo() << ">> slot_process: sig_ready emitted, exiting slot";
	// Дальше работает событийный цикл потока:
	// все handleCallback() придут как queued-сигналы
}

void GraphWorker::slot_handleCallback(int iMSG, const QString& params) {
	dispatchCallback(iMSG, params.toStdString());
}

void GraphWorker::loadSymbols() {
	auto resolve = [&](const char* name) -> QFunctionPointer {
		auto fn = m_lib->resolve(name);
		if (!fn) qWarning() << "GraphWorker: symbol not found:" << name;
		return fn;
	};
	m_fnInit         = reinterpret_cast<InitPlainDLL_t>      (resolve("InitPlainDLL"));
	m_fnPutTextLayer = reinterpret_cast<PutTextLayer_t>      (resolve("PutTextLayer"));
	m_fnUpdateAll    = reinterpret_cast<UpdateAllContent_t>  (resolve("UpdateAllContent"));
	m_fnRemoveNode   = reinterpret_cast<RemoveGraphNode_t>   (resolve("RemoveGraphNode"));
	m_fnMoveOrAdd    = reinterpret_cast<MoveOrAddGraphNode_t>(resolve("MoveOrAddGraphNode"));
}

void GraphWorker::dispatchCallback(int iMSG, const std::string& params) {
	switch (iMSG) {
	case 1799:
		if (m_fnUpdateAll) m_fnUpdateAll();
		break;
	case 1801:
		if (m_fnRemoveNode) m_fnRemoveNode(std::stoi(params));
		break;
	case 1802: case 1803: {
		std::vector<std::string> v;
		int start = 0, end = 0;
		while ((start = params.find_first_not_of('&', end)) != std::string::npos) {
			end = params.find('&', start);
			v.push_back(params.substr(start, end - start));
		}
		if (v.size() == 3 && m_fnMoveOrAdd)
			m_fnMoveOrAdd(std::stoi(v[0]), std::stoi(v[1]), std::stoi(v[2]));
		break;
	}
	case 1400: case 1401: case 1402: case 1403: case 1404:
		if (m_fnPutTextLayer) m_fnPutTextLayer(iMSG - 1400);
		break;
	default: break;
	}
}
