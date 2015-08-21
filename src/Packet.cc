/*
 * Copyright (C) 2014 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <cstring>
#include <cstdint>
#include <string>
#include "ignition/transport/Packet.hh"

using namespace ignition;
using namespace transport;

//////////////////////////////////////////////////
Header::Header(const uint16_t _version,
               const std::string &_pUuid,
               const uint8_t _type,
               const uint16_t _flags)
{
  this->Version(_version);
  this->PUuid(_pUuid);
  this->Type(_type);
  this->Flags(_flags);
}

//////////////////////////////////////////////////
uint16_t Header::Version() const
{
  return this->version;
}

//////////////////////////////////////////////////
std::string Header::PUuid() const
{
  return this->pUuid;
}

//////////////////////////////////////////////////
uint8_t Header::Type() const
{
  return this->type;
}

//////////////////////////////////////////////////
uint16_t Header::Flags() const
{
  return this->flags;
}

//////////////////////////////////////////////////
void Header::Version(const uint16_t _version)
{
  this->version = _version;
}

//////////////////////////////////////////////////
void Header::PUuid(const std::string &_pUuid)
{
  this->pUuid = _pUuid;
}

//////////////////////////////////////////////////
void Header::Type(const uint8_t _type)
{
  this->type = _type;
}

//////////////////////////////////////////////////
void Header::Flags(const uint16_t _flags)
{
  this->flags = _flags;
}

//////////////////////////////////////////////////
int Header::HeaderLength() const
{
  return sizeof(this->version) +
         sizeof(uint64_t) + this->pUuid.size() +
         sizeof(this->type) + sizeof(this->flags);
}

//////////////////////////////////////////////////
size_t Header::Pack(char *_buffer) const
{
  // Uninitialized.
  if ((this->version == 0) || (this->pUuid == "") ||
      (this->type  == Uninitialized))
  {
    std::cerr << "Header::Pack() error: You're trying to pack an incomplete "
              << "header:" << std::endl << *this;
    return 0;
  }

  // null buffer.
  if (!_buffer)
  {
    std::cerr << "Header::Pack() error: NULL output buffer" << std::endl;
    return 0;
  }

  // Pack the discovery protocol version, which is a uint16_t
  memcpy(_buffer, &this->version, sizeof(this->version));
  _buffer += sizeof(this->version);

  // Pack the process UUID length.
  uint64_t pUuidLength = this->pUuid.size();
  memcpy(_buffer, &pUuidLength, sizeof(pUuidLength));
  _buffer += sizeof(pUuidLength);

  // Pack the process UUID.
  memcpy(_buffer, this->pUuid.data(), static_cast<size_t>(pUuidLength));
  _buffer += pUuidLength;

  // Pack the message type (ADVERTISE, SUBSCRIPTION, ...), which is uint8_t
  memcpy(_buffer, &this->type, sizeof(this->type));
  _buffer += sizeof(this->type);

  // Pack the flags, which is uint16_t
  memcpy(_buffer, &this->flags, sizeof(this->flags));

  return this->HeaderLength();
}

//////////////////////////////////////////////////
size_t Header::Unpack(const char *_buffer)
{
  // null buffer.
  if (!_buffer)
  {
    std::cerr << "Header::Unpack() error: NULL input buffer" << std::endl;
    return 0;
  }

  // Unpack the version.
  memcpy(&this->version, _buffer, sizeof(this->version));
  _buffer += sizeof(this->version);

  // Unpack the process UUID length.
  uint64_t pUuidLength;
  memcpy(&pUuidLength, _buffer, sizeof(pUuidLength));
  _buffer += sizeof(pUuidLength);

  // Unpack the process UUID.
  this->pUuid = std::string(_buffer, _buffer + pUuidLength);
  _buffer += pUuidLength;

  // Unpack the message type.
  memcpy(&this->type, _buffer, sizeof(this->type));
  _buffer += sizeof(this->type);

  // Unpack the flags.
  memcpy(&this->flags, _buffer, sizeof(this->flags));
  _buffer += sizeof(this->flags);

  return this->HeaderLength();
}

//////////////////////////////////////////////////
SubscriptionMsg::SubscriptionMsg(const Header &_header,
                                 const std::string &_topic)
{
  this->SetHeader(_header);
  this->Topic(_topic);
}

//////////////////////////////////////////////////
Header SubscriptionMsg::GetHeader() const
{
  return this->header;
}

//////////////////////////////////////////////////
std::string SubscriptionMsg::Topic() const
{
  return this->topic;
}

//////////////////////////////////////////////////
void SubscriptionMsg::SetHeader(const Header &_header)
{
  this->header = _header;
}

//////////////////////////////////////////////////
void SubscriptionMsg::Topic(const std::string &_topic)
{
  this->topic = _topic;
}

//////////////////////////////////////////////////
size_t SubscriptionMsg::MsgLength() const
{
  return this->header.HeaderLength() + sizeof(uint64_t) + this->topic.size();
}

//////////////////////////////////////////////////
size_t SubscriptionMsg::Pack(char *_buffer) const
{
  // Pack the header.
  size_t headerLen = this->GetHeader().Pack(_buffer);
  if (headerLen == 0)
    return 0;

  if (this->topic == "")
  {
    std::cerr << "SubscriptionMsg::Pack() error: You're trying to pack a "
              << "message with an empty topic" << std::endl;
    return 0;
  }

  _buffer += headerLen;

  // Pack the topic length.
  uint64_t topicLength = this->topic.size();
  memcpy(_buffer, &topicLength, sizeof(topicLength));
  _buffer += sizeof(topicLength);

  // Pack the topic.
  memcpy(_buffer, this->topic.data(), static_cast<size_t>(topicLength));

  return this->MsgLength();
}

//////////////////////////////////////////////////
size_t SubscriptionMsg::Unpack(char *_buffer)
{
  // null buffer.
  if (!_buffer)
  {
    std::cerr << "SubscriptionMsg::UnpackBody() error: NULL input buffer"
              << std::endl;
    return 0;
  }

  // Unpack the topic length.
  uint64_t topicLength;
  memcpy(&topicLength, _buffer, sizeof(topicLength));
  _buffer += sizeof(topicLength);

  // Unpack the topic.
  this->topic = std::string(_buffer, _buffer + topicLength);

  return sizeof(topicLength) + static_cast<size_t>(topicLength);
}

//////////////////////////////////////////////////
AdvertiseMessage::AdvertiseMessage(const Header &_header,
                               const Publisher &_publisher)
  : header(_header),
    publisher(_publisher)
{
}

//////////////////////////////////////////////////
Header AdvertiseMessage::GetHeader() const
{
  return this->header;
}

//////////////////////////////////////////////////
Publisher& AdvertiseMessage::GetPublisher()
{
  return this->publisher;
}

//////////////////////////////////////////////////
void AdvertiseMessage::SetHeader(const Header &_header)
{
  this->header = _header;
}

//////////////////////////////////////////////////
void AdvertiseMessage::SetPublisher(const Publisher &_publisher)
{
  this->publisher = _publisher;
}

//////////////////////////////////////////////////
size_t AdvertiseMessage::MsgLength() const
{
  return this->header.HeaderLength() + this->publisher.MsgLength();
}

//////////////////////////////////////////////////
size_t AdvertiseMessage::Pack(char *_buffer) const
{
  // Pack the common part of any advertise message.
  size_t len = this->header.Pack(_buffer);
  if (len == 0)
    return 0;

  _buffer += len;

  // Pack the part of the publisher.
  if (this->publisher.Pack(_buffer) == 0)
    return 0;

  return this->MsgLength();
}

//////////////////////////////////////////////////
size_t AdvertiseMessage::Unpack(char *_buffer)
{
  // Unpack the message publisher.
  if (this->publisher.Unpack(_buffer) == 0)
    return 0;

  return this->publisher.MsgLength();
}
