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

#include "talk/p2p/base/constants.h"

namespace cricket {

const std::string NS_EMPTY("");
const std::string NS_GOOGLE_SESSION("http://www.google.com/session");
#ifdef FEATURE_ENABLE_VOICEMAIL
const std::string NS_GOOGLE_VOICEMAIL("http://www.google.com/session/voicemail");
#endif
const std::string NS_GOOGLE_PHONE("http://www.google.com/session/phone");
const std::string NS_GOOGLE_VIDEO("http://www.google.com/session/video");

// Session info
const buzz::QName QN_SESSION(true, NS_GOOGLE_SESSION, "session");

const buzz::QName QN_REDIRECT_TARGET(true, NS_GOOGLE_SESSION, "target");
const buzz::QName QN_REDIRECT_COOKIE(true, NS_GOOGLE_SESSION, "cookie");
const buzz::QName QN_REDIRECT_REGARDING(true, NS_GOOGLE_SESSION, "regarding");

#ifdef FEATURE_ENABLE_VOICEMAIL
const buzz::QName QN_VOICEMAIL_REGARDING(true, NS_GOOGLE_VOICEMAIL, "regarding");
#endif

const buzz::QName QN_INITIATOR(true, NS_EMPTY, "initiator");

// Voice session description info
const buzz::QName QN_PHONE_DESCRIPTION(true, NS_GOOGLE_PHONE, "description");
const buzz::QName QN_PHONE_PAYLOADTYPE(true, NS_GOOGLE_PHONE, "payload-type");
const buzz::QName QN_PHONE_PAYLOADTYPE_ID(true, NS_EMPTY, "id");
const buzz::QName QN_PHONE_PAYLOADTYPE_NAME(true, NS_EMPTY, "name");
const buzz::QName QN_PHONE_PAYLOADTYPE_RATE(true, NS_EMPTY, "clockrate");
const buzz::QName QN_PHONE_PAYLOADTYPE_BITRATE(true, NS_EMPTY, "bitrate");
const buzz::QName QN_PHONE_PAYLOADTYPE_CHANNELS(true, NS_EMPTY, "channels");
const buzz::QName QN_PHONE_SRCID(true, NS_GOOGLE_PHONE, "src-id");

// Video session description info
const buzz::QName QN_VIDEO_DESCRIPTION(true, NS_GOOGLE_VIDEO, "description");
const buzz::QName QN_VIDEO_PAYLOADTYPE(true, NS_GOOGLE_VIDEO, "payload-type");
const buzz::QName QN_VIDEO_PAYLOADTYPE_ID(true, NS_EMPTY, "id");
const buzz::QName QN_VIDEO_PAYLOADTYPE_NAME(true, NS_EMPTY, "name");
const buzz::QName QN_VIDEO_PAYLOADTYPE_WIDTH(true, NS_EMPTY, "width");
const buzz::QName QN_VIDEO_PAYLOADTYPE_HEIGHT(true, NS_EMPTY, "height");
const buzz::QName QN_VIDEO_PAYLOADTYPE_FRAMERATE(true, NS_EMPTY, "framerate");
const buzz::QName QN_VIDEO_SRCID(true, NS_GOOGLE_VIDEO, "src-id");
const buzz::QName QN_VIDEO_BANDWIDTH(true, NS_GOOGLE_VIDEO, "bandwidth");

// Candidate info
const buzz::QName QN_ADDRESS(true, cricket::NS_EMPTY, "address");
const buzz::QName QN_PORT(true, cricket::NS_EMPTY, "port");
const buzz::QName QN_NETWORK(true, cricket::NS_EMPTY, "network");
const buzz::QName QN_GENERATION(true, cricket::NS_EMPTY, "generation");
const buzz::QName QN_USERNAME(true, cricket::NS_EMPTY, "username");
const buzz::QName QN_PASSWORD(true, cricket::NS_EMPTY, "password");
const buzz::QName QN_PREFERENCE(true, cricket::NS_EMPTY, "preference");
const buzz::QName QN_PROTOCOL(true, cricket::NS_EMPTY, "protocol");

// Legacy transport messages
const buzz::QName kQnLegacyCandidate(true, cricket::NS_GOOGLE_SESSION,
                                     "candidate");
}  // namespace cricket
