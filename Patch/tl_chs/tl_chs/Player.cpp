#include "Urho3D/Core/Main.h"
#include "Urho3D/Engine/Engine.h"
#include "Urho3D/Engine/EngineDefs.h"
#include "Urho3D/IO/FileSystem.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/ResourceEvents.h"

#include "Player.h"

#include "Urho3D/DebugNew.h"


Urho3DPlayer::Urho3DPlayer(Context* context) :
Application(context),
commandLineRead_(false)
{
}

void Urho3DPlayer::Setup()
{
}

void Urho3DPlayer::Start()
{
}

void Urho3DPlayer::Stop()
{
}

void Urho3DPlayer::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
}

void Urho3DPlayer::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
}

void Urho3DPlayer::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
}

void Urho3DPlayer::GetScriptFileName()
{
}
