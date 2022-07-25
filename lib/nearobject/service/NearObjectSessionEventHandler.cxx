
#include <nearobject/NearObjectSessionEventCallbacks.hxx>
#include "NearObjectSessionEventHandler.hxx"

using namespace nearobject;
using namespace nearobject::service;

NearObjectSessionEventHandler::NearObjectSessionEventHandler(NearObjectSession* session, std::shared_ptr<NearObjectSessionEventCallbacks> callbacks) :
    m_session(session),
    m_callbacks(callbacks)
{
    m_session->RegisterCallbacks(callbacks);
}