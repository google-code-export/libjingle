/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "talk/session/phone/mediasessionclient.h"

#include <stdlib.h>

#include "talk/base/logging.h"
#include "talk/base/stringutils.h"
#include "talk/p2p/base/constants.h"
#include "talk/xmpp/constants.h"
#include "talk/xmllite/qname.h"

using namespace talk_base;

namespace cricket {

MediaSessionClient::MediaSessionClient(
    const buzz::Jid& jid, SessionManager *manager)
    : jid_(jid), session_manager_(manager), focus_call_(NULL),
      channel_manager_(new ChannelManager(session_manager_->worker_thread())) {
  Construct();
}

MediaSessionClient::MediaSessionClient(
    const buzz::Jid& jid, SessionManager *manager,
    MediaEngine* media_engine, DeviceManager* device_manager)
    : jid_(jid), session_manager_(manager), focus_call_(NULL),
      channel_manager_(new ChannelManager(
          media_engine, device_manager, session_manager_->worker_thread())) {
  Construct();
}


void MediaSessionClient::Construct() {
  // Register ourselves as the handler of phone and video sessions.
  session_manager_->AddClient(NS_GOOGLE_PHONE, this);
  session_manager_->AddClient(NS_GOOGLE_VIDEO, this);
  // Forward device notifications.
  SignalDevicesChange.repeat(channel_manager_->SignalDevicesChange);
}

MediaSessionClient::~MediaSessionClient() {
  // Destroy all calls
  std::map<uint32, Call *>::iterator it;
  while (calls_.begin() != calls_.end()) {
    std::map<uint32, Call *>::iterator it = calls_.begin();
    DestroyCall((*it).second);
  }

  // Delete channel manager. This will wait for the channels to exit
  delete channel_manager_;

  // Remove ourselves from the client map.
  session_manager_->RemoveClient(NS_GOOGLE_VIDEO);
  session_manager_->RemoveClient(NS_GOOGLE_PHONE);
}

MediaSessionDescription* MediaSessionClient::CreateOfferSessionDescription(
    bool video) {
  MediaSessionDescription* session_desc = new MediaSessionDescription();


  // add audio codecs
  std::vector<Codec> codecs;
  channel_manager_->GetSupportedCodecs(&codecs);
  for (std::vector<Codec>::const_iterator i = codecs.begin();
       i != codecs.end(); ++i)
    session_desc->voice().AddCodec(*i);

  // add video codecs, if this is a video call
  if (video) {
    std::vector<VideoCodec> video_codecs;
    channel_manager_->GetSupportedVideoCodecs(&video_codecs);
    for (std::vector<VideoCodec>::const_iterator i = video_codecs.begin();
         i != video_codecs.end(); i++)
      session_desc->video().AddCodec(*i);
  }

  session_desc->Sort();
  return session_desc;
}

MediaSessionDescription* MediaSessionClient::CreateAcceptSessionDescription(
  const SessionDescription* offer) {
  const MediaSessionDescription* offer_desc =
      static_cast<const MediaSessionDescription*>(offer);
  MediaSessionDescription* accept_desc = new MediaSessionDescription();

  // add audio codecs
  std::vector<Codec> codecs;
  channel_manager_->GetSupportedCodecs(&codecs);
  for (unsigned int i = 0; i < offer_desc->voice().codecs().size(); ++i) {
    if (channel_manager_->FindCodec(offer_desc->voice().codecs()[i]))
      accept_desc->voice().AddCodec(offer_desc->voice().codecs()[i]);
  }

  // add video codecs, if the incoming session description has them
  if (!offer_desc->video().codecs().empty()) {
    std::vector<VideoCodec> video_codecs;
    channel_manager_->GetSupportedVideoCodecs(&video_codecs);
    for (unsigned int i = 0; i < offer_desc->video().codecs().size(); ++i) {
      if (channel_manager_->FindVideoCodec(offer_desc->video().codecs()[i]))
        accept_desc->video().AddCodec(offer_desc->video().codecs()[i]);
    }
  }

  accept_desc->Sort();
  return accept_desc;
}

const SessionDescription* MediaSessionClient::CreateSessionDescription(
    const buzz::XmlElement* element) {
  MediaSessionDescription* session_desc = new MediaSessionDescription();

  // translate audio codecs
  const buzz::XmlElement* payload_type =
      element->FirstNamed(QN_PHONE_PAYLOADTYPE);
  int num_payload_types = 0;

  while (payload_type) {
    if (payload_type->HasAttr(QN_PHONE_PAYLOADTYPE_ID)) {
      int id = GetAttr(payload_type, QN_PHONE_PAYLOADTYPE_ID, 0);

      std::string name;
      if (payload_type->HasAttr(QN_PHONE_PAYLOADTYPE_NAME))
        name = payload_type->Attr(QN_PHONE_PAYLOADTYPE_NAME);

      int clockrate = GetAttr(payload_type, QN_PHONE_PAYLOADTYPE_RATE, 0);
      int bitrate = GetAttr(payload_type, QN_PHONE_PAYLOADTYPE_BITRATE, 0);
      int channels = GetAttr(payload_type, QN_PHONE_PAYLOADTYPE_CHANNELS, 1);

      session_desc->voice().AddCodec(
          Codec(id, name, clockrate, bitrate, channels, 0));
    }

    payload_type = payload_type->NextNamed(QN_PHONE_PAYLOADTYPE);
    num_payload_types++;
  }

  // For backward compatibility, we can assume the other client is (an old
  // version of Talk) if it has no audio payload types at all.
  if (num_payload_types == 0) {
    session_desc->voice().AddCodec(Codec(103, "ISAC", 16000, -1, 1, 1));
    session_desc->voice().AddCodec(Codec(0, "PCMU", 8000, 64000, 1, 0));
  }

  // translate video codecs
  payload_type = element->FirstNamed(QN_VIDEO_PAYLOADTYPE);
  while (payload_type) {
    if (payload_type->HasAttr(QN_VIDEO_PAYLOADTYPE_ID)) {
      int id = GetAttr(payload_type, QN_VIDEO_PAYLOADTYPE_ID, 0);

      std::string name;
      if (payload_type->HasAttr(QN_VIDEO_PAYLOADTYPE_NAME))
        name = payload_type->Attr(QN_VIDEO_PAYLOADTYPE_NAME);

      int width = GetAttr(payload_type, QN_VIDEO_PAYLOADTYPE_WIDTH, 0);
      int height = GetAttr(payload_type, QN_VIDEO_PAYLOADTYPE_HEIGHT, 0);
      int framerate = GetAttr(payload_type, QN_VIDEO_PAYLOADTYPE_FRAMERATE, 0);

      session_desc->video().AddCodec(
          VideoCodec(id, name, width, height, framerate, 0));
    }

    payload_type = payload_type->NextNamed(QN_VIDEO_PAYLOADTYPE);
  }

  // get ssrcs, if present
  const buzz::XmlElement* src_id;
  src_id = element->FirstNamed(QN_PHONE_SRCID);
  if (src_id) {
    session_desc->voice().set_ssrc(strtoul(src_id->BodyText().c_str(),
        NULL, 10));
  }
  src_id = element->FirstNamed(QN_VIDEO_SRCID);
  if (src_id) {
    session_desc->video().set_ssrc(strtoul(src_id->BodyText().c_str(),
        NULL, 10));
  }

  return session_desc;
}

static void SetBodyUint(buzz::XmlElement* elem, uint32 u) {
  char buf[16];
  sprintfn(buf, sizeof(buf), "%u", u);
  elem->SetBodyText(buf);
}

buzz::XmlElement* MediaSessionClient::TranslateSessionDescription(
    const SessionDescription* _session_desc) {
  const MediaSessionDescription* session_desc =
      static_cast<const MediaSessionDescription*>(_session_desc);

  bool video = !session_desc->video().codecs().empty();
  buzz::XmlElement* description =
      new buzz::XmlElement(video ?
          QN_VIDEO_DESCRIPTION : QN_PHONE_DESCRIPTION, true);


  // add audio codecs
  for (size_t i = 0; i < session_desc->voice().codecs().size(); ++i) {
    const Codec& codec(session_desc->voice().codecs()[i]);
    buzz::XmlElement* payload_type =
        new buzz::XmlElement(QN_PHONE_PAYLOADTYPE, true);

    AddAttr(payload_type, QN_PHONE_PAYLOADTYPE_ID, codec.id);
    payload_type->AddAttr(QN_PHONE_PAYLOADTYPE_NAME, codec.name);
    if (codec.clockrate > 0) {
      AddAttr(payload_type, QN_PHONE_PAYLOADTYPE_RATE, codec.clockrate);
    }
    if (codec.bitrate > 0) {
      AddAttr(payload_type, QN_PHONE_PAYLOADTYPE_BITRATE, codec.bitrate);
    }
    if (codec.channels > 1) {
      AddAttr(payload_type, QN_PHONE_PAYLOADTYPE_CHANNELS, codec.channels);
    }

    description->AddElement(payload_type);
  }

  // add video codecs, if there are any
  if (video) {
    for (size_t i = 0; i < session_desc->video().codecs().size(); ++i) {
      const VideoCodec& codec(session_desc->video().codecs()[i]);
      buzz::XmlElement* payload_type =
          new buzz::XmlElement(QN_VIDEO_PAYLOADTYPE, true);

      AddAttr(payload_type, QN_VIDEO_PAYLOADTYPE_ID, codec.id);
      payload_type->AddAttr(QN_VIDEO_PAYLOADTYPE_NAME, codec.name);
      AddAttr(payload_type, QN_VIDEO_PAYLOADTYPE_WIDTH, codec.width);
      AddAttr(payload_type, QN_VIDEO_PAYLOADTYPE_HEIGHT, codec.height);
      AddAttr(payload_type, QN_VIDEO_PAYLOADTYPE_FRAMERATE, codec.framerate);

      description->AddElement(payload_type);
    }
  }

  // add ssrcs, if set
  if (session_desc->voice().ssrc_set()) {
    buzz::XmlElement* src_id = new buzz::XmlElement(QN_PHONE_SRCID, true);
    if (session_desc->voice().ssrc()) {
      SetBodyUint(src_id, session_desc->voice().ssrc());
    }
    description->AddElement(src_id);
  }
  if (video && session_desc->video().ssrc_set()) {
    buzz::XmlElement* src_id = new buzz::XmlElement(QN_VIDEO_SRCID, true);
    if (session_desc->video().ssrc()) {
      SetBodyUint(src_id, session_desc->video().ssrc());
    }
    description->AddElement(src_id);
  }

  return description;
}

Call *MediaSessionClient::CreateCall(bool video, bool mux) {
  Call *call = new Call(this, video, mux);
  calls_[call->id()] = call;
  SignalCallCreate(call);
  return call;
}

void MediaSessionClient::OnSessionCreate(Session *session,
                                         bool received_initiate) {
  if (received_initiate) {
    session->SignalState.connect(this, &MediaSessionClient::OnSessionState);

    Call *call = CreateCall(session->session_type() == NS_GOOGLE_VIDEO);
    session_map_[session->id()] = call;
    call->AddSession(session);
  }
}

void MediaSessionClient::OnSessionState(BaseSession *session,
                                        BaseSession::State state) {
  if (state == Session::STATE_RECEIVEDINITIATE) {
    // If our accept would have no codecs, then we must reject this call.
    MediaSessionDescription* accept_desc =
        CreateAcceptSessionDescription(session->remote_description());
    if (accept_desc->voice().codecs().size() == 0) {
      // TODO(?): include an error description with the rejection.
      session->Reject();
    }
    delete accept_desc;
  }
}

void MediaSessionClient::DestroyCall(Call *call) {
  // Change focus away, signal destruction

  if (call == focus_call_)
    SetFocus(NULL);
  SignalCallDestroy(call);

  // Remove it from calls_ map and delete

  std::map<uint32, Call *>::iterator it = calls_.find(call->id());
  if (it != calls_.end())
    calls_.erase(it);

  delete call;
}

void MediaSessionClient::OnSessionDestroy(Session *session) {
  // Find the call this session is in, remove it

  std::map<SessionID, Call *>::iterator it = session_map_.find(session->id());
  assert(it != session_map_.end());
  if (it != session_map_.end()) {
    Call *call = (*it).second;
    session_map_.erase(it);
    call->RemoveSession(session);
  }
}

Call *MediaSessionClient::GetFocus() {
  return focus_call_;
}

void MediaSessionClient::SetFocus(Call *call) {
  Call *old_focus_call = focus_call_;
  if (focus_call_ != call) {
    if (focus_call_ != NULL)
      focus_call_->EnableChannels(false);
    focus_call_ = call;
    if (focus_call_ != NULL)
      focus_call_->EnableChannels(true);
    SignalFocus(focus_call_, old_focus_call);
  }
}

void MediaSessionClient::JoinCalls(Call *call_to_join, Call *call) {
  // Move all sessions from call to call_to_join, delete call.
  // If call_to_join has focus, added sessions should have enabled channels.

  if (focus_call_ == call)
    SetFocus(NULL);
  call_to_join->Join(call, focus_call_ == call_to_join);
  DestroyCall(call);
}

Session *MediaSessionClient::CreateSession(Call *call) {
  const std::string& type = call->video() ? NS_GOOGLE_VIDEO : NS_GOOGLE_PHONE;
  Session *session = session_manager_->CreateSession(jid().Str(), type);
  session_map_[session->id()] = call;
  return session;
}

int MediaSessionClient::GetAttr(const buzz::XmlElement* elem,
                                const buzz::QName& name, int def) {
  std::string val = elem->Attr(name);
  return (!val.empty()) ? atoi(val.c_str()) : def;
}

void MediaSessionClient::AddAttr(buzz::XmlElement* elem,
                                 const buzz::QName& name, int n) {
  char buf[32];
  sprintfn(buf, sizeof(buf), "%d", n);
  elem->AddAttr(name, buf);
}

}
