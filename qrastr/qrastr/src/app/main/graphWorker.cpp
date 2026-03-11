#include "graphWorker.h"
#include <QCoreApplication>
#include <QLibrary>
#include <QFile>
#include "astra/IPlainRastr.h"
#include "graphServer.h"

GraphWorker::GraphWorker(IPlainRastr* rastr, QLibrary* lib)
        : m_rastr(rastr), m_lib(lib) {}

GraphWorker::~GraphWorker(){
    qInfo() << "GraphWorker was deleted";
}

void GraphWorker::slot_process() {
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

	const QString graphLibsPath =
        QCoreApplication::applicationDirPath() + "/../Data/graphics/graph2libs.xml";
	if (!QFile::exists(graphLibsPath)) {
		qWarning() << "GraphWorker: graph2libs.xml not found";
        emit sig_finished();
		return;
	}

	m_fnInit(m_rastr,
			 graphLibsPath.toStdString().c_str(),
			 "127.0.0.1", 8081,
			 GraphServer::staticCallback);   // подключение к либе

	qInfo() << "GraphWorker: HTTP server ready";
    emit sig_ready();
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
